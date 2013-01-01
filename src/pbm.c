#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pbm.h"

#define DIV_CEIL(x,y) ((x + y - 1) / y)

bool read_pbm_file(const char* path, pbm_file_t* pbm)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    // check if file is a valid raw PBM
    char id[3] = "";
    fscanf(f, "%s\n", id);
    if (strcmp(id, "P4")) {
        fclose(f);
        fprintf(stderr, "%s: not a valid raw PBM file.\n", path);
        return false;
    }

    // get width and height
    fscanf(f, "%i %i\n", &(pbm->width), &(pbm->height));

    // allocate enough memory for reading raw content
    unsigned int raw_buf_size = DIV_CEIL(pbm->width * pbm->height, 8);
    uint8_t* raw_buf = malloc(raw_buf_size);
    if (!raw_buf) {
        fclose(f);
        fprintf(stderr, "%s: not enough memory to read PBM file.\n", path);
        return false;
    }

    // read raw content from file
    fread(raw_buf, sizeof(uint8_t), raw_buf_size, f);

    // allocate memory for output buffer
    pbm->buffer = malloc(pbm->width * pbm->height);
    if (!pbm->buffer) {
        free(raw_buf);
        fclose(f);
        fprintf(stderr, "%s: not enough memory to read PBM file.\n", path);
        return false;
    }

    // read raw buffer into output buffer
    unsigned int i, j, k, x;
    for (i = 0, j = 0, x = 0; i < raw_buf_size; i++) {
        uint8_t byte = raw_buf[i];
        for (k = 0; k < 8; k++, j++, x++) {
            if (x == pbm->width) {
                x = 0;
                break;
            }
            if (byte & 0x80) {
                pbm->buffer[j] = 1;
            } else {
                pbm->buffer[j] = 0;
            }
            byte = (byte << 1) & 0xff;
        }
    }

    free(raw_buf);
    fclose(f);

    return true;
}
