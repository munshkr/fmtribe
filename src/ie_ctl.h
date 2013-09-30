#ifndef __IE_CTL_H__
#define __IE_CTL_H__

#include "seq.h"
#include "ie_vw.h"

typedef struct {
    seq_t     * const seq;
    ie_vw_t * const ie_vw;
} ie_ctl_t;

ie_ctl_t ie_ctl_new(seq_t* seq, ie_vw_t* ie_vw);
void ie_ctl_handle_keyboard(ie_ctl_t* this, const int key);

#endif // __IE_CTL_H__
