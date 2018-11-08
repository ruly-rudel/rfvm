#define DEF_EXTERN

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "rfvm.h"
#include "allocator.h"
#include "rftype.h"

#ifdef NBCHECK
#define PSP_UF_CHK(X)
#define PSP_OF_CHK(X)
#define RSP_UF_CHK
#define RSP_OF_CHK
#else // NBCHECK
#define PSP_UF_CHK(X)	if(psp       < (X) + 1) { ret = E_STACKUFLOW; goto LB_HALT; }
#define PSP_OF_CHK(X)	if(pst - psp < (X) + 1) { ret = E_STACKOFLOW; goto LB_HALT; }
#define RSP_UF_CHK	if(rsp       <       1) { ret = E_STACKUFLOW; goto LB_HALT; }
#define RSP_OF_CHK	if(rsp       >    1023) { ret = E_STACKOFLOW; goto LB_HALT; }
#endif // NBCHECK

#define JUMP_OP		goto *jtbl[IMM(*ip.ptr)]
#define NEXT_OP		goto *jtbl[IMM(*++ip.ptr)]

#define DEF_BOP(OP) \
	PSP_UF_CHK(2) \
	psp -= 1; pstack.svec->data[psp - 1] = RFINT(IMM(pstack.svec->data[psp - 1]) OP IMM(pstack.svec->data[psp])); \
	NEXT_OP;

/////////////////////////////////////////////////////////////////////
// public: vm

int exec_rfvm(rfval_t* code, rfval_t pstack)
{
	assert(svecp(pstack));

	// instruction pointer
	rfval_t ip	= RFPTR(code);

	// parameter stack
	int	pst	= IMM(pstack.svec->size) - 1;
	int	psp	= 1 + IMM(pstack.svec->data[0]);

	// return stack
	rfval_t	rstack	= alloc_svec(1024);
	int	rsp	= 0;

	// working register
	rfval_t  r0;

	// jump table
	static const void *jtbl[] = {
		&&LB_HALT,
		&&LB_CALL,
		&&LB_RET,
		&&LB_BB,
		&&LB_BPL,
		&&LB_BMI,
//		&&LB_BBZ,
		&&LB_PUSHB,
		&&LB_NOTIMPL,		// OP_PUSHD
		&&LB_NOTIMPL,		// OP_PUSHQ
		&&LB_ADD,
		&&LB_SUB,
		&&LB_MUL,
		&&LB_DIV,
		/*
		&&LB_EQ,
		&&LB_GNE,
		&&LB_GE,
		&&LB_LNE,
		&&LB_LE,
		*/
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
	rstack.svec->data[rsp++] = RFPTR(ip.ptr + 2);
	ip.ptr = (ip.ptr + 1)->ptr;
	JUMP_OP;

LB_RET:
	RSP_UF_CHK;
	ip.ptr = rstack.svec->data[--rsp].ptr;
	JUMP_OP;

LB_BB:
	ip.ptr += IMM(*(ip.ptr + 1));
	JUMP_OP;

LB_BPL:
	PSP_UF_CHK(1);
	ip.ptr += (IMM(pstack.svec->data[--psp]) >= 0 ? IMM(*(ip.ptr + 1)) : 2);
	JUMP_OP;

LB_BMI:
	PSP_UF_CHK(1);
	ip.ptr += (IMM(pstack.svec->data[--psp]) <  0 ? IMM(*(ip.ptr + 1)) : 2);
	JUMP_OP;
	/*
LB_BBZ:
	PSP_UF_CHK(1);
	ip = ip + (*--psp ? 2 : *(int8_t*)(ip + 1));
	JUMP_OP;
	*/

LB_PUSHB:
	PSP_OF_CHK(1);
	pstack.svec->data[psp++] = *++ip.ptr;
	NEXT_OP;

LB_ADD: DEF_BOP(+)
LB_SUB:	DEF_BOP(-)
LB_MUL:	DEF_BOP(*)
LB_DIV:	DEF_BOP(/)
/*
LB_EQ:	DEF_BOP(==)
LB_GNE:	DEF_BOP(>)
LB_GE:	DEF_BOP(>=)
LB_LNE:	DEF_BOP(<)
LB_LE:	DEF_BOP(<=)
*/
LB_AND:	DEF_BOP(&&)
LB_OR:	DEF_BOP(||)

LB_DUP:
	PSP_UF_CHK(1); PSP_OF_CHK(1);
	pstack.svec->data[psp] = pstack.svec->data[psp - 1];
	psp++; NEXT_OP;

LB_DROP:
	PSP_UF_CHK(1);
	psp--;
	NEXT_OP;

LB_SWAP:
	PSP_UF_CHK(2);
	r0 = pstack.svec->data[psp - 1];
	pstack.svec->data[psp - 1] = pstack.svec->data[psp - 2];
	pstack.svec->data[psp - 2] = r0;
	NEXT_OP;

LB_NOT:
	PSP_UF_CHK(1);
	pstack.svec->data[psp - 1] = RFINT(!IMM(pstack.svec->data[psp - 1]));
	NEXT_OP;

LB_DOT:
	PSP_UF_CHK(1);
#if __WORDSIZE == 32
	printf("%d ",  IMM(pstack.svec->data[--psp]));
#else
	printf("%ld ", IMM(pstack.svec->data[--psp]));
#endif
	NEXT_OP;

LB_NOTIMPL:
	ret = E_NOTIMPL;
	// fall thru

LB_HALT:
	pstack.svec->data[0] = RFINT(psp - 1);
	return ret;
}

