
#include <stdio.h>
#include <assert.h>
#include "rfvm.h"

int main(int argc, char* argv[])
{
	uint8_t code[] = { OP_PUSHB, 10, OP_PUSHB, -2, OP_ADD, OP_DOT, OP_HALT };
	exec_rfvm(code);

	return 0;
}
