#include <math.h>
#include "fm.h"

#define BASE_PORT   0x388
#define ADDR_PORT   BASE_PORT
#define DATA_PORT   (BASE_PORT + 1)
#define DATA_PORT_B (BASE_PORT + 3)

#define OP1 0x00
#define OP2 0x03

#define CH(n) (n >= 0 && n <= 2 ? n : \
              (n >= 3 && n <= 5 ? (n + 5) : \
              (n >= 6 && n <= 8 ? (n + 10) : 0)))

#define TEST_LSI_ENABLE_WAVEFORM            0x01
#define TIMER1_DATA                         0x02
#define TIMER2_DATA                         0x03
#define TIMER_CONTROL_FLAGS                 0x04
#define SPEECH_MODE_KBD_SPLIT_NOTE          0x08
#define AM_DEPTH_VIBRATO_DEPTH_RHYTHM_CTRL  0xBD

// Base address for operator-channel set of registers
// e.g: ATTACK_RATE_DECAY_RATE__BASE + CH(4) + OP1
//
#define AM_VIB_EG_KSR_MULT__BASE            0x20
#define KEY_SCALING_OPERATOR_LEVELS__BASE   0x40
#define ATTACK_RATE_DECAY_RATE__BASE        0x60
#define SUSTAIN_LEVEL_RELEASE_RATE__BASE    0x80
#define WAVEFORM_SELECT__BASE               0xE0

// Register for Channels
#define FREQ_LOW(c)                         (0xA0 + c)
#define FREQ_HIGH_KEYON_OCTAVE(c)           (0xB0 + c)
#define FEEDBACK_STRENGTH_CONN_TYPE(c)      (0xC0 + c)


typedef struct {
    note_t note;
    double freq;
} note_freq_t;

static const note_freq_t FREQS[12] = {
    { C,  261.626 },
    { Db, 277.183 },
    { D,  293.665 },
    { Eb, 311.127 },
    { E,  329.628 },
    { F,  349.228 },
    { Gb, 369.994 },
    { G,  391.995 },
    { Ab, 415.305 },
    { A,  440.000 },
    { Bb, 466.164 },
    { B,  493.883 },
};

static double note_frequency(const note_t note);
static int calculate_fnumber(const unsigned int octave, const note_t note);


__inline void fm_write(const int reg, const int value) {
    // set requested register into address port
    outp(ADDR_PORT, reg);
    msleep(10);
    // write value into data port
    outp(DATA_PORT, value);
    msleep(60);
}

__inline void fm_write_b(const int reg, const int value) {
    outp(ADDR_PORT, reg);
    msleep(10);
    outp(DATA_PORT_B, value);
    msleep(60);
}

void fm_reset() {
    // reset sound by setting *all* registers to 0
    int i;
    for (i = 0; i < 0xf5; i++) {
        fm_write(i, 0);
    }
}

void fm_init()
{
    // enable OPL3
    fm_write_b(0x05, 1);
    // enable waveform select
    fm_write(TEST_LSI_ENABLE_WAVEFORM, 0x20);

    fm_write(AM_DEPTH_VIBRATO_DEPTH_RHYTHM_CTRL, 0x00);
}

void fm_set_instrument(const unsigned int c, const fm_instr_t* instr)
{
    fm_write(CH(c) + OP1 + AM_VIB_EG_KSR_MULT__BASE, instr->m__am_vib_eg);
    fm_write(CH(c) + OP1 + KEY_SCALING_OPERATOR_LEVELS__BASE, instr->m__ksl_volume);
    fm_write(CH(c) + OP1 + ATTACK_RATE_DECAY_RATE__BASE, instr->m__attack_decay);
    fm_write(CH(c) + OP1 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, instr->m__sustain_release);
    fm_write(CH(c) + OP1 + WAVEFORM_SELECT__BASE, instr->m__waveform);

    fm_write(CH(c) + OP2 + AM_VIB_EG_KSR_MULT__BASE, instr->c__am_vib_eg);
    fm_write(CH(c) + OP2 + KEY_SCALING_OPERATOR_LEVELS__BASE, instr->c__ksl_volume);
    fm_write(CH(c) + OP2 + ATTACK_RATE_DECAY_RATE__BASE, instr->c__attack_decay);
    fm_write(CH(c) + OP2 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, instr->c__sustain_release);
    fm_write(CH(c) + OP2 + WAVEFORM_SELECT__BASE, instr->c__waveform);

    fm_write(FEEDBACK_STRENGTH_CONN_TYPE(c), 0x30 | instr->feedback_fm);
}

void fm_key_on(const unsigned int c, const uint8_t octave, const note_t note)
{
    const int fnum = calculate_fnumber(octave, note);
    fm_write(FREQ_LOW(c), fnum & 0xff);
    fm_write(FREQ_HIGH_KEYON_OCTAVE(c), 0x20 | ((octave << 2) & 0x1c) | ((fnum >> 8) & 3));
}

void fm_key_off(const unsigned int c)
{
    fm_write(FREQ_HIGH_KEYON_OCTAVE(c), 0);
}

// fm_instr_t getters
__inline unsigned int fm_get_carrier_attack_rate(const fm_instr_t* instr) {
    return (instr->c__attack_decay >> 4) & 0xf;
}
__inline unsigned int fm_get_carrier_decay_rate(const fm_instr_t* instr) {
    return instr->c__attack_decay & 0xf;
}
__inline unsigned int fm_get_carrier_sustain_level(const fm_instr_t* instr) {
    return (instr->c__sustain_release >> 4) & 0xf;
}
__inline unsigned int fm_get_carrier_release_rate(const fm_instr_t* instr) {
    return instr->c__sustain_release & 0xf;
}
__inline unsigned int fm_get_modulator_attack_rate(const fm_instr_t* instr) {
    return (instr->m__attack_decay >> 4) & 0xf;
}
__inline unsigned int fm_get_modulator_decay_rate(const fm_instr_t* instr) {
    return instr->m__attack_decay & 0xf;
}
__inline unsigned int fm_get_modulator_sustain_level(const fm_instr_t* instr) {
    return (instr->m__sustain_release >> 4) & 0xf;
}
__inline unsigned int fm_get_modulator_release_rate(const fm_instr_t* instr) {
    return instr->m__sustain_release & 0xf;
}

// fm_instr_t setters
__inline void fm_set_carrier_attack_rate(fm_instr_t* instr, unsigned int value) {
    instr->c__attack_decay = ((value & 0xf) << 4) | (instr->c__attack_decay & 0xf);
}
__inline void fm_set_carrier_decay_rate(fm_instr_t* instr, unsigned int value) {
    instr->c__attack_decay = (instr->c__attack_decay & 0xf0) | (value & 0xf);
}
__inline void fm_set_carrier_sustain_level(fm_instr_t* instr, unsigned int value) {
    instr->c__sustain_release = ((value & 0xf) << 4) | (instr->c__sustain_release & 0xf);
}
__inline void fm_set_carrier_release_rate(fm_instr_t* instr, unsigned int value) {
    instr->c__sustain_release = (instr->c__sustain_release & 0xf0) | (value & 0xf);
}
__inline void fm_set_modulator_attack_rate(fm_instr_t* instr, unsigned int value) {
    instr->m__attack_decay = ((value & 0xf) << 4) | (instr->m__attack_decay & 0xf);
}
__inline void fm_set_modulator_decay_rate(fm_instr_t* instr, unsigned int value) {
    instr->m__attack_decay = (instr->m__attack_decay & 0xf0) | (value & 0xf);
}
__inline void fm_set_modulator_sustain_level(fm_instr_t* instr, unsigned int value) {
    instr->m__sustain_release = ((value & 0xf) << 4) | (instr->m__sustain_release & 0xf);
}
__inline void fm_set_modulator_release_rate(fm_instr_t* instr, unsigned int value) {
    instr->m__sustain_release = (instr->m__sustain_release & 0xf0) | (value & 0xf);
}

static double note_frequency(const note_t note)
{
    int i;
    for (i = 0; i < 12; i++) {
        if (FREQS[i].note == note) break;
    }
    return FREQS[i].freq;
}

static int calculate_fnumber(const unsigned int octave, const note_t note)
{
    // F-Number = Music Frequency * 2^(20-Block) / 49716 Hz
    const double n = note_frequency(note) * pow(2, 20 - octave);
    const int fnum = floor(n / 49716);
    return fnum;
}
