#include "sound.h"

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
    sound_write(0x20, 0x01);  // Set the modulator's multiple to 1
    sound_write(0x40, 0x10);  // Set the modulator's level to about 40 dB
    sound_write(0x60, 0xf0);  // Modulator attack: quick; decay: long
    sound_write(0x80, 0x77);  // Modulator sustain: medium; release: medium
    sound_write(0xa0, 0x98);  // Set voice frequency's LSB (it'll be a D#)
    sound_write(0x23, 0x01);  // Set the carrier's multiple to 1
    sound_write(0x43, 0x00);  // Set the carrier to maximum volume (about 47 dB)
    sound_write(0x63, 0xf0);  // Carrier attack: quick; decay: long
    sound_write(0x83, 0x77);  // Carrier sustain: medium; release: medium

    sound_write(0xb0, 0x31);  // Turn the voice on; set the octave and freq MSB
    msleep(1000);
    sound_write(0xb0, 0x11);  // Turn the voice off
}
