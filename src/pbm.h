#ifndef __PBM_H__
#define __PBM_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct pbm_file_t {
    unsigned int width;
    unsigned int height;
    uint8_t* buffer;
} pbm_file_t;

bool read_pbm_file(const char* path, pbm_file_t* pbm);

#endif // __PBM_H__
