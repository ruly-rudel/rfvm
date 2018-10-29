#define DEF_EXTERN

#include <stdio.h>
#include <stdlib.h>
#include "rfvm.h"


#define NEXT_OP		goto *jtbl[code[++pc]]

/////////////////////////////////////////////////////////////////////
// public: vm

int exec_rfvm(uint8_t* code)
{
	int pc = 0;

	int64_t* pstack = malloc(sizeof(int64_t) * 1024);
	int psp = 0;

	static const void *jtbl[] = {
		&&LB_HALT,
		&&LB_PUSHB,
		&&LB_NOTIMPL,
		&&LB_NOTIMPL,
		&&LB_ADD,
		&&LB_DOT,
	};

LB_PUSHB:
	pstack[psp++] = (int8_t)code[++pc];
	NEXT_OP;

LB_ADD:
	pstack[psp - 2] += pstack[psp - 1];
	psp -= 1;
	NEXT_OP;

LB_DOT:
	printf("%ld\n", pstack[--psp]);
	NEXT_OP;

LB_NOTIMPL:
	fprintf(stderr, "opcode not implemented yet.\n");
	abort();

LB_HALT:
	return 0;

}

