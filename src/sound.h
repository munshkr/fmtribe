#ifndef __SOUND_H__
#define __SOUND_H__

#include <pc.h>
#include <unistd.h>

#include "common.h"

#define NOTE_Cs  0x16b
#define NOTE_D   0x181
#define NOTE_Ds  0x198
#define NOTE_E   0x1b0
#define NOTE_F   0x1ca
#define NOTE_Fs  0x1e5
#define NOTE_G   0x202
#define NOTE_Gs  0x220
#define NOTE_A   0x241
#define NOTE_As  0x263
#define NOTE_B   0x287
#define NOTE_C   0x2ae

__inline void sound_write(const int reg, const int value);
void sound_reset();

void sound_play_metronome_tick(unsigned int channel, unsigned short fnum);

#endif // __SOUND_H__
