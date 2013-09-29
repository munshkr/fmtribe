#ifndef __SEQ_UI_H__
#define __SEQ_UI_H__

#include "seq.h"

typedef struct {
    seq_t* const seq;
} seq_ui_t;

seq_ui_t seq_ui_new(seq_t* seq);
void seq_ui_handle_keyboard(seq_ui_t* this, const int key);

#endif // __SEQ_UI_H__
