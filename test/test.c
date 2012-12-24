/*
 * FM hello world
 * based on Programming the AdLib/Sound Blaster FM Music Chips
 * http://www.shipbrook.net/jeff/sb.html
 *
 * Tested with djgpp and DOSBox
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ADDR_PORT 0x388
#define DATA_PORT 0x389

typedef char* string;


void outb(const int port, const char data) {
    __asm __volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

int inb(const int port) {
    char data;
    __asm __volatile("inb %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

void msleep(const int microsecs) {
   const int wait = 1000000 / microsecs;
   const uclock_t start = uclock();
   while (uclock() < start + UCLOCKS_PER_SEC / wait);
}

void write(const int reg, const int value) {
    printf("write %x to %x\n", value, reg);
    outb(ADDR_PORT, reg);
    msleep(4); 
    outb(DATA_PORT, value);
    msleep(24); 
}

void reset_sound() {
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

void make_a_sound() {
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

    sleep(1);

    write(0xb0, 0x11);
}


int main(int argc, char** argv) {
    printf("start\n"); 
    reset_sound();

    make_a_sound();

    reset_sound();
    printf("end\n");

    return EXIT_SUCCESS;
}
