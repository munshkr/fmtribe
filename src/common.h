#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef char* string;

#define KEY_ESC 27
#define UCLOCKS_PER_MIN (UCLOCKS_PER_SEC * 60)

#define Sgn(x) (x > 0 ? 1 : (x < 0 ? (-1) : 0))
#define Not(x) (x ? false : true)
#define DivCeil(x,y) ((x + y - 1) / y)

// Use GCC built-in functionality for variable arguments
#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)

void swap(int* a, int* b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

typedef __builtin_va_list va_list;

#endif // __COMMON_H__
