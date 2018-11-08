#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "rftype.h"
#include "allocator.h"


#ifndef NOGC
rfval_t* exec_gc(void)
{
	return 0;
}
#endif

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

cons_t* alloc_cons(void)
{
#ifdef CHECK_GC_SANITY
	check_sanity();
#endif // CHECK_GC_SANITY

	// allocation try
	cons_t* c = (cons_t*)g_memory_top;
	g_memory_top += 3;

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
			c = (cons_t*)g_memory_top;
			g_memory_top += 3;
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

	// set type
	c->type = TYPE_CONS;

#ifdef DUMP_ALLOC_ADDR
#if __WORDSIZE == 32
	fprintf(stderr, "cons: %08x\n", (int)c);
#else
	fprintf(stderr, "cons: %016lx\n", (int64_t)c);
#endif
#endif	// DUMP_ALLOC_ADDR

#ifdef DEBUG_GC
	g_memory_top -= 3;
	check_sanity();
	g_memory_top += 3;
#endif // DEBUG_GC
	return c;
}

vector_t* alloc_vector(size_t size)
{
#ifdef CHECK_GC_SANITY
	check_sanity();
#endif // CHECK_GC_SANITY

	// allocation try
	vector_t* v = (vector_t*)g_memory_top;
	g_memory_top += 2 + size;

	// boundary check
#ifdef NOGC
	if(g_memory_top >= g_memory_max)
	{
		return 0;
	}
#else	// NOGC
	if(g_memory_top >= g_memory_gc || FORCE_GC)
	{
		g_memory_top -= (2 + size);
		if(exec_gc())
		{
			v = (vector_t*)g_memory_top;
			g_memory_top += (2 + size);
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

	// set type and size
	v->type = TYPE_VEC;
	v->size = RFINT(size);

#ifdef DUMP_ALLOC_ADDR
#if __WORDSIZE == 32
	fprintf(stderr, "vect: %08x\n", (int)v);
#else
	fprintf(stderr, "vect: %016lx\n", (int64_t)v);
#endif
#endif	// DUMP_ALLOC_ADDR

#ifdef CHECK_GC_SANITY
	g_memory_top -= (2 + size);
	check_sanity();
	g_memory_top += (2 + size);
#endif // CHECK_GC_SANITY
	return v;
}

// End of File
/////////////////////////////////////////////////////////////////////
