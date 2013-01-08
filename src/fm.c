#include <unistd.h>
#include <math.h>
#include <dos.h>

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
    uint16_t fnum;
} note_fnum_t;

static const note_fnum_t FNUMS[12] = {
    { Db, 0x16b },
    { D,  0x181 },
    { Eb, 0x198 },
    { E,  0x1b0 },
    { F,  0x1ca },
    { Gb, 0x1e5 },
    { G,  0x202 },
    { Ab, 0x220 },
    { A,  0x241 },
    { Bb, 0x263 },
    { B,  0x287 },
    { C,  0x2ae },
};

static __inline__ void fm_write(const int reg, const int value);
static __inline__ void fm_write_b(const int reg, const int value);
static __inline__ uint16_t note_fnumber(const note_t note);


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
    const uint16_t fnum = note_fnumber(note);
    fm_write(FREQ_LOW(c), fnum & 0xff);
    fm_write(FREQ_HIGH_KEYON_OCTAVE(c), 0x20 | ((octave << 2) & 0x1c) | ((fnum >> 8) & 3));
}

void fm_key_off(const unsigned int c)
{
    fm_write(FREQ_HIGH_KEYON_OCTAVE(c), 0);
}

// fm_instr_t getters
unsigned int fm_get_carrier_attack_rate(const fm_instr_t* instr) {
    return (instr->c__attack_decay >> 4) & 0xf;
}
unsigned int fm_get_carrier_decay_rate(const fm_instr_t* instr) {
    return instr->c__attack_decay & 0xf;
}
unsigned int fm_get_carrier_sustain_level(const fm_instr_t* instr) {
    return (instr->c__sustain_release >> 4) & 0xf;
}
unsigned int fm_get_carrier_release_rate(const fm_instr_t* instr) {
    return instr->c__sustain_release & 0xf;
}
fm_waveform_type_t fm_get_carrier_waveform_type(const fm_instr_t* instr) {
    return instr->c__waveform & 0x7;
}
unsigned int fm_get_modulator_attack_rate(const fm_instr_t* instr) {
    return (instr->m__attack_decay >> 4) & 0xf;
}
unsigned int fm_get_modulator_decay_rate(const fm_instr_t* instr) {
    return instr->m__attack_decay & 0xf;
}
unsigned int fm_get_modulator_sustain_level(const fm_instr_t* instr) {
    return (instr->m__sustain_release >> 4) & 0xf;
}
unsigned int fm_get_modulator_release_rate(const fm_instr_t* instr) {
    return instr->m__sustain_release & 0xf;
}
fm_waveform_type_t fm_get_modulator_waveform_type(const fm_instr_t* instr) {
    return instr->m__waveform & 0x7;
}

// fm_instr_t setters
void fm_set_carrier_attack_rate(fm_instr_t* instr, unsigned int value) {
    instr->c__attack_decay = ((value & 0xf) << 4) | (instr->c__attack_decay & 0xf);
}
void fm_set_carrier_decay_rate(fm_instr_t* instr, unsigned int value) {
    instr->c__attack_decay = (instr->c__attack_decay & 0xf0) | (value & 0xf);
}
void fm_set_carrier_sustain_level(fm_instr_t* instr, unsigned int value) {
    instr->c__sustain_release = ((value & 0xf) << 4) | (instr->c__sustain_release & 0xf);
}
void fm_set_carrier_release_rate(fm_instr_t* instr, unsigned int value) {
    instr->c__sustain_release = (instr->c__sustain_release & 0xf0) | (value & 0xf);
}
void fm_set_carrier_waveform_type(fm_instr_t* instr, fm_waveform_type_t value) {
    instr->c__waveform = value & 0x7;
}
void fm_set_modulator_attack_rate(fm_instr_t* instr, unsigned int value) {
    instr->m__attack_decay = ((value & 0xf) << 4) | (instr->m__attack_decay & 0xf);
}
void fm_set_modulator_decay_rate(fm_instr_t* instr, unsigned int value) {
    instr->m__attack_decay = (instr->m__attack_decay & 0xf0) | (value & 0xf);
}
void fm_set_modulator_sustain_level(fm_instr_t* instr, unsigned int value) {
    instr->m__sustain_release = ((value & 0xf) << 4) | (instr->m__sustain_release & 0xf);
}
void fm_set_modulator_release_rate(fm_instr_t* instr, unsigned int value) {
    instr->m__sustain_release = (instr->m__sustain_release & 0xf0) | (value & 0xf);
}
void fm_set_modulator_waveform_type(fm_instr_t* instr, fm_waveform_type_t value) {
    instr->m__waveform = value & 0x7;
}

static __inline__ void fm_write(const int reg, const int value) {
    // set requested register into address port
    outportb(ADDR_PORT, reg);
    usleep(10);
    // write value into data port
    outportb(DATA_PORT, value);
    usleep(60);
}

static __inline__ void fm_write_b(const int reg, const int value) {
    outportb(ADDR_PORT, reg);
    usleep(10);
    outportb(DATA_PORT_B, value);
    usleep(60);
}

static __inline__ uint16_t note_fnumber(const note_t note)
{
    int i;
    for (i = 0; i < 12; i++) {
        if (FNUMS[i].note == note) break;
    }
    return FNUMS[i].fnum;
}
