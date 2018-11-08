#include <string.h>
#include "allocator.h"
#include "rfvm.h"
#include "dict.h"

inline static int64_t	dict_get_word_size(rfval_t* word)
{
	return IMM(*(word - 1));
}

inline static rfval_t*	dict_get_next_word(rfval_t* word)
{
	return word - dict_get_word_size(word);
}

inline static void*	dict_get_word_native(rfval_t* word)
{
	return (word - 2)->ptr;
}

inline static rfval_t*	dict_set_word_native(void* native, rfval_t* word)
{
	*(word - 2) = RFPTR(native);
	return word;
}

inline static char*	dict_get_word_name(rfval_t* word)
{
	return (char*)dict_get_next_word(word);
}

inline static rfval_t*	dict_get_word_body(rfval_t* word)
{
	return word - dict_get_word_size(word) + 32 + 1;
}


/////////////////////////////////////////////////////////////////////
// public: dictornary management functions

dict_t	dict_init(int size)
{
	dict_t	dict = { 0 };

	dict.buf	= RFPTR(alloc_vector(size));

	// sentinel(first word, body size is 0)
	dict.ep		= dict.buf.vector->data + 33;
	dict.mrd	= dict.ep;
	memset(dict.buf.vector->data, 0, 33 * sizeof(rfval_t));

	return dict;
}



dict_t*	dict_begin_def(const char* name, bool immediate, dict_t* dict)
{
	// set name
	strncpy((char*)dict->ep, name, 32);
	dict->ep += 32;

	// set immediate
	dict_emit_qw(immediate, dict);

	return dict;
}

void*	dict_get_current_body(dict_t* dict)
{
	return dict->mrd + 32 + 1;
}

/*
#define DEF_EMITTER(T, SUFIX) \
dict_t*	dict_emit_ ## SUFIX (T val, dict_t* dict) \
{ \
	*(T*)dict->ep = val; \
	dict->ep += sizeof(T); \
	return dict; \
}
*/

#define DEF_EMITTER_IMM(T, SUFIX) \
dict_t*	dict_emit_ ## SUFIX (T val, dict_t* dict) \
{ \
	*dict->ep++ = RFINT(val); \
	return dict; \
}

#define DEF_EMITTER_PTR(T, SUFIX) \
dict_t*	dict_emit_ ## SUFIX (T val, dict_t* dict) \
{ \
	*dict->ep++ = RFPTR(val); \
	return dict; \
}

DEF_EMITTER_IMM(uint8_t, op)
DEF_EMITTER_IMM(uint8_t, b)
DEF_EMITTER_PTR(void*,   ptr)
DEF_EMITTER_IMM(int32_t, dw)
DEF_EMITTER_IMM(int64_t, qw)

dict_t*	dict_end_def(dict_t* dict)
{
	// set pointer to native code (is NULL)
	dict_emit_ptr(0, dict);

	// set size
	dict_emit_qw(dict->ep - dict->mrd + 1, dict);
	dict->mrd = dict->ep;

	return dict;
}

void*	dict_get_word(const char* name, dict_t* dict)
{
	void* word = dict->mrd;
	while(dict_get_word_size(word) != 0)
	{
		if(!strncmp(dict_get_word_name(word), name, 32))
		{
			return word;
		}
		else
		{
			word = dict_get_next_word(word);
		}
	}

	return 0;	// word not found
}

void*	get_word_body(void* word)
{
	return word ? dict_get_word_body(word) : 0;
}

void*	get_word_native(void* word)
{
	return word ? dict_get_word_native(word) : 0;
}

void*	set_word_native(void* native, void* word)
{
	return word ? dict_set_word_native(native, word) : 0;
}

	/*
void*	dict_get_body(const char* name, dict_t* dict)
{
	void* word = dict->mrd;
	while(dict_get_word_size(word) != 0)
	{
		if(!strncmp(dict_get_word_name(word), name, 32))
		{
			return dict_get_word_body(word);
		}
		else
		{
			word = dict_get_next_word(word);
		}
	}

	return 0;	// word not found
}
	*/

