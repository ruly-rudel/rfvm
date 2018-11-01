#ifndef _rfvm_h_
#define _rfvm_h_

#include <stdint.h>

typedef enum {
	OP_HALT = 0,

	OP_CALL,
	OP_RET,

	OP_BB,
	OP_BBZ,

	OP_PUSHB,
	OP_PUSHD,
	OP_PUSHQ,

	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,

	OP_EQ,
	OP_GNE,
	OP_GE,
	OP_LNE,
	OP_LE,

	OP_AND,
	OP_OR,

	OP_DUP,
	OP_DROP,
	OP_SWAP,

	OP_NOT,

	OP_DOT,
} prim_t;

typedef enum {
	E_OK = 0,
	E_STACKUFLOW,
	E_STACKOFLOW,
	E_NOTIMPL,
} err_t;


int exec_rfvm(uint8_t* code);


#endif // _rfvm_h_
