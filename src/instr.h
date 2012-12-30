#ifndef __INSTR_H__
#define __INSTR_H__

#include "fm.h"

fm_instr_t tick1 = {
    .c__am_vib_eg = 0x01,
    .c__ksl_volume = 0x10,
    .c__attack_decay = 0xf0,
    .c__sustain_release = 0x77,
    .c__waveform = 0x00,

    .m__am_vib_eg = 0x01,
    .m__ksl_volume = 0x00,
    .m__attack_decay = 0xf0,
    .m__sustain_release = 0x77,
    .m__waveform = 0x00,

    .feedback_fm = 0x00,
    .fine_tune = 0x00,
    .panning = Center,
    .voice_type = Melodic,
};

fm_instr_t bass1 = {
    .c__am_vib_eg = 0x00,
    .c__ksl_volume = 0x00,
    .c__attack_decay = 0xa4,
    .c__sustain_release = 0xa1,
    .c__waveform = 0x00,

    .m__am_vib_eg = 0x40,
    .m__ksl_volume = 0x03,
    .m__attack_decay = 0xa4,
    .m__sustain_release = 0x06,
    .m__waveform = 0x05,

    .feedback_fm = 0x00,
    .fine_tune = 0x00,
    .panning = Center,
    .voice_type = Melodic,
};


#endif // __INSTR_H__
