#include "pe_ctl.h"
#include "keys.h"


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

    if (this->seq->recording) {
        // on recording, record current note
        if (this->seq->playing) {
            for (int i = 0; i < KEYBOARD_KEYS_COUNT; i++) {
                if (key == KEYBOARD_KEYS[i] || key == KEYBOARD_UPPER_KEYS[i]) {
                    this->seq->record_step = true;
                    this->seq->record_note = i;
                }
            }
        }
    } else {
        // when not recording, use step mode (default)
        for (int i = 0; i < STEPS; i++) {
            if (key == STEP_KEYS[i] || key == STEP_UPPER_KEYS[i]) {
                seq_toggle_step(this->seq, this->seq->current_selected_channel, this->seq->current_selected_frame, i);
            }
        }

        for (int i = 0; i < STEPS; i++) {
            if (key == MICROSTEP_KEYS[i]) {
                seq_toggle_microstep(this->seq, this->seq->current_selected_channel, this->seq->current_selected_frame, i);
            }
        }
    }
}
