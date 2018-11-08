#ifndef _rftype_h_
#define _rftype_h_

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>
#include "misc.h"

typedef enum _rft_t {
	PTR_T  = 0,
	GC_T   = 1,
	TYPE_T = 2,
	IMM_T  = 3,
} rft_t;

typedef enum {
	RFINT_T = 4,
	RFCHAR_T,
	RFVMOP_T,
	CONS_T,
	SVEC_T,
	SYM_T,
	CLOJ_T,
	MACRO_T,
	ERR_T,
} rftit_t;

#if __WORDSIZE == 32
typedef struct
{
	uint32_t	main:   2;
	uint32_t	sub:    6;
	int32_t		val:   24;
} imm_t;
#else
typedef struct
{
	uint64_t	main:   2;
	uint64_t	sub:    6;
	int64_t		val:   56;
} imm_t;
#endif

struct _cons_t;
struct _svec_t;
typedef union _rfval_t
{
	imm_t			imm;
	struct _cons_t*		cons;
	struct _svec_t*		svec;
	union _rfval_t*		ptr;
} rfval_t;


typedef struct _cons_t
{
	rfval_t		type;
	rfval_t		car;
	rfval_t		cdr;
} cons_t;

typedef struct _svec_t
{
	rfval_t		type;
	rfval_t		size;
	rfval_t		data[1];
} svec_t;

#define NIL          ((rfval_t){ .ptr = 0 })
#define RFPTR(X)     ((rfval_t){ .ptr = (X) })
#define RFCHAR(X)    ((rfval_t){ .imm.main = IMM_T, .imm.sub = RFCHAR_T, .imm.val   = (X) })
#define RFINT(X)     ((rfval_t){ .imm.main = IMM_T, .imm.sub = RFINT_T,  .imm.val   = (X) })
#define TYPE_CONS    ((rfval_t){ .imm.main = IMM_T, .imm.sub = CONS_T,   .imm.val   = 0 })
#define TYPE_SVEC    ((rfval_t){ .imm.main = IMM_T, .imm.sub = SVEC_T,   .imm.val   = 0 })

#define IMM(X)        (((intptr_t)(X).ptr) >> 8)
#define EQ(X, Y)	((X).ptr == (Y).ptr)


INLINE(int     rftypeof(rfval_t v), v.imm.main == IMM_T ? v.imm.sub : v.imm.main)

INLINE(bool    ptrp(rfval_t x),  x.imm.main == PTR_T)
INLINE(bool    typep(rfval_t x), x.imm.main == TYPE_T)
INLINE(bool    intp(rfval_t x),  x.imm.main == IMM_T &&  x.imm.sub == RFINT_T)
INLINE(bool    charp(rfval_t x), x.imm.main == IMM_T &&  x.imm.sub == RFCHAR_T)
INLINE(bool    consp(rfval_t x), x.imm.main == PTR_T &&  ((rfval_t*)x.ptr)->imm.main == IMM_T && ((rfval_t*)x.ptr)->imm.sub == CONS_T)
INLINE(bool    svecp(rfval_t x), x.imm.main == PTR_T &&  ((rfval_t*)x.ptr)->imm.main == IMM_T && ((rfval_t*)x.ptr)->imm.sub == SVEC_T)

INLINE(bool    nilp(rfval_t x),  x.ptr == 0)

#endif // _rftype_h_
