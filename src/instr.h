#ifndef __INSTR_H__
#define __INSTR_H__

#include "fm.h"

typedef struct {
    note_t note;
    unsigned int octave;
    fm_instr_t fm_instr;
} instr_t;

fm_instr_t tick1 = {
    .c__am_vib_eg = 0x05,
    .c__ksl_volume = 0x0b,
    .c__attack_decay = 0xf6,
    .c__sustain_release = 0x94,
    .c__waveform = 0x00,

    .m__am_vib_eg = 0x07,
    .m__ksl_volume = 0x40,
    .m__attack_decay = 0x09,
    .m__sustain_release = 0x53,
    .m__waveform = 0x00,

    .feedback_fm = 0x0e,
    .fine_tune = 0x00,
    .panning = Center,
    .voice_type = Melodic,
};

#endif // __INSTR_H__
