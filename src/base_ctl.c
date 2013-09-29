#include "base_ctl.h"
#include <keys.h>

const char CHANNEL_KEYS[] = "12345678";
const unsigned int CHANNEL_MUTE_KEYS[] = {
    K_Alt_1, K_Alt_2, K_Alt_3, K_Alt_4, K_Alt_5, K_Alt_6, K_Alt_7, K_Alt_8
};


base_ctl_t base_ctl_new(seq_t* seq)
{
    return (base_ctl_t) {
        .seq = seq,
    };
};

void base_ctl_handle_keyboard(base_ctl_t* this, const int key)
{
    switch (key) {
      case K_F5:
        if (this->seq->playing) {
            this->seq->pause_after_current_step = true;
        } else {
            this->seq->playing = true;
            this->seq->prev = uclock();
            seq_play_step(this->seq);
        }
        break;
      case K_F7:
        if (this->seq->playing) {
            // on second F7, stop immediately, by pausing after current
            // step and resetting current_step.
            if (this->seq->stop_after_pattern_ends) {
                this->seq->stop_after_pattern_ends = false;
                this->seq->pause_after_current_step = true;
                this->seq->current_step = 0;
                this->seq->current_frame = 0;
                if (this->seq->follow) {
                    this->seq->current_selected_frame = this->seq->current_frame;
                }
            } else {
                this->seq->stop_after_pattern_ends = true;
            }
        }
        break;
      case K_Shift_F9:
        seq_toggle_metronome(this->seq);
        break;
      case K_F9:
        seq_tap_tempo(this->seq);
        break;
    }

    // select (and play) channel if key was pressed
    for (int i = 0; i < CHANNELS; i++) {
        if (key == CHANNEL_KEYS[i]) {
            this->seq->current_selected_channel = i;
            if (this->seq->play_instruments) {
                seq_play_channel(this->seq, i);
            }
            // on recording, record current step
            if (this->seq->playing && this->seq->recording) {
                this->seq->record_step = true;
            }
            this->seq->dirty = true;
            break;
        }
    }

    // mute channel if key was pressed
    for (int i = 0; i < CHANNELS; i++) {
        if (key == CHANNEL_MUTE_KEYS[i]) {
            this->seq->muted_channels[i] = Not(this->seq->muted_channels[i]);
            this->seq->dirty = true;
        }
    }
}
