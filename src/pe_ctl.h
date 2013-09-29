#ifndef __PE_CTL_H__
#define __PE_CTL_H__

#include "seq.h"
#include "pe_view.h"

typedef struct {
    seq_t     * const seq;
    pe_view_t * const pe_view;
} pe_ctl_t;

pe_ctl_t pe_ctl_new(seq_t* seq, pe_view_t* pe_view);
void pe_ctl_handle_keyboard(pe_ctl_t* this, const int key);

#endif // __PE_CTL_H__
