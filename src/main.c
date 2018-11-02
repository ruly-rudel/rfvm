
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include "rfvm.h"
#include "dict.h"
#include "jit.h"

void print_err(int ret)
{
	switch(ret)
	{
		case E_OK:		fprintf(stdout, "ok.\n");				break;
		case E_NOTIMPL:		fprintf(stderr, "opcode not implemented yet.\n");	break;
		case E_STACKUFLOW:	fprintf(stderr, "too few parameter or return stack entry.\n");		break;
		case E_STACKOFLOW:	fprintf(stderr, "parameter or return stack overflow.\n");		break;
		default:		fprintf(stderr, "unknown error code.\n");		break;
	}
}

int main(int argc, char* argv[])
{
	uint8_t code[] = { OP_PUSHB, 10, OP_PUSHB, -2, OP_ADD, OP_PUSHB, 3, OP_MUL, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code));

	uint8_t code2[] = { OP_PUSHB, 10, OP_ADD, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code2));

	uint8_t code3[] = { OP_PUSHB, 10, OP_PUSHB, 0, OP_AND, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code3));

	uint8_t code4[] = { OP_PUSHB, 10, OP_PUSHB, 0, OP_OR, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code4));

	uint8_t code5[] = { OP_PUSHB, 10, OP_PUSHB, 0, OP_GNE, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code5));

	uint8_t code6[] = { OP_DUP, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code6));

	uint8_t code7[] = { OP_PUSHB, 20, OP_DUP, OP_DOT, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code7));

	void* buf = malloc(4096);
	dict_t dict = dict_init(buf, 4096);

	// square
	dict_begin_def("square", false, &dict);
	dict_emit_op(OP_DUP, &dict);
	dict_emit_op(OP_MUL, &dict);
	dict_emit_op(OP_RET, &dict);
	dict_end_def(&dict);

	uint8_t* square = dict_get_body("square", &dict);
	assert(square != 0);
	assert(square[0] == OP_DUP);
	assert(square[1] == OP_MUL);
	assert(square[2] == OP_RET);

	dict_begin_def("main1", false, &dict);
	dict_emit_op (OP_PUSHB, &dict);
	dict_emit_b  (9,        &dict);
	dict_emit_op (OP_CALL,  &dict);
	dict_emit_ptr(square,   &dict);
	dict_emit_op (OP_DOT,   &dict);
	dict_emit_op (OP_HALT,  &dict);
	dict_end_def(&dict);

	uint8_t* main1 = dict_get_body("main1", &dict);
	assert(main1 != 0);
	print_err(exec_rfvm(main1));


	// fib
	dict_begin_def("fib", false, &dict);
	uint8_t* self = dict_get_current_body(&dict);
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
	dict_emit_b  (40,       &dict);
	dict_emit_op (OP_CALL,  &dict);
	dict_emit_ptr(fib,      &dict);
	dict_emit_op (OP_DOT,   &dict);
	dict_emit_op (OP_HALT,  &dict);
	dict_end_def(&dict);

	uint8_t* main2 = dict_get_body("main2", &dict);
	assert(main2 != 0);
	print_err(exec_rfvm(main2));


	// jit test
	void* jitc = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(jitc == (void*)-1) abort();

	dict_t jdict = dict_init(jitc, 4096);
	dict_begin_def("ret42", false, &jdict);
	dict_emit_b   (0xb8, &jdict);	// mov eax, 42
	dict_emit_dw  (42,   &jdict);
	dict_emit_b   (0xc3, &jdict);	// ret
	dict_end_def(&jdict);

	uint8_t* ret42 = dict_get_body("ret42", &jdict);
	assert(ret42 != 0);

	if (mprotect(jitc, 4096, PROT_READ | PROT_EXEC) == -1) abort();

	printf("%d\n", ((int (*)(void))ret42)());

	// jit test2(pushb)
	jit_t jit = jit_init(4096);
	jit_begin_def("push84", false, &jit);
	jit_emit_pushb(84, &jit);
	jit_emit_ret(&jit);
	jit_end_def(&jit);

	jit_make_executable(&jit);
	printf("%ld\n", jit_run("push84", &jit, 0));

	// jit test 3(dup, add)
	jit_make_writable(&jit);
	jit_begin_def("mul2", false, &jit);
	jit_emit_dup(&jit);
	jit_emit_add(&jit);
	jit_emit_ret(&jit);
	jit_end_def(&jit);

	jit_make_executable(&jit);
	printf("%ld\n", jit_run("mul2", &jit, 1, 12));

	// jit test 4(sub)
	jit_make_writable(&jit);
	jit_begin_def("sub4", false, &jit);
	jit_emit_pushb(4, &jit);
	jit_emit_sub(&jit);
	jit_emit_ret(&jit);
	jit_end_def(&jit);

	jit_make_executable(&jit);
	printf("%ld\n", jit_run("sub4", &jit, 1, 12));

	// jit test 4(js)
	jit_make_writable(&jit);
	jit_begin_def("ceil4", false, &jit);
	jit_emit_dup(&jit);
	jit_emit_pushb(4, &jit);
	jit_emit_sub(&jit);
	jit_emit_bpl(2, &jit);
	jit_emit_ret(&jit);
	jit_emit_drop(&jit);
	jit_emit_pushb(0, &jit);
	jit_emit_ret(&jit);
	jit_end_def(&jit);

	jit_make_executable(&jit);
	printf("%ld\n", jit_run("ceil4", &jit, 1, 2));
	printf("%ld\n", jit_run("ceil4", &jit, 1, 3));
	printf("%ld\n", jit_run("ceil4", &jit, 1, 4));
	printf("%ld\n", jit_run("ceil4", &jit, 1, 5));

	// jit test 4(js)
	jit_make_writable(&jit);
	jit_begin_def("fib", false, &jit);
	uint8_t* jit_self = jit_get_current_body(&jit);
	jit_emit_dup(&jit);
	jit_emit_pushb(2, &jit);
	jit_emit_sub(&jit);
	jit_emit_bpl(2, &jit);
	jit_emit_ret(&jit);

	jit_emit_dup(&jit);
	jit_emit_pushb(1, &jit);
	jit_emit_sub(&jit);
	jit_emit_call(jit_self, &jit);

	jit_emit_swap(&jit);
	jit_emit_pushb(2, &jit);
	jit_emit_sub(&jit);
	jit_emit_call(jit_self, &jit);

	jit_emit_add(&jit);
	jit_emit_ret(&jit);

	jit_end_def(&jit);

	jit_make_executable(&jit);
	printf("%ld\n", jit_run("fib", &jit, 1, 25));

	return 0;
}
