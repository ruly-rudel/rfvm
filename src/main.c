
#include <stdio.h>
#include <assert.h>
#include "rfvm.h"

void print_err(int ret)
{
	switch(ret)
	{
		case E_OK:		fprintf(stdout, "Ok.\n");				break;
		case E_NOTIMPL:		fprintf(stderr, "opcode not implemented yet.\n");	break;
		case E_FEWSTACK:	fprintf(stderr, "too few stack entry.\n");		break;
		case E_STACKOFLOW:	fprintf(stderr, "parameter stack overflow.\n");		break;
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

	return 0;
}
