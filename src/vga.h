#ifndef __VGA_H__
#define __VGA_H__

#include <stdint.h>
#include "common.h"

#define VIDEO_MODE 0x13
#define TEXT_MODE  0x03

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE   (SCREEN_WIDTH * SCREEN_HEIGHT)
#define NUM_COLORS    256

void vga_init();
void vga_set_mode(uint8_t mode);

void vga_update();

void vga_clear();

void vga_putp(int x, int y, uint8_t color);
void vga_line(int x1, int y1, int x2, int y2, uint8_t color);

void vga_rect(int left, int top, int right, int bottom, uint8_t color);
void vga_rect_fill(int left, int top, int right, int bottom, uint8_t color);

extern __inline__ void vga_square(int left, int top, int size, uint8_t color) {
    vga_rect(left, top, left + size, top + size, color);
}
extern __inline__ void vga_square_fill(int left, int top, int size, uint8_t color) {
    vga_rect_fill(left, top, left + size, top + size, color);
}

#endif // __VGA_H__
