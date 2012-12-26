#include "sound.h"

void write(const int reg, const int value) {
    //printf("write %x to %x\n", value, reg);

    // Set requeste register into address port
    outb(ADDR_PORT, reg);
    msleep(10);

    // Write value into data port
    outb(DATA_PORT, value);
    msleep(60);
}

void reset_sound() {
    // Reset sound by setting *all* registers to 0

    int i, j;

    write(1, 0);
    write(2, 0);
    write(3, 0);
    write(4, 0);
    write(8, 0);

    for (i = 0; i < 4; ++i) {
        for (j = 0; j <= 0x15; ++j) {
            write(0x20 + (0x20 * i) + j, 0);
        }
    }

    for (i = 0; i < 3; ++i) {
        for (j = 0; j <= 8; ++j) {
            write(0xa0 + (0x10 * i) + j, 0);
        }
    }

    write(0xbd, 0);

    for (i = 0; i <= 0x15; ++i) {
        write(0xe0 + i, 0);
    }
}

void play_metronome() {
    write(0x20, 0x01);  // Set the modulator's multiple to 1
    write(0x40, 0x10);  // Set the modulator's level to about 40 dB
    write(0x60, 0xf0);  // Modulator attack: quick; decay: long
    write(0x80, 0x77);  // Modulator sustain: medium; release: medium
    write(0xa0, 0x98);  // Set voice frequency's LSB (it'll be a D#)
    write(0x23, 0x01);  // Set the carrier's multiple to 1
    write(0x43, 0x00);  // Set the carrier to maximum volume (about 47 dB)
    write(0x63, 0xf0);  // Carrier attack: quick; decay: long
    write(0x83, 0x77);  // Carrier sustain: medium; release: medium

    write(0xb0, 0x31);  // Turn the voice on; set the octave and freq MSB
    msleep(1000);
    write(0xb0, 0x11);  // Turn the voice off
}
