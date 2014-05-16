#ifndef __PBM_H__
#define __PBM_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct pbm_t {
    unsigned int width;
    unsigned int height;
    uint8_t* buffer;
} pbm_t;

bool pbm_read(const char* path, pbm_t* pbm);
void pbm_free(pbm_t* pbm);

#endif // __PBM_H__
