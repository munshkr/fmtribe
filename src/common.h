#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>

typedef char* string;

#define true  1
#define false 0

#define KEY_ESC 27
#define USECS_PER_MINUTE (1000000 * 60)

void outb(const int port, const char data);
int inb(const int port);

void msleep(const int microsecs);

#endif // __COMMON_H__
