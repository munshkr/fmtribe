#ifndef __FM_H__
#define __FM_H__

#include <pc.h>
#include <unistd.h>

#include "common.h"

typedef enum { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B } note_t;
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
void fm_key_on(const unsigned int c, const uint8_t octave, const note_t note);
void fm_key_off(const unsigned int channel);

__inline unsigned int fm_get_carrier_attack_rate(const fm_instr_t* instr);
__inline unsigned int fm_get_carrier_decay_rate(const fm_instr_t* instr);
__inline unsigned int fm_get_carrier_sustain_level(const fm_instr_t* instr);
__inline unsigned int fm_get_carrier_release_rate(const fm_instr_t* instr);
__inline unsigned int fm_get_modulator_attack_rate(const fm_instr_t* instr);
__inline unsigned int fm_get_modulator_decay_rate(const fm_instr_t* instr);
__inline unsigned int fm_get_modulator_sustain_level(const fm_instr_t* instr);
__inline unsigned int fm_get_modulator_release_rate(const fm_instr_t* instr);

__inline void fm_set_carrier_attack_rate(fm_instr_t* instr, unsigned int value);
__inline void fm_set_carrier_decay_rate(fm_instr_t* instr, unsigned int value);
__inline void fm_set_carrier_sustain_level(fm_instr_t* instr, unsigned int value);
__inline void fm_set_carrier_release_rate(fm_instr_t* instr, unsigned int value);
__inline void fm_set_modulator_attack_rate(fm_instr_t* instr, unsigned int value);
__inline void fm_set_modulator_decay_rate(fm_instr_t* instr, unsigned int value);
__inline void fm_set_modulator_sustain_level(fm_instr_t* instr, unsigned int value);
__inline void fm_set_modulator_release_rate(fm_instr_t* instr, unsigned int value);

#endif // __FM_H__
