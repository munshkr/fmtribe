#include "sound.h"

void write(const int reg, const int value) {
    //printf("write %x to %x\n", value, reg);

    // Set requeste register into address port
    outb(ADDR_PORT, reg);
    msleep(4);

    // Write value into data port
    outb(DATA_PORT, value);
    msleep(24);
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

    for (i = 0; i <= 5; ++i) {
        write(0xe0 + i, 0);
    }
}
