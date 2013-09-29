#ifndef __INSTR_H__
#define __INSTR_H__

#include "fm.h"

typedef struct {
    note_t note;
    unsigned int octave;
    fm_instr_t fm_instr;
} instr_t;

#endif // __INSTR_H__
