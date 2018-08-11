#include "ie_ctl.h"
#include "ie_vw.h"
#include "keys.h"


const note_t KEYBOARD_NOTES[] = { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B };


ie_ctl_t ie_ctl_new(seq_t* seq, ie_vw_t* ie_vw)
{
    return (ie_ctl_t) {
        .seq   = seq,
        .ie_vw = ie_vw,
    };
};

void ie_ctl_handle_keyboard(ie_ctl_t* this, const int key)
{
    switch (key) {
      case K_Right:
        ie_vw_change(this->ie_vw, Increase);
        break;
      case K_Left:
        ie_vw_change(this->ie_vw, Decrease);
        break;
      case K_Up:
        ie_vw_move(this->ie_vw, Up);
        break;
      case K_Down:
        ie_vw_move(this->ie_vw, Down);
        break;
      case K_PageDown:
        if (this->seq->instrs[this->seq->current_selected_channel].octave > 1) {
            this->seq->instrs[this->seq->current_selected_channel].octave--;
            this->ie_vw->dirty = true;
        }
        break;
      case K_PageUp:
        if (this->seq->instrs[this->seq->current_selected_channel].octave < 8) {
            this->seq->instrs[this->seq->current_selected_channel].octave++;
            this->ie_vw->dirty = true;
        }
        break;
    }

    for (int i = 0; i < KEYBOARD_KEYS_COUNT; i++) {
        if (key == KEYBOARD_KEYS[i] || key == KEYBOARD_UPPER_KEYS[i]) {
            this->seq->instrs[this->seq->current_selected_channel].note = KEYBOARD_NOTES[i];
            if (!this->seq->playing) seq_play_channel(this->seq, this->seq->current_selected_channel);
        }
    }
}
