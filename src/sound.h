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

typedef enum { Center, Left, Right } panning_t;
typedef enum { Melodic, BD, SD, TT, TC, HH } voice_type_t;
typedef struct {
    unsigned char m__am_vib_eg;
    unsigned char c__am_vib_eg;
    unsigned char m__ksl_volume;
    unsigned char c__ksl_volume;
    unsigned char m__attack_decay;
    unsigned char c__attack_decay;
    unsigned char m__sustain_release;
    unsigned char c__sustain_release;
    unsigned char m__waveform;
    unsigned char c__waveform;
    unsigned char feedback_fm;
    panning_t     panning;
    signed char   fine_tune;
    voice_type_t  voice_type;
} instr_t;

void sound_reset();
void sound_play_metronome_tick(unsigned int c, unsigned int octave, unsigned short fnum);
void sound_play_bass1(unsigned int c, unsigned int octave, unsigned short fnum);

#endif // __SOUND_H__
