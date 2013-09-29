#ifndef __PE_VIEW_H__
#define __PE_VIEW_H__

#include "seq.h"
#include "font.h"

typedef struct {
    const seq_t  * const seq;
    const font_t * const font;
} pe_view_t;

pe_view_t pe_view_new(const seq_t* seq, const font_t* font);
void pe_view_render(const pe_view_t* this);

#endif // __PE_VIEW_H__
