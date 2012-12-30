#include "sound.h"

#define BASE_PORT   0x388
#define ADDR_PORT   BASE_PORT
#define DATA_PORT   (BASE_PORT + 1)
#define DATA_PORT_B (BASE_PORT + 3)

#define OP1 0x00
#define OP2 0x03

#define CH(n) (n >= 1 && n <= 3 ? (n - 1) : \
              (n >= 4 && n <= 6 ? (n + 4) : \
              (n >= 7 && n <= 9 ? (n + 9) : 0)))

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
#define FREQ_LOW(c)                         (0xA0 + c - 1)
#define FREQ_HIGH_KEYON_OCTAVE(c)           (0xB0 + c - 1)
#define FEEDBACK_STRENGTH_CONN_TYPE(c)      (0xC0 + c - 1)



__inline void setr(const int reg, const int value) {
    // set requested register into address port
    outp(ADDR_PORT, reg);
    msleep(10);
    // write value into data port
    outp(DATA_PORT, value);
    msleep(60);
}

__inline void setr_b(const int reg, const int value) {
    outp(ADDR_PORT, reg);
    msleep(10);
    outp(DATA_PORT_B, value);
    msleep(60);
}

void sound_reset() {
    // reset sound by setting *all* registers to 0

    int i, j;

    setr(1, 0);
    setr(2, 0);
    setr(3, 0);
    setr(4, 0);
    setr(8, 0);

    for (i = 0; i < 4; ++i) {
        for (j = 0; j <= 0x15; ++j) {
            setr(0x20 + (0x20 * i) + j, 0);
        }
    }

    for (i = 0; i < 3; ++i) {
        for (j = 0; j <= 8; ++j) {
            setr(0xa0 + (0x10 * i) + j, 0);
        }
    }

    setr(0xbd, 0);

    for (i = 0; i <= 0x15; ++i) {
        setr(0xe0 + i, 0);
    }

    // enable OPL3
    setr_b(0x05, 1);
}

void sound_init()
{
    setr_b(TEST_LSI_ENABLE_WAVEFORM, 0x20);
    setr_b(AM_DEPTH_VIBRATO_DEPTH_RHYTHM_CTRL, 0x00);
}

void sound_play_metronome_tick(unsigned int c, unsigned int octave, unsigned short fnum)
{
    setr(CH(c) + OP1 + AM_VIB_EG_KSR_MULT__BASE, 0x01);  // Set the modulator's multiple to 1
    setr(CH(c) + OP1 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x10);  // Set the modulator's level to about 40 dB
    setr(CH(c) + OP1 + ATTACK_RATE_DECAY_RATE__BASE, 0xf0);  // Modulator attack: quick; decay: long
    setr(CH(c) + OP1 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0x77);  // Modulator sustain: medium; release: medium

    setr(CH(c) + OP2 + AM_VIB_EG_KSR_MULT__BASE, 0x01);  // Set the carrier's multiple to 1
    setr(CH(c) + OP2 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x00);  // Set the carrier to maximum volume (about 47 dB)
    setr(CH(c) + OP2 + ATTACK_RATE_DECAY_RATE__BASE, 0xf0);  // Carrier attack: quick; decay: long
    setr(CH(c) + OP2 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0x77);  // Carrier sustain: medium; release: medium

    setr(FREQ_LOW(c), fnum & 0xff);  // Set voice frequency's LSB
    setr(FREQ_HIGH_KEYON_OCTAVE(c), 0x20 | (octave << 2) | ((fnum >> 8) & 3));  // Turn the voice on; set the octave and freq MSB

    msleep(2000);
    setr(FREQ_HIGH_KEYON_OCTAVE(c), (octave << 2) | ((fnum >> 8) & 3));  // Turn the voice off
}

void sound_play_bass1(unsigned int c, unsigned int octave, unsigned short fnum)
{
    setr(FREQ_HIGH_KEYON_OCTAVE(c), (octave << 2) | ((fnum >> 8) & 3));  // Turn the voice off

    setr(CH(c) + OP1 + AM_VIB_EG_KSR_MULT__BASE, 0x00);
    setr(CH(c) + OP1 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x00);
    setr(CH(c) + OP1 + ATTACK_RATE_DECAY_RATE__BASE, 0xa4);
    setr(CH(c) + OP1 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0xa1);
    setr(CH(c) + OP1 + WAVEFORM_SELECT__BASE, 0x00);

    setr(CH(c) + OP2 + AM_VIB_EG_KSR_MULT__BASE, 0x40);
    setr(CH(c) + OP2 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x03);
    setr(CH(c) + OP2 + ATTACK_RATE_DECAY_RATE__BASE, 0xa4);
    setr(CH(c) + OP2 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0x06);
    setr(CH(c) + OP2 + WAVEFORM_SELECT__BASE, 0x05);

    setr(FEEDBACK_STRENGTH_CONN_TYPE(c), 0x30);
    setr(FREQ_LOW(c), fnum & 0xff);
    setr(FREQ_HIGH_KEYON_OCTAVE(c), 0x20 | (octave << 2) | ((fnum >> 8) & 3));  // Turn the voice on

    //msleep(1000);
    //setr(FREQ_HIGH_KEYON_OCTAVE(c), 0x10 | ((fnum >> 8) & 3));  // Turn the voice off
}

void sound_play_kick1(unsigned int c, unsigned int octave, unsigned short fnum)
{
    setr(FREQ_HIGH_KEYON_OCTAVE(c), (octave << 2) | ((fnum >> 8) & 3));  // Turn the voice off

    setr(CH(c) + OP1 + AM_VIB_EG_KSR_MULT__BASE, 0x00);
    setr(CH(c) + OP1 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x00);
    setr(CH(c) + OP1 + ATTACK_RATE_DECAY_RATE__BASE, 0xd6);
    setr(CH(c) + OP1 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0x4f);
    setr(CH(c) + OP1 + WAVEFORM_SELECT__BASE, 0x00);

    setr(CH(c) + OP2 + AM_VIB_EG_KSR_MULT__BASE, 0x00);
    setr(CH(c) + OP2 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x0b);
    setr(CH(c) + OP2 + ATTACK_RATE_DECAY_RATE__BASE, 0xa8);
    setr(CH(c) + OP2 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0x4c);
    setr(CH(c) + OP2 + WAVEFORM_SELECT__BASE, 0x00);

    //setr(FEEDBACK_STRENGTH_CONN_TYPE(c),
    setr(FREQ_LOW(c), fnum & 0xff);  // Set voice frequency's LSB
    setr(FREQ_HIGH_KEYON_OCTAVE(c), 0x20 | (octave << 2) | ((fnum >> 8) & 3));  // Turn the voice on; set the octave and freq MSB

    //msleep(1000);
    //setr(FREQ_HIGH_KEYON_OCTAVE(c), 0x10 | ((fnum >> 8) & 3));  // Turn the voice off
}
