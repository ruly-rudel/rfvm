#ifndef _dict_h_
#define _dict_h_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	void*		buf;
	uint64_t	buf_size;
	void*		ep;
	void*		mrd;
} dict_t;

dict_t	dict_init(void* buf, int size);

dict_t*	dict_begin_def(const char* name, bool immediate, dict_t* dict);
void*	dict_get_current_body(dict_t* dict);
dict_t*	dict_emit_op (uint8_t val, dict_t* dict);
dict_t*	dict_emit_b  (uint8_t val, dict_t* dict);
dict_t*	dict_emit_ptr(void*   val, dict_t* dict);
dict_t*	dict_emit_qw (int64_t val, dict_t* dict);
dict_t*	dict_end_def(dict_t* dict);

void*	dict_get_body(const char* name, dict_t* dict);

#endif // _dict_h_
