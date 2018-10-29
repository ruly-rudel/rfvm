#define DEF_EXTERN

#include <stdio.h>
#include <stdlib.h>
#include "rfvm.h"


#define NEXT_OP		goto *jtbl[*++ip]

/////////////////////////////////////////////////////////////////////
// public: vm

int exec_rfvm(uint8_t* code)
{
	// instruction pointer
	uint8_t* ip     = code;

	// parameter stack
	int64_t* pstack = malloc(sizeof(int64_t) * 1024);
	int64_t* psp    = pstack;

	// jamp table
	static const void *jtbl[] = {
		&&LB_HALT,
		&&LB_PUSHB,
		&&LB_NOTIMPL,		// OP_PUSHD
		&&LB_NOTIMPL,		// OP_PUSHQ
		&&LB_ADD,
		&&LB_SUB,
		&&LB_MUL,
		&&LB_NOTIMPL,		// OP_DIV
		&&LB_DOT,
	};

	//////////////////////////////////////////////////
	// opcodes
LB_PUSHB:
	*psp++ = (int8_t)*++ip;
	NEXT_OP;

LB_ADD:
	psp -= 1;
	*(psp - 1) += *psp;
	NEXT_OP;

LB_SUB:
	psp -= 1;
	*(psp - 1) -= *psp;
	NEXT_OP;

LB_MUL:
	psp -= 1;
	*(psp - 1) *= *psp;
	NEXT_OP;

LB_DOT:
	printf("%ld\n", *--psp);
	NEXT_OP;

LB_NOTIMPL:
	fprintf(stderr, "opcode not implemented yet.\n");
	abort();

LB_HALT:
	return 0;

}

