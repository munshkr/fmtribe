#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/nearptr.h>
#include <dos.h>

#include "vga.h"

#define VIDEO_INT       0x10
#define INPUT_STATUS_1  0x03da
#define VRETRACE        0x08

uint8_t *VGA = NULL;
uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];


void vga_init()
{
    if (__djgpp_nearptr_enable() == 0) {
        printf("Could get access to first 640K of memory.\n");
        exit(-1);
    }
    // FIXME if a malloc is called, this should be recalculated!
    VGA = (uint8_t*) 0xA0000 + __djgpp_conventional_base;
}

void vga_set_mode(uint8_t mode)
{
    union REGS regs;
    regs.h.ah = 0;
    regs.h.al = mode;
    int86(VIDEO_INT, &regs, &regs);
}

void vga_update()
{
    while ((inp(INPUT_STATUS_1) & VRETRACE));
    while (!(inp(INPUT_STATUS_1) & VRETRACE));
    memcpy(VGA, buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void vga_clear()
{
    memset(buffer, 0, SCREEN_SIZE);
}

void vga_putp(int x, int y, uint8_t color)
{
    buffer[(y << 8) + (y << 6) + x] = color;
}

void vga_line(int x1, int y1, int x2, int y2, uint8_t color)
{
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1;
    dy = y2 - y1;
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = Sgn(dx);
    sdy = Sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    vga_putp(px, py, color);

    // if the line is more horizontal than vertical
    if (dxabs >= dyabs) {
        for(i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            vga_putp(px, py, color);
        }
    } else {
        for(i = 0; i < dyabs; i++) {
            x += dxabs;
            if (x >= dyabs) {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            vga_putp(px, py, color);
        }
    }
}

void vga_rect(int left, int top, int right, int bottom, uint8_t color)
{
    uint16_t top_offset, bottom_offset, temp;

    if (top > bottom) {
        temp = top;
        top = bottom;
        bottom = temp;
    }
    if (left > right) {
        temp = left;
        left = right;
        right = temp;
    }

    top_offset = (top << 8) + (top << 6);
    bottom_offset = (bottom << 8) + (bottom << 6);

    for (uint16_t i = left; i <= right; i++) {
        buffer[top_offset + i] = color;
        buffer[bottom_offset + i] = color;
    }

    for (uint16_t i = top_offset; i <= bottom_offset; i += SCREEN_WIDTH) {
        buffer[left + i] = color;
        buffer[right + i] = color;
    }
}

void vga_rect_fill(int left, int top, int right, int bottom, uint8_t color)
{
    uint16_t top_offset, bottom_offset, temp, width;

    if (top > bottom) {
        temp = top;
        top = bottom;
        bottom = temp;
    }
    if (left > right) {
        temp = left;
        left = right;
        right = temp;
    }

    top_offset = (top << 8) + (top << 6) + left;
    bottom_offset = (bottom << 8) + (bottom << 6) + left;
    width = right - left + 1;

    for (uint16_t i = top_offset; i <= bottom_offset; i += SCREEN_WIDTH) {
        memset(&buffer[i], color, width);
    }
}
