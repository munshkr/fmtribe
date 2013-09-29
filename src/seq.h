#ifndef __SEQ_H__
#define __SEQ_H__

#include "instr.h"
#include "fm.h"

#define CHANNELS        8
#define STEPS           16
#define FRAMES          4

typedef struct {
    instr_t   instrs[CHANNELS];

    // TODO mseq and seq should be merged (1 microstep == 1 step...)
    bool         seq[CHANNELS][FRAMES][STEPS];
    unsigned int mseq[CHANNELS][FRAMES][STEPS];
    bool         muted_channels[CHANNELS];

    uclock_t     current_uclocks_per_step;
    unsigned int current_bpm;
    int          current_frame;
    int          current_step;

    uclock_t prev;
    uclock_t mprev[CHANNELS];
    uclock_t prev_tap;

    bool pause_after_current_step;
    bool record_step;
    bool stop_after_pattern_ends;
    bool playing;
    bool metronome_on;

    int  current_selected_channel;
    int  current_selected_frame;

    bool follow;
    bool apply_all_frames;
    bool play_instruments;
    bool recording;

    bool dirty;
} seq_t;

seq_t seq_new();

void seq_tick(seq_t* this);
void seq_advance_step(seq_t* this);

void seq_play_channel(const seq_t* this, const unsigned int c);
void seq_play_step(const seq_t* this);

void seq_set_bpm(seq_t* this, const unsigned int value);
void seq_set_bpm_from_uclocks_per_beat(seq_t* this, const uclock_t uclocks);

void seq_clear_seq(seq_t* this, const int channel);
void seq_clear_seq_all(seq_t* this);

void seq_select_prev_channel(seq_t* this);
void seq_select_next_channel(seq_t* this);
void seq_select_prev_frame(seq_t* this);
void seq_select_next_frame(seq_t* this);

void seq_toggle_metronome(seq_t* this);
void seq_toggle_follow(seq_t* this);
void seq_toggle_apply_all_frames(seq_t* this);
void seq_toggle_play_instruments(seq_t* this);
void seq_toggle_recording(seq_t* this);

void seq_tap_tempo(seq_t* this);

#endif // __SEQ_H__
