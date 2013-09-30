#ifndef __PE_VW_H__
#define __PE_VW_H__

#include "seq.h"
#include "font.h"

typedef struct {
    const seq_t  * const seq;
    const font_t * const font;

    bool dirty;
} pe_vw_t;

pe_vw_t pe_vw_new(const seq_t* seq, const font_t* font);
void pe_vw_render(pe_vw_t* this);

#endif // __PE_VW_H__
