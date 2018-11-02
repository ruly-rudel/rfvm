#ifndef _jit_h_
#define _jit_h_

#include <stdint.h>
#include <stdbool.h>
#include "dict.h"

typedef struct
{
	dict_t		dict;
	uint64_t*	pstack;
} jit_t;

jit_t	jit_init(int size);

jit_t*	jit_begin_def(const char* name, bool immediate, jit_t* jit);
void*	jit_get_current_body(jit_t* jit);

jit_t*	jit_emit_pushb (int8_t val, jit_t* jit);
jit_t*	jit_emit_dup   (jit_t* jit);
jit_t*	jit_emit_drop (jit_t* jit);
jit_t*	jit_emit_swap (jit_t* jit);
jit_t*	jit_emit_add (jit_t* jit);
jit_t*	jit_emit_sub (jit_t* jit);
jit_t*	jit_emit_bmi (int32_t offset, jit_t* jit);		// branch minus / negative
jit_t*	jit_emit_bpl (int32_t offset, jit_t* jit);		// branch plus  / zero
jit_t*	jit_emit_call(void* fn, jit_t* jit);
jit_t*	jit_emit_ret(jit_t* jit);

jit_t*	jit_end_def(jit_t* jit);

void*	jit_get_body(const char* name, jit_t* jit);

jit_t*	jit_make_executable(jit_t* jit);
jit_t*	jit_make_writable(jit_t* jit);
int64_t jit_run(const char* name, jit_t* jit, int num, ...);

#endif // _jit_h_
