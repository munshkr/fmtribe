#include "ie_vw.h"
#include "vga.h"
#include "font.h"
#include "fm.h"

#define C_COL_LEFT  10
#define M_COL_LEFT  150
#define C_COL_TOP   10
#define M_COL_TOP   C_COL_TOP

const unsigned int instr_fields_pos[12][2] = {
    { C_COL_LEFT + 120, C_COL_TOP + 20 },
    { C_COL_LEFT + 120, C_COL_TOP + 35 },
    { C_COL_LEFT + 120, C_COL_TOP + 50 },
    { C_COL_LEFT + 120, C_COL_TOP + 65 },
    { C_COL_LEFT + 120, C_COL_TOP + 80 },
    { C_COL_LEFT + 120, C_COL_TOP + 95 },
    { M_COL_LEFT + 120, M_COL_TOP + 20 },
    { M_COL_LEFT + 120, M_COL_TOP + 35 },
    { M_COL_LEFT + 120, M_COL_TOP + 50 },
    { M_COL_LEFT + 120, M_COL_TOP + 65 },
    { M_COL_LEFT + 120, M_COL_TOP + 80 },
    { M_COL_LEFT + 120, M_COL_TOP + 95 },
};

static unsigned int get_value_for_instrument(const instr_t* ins, const unsigned int field);
static void set_value_for_instrument(ie_vw_t* this, instr_t* ins, const unsigned int field, const unsigned int value);


ie_vw_t ie_vw_new(const seq_t* seq, const font_t* font)
{
    return (ie_vw_t) {
        .seq  = seq,
        .font = font,

        .current_instr_field = 0,
        .dirty = false,
    };
};

void ie_vw_render(ie_vw_t* this)
{
    const instr_t* ins = &this->seq->instrs[this->seq->current_selected_channel];
    const fm_instr_t* fm = &ins->fm_instr;

    // Render labels
    render_str(this->font, C_COL_LEFT, C_COL_TOP, 7, "Carrier");
    render_str(this->font, C_COL_LEFT, C_COL_TOP + 20, 7, "Attack Rate:");
    render_str(this->font, C_COL_LEFT, C_COL_TOP + 35, 7, "Decay Rate:");
    render_str(this->font, C_COL_LEFT, C_COL_TOP + 50, 7, "Sustain Level");
    render_str(this->font, C_COL_LEFT, C_COL_TOP + 65, 7, "Release Rate:");
    render_str(this->font, C_COL_LEFT, C_COL_TOP + 80, 7, "Waveform:");
    render_str(this->font, C_COL_LEFT, C_COL_TOP + 95, 7, "Volume:");
    render_str(this->font, M_COL_LEFT, M_COL_TOP, 7, "Modulator");
    render_str(this->font, M_COL_LEFT, M_COL_TOP + 20, 7, "Attack Rate:");
    render_str(this->font, M_COL_LEFT, M_COL_TOP + 35, 7, "Decay Rate:");
    render_str(this->font, M_COL_LEFT, M_COL_TOP + 50, 7, "Sustain Level:");
    render_str(this->font, M_COL_LEFT, M_COL_TOP + 65, 7, "Release Rate:");
    render_str(this->font, M_COL_LEFT, M_COL_TOP + 80, 7, "Waveform:");
    render_str(this->font, M_COL_LEFT, M_COL_TOP + 95, 7, "Volume:");

    // Render field values
    render_strf(this->font, instr_fields_pos[0][0], instr_fields_pos[0][1], 7, "%X", (fm->c__attack_decay >> 4) & 0xf);
    render_strf(this->font, instr_fields_pos[1][0], instr_fields_pos[1][1], 7, "%X", fm->c__attack_decay & 0xf);
    render_strf(this->font, instr_fields_pos[2][0], instr_fields_pos[2][1], 7, "%X", (fm->c__sustain_release >> 4) & 0xf);
    render_strf(this->font, instr_fields_pos[3][0], instr_fields_pos[3][1], 7, "%X", fm->c__sustain_release & 0xf);
    render_strf(this->font, instr_fields_pos[4][0], instr_fields_pos[4][1], 7, "%X", fm_get_carrier_waveform_type(fm));
    render_strf(this->font, instr_fields_pos[5][0], instr_fields_pos[5][1], 7, "%X", fm_get_carrier_level(fm));

    render_strf(this->font, instr_fields_pos[6][0], instr_fields_pos[6][1], 7, "%X", (fm->m__attack_decay >> 4) & 0xf);
    render_strf(this->font, instr_fields_pos[7][0], instr_fields_pos[7][1], 7, "%X", fm->m__attack_decay & 0xf);
    render_strf(this->font, instr_fields_pos[8][0], instr_fields_pos[8][1], 7, "%X", (fm->m__sustain_release >> 4) & 0xf);
    render_strf(this->font, instr_fields_pos[9][0], instr_fields_pos[9][1], 7, "%X", fm->m__sustain_release & 0xf);
    render_strf(this->font, instr_fields_pos[10][0], instr_fields_pos[10][1], 7, "%X", fm_get_modulator_waveform_type(fm));
    render_strf(this->font, instr_fields_pos[11][0], instr_fields_pos[11][1], 7, "%X", fm_get_modulator_level(fm));

    // draw "current field" rectangle
    rect(instr_fields_pos[this->current_instr_field][0] - 4,
         instr_fields_pos[this->current_instr_field][1] - 2,
         instr_fields_pos[this->current_instr_field][0] + 10,
         instr_fields_pos[this->current_instr_field][1] + 12,
         6);

    this->dirty = false;
}

void ie_vw_change(ie_vw_t* this, const action_t action)
{
    instr_t* ins = &this->seq->instrs[this->seq->current_selected_channel];
    unsigned int value = get_value_for_instrument(ins, this->current_instr_field);

    if (action == Increase &&
            ( ((this->current_instr_field == 4 || this->current_instr_field == 10) && value < 7) ||
              ((this->current_instr_field == 5 || this->current_instr_field == 11) && value < 31) ||
              ((this->current_instr_field != 4 && this->current_instr_field != 5 &&
                this->current_instr_field != 10 && this->current_instr_field != 11) && value < 15)) )
    {
        value++;
        set_value_for_instrument(this, ins, this->current_instr_field, value);
        this->dirty = true;
    } else if (action == Decrease && value > 0) {
        value--;
        set_value_for_instrument(this, ins, this->current_instr_field, value);
        this->dirty = true;
    }
}

void ie_vw_move(ie_vw_t* this, const direction_t dir)
{
    if (dir == Up && this->current_instr_field > 0) {
        this->current_instr_field--;
        this->dirty = true;
    } else if (dir == Down && this->current_instr_field < 11) {
        this->current_instr_field++;
        this->dirty = true;
    }
}


static unsigned int get_value_for_instrument(const instr_t* ins, const unsigned int field)
{
    const fm_instr_t* fm = &ins->fm_instr;
    switch (field) {
      case 0: return fm_get_carrier_attack_rate(fm);
      case 1: return fm_get_carrier_decay_rate(fm);
      case 2: return fm_get_carrier_sustain_level(fm);
      case 3: return fm_get_carrier_release_rate(fm);
      case 4: return fm_get_carrier_waveform_type(fm);
      case 5: return fm_get_carrier_level(fm);
      case 6: return fm_get_modulator_attack_rate(fm);
      case 7: return fm_get_modulator_decay_rate(fm);
      case 8: return fm_get_modulator_sustain_level(fm);
      case 9: return fm_get_modulator_release_rate(fm);
      case 10: return fm_get_modulator_waveform_type(fm);
      case 11: return fm_get_modulator_level(fm);
    }
    return 0;
}

static void set_value_for_instrument(ie_vw_t* this, instr_t* ins, const unsigned int field, const unsigned int value)
{
    fm_instr_t* fm = &ins->fm_instr;
    switch (field) {
      case 0: fm_set_carrier_attack_rate(fm, value); break;
      case 1: fm_set_carrier_decay_rate(fm, value); break;
      case 2: fm_set_carrier_sustain_level(fm, value); break;
      case 3: fm_set_carrier_release_rate(fm, value); break;
      case 4: fm_set_carrier_waveform_type(fm, value); break;
      case 5: fm_set_carrier_level(fm, value); break;
      case 6: fm_set_modulator_attack_rate(fm, value); break;
      case 7: fm_set_modulator_decay_rate(fm, value); break;
      case 8: fm_set_modulator_sustain_level(fm, value); break;
      case 9: fm_set_modulator_release_rate(fm, value); break;
      case 10: fm_set_modulator_waveform_type(fm, value); break;
      case 11: fm_set_modulator_level(fm, value); break;
    }
    seq_set_instrument(this->seq, ins, this->seq->current_selected_channel);
}
