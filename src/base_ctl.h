#ifndef __BASE_CTL_H__
#define __BASE_CTL_H__

#include "seq.h"

typedef struct {
    seq_t* const seq;
} base_ctl_t;

base_ctl_t base_ctl_new(seq_t* seq);
void base_ctl_handle_keyboard(base_ctl_t* this, const int key);

#endif // __BASE_CTL_H__
