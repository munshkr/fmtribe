#ifndef __PE_CTL_H__
#define __PE_CTL_H__

#include "seq.h"
#include "pe_vw.h"

typedef struct {
    seq_t     * const seq;
    pe_vw_t * const pe_vw;
} pe_ctl_t;

pe_ctl_t pe_ctl_new(seq_t* seq, pe_vw_t* pe_vw);
void pe_ctl_handle_keyboard(pe_ctl_t* this, const int key);

#endif // __PE_CTL_H__
