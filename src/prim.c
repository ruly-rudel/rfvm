
#include "allocator.h"
#include "prim.h"

void print_val(rfval_t r0)
{
	switch(r0.imm.tag)
	{
	case IMM_T:
#if __WORDSIZE == 32
		wprintf(L"%d ",  IMM(r0));
#else
		wprintf(L"%ld ", IMM(r0));
#endif
		break;

	case PTR_T:
		if(strp(r0))
		{
			for(int i = 0; i < IMM(r0.svec->size); i++)
			{
				fputwc(IMM(r0.svec->data[i]), stdout);
			}
			fputwc(L' ', stdout);
		}
		else
		{
#if __WORDSIZE == 32
			wprintf(L"PTR %x ",  IMM(r0));
#else
			wprintf(L"PTR %lx ", IMM(r0));
#endif
		}
		break;

	default:
		fputws(L"unsupported print type. ", stdout);
		break;
	}
}

rfval_t read_line(FILE* fp)
{
	const int max_len = 256;
	rfval_t	 r	= alloc_svec(max_len);

	int n = 0;
	for(;;)
	{
		wint_t c = fgetwc(fp);
		if(c == WEOF && n == 0)	// EOF
		{
			return NIL;
		}
		else if(c == '\n' || c == WEOF)
		{
			r.svec->type = TYPE_STR;
			r.svec->size = RFINT(n);
			for(int i = n; i < max_len; i++)
			{
				r.svec->data[i] = RFPTR(0);
			}
			return r;
		}
		else
		{
			r.svec->data[n++] = RFCHAR(c);
		}
	}
}

rfval_t read_line_prompt(const wchar_t* prompt, FILE* fp)
{
	fputws(prompt, stdout);
	return read_line(fp);
}


// End of File
/////////////////////////////////////////////////////////////////////
