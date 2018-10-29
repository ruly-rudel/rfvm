#ifndef _rfvm_h_
#define _rfvm_h_

#include <stdint.h>

typedef enum {
	OP_HALT = 0,
	OP_PUSHB,
	OP_PUSHD,
	OP_PUSHQ,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_DOT,
} prim_t;


int exec_rfvm(uint8_t* code);


#endif // _rfvm_h_
