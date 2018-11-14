#ifndef _prim_h_
#define _prim_h_

#include <stdio.h>

void print_val(rfval_t r0);
rfval_t read_line(FILE* fp);
rfval_t read_line_prompt(const wchar_t* prompt, FILE* fp);

#endif  // _prim_h_
