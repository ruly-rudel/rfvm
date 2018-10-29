#define DEF_EXTERN

#include <stdio.h>
#include <stdlib.h>
#include "rfvm.h"


#define PSP_UF_CHK_M1	if(psp <= psb) { ret = E_FEWSTACK;   goto LB_HALT; }
#define PSP_UF_CHK	if(psp <  psb) { ret = E_FEWSTACK;   goto LB_HALT; }
#define PSP_OF_CHK	if(psp >  pst) { ret = E_STACKOFLOW; goto LB_HALT; }
#define NEXT_OP		goto *jtbl[*++ip]

#define DEF_BOP(OP) \
	psp -= 1; *(psp - 1) = *(psp - 1) OP *psp; \
	PSP_UF_CHK_M1; NEXT_OP;

/////////////////////////////////////////////////////////////////////
// public: vm

int exec_rfvm(uint8_t* code)
{
	// instruction pointer
	uint8_t* ip     = code;

	// parameter stack
	int64_t* pstack = malloc(sizeof(int64_t) * 1024);
	int64_t* psb    = pstack + 3;		// safe until three arguments exaust
	int64_t* pst    = pstack + 1024 - 3;	// safe until three arguments overflow
	int64_t* psp    = psb;

	// jamp table
	static const void *jtbl[] = {
		&&LB_HALT,
		&&LB_BB,
		&&LB_BBZ,
		&&LB_PUSHB,
		&&LB_NOTIMPL,		// OP_PUSHD
		&&LB_NOTIMPL,		// OP_PUSHQ
		&&LB_ADD,
		&&LB_SUB,
		&&LB_MUL,
		&&LB_DIV,
		&&LB_EQ,
		&&LB_GNE,
		&&LB_AND,
		&&LB_OR,
		&&LB_DUP,
		&&LB_NOT,
		&&LB_DOT,
	};

	// return code
	int ret = E_OK;

	// initialize done. now execute vm.
	goto *jtbl[*ip];

	//////////////////////////////////////////////////
	// opcodes
LB_BB:
	goto *jtbl[*(ip + (int8_t)*(ip + 1))];

LB_BBZ:
	ip++;
	if(*psp-- == 0)
	{
		goto *jtbl[*(ip + (int8_t)*ip)];
	}
	else
	{
		NEXT_OP;
	}

LB_PUSHB:
	*psp++ = (int8_t)*++ip;
	PSP_OF_CHK; NEXT_OP;

LB_ADD: DEF_BOP(+)
LB_SUB:	DEF_BOP(-)
LB_MUL:	DEF_BOP(*)
LB_DIV:	DEF_BOP(/)
LB_EQ:	DEF_BOP(==)
LB_GNE:	DEF_BOP(>)
LB_AND:	DEF_BOP(&&)
LB_OR:	DEF_BOP(||)

LB_DUP:
	*psp = *(psp - 1);
	PSP_UF_CHK_M1; psp++; PSP_OF_CHK; NEXT_OP;

LB_NOT:
	*psp = !*psp;
	PSP_UF_CHK_M1; NEXT_OP;

LB_DOT:
	printf("%ld ", *--psp);
	PSP_UF_CHK; NEXT_OP;

LB_NOTIMPL:
	ret = E_NOTIMPL;
	// fall thru

LB_HALT:
	return ret;
}

