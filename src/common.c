#include "common.h"

void msleep(const int microsecs) {
   const int wait = 1000000 / microsecs;
   const uclock_t start = uclock();
   while (uclock() < start + UCLOCKS_PER_SEC / wait);
}
