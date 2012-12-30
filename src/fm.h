#ifndef __FM_H__
#define __FM_H__

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

typedef enum { Center, Left, Right } fm_panning_t;
typedef enum { Melodic, BD, SD, TT, TC, HH } fm_voice_type_t;

typedef struct {
    uint8_t m__am_vib_eg;
    uint8_t c__am_vib_eg;
    uint8_t m__ksl_volume;
    uint8_t c__ksl_volume;
    uint8_t m__attack_decay;
    uint8_t c__attack_decay;
    uint8_t m__sustain_release;
    uint8_t c__sustain_release;
    uint8_t m__waveform;
    uint8_t c__waveform;
    uint8_t feedback_fm;
    int8_t fine_tune;
    fm_panning_t panning;
    fm_voice_type_t voice_type;
} fm_instr_t;

void fm_reset();
void fm_init();
void fm_set_instrument(const unsigned int channel, const fm_instr_t* instr);
void fm_key_on(const unsigned int channel, const uint8_t octave, const uint16_t fnum);
void fm_key_off(const unsigned int channel);

#endif // __FM_H__
