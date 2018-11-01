#include <stdlib.h>
#include <string.h>
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

jit_t*	jit_emit_pushb (int8_t val, jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov QWORD PTR [rbx], val
	dict_emit_b(0xc7, &jit->dict);
	dict_emit_b(0x03, &jit->dict);
	dict_emit_w(val,  &jit->dict);
	dict_emit_b(0x48, &jit->dict);	// add rbx, 0x8
	dict_emit_b(0x83, &jit->dict);
	dict_emit_b(0xc3, &jit->dict);
	dict_emit_b(0x08, &jit->dict);

	return jit;
}

jit_t*	jit_emit_dup (jit_t* jit)
{
	dict_emit_b(0x48, &jit->dict);	// mov rcx, QWORD PTR [rbx-0x8]
	dict_emit_b(0x8b, &jit->dict);
	dict_emit_b(0x4b, &jit->dict);
	dict_emit_b(0xf8, &jit->dict);
	dict_emit_b(0x48, &jit->dict);	// mov QWORD PTR [rbx], rcx
	dict_emit_b(0x89, &jit->dict);
	dict_emit_b(0x0b, &jit->dict);
	dict_emit_b(0x48, &jit->dict);	// add rbx, 0x8
	dict_emit_b(0x83, &jit->dict);
	dict_emit_b(0xc3, &jit->dict);
	dict_emit_b(0x08, &jit->dict);

	return jit;
}

/*
	dict_emit_op (OP_DUP, &dict);
	dict_emit_op (OP_PUSHB, &dict);
	dict_emit_b  (1,        &dict);
	dict_emit_op (OP_LE,    &dict);
	dict_emit_op (OP_BBZ,   &dict);
	dict_emit_b  (3,        &dict);
	dict_emit_op (OP_RET,   &dict);

	dict_emit_op (OP_DUP,   &dict);
	dict_emit_op (OP_PUSHB, &dict);
	dict_emit_b  (1,        &dict);
	dict_emit_op (OP_SUB,   &dict);
	dict_emit_op (OP_CALL,  &dict);
	dict_emit_ptr(self,     &dict);

	dict_emit_op (OP_SWAP,  &dict);
	dict_emit_op (OP_PUSHB, &dict);
	dict_emit_b  (2,        &dict);
	dict_emit_op (OP_SUB,   &dict);
	dict_emit_op (OP_CALL,  &dict);
	dict_emit_ptr(self,     &dict);

	dict_emit_op (OP_ADD,   &dict);
	dict_emit_op (OP_RET,   &dict);

	dict_end_def(&dict);

	uint8_t* fib = dict_get_body("fib", &dict);
	assert(fib != 0);
	assert(fib == self);

	dict_begin_def("main2", false, &dict);
	dict_emit_op (OP_PUSHB, &dict);
	dict_emit_b  (25,       &dict);
	dict_emit_op (OP_CALL,  &dict);
	dict_emit_ptr(fib,      &dict);
	dict_emit_op (OP_DOT,   &dict);
	dict_emit_op (OP_HALT,  &dict);
	dict_end_def(&dict);
	*/

jit_t*	jit_end_def(jit_t* jit)
{
	// emit epilogue
	dict_emit_b(0x5d, &jit->dict);	// pop rbp
	dict_emit_b(0xc3, &jit->dict);	// ret

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

int64_t jit_run(const char* name, jit_t* jit)
{
	uint8_t* invoke = mmap(0, 64, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(invoke == (void*)-1) abort();
	invoke[0]  = 0x48;	// movabs rbx, jit->pstack
	invoke[1]  = 0xbb;
	*(void**)(invoke + 2)  = jit->pstack;
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


