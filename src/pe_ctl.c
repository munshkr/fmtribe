#include "pe_ctl.h"
#include <keys.h>

#define MAX_MICROSTEPS 3

const char STEP_KEYS[]       = "qwertyuiasdfghjk";
const char STEP_UPPER_KEYS[] = "QWERTYUIASDFGHJK";

const unsigned int MICROSTEP_KEYS[] = {
    K_Alt_Q, K_Alt_W, K_Alt_E, K_Alt_R, K_Alt_T, K_Alt_Y, K_Alt_U, K_Alt_I,
    K_Alt_A, K_Alt_S, K_Alt_D, K_Alt_F, K_Alt_G, K_Alt_H, K_Alt_J, K_Alt_K
};


pe_ctl_t pe_ctl_new(seq_t* seq, pe_vw_t* pe_vw)
{
    return (pe_ctl_t) {
        .seq   = seq,
        .pe_vw = pe_vw,
    };
};

void pe_ctl_handle_keyboard(pe_ctl_t* this, const int key)
{
    switch (key) {
      case K_Up:
        seq_select_prev_channel(this->seq);
        break;
      case K_Down:
        seq_select_next_channel(this->seq);
        break;
      case K_Left:
        seq_select_prev_frame(this->seq);
        break;
      case K_Right:
        seq_select_next_frame(this->seq);
        break;
      case 'm':
      case 'M':
        seq_toggle_apply_all_frames(this->seq);
        break;
      case 'p':
      case 'P':
        seq_toggle_play_instruments(this->seq);
        break;
      case 'z':
      case 'Z':
        seq_toggle_recording(this->seq);
        break;
      case K_Delete:
        seq_clear_seq(this->seq, this->seq->current_selected_channel);
        break;
      case K_Control_Delete:
        seq_clear_seq_all(this->seq);
        break;
      case K_F10:
        seq_toggle_follow(this->seq);
        break;
    }

    // toggle steps based on key
    for (int i = 0; i < STEPS; i++) {
        if (key == STEP_KEYS[i] || key == STEP_UPPER_KEYS[i]) {
            this->seq->seq[this->seq->current_selected_channel][this->seq->current_selected_frame][i] =
              Not(this->seq->seq[this->seq->current_selected_channel][this->seq->current_selected_frame][i]);

            if (this->seq->apply_all_frames) {
                for (int j = 0; j < FRAMES; j++) {
                    if (j != this->seq->current_selected_frame) {
                        this->seq->seq[this->seq->current_selected_channel][j][i] =
                          this->seq->seq[this->seq->current_selected_channel][this->seq->current_selected_frame][i];
                    }
                }
            }

            this->seq->dirty = true;
        }
    }

    // toggle microsteps based on key
    for (int i = 0; i < STEPS; i++) {
        if (key == MICROSTEP_KEYS[i]) {
            this->seq->seq[this->seq->current_selected_channel][this->seq->current_selected_frame][i] = true;
            this->seq->mseq[this->seq->current_selected_channel][this->seq->current_selected_frame][i] =
              (this->seq->mseq[this->seq->current_selected_channel][this->seq->current_selected_frame][i] + 1) % MAX_MICROSTEPS;

            if (this->seq->apply_all_frames) {
                for (int j = 0; j < FRAMES; j++) {
                    if (j != this->seq->current_selected_frame) {
                        this->seq->seq[this->seq->current_selected_channel][j][i] = true;
                        this->seq->mseq[this->seq->current_selected_channel][j][i] =
                          this->seq->mseq[this->seq->current_selected_channel][this->seq->current_selected_frame][i];
                    }
                }
            }

            this->seq->dirty = true;
        }
    }
}
