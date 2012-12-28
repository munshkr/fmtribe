#include "sound.h"

#define ADDR_PORT 0x388
#define DATA_PORT 0x389

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

// Base address for operator-channel set of registers
// e.g: ATTACK_RATE_DECAY_RATE__BASE + CH(4) + OP1
//
#define AM_VIB_EG_KSR_MULT__BASE            0x20
#define KEY_SCALING_OPERATOR_LEVELS__BASE   0x40
#define ATTACK_RATE_DECAY_RATE__BASE        0x60
#define SUSTAIN_LEVEL_RELEASE_RATE__BASE    0x80
#define WAVE_SELECT__BASE                   0xE0

#define FREQ_LOW(c)                         (0xA0 + c - 1)
#define FREQ_HIGH_KEYON_OCTAVE(c)           (0xB0 + c - 1)
#define FEEDBACK_STRENGTH_CONN_TYPE(c)      (0xC0 + c - 1)

#define AM_DEPTH_VIBRATO_DEPTH_RHYTHM_CTRL  0xBD


__inline void sound_write(const int reg, const int value) {
    // set requested register into address port
    outp(ADDR_PORT, reg);
    msleep(10);
    // write value into data port
    outp(DATA_PORT, value);
    msleep(60);
}

void sound_reset() {
    // reset sound by setting *all* registers to 0

    int i, j;

    sound_write(1, 0);
    sound_write(2, 0);
    sound_write(3, 0);
    sound_write(4, 0);
    sound_write(8, 0);

    for (i = 0; i < 4; ++i) {
        for (j = 0; j <= 0x15; ++j) {
            sound_write(0x20 + (0x20 * i) + j, 0);
        }
    }

    for (i = 0; i < 3; ++i) {
        for (j = 0; j <= 8; ++j) {
            sound_write(0xa0 + (0x10 * i) + j, 0);
        }
    }

    sound_write(0xbd, 0);

    for (i = 0; i <= 0x15; ++i) {
        sound_write(0xe0 + i, 0);
    }
}

void sound_play_metronome_tick() {
    sound_write(CH(1) + OP1 + AM_VIB_EG_KSR_MULT__BASE, 0x01);  // Set the modulator's multiple to 1
    sound_write(CH(1) + OP1 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x10);  // Set the modulator's level to about 40 dB
    sound_write(CH(1) + OP1 + ATTACK_RATE_DECAY_RATE__BASE, 0xf0);  // Modulator attack: quick; decay: long
    sound_write(CH(1) + OP1 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0x77);  // Modulator sustain: medium; release: medium

    sound_write(CH(1) + OP2 + AM_VIB_EG_KSR_MULT__BASE, 0x01);  // Set the carrier's multiple to 1
    sound_write(CH(1) + OP2 + KEY_SCALING_OPERATOR_LEVELS__BASE, 0x00);  // Set the carrier to maximum volume (about 47 dB)
    sound_write(CH(1) + OP2 + ATTACK_RATE_DECAY_RATE__BASE, 0xf0);  // Carrier attack: quick; decay: long
    sound_write(CH(1) + OP2 + SUSTAIN_LEVEL_RELEASE_RATE__BASE, 0x77);  // Carrier sustain: medium; release: medium

    sound_write(FREQ_LOW(1), 0x98);  // Set voice frequency's LSB (it'll be a D#)

    sound_write(FREQ_HIGH_KEYON_OCTAVE(1), 0x31);  // Turn the voice on; set the octave and freq MSB
    msleep(1000);

    sound_write(FREQ_HIGH_KEYON_OCTAVE(1), 0x11);  // Turn the voice off
}
