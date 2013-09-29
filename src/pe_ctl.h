#ifndef __PE_CTL_H__
#define __PE_CTL_H__

#include "seq.h"

typedef struct {
    seq_t* const seq;
} pe_ctl_t;

pe_ctl_t pe_ctl_new(seq_t* seq);
void pe_ctl_handle_keyboard(pe_ctl_t* this, const int key);

#endif // __PE_CTL_H__
