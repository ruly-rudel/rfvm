#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include "misc.h"
#include "rftype.h"

#define INITIAL_ALLOC_SIZE	(32 * 1024 * 1024)
#define ROOT_SIZE		1024

#ifdef DEBUG_GC
#define FORCE_GC 1
#else // DEBUG_GC
#define FORCE_GC 0
#endif // DEBUG_GC

EXTERN rfval_t* g_memory_pool;
#ifndef NOGC
EXTERN rfval_t* g_memory_pool_from;
#endif  // NOGC
EXTERN rfval_t* g_memory_top;
EXTERN rfval_t* g_memory_max;
EXTERN rfval_t* g_memory_gc;

void		init_allocator		(void);
rfval_t		alloc_cons		(void);
rfval_t		alloc_svec		(size_t size);

#endif // _ALLOCATOR_H_
