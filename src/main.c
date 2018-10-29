
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
	uint8_t code[] = { OP_PUSHB, 10, OP_PUSHB, -2, OP_ADD, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code));

	uint8_t code2[] = { OP_PUSHB, 10, OP_ADD, OP_DOT, OP_HALT };
	print_err(exec_rfvm(code2));

	return 0;
}
