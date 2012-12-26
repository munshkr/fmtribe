#ifndef __SOUND_H__
#define __SOUND_H__

#include <pc.h>
#include <unistd.h>

#include "common.h"

#define ADDR_PORT 0x388
#define DATA_PORT 0x389

__inline void sound_write(const int reg, const int value);
void sound_reset();

void sound_play_metronome_tick();

#endif // __SOUND_H__
