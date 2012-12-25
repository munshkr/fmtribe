#include "vga.h"

byte *VGA = NULL;

void init_vga()
{
    if (__djgpp_nearptr_enable() == 0) {
        printf("Could get access to first 640K of memory.\n");
        exit(-1);
    }
    VGA = (byte*) 0xA0000 + __djgpp_conventional_base;
}

void set_mode(byte mode)
{
    union REGS regs;
    regs.h.ah = 0;
    regs.h.al = mode;
    int86(VIDEO_INT, &regs, &regs);
}

__inline void putp(int x, int y, byte color)
{
    VGA[(y << 8) + (y << 6) + x] = color;
}

void line(int x1, int y1, int x2, int y2, byte color)
{
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1;
    dy = y2 - y1;
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    putp(px, py, color);

    // if the line is more horizontal than vertical
    if (dxabs >= dyabs) {
        for(i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            putp(px, py, color);
        }
    } else {
        for(i = 0; i < dyabs; i++) {
            x += dxabs;
            if (x >= dyabs) {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            putp(px, py, color);
        }
    }
}

void rect(int left, int top, int right, int bottom, byte color)
{
    word top_offset, bottom_offset, i, temp;

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

    for (i = left; i <= right; i++) {
        VGA[top_offset + i] = color;
        VGA[bottom_offset + i] = color;
    }

    for (i = top_offset; i <= bottom_offset; i += SCREEN_WIDTH) {
        VGA[left + i] = color;
        VGA[right + i] = color;
    }
}

void rect_fill(int left, int top, int right, int bottom, byte color)
{
    word top_offset, bottom_offset, i, temp, width;

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

    for (i = top_offset; i <= bottom_offset; i += SCREEN_WIDTH) {
        memset(&VGA[i], color, width);
    }
}

