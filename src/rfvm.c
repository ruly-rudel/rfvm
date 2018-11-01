#define DEF_EXTERN

#include <stdio.h>
#include <stdlib.h>
#include "rfvm.h"

#ifdef NBCHECK
#define PSP_UF_CHK(X)
#define PSP_OF_CHK(X)
#define RSP_UF_CHK
#define RSP_OF_CHK
#else // NBCHECK
#define PSP_UF_CHK(X)	if(psp - psb < (X)) { ret = E_STACKUFLOW; goto LB_HALT; }
#define PSP_OF_CHK(X)	if(pst - psp < (X)) { ret = E_STACKOFLOW; goto LB_HALT; }
#define RSP_UF_CHK	if(rsp - rsb < 1)   { ret = E_STACKUFLOW; goto LB_HALT; }
#define RSP_OF_CHK	if(rst - rsp < 1)   { ret = E_STACKOFLOW; goto LB_HALT; }
#endif // NBCHECK

#define JUMP_OP		goto *jtbl[*ip]
#define NEXT_OP		goto *jtbl[*++ip]

#define DEF_BOP(OP) \
	PSP_UF_CHK(2) \
	psp -= 1; *(psp - 1) = *(psp - 1) OP *psp; \
	NEXT_OP;

/////////////////////////////////////////////////////////////////////
// public: vm

int exec_rfvm(uint8_t* code)
{
	// instruction pointer
	uint8_t* ip     = code;

	// parameter stack
	int64_t* pstack  = malloc(sizeof(int64_t) * 1024);
	int64_t* psb     = pstack + 3;		// safe until three arguments exaust
	int64_t* pst     = pstack + 1024 - 3;	// safe until three arguments overflow
	int64_t* psp     = psb;

	// return stack
	uint8_t** rstack = malloc(sizeof(uint8_t*) * 1024);
	uint8_t** rsb    = rstack;
	uint8_t** rst    = rstack + 1024;
	uint8_t** rsp    = rstack;

	// working register
	int64_t  r0;

	// jump table
	static const void *jtbl[] = {
		&&LB_HALT,
		&&LB_CALL,
		&&LB_RET,
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
		&&LB_GE,
		&&LB_LNE,
		&&LB_LE,
		&&LB_AND,
		&&LB_OR,
		&&LB_DUP,
		&&LB_DROP,
		&&LB_SWAP,
		&&LB_NOT,
		&&LB_DOT,
	};

	// return code
	int ret = E_OK;

	// initialize done. now execute vm.
	JUMP_OP;

	//////////////////////////////////////////////////
	// opcodes
LB_CALL:
	RSP_OF_CHK;
	*rsp++ = ip + 1 + sizeof(ip);
	ip = *(uint8_t**)(ip + 1);
	JUMP_OP;

LB_RET:
	RSP_UF_CHK;
	ip = *--rsp;
	JUMP_OP;

LB_BB:
	ip = ip + *(int8_t*)(ip + 1);
	JUMP_OP;

LB_BBZ:
	PSP_UF_CHK(1);
	ip = ip + (*--psp ? 2 : *(int8_t*)(ip + 1));
	JUMP_OP;

LB_PUSHB:
	PSP_OF_CHK(1);
	*psp++ = (int8_t)*++ip;
	NEXT_OP;

LB_ADD: DEF_BOP(+)
LB_SUB:	DEF_BOP(-)
LB_MUL:	DEF_BOP(*)
LB_DIV:	DEF_BOP(/)
LB_EQ:	DEF_BOP(==)
LB_GNE:	DEF_BOP(>)
LB_GE:	DEF_BOP(>=)
LB_LNE:	DEF_BOP(<)
LB_LE:	DEF_BOP(<=)
LB_AND:	DEF_BOP(&&)
LB_OR:	DEF_BOP(||)

LB_DUP:
	PSP_UF_CHK(1); PSP_OF_CHK(1);
	*psp = *(psp - 1);
	psp++; NEXT_OP;

LB_DROP:
	PSP_UF_CHK(1);
	psp--;
	NEXT_OP;

LB_SWAP:
	PSP_UF_CHK(2);
	r0 = *(psp - 1);
	*(psp - 1) = *(psp - 2);
	*(psp - 2) = r0;
	r0 = 0;
	NEXT_OP;

LB_NOT:
	PSP_UF_CHK(1);
	*psp = !*psp;
	NEXT_OP;

LB_DOT:
	PSP_UF_CHK(1);
	printf("%ld ", *--psp);
	NEXT_OP;

LB_NOTIMPL:
	ret = E_NOTIMPL;
	// fall thru

LB_HALT:
	free(pstack);
	free(rstack);
	return ret;
}

