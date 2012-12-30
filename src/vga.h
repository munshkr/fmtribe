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

void init_vga();
void set_mode(uint8_t mode);

void update();

void clear();

void line(int x1, int y1, int x2, int y2, uint8_t color);

void rect(int left, int top, int right, int bottom, uint8_t color);
void rect_fill(int left, int top, int right, int bottom, uint8_t color);

__inline void square(int left, int top, int size, uint8_t color);
__inline void square_fill(int left, int top, int size, uint8_t color);
__inline void putp(int x, int y, uint8_t color);

#endif // __VGA_H__
