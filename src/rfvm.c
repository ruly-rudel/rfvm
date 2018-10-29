#define DEF_EXTERN

#include <stdio.h>
#include <stdlib.h>
#include "rfvm.h"


/////////////////////////////////////////////////////////////////////
// public: vm

int exec_rfvm(uint8_t* code)
{
	int pc = 0;

	int64_t* pstack = malloc(sizeof(int64_t) * 1024);
	int psp = 0;

	for(;;)
	{
		uint8_t op = code[pc];
		switch(op)
		{
			case OP_HALT:
				return 0;

			case OP_PUSHB:
				pstack[psp] = (int8_t)code[pc + 1];
				psp += 1;
				pc  += 2;
				break;

			case OP_ADD:
				pstack[psp - 2] += pstack[psp - 1];
				psp -= 1;
				pc  += 1;
				break;

			case OP_DOT:
				printf("%ld\n", pstack[psp - 1]);
				psp -= 1;
				pc  += 1;
				break;

			default:
				fprintf(stderr, "unknown opcode.\n");
				abort();
		}
	}

	return -1;	// not reached
}

