#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>
#include "rfvm.h"
#include "dict.h"
#include "jit.h"

/////////////////////////////////////////////////////////////////////
// public: jit dictornary management functions

jit_t	jit_init(int size)
{
	jit_t jit = { 0 };
	void* buf	= mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(buf == (void*)-1) abort();
	jit.dict = dict_init(buf, size);
	jit.pstack = malloc(sizeof(int64_t) * 1024);

	return jit;
}

jit_t*	jit_begin_def(const char* name, bool immediate, jit_t* jit)
{
	dict_begin_def(name, immediate, &jit->dict);

	// emit prologue
	dict_emit_b(0x55, &jit->dict);	// push rbp
	dict_emit_b(0x48, &jit->dict);	// mov rbp, rsp
	dict_emit_b(0x89, &jit->dict);
	dict_emit_b(0xe5, &jit->dict);

	return jit;
}

void*	jit_get_current_body(jit_t* jit)
{
	return dict_get_current_body(&jit->dict);
}

inline static jit_t*	jit_emit_add_rbx (int8_t val, jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// add rbx, 0x8
	dict_emit_b(0x83, &jit->dict);
	dict_emit_b(0xc3, &jit->dict);
	dict_emit_b(val,  &jit->dict);

	return jit;
}

inline static jit_t*	jit_emit_inc_psp (jit_t* jit)
{
	return jit_emit_add_rbx( 8, jit);
}

inline static jit_t*	jit_emit_dec_psp (jit_t* jit)
{
	return jit_emit_add_rbx(-8, jit);
}

inline static jit_t*	jit_emit_peek (jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov rax, QWORD PTR [rbx-0x8]
	dict_emit_b(0x8b, &jit->dict);
	dict_emit_b(0x43, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);

	return jit;
}

inline static jit_t*	jit_emit_peek1 (jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov rcx, QWORD PTR [rbx-0xf]
	dict_emit_b(0x8b, &jit->dict);
	dict_emit_b(0x4b, &jit->dict);
	dict_emit_b(0xf0, &jit->dict);

	return jit;
}

inline static jit_t*	jit_emit_pop (jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov rax, QWORD PTR [rbx-0x8]
	dict_emit_b(0x8b, &jit->dict);
	dict_emit_b(0x43, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);
	return jit_emit_dec_psp (jit);	// rbx--
}

inline static jit_t*	jit_emit_push (jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov QWORD PTR [rbx], rax
	dict_emit_b(0x89, &jit->dict);
	dict_emit_b(0x03, &jit->dict);

	return jit_emit_inc_psp(jit);	// rbx++
}

inline static jit_t*	jit_emit_replace (jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov QWORD PTR [rbx-8], rax
	dict_emit_b(0x89, &jit->dict);
	dict_emit_b(0x43, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);

	return jit;
}

inline static jit_t*	jit_emit_replace1 (jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov QWORD PTR [rbx-16], rcx
	dict_emit_b(0x89, &jit->dict);
	dict_emit_b(0x4b, &jit->dict);
	dict_emit_b(0xf0, &jit->dict);

	return jit;
}

jit_t*	jit_emit_pushb (int8_t val, jit_t* jit)
{
	dict_emit_b (0x48, &jit->dict);		// mov QWORD PTR [rbx], val
	dict_emit_b (0xc7, &jit->dict);
	dict_emit_b (0x03, &jit->dict);
	dict_emit_dw(val,  &jit->dict);
	return jit_emit_inc_psp(jit);		// rbx++
}

jit_t*	jit_emit_dup (jit_t* jit)
{
	jit_emit_peek(jit);			// rax     = [rbx - 1]
	return jit_emit_push(jit);		// [rbx++] = rax
}

jit_t*	jit_emit_drop (jit_t* jit)
{
	return jit_emit_dec_psp(jit);		// rbx--
}

jit_t*	jit_emit_swap (jit_t* jit)
{
	jit_emit_peek(jit);			// rax     = [rbx - 1]
	jit_emit_peek1(jit);			// rcx     = [rbx - 2]
	dict_emit_b(0x48, &jit->dict);		// xchg rcx, rax
	dict_emit_b(0x91, &jit->dict);
	jit_emit_replace(jit);			// [rbx - 1] = rax
	jit_emit_replace1(jit);			// [rbx - 2] = rcx

	return jit;
}

jit_t*	jit_emit_add (jit_t* jit)
{
	jit_emit_peek(jit);			// rax     = [rbx - 1]
	jit_emit_dec_psp(jit);			// rbx--

	dict_emit_b(0x48, &jit->dict);		// add QWORD PTR [rbx-0x8], rax
	dict_emit_b(0x01, &jit->dict);
	dict_emit_b(0x43, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);

	return jit;
}

jit_t*	jit_emit_sub (jit_t* jit)
{
	jit_emit_peek(jit);			// rax     = [rbx - 1]
	jit_emit_dec_psp(jit);			// rbx--

	dict_emit_b(0x48, &jit->dict);		// sub QWORD PTR [rbx-0x8], rax
	dict_emit_b(0x29, &jit->dict);
	dict_emit_b(0x43, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);

	return jit;
}

jit_t*	jit_emit_bmi (int32_t offset, jit_t* jit)		// branch minus / negative
{
	jit_emit_pop(jit);			// rax     = [--rbx1]

	dict_emit_b(0x48, &jit->dict);		// cmp rax, 0
	dict_emit_b(0x83, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);
	dict_emit_b(0x00, &jit->dict);

	dict_emit_b(0x0f, &jit->dict);		// js offset
	dict_emit_b(0x88, &jit->dict);
	dict_emit_dw(offset, &jit->dict);

	return jit;
}

jit_t*	jit_emit_bpl (int32_t offset, jit_t* jit)		// branch plus / zero
{
	jit_emit_pop(jit);			// rax     = [--rbx1]

	dict_emit_b(0x48, &jit->dict);		// cmp rax, 0
	dict_emit_b(0x83, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);
	dict_emit_b(0x00, &jit->dict);

	dict_emit_b(0x0f, &jit->dict);		// jns offset
	dict_emit_b(0x89, &jit->dict);
	dict_emit_dw(offset, &jit->dict);

	return jit;
}

jit_t*	jit_emit_call(void* fn, jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// movabs rax, fn
	dict_emit_b(0xb8, &jit->dict);
	dict_emit_ptr(fn, &jit->dict);

	dict_emit_b(0xff, &jit->dict);	// call rax
	dict_emit_b(0xd0, &jit->dict);

	return jit;
}

jit_t*	jit_emit_ret(jit_t* jit)
{
	// emit epilogue
	dict_emit_b(0x5d, &jit->dict);	// pop rbp
	dict_emit_b(0xc3, &jit->dict);	// ret

	return jit;
}

jit_t*	jit_end_def(jit_t* jit)
{
	dict_end_def(&jit->dict);
	return jit;
}

void*	jit_get_body(const char* name, jit_t* jit)
{
	return dict_get_body(name, &jit->dict);
}

jit_t*	jit_make_executable(jit_t* jit)
{
	if (mprotect(jit->dict.buf, jit->dict.buf_size, PROT_READ | PROT_EXEC) == -1) return 0;
	return jit;
}

jit_t*	jit_make_writable(jit_t* jit)
{
	if (mprotect(jit->dict.buf, jit->dict.buf_size, PROT_READ | PROT_WRITE) == -1) return 0;
	return jit;
}

int64_t jit_run(const char* name, jit_t* jit, int num, ...)
{
	// initialize pstack
	va_list arg_ptr;
	va_start(arg_ptr, num);
	for(int i = 0; i < num; i++)
	{
		jit->pstack[i] = va_arg(arg_ptr, int);
	}
	va_end(arg_ptr);

	// invoke code
	uint8_t* invoke = mmap(0, 64, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(invoke == (void*)-1) abort();
	invoke[0]  = 0x48;	// movabs rbx, jit->pstack + num * 8
	invoke[1]  = 0xbb;
	*(void**)(invoke + 2)  = jit->pstack + num;
	invoke[10] = 0x48;	// movabs rax, jit_get_body(name)
	invoke[11] = 0xb8;
	*(void**)(invoke + 12) = jit_get_body(name, jit);

	invoke[20] = 0x55;	// push rbp
	invoke[21] = 0x48;	// move rbp, rsp
	invoke[22] = 0x89;
	invoke[23] = 0xe5;
	invoke[24] = 0xff;	// call rax
	invoke[25] = 0xd0;
	invoke[26] = 0x48;	// mov rax, QWORD PTR [rbx - 0x8]
	invoke[27] = 0x8b;
	invoke[28] = 0x43;
	invoke[29] = 0xf8;
	invoke[30] = 0x5d;	// pop rbp
	invoke[31] = 0xc3;	// ret

	if(mprotect(invoke, 64, PROT_READ | PROT_EXEC) == -1) abort();

	int64_t ret = ((int64_t(*)(void))invoke)();

	munmap(invoke, 64);

	return ret;
}


