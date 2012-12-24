#ifndef __SOUND_H__
#define __SOUND_H__

#include "common.h"

#define ADDR_PORT 0x388
#define DATA_PORT 0x389

void write(const int reg, const int value);
void reset_sound();

void play_tick();

#endif // __SOUND_H__
