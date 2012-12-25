#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>

typedef char* string;
typedef unsigned char  byte;
typedef unsigned short word;

#define sgn(x) (x > 0 ? 1 : (x < 0 ? (-1) : 0))

#define true  1
#define false 0

#define KEY_ESC 27
#define USECS_PER_MINUTE (1000000 * 60)

__inline void outb(const int port, const char data);
__inline int inb(const int port);

void msleep(const int microsecs);

#endif // __COMMON_H__
