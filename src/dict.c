#include <string.h>
#include "rfvm.h"
#include "dict.h"

inline static int64_t	dict_get_word_size(void* word)
{
	return *(int64_t*)(word - 8);
}

inline static char*	dict_get_word_name(void* word)
{
	return word - dict_get_word_size(word);
}

inline static void*	dict_get_word_body(void* word)
{
	return word - dict_get_word_size(word) + 32 + 8;
}

inline static void*	dict_get_next_word(void* word)
{
	return dict_get_word_name(word);
}

/////////////////////////////////////////////////////////////////////
// public: dictornary management functions

dict_t	dict_init(void* buf, int size)
{
	dict_t	dict = { 0 };

	dict.buf	= buf;
	dict.buf_size	= size;

	// sentinel(first word, body size is 0)
	dict.ep		= buf + 48;
	dict.mrd	= dict.ep;
	memset(buf, 0, 48);

	return dict;
}



dict_t*	dict_begin_def(const char* name, bool immediate, dict_t* dict)
{
	// set name
	strncpy(dict->ep, name, 32);
	dict->ep += 32;

	// set immediate
	dict_emit_qw(immediate, dict);

	return dict;
}

void*	dict_get_current_body(dict_t* dict)
{
	return dict->mrd + 32 + 8;
}

#define DEF_EMITTER(T, SUFIX) \
dict_t*	dict_emit_ ## SUFIX (T val, dict_t* dict) \
{ \
	*(T*)dict->ep = val; \
	dict->ep += sizeof(T); \
	return dict; \
}

DEF_EMITTER(uint8_t, op)
DEF_EMITTER(uint8_t, b)
DEF_EMITTER(void*,   ptr)
DEF_EMITTER(int64_t, qw)

dict_t*	dict_end_def(dict_t* dict)
{
	// set size
	dict_emit_qw(dict->ep - dict->mrd + 8, dict);
	dict->mrd = dict->ep;

	return dict;
}

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

