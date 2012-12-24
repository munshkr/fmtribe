#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>

typedef char* string;

void outb(const int port, const char data);
int inb(const int port);

void msleep(const int microsecs);

#endif // __COMMON_H__
