#ifndef __IE_VW_H__
#define __IE_VW_H__

#include "seq.h"
#include "font.h"

typedef struct {
    seq_t  * const seq;
    const font_t * const font;

    int current_instr_field;
    bool dirty;
} ie_vw_t;

typedef enum { Up, Down } direction_t;
typedef enum { Increase, Decrease } action_t;


ie_vw_t ie_vw_new(seq_t* seq, const font_t* font);
void ie_vw_render(ie_vw_t* this);
void ie_vw_change(ie_vw_t* this, const action_t action);
void ie_vw_move(ie_vw_t* this, const direction_t dir);

#endif // __IE_VW_H__
