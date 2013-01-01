#include <string.h>
#include "font.h"

#define TOTAL_CHARS   96
#define DIV_CEIL(x,y) ((x + y - 1) / y)

bool create_font_from_pbm(const pbm_file_t* pbm, const int chars_per_row, font_t* font)
{
    unsigned int cols = chars_per_row;
    unsigned int rows = DIV_CEIL(TOTAL_CHARS, chars_per_row);

    font->width  = pbm->width / cols;
    font->height = pbm->height / rows;

    unsigned int buf_size = pbm->width * pbm->height;
    font->buffer = malloc(buf_size);
    if (!font->buffer) {
        fprintf(stderr, "Not enough memory to create font.\n");
        return false;
    }

    if (cols == 1) {
        memcpy(font->buffer, pbm->buffer, buf_size);
        return true;
    }

    uint8_t* buf_p = font->buffer;
    uint8_t* pbm_buf_p = pbm->buffer;

    int r, c, i, j;
    for (r = 0; r < rows; r++) {
        uint8_t* old = pbm_buf_p;
        for (c = 0; c < cols; c++) {
            uint8_t* old = pbm_buf_p;
            for (i = 0; i < font->height; i++) {
                for (j = 0; j < font->width; j++) {
                    *(buf_p++) = *(pbm_buf_p++);
                }
                pbm_buf_p += font->width * (cols - 1);
            }
            pbm_buf_p = old + font->width;
        }
        pbm_buf_p = old + (pbm->width * font->height);
    }

    return true;
}

void render_chr(const font_t* f, const int x, const int y, const uint8_t color, const char c)
{
    int char_pos = ((int) c) - 32;

    // if char is out of range, print a whitespace char
    if (char_pos < 0 || char_pos >= TOTAL_CHARS) {
        char_pos = 0;
    }

    uint8_t* buf_ptr = f->buffer + (char_pos * f->width * f->height);

    int i, j;
    for (j = 0; j < f->height; j++) {
        for (i = 0; i < f->width; i++) {
            if (*buf_ptr) {
                putp(x + i, y + j, color);
            }
            buf_ptr++;
        }
    }
}

void render_str(const font_t* f, const int x, const int y, const uint8_t color, const char* str)
{
    char* str_p = (char*) str;
    int curx = x;
    while (*str_p) {
        render_chr(f, curx, y, color, *str_p);
        curx += f->width;
        str_p++;
    }
}

void free_font(font_t* f)
{
    if (f->buffer) {
        free(f->buffer);
        f->buffer = NULL;
    }
}
