#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "rftype.h"
#include "allocator.h"

/////////////////////////////////////////////////////////////////////
// private: primal memory allocator

#ifndef NOGC
static rfval_t* exec_gc(void)
{
	return 0;
}
#endif

static rfval_t* alloc(size_t size)
{
#ifdef CHECK_GC_SANITY
	check_sanity();
#endif // CHECK_GC_SANITY

	// allocation try
	rfval_t* r = g_memory_top;
	g_memory_top += size;

	// boundary check
#ifdef NOGC
	if(g_memory_top >= g_memory_max)
	{
		return 0;
	}
#else	// NOGC
	if(g_memory_top >= g_memory_gc || FORCE_GC)
	{
		g_memory_top -= 3;
		if(exec_gc())
		{
			r = g_memory_top;
			g_memory_top += size;
			if(g_memory_top >= g_memory_max)
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
#endif	// NOGC

#ifdef DUMP_ALLOC_ADDR
#if __WORDSIZE == 32
	fprintf(stderr, "alloc at %08x, size %d\n", (int)r, size);
#else
	fprintf(stderr, "alloc at %016lx, size %d\n", (int64_t)r, size);
#endif
#endif	// DUMP_ALLOC_ADDR

#ifdef DEBUG_GC
	g_memory_top -= size;
	check_sanity();
	g_memory_top += size;
#endif // DEBUG_GC
	return r;
}

/////////////////////////////////////////////////////////////////////
// public: memory allocator

void init_allocator(void)
{
	g_memory_pool      = (rfval_t*)malloc(sizeof(rfval_t) * INITIAL_ALLOC_SIZE);
#ifndef NOGC
	g_memory_pool_from = (rfval_t*)malloc(sizeof(rfval_t) * INITIAL_ALLOC_SIZE);
#endif  // NOGC
	g_memory_top       = g_memory_pool;
	g_memory_max       = g_memory_pool + INITIAL_ALLOC_SIZE;
	g_memory_gc        = g_memory_pool + INITIAL_ALLOC_SIZE / 2;
}

rfval_t alloc_cons(void)
{
	rfval_t c = { .ptr = alloc(3) };
	if(!nilp(c))
	{
		c.cons->type = TYPE_CONS;
	}

	return c;
}

rfval_t alloc_svec(size_t size)
{
	rfval_t v = { .ptr = alloc(2 + size) };

	if(!nilp(v))
	{
		// set type and size
		v.svec->type = TYPE_SVEC;
		v.svec->size = RFINT(size);
	}

	return v;
}

// End of File
/////////////////////////////////////////////////////////////////////
