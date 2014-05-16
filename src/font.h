#ifndef __FONT_H__
#define __FONT_H__

#include "pbm.h"
#include "vga.h"

typedef struct font_t {
    unsigned int width;
    unsigned int height;
    uint8_t* buffer;
} font_t;

bool font_create_from_pbm(const pbm_t* pbm, const int chars_per_row, font_t* font);
void font_render_chr(const font_t* f, const int x, const int y, const uint8_t color, const char c);
void font_render_str(const font_t* f, const int x, const int y, const uint8_t color, const char* str);
void font_render_strf(const font_t* f, const int x, const int y, const uint8_t color, const char* format, ...);
void font_free(font_t* f);

#endif // __FONT_H__
