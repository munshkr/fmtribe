#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef char* string;
typedef int bool;
typedef unsigned char  byte;
typedef unsigned short word;

#define true  1
#define false 0

#define KEY_ESC 27
#define USECS_PER_MINUTE (1000000 * 60)

#define sgn(x) (x > 0 ? 1 : (x < 0 ? (-1) : 0))
#define not(x) (x ? false : true)

void msleep(const int microsecs);

#endif // __COMMON_H__
