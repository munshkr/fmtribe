#include "common.h"

__inline void outb(const int port, const char data) {
    __asm __volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

__inline int inb(const int port) {
    char data;
    __asm __volatile("inb %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

void msleep(const int microsecs) {
   const int wait = 1000000 / microsecs;
   const uclock_t start = uclock();
   while (uclock() < start + UCLOCKS_PER_SEC / wait);
}
