#include <conio.h>
#include <keys.h>

#include "common.h"
#include "sound.h"
#include "vga.h"

#define CHANNELS 8
#define STEPS    16

#define BOARD_SQUARE_SIZE 25
#define BOARD_SQUARE_PADDING 10
#define BOARD_ROWS 2
#define BOARD_COLS 8

#define BOARD_WIDTH  ((BOARD_SQUARE_SIZE * BOARD_COLS) + (BOARD_SQUARE_PADDING * (BOARD_COLS - 1)))
#define BOARD_HEIGHT ((BOARD_SQUARE_SIZE * BOARD_ROWS) + (BOARD_SQUARE_PADDING * (BOARD_ROWS - 1)))
#define BOARD_SIZE   (BOARD_WIDTH * BOARD_HEIGHT)
#define BOARD_LEFT   ((SCREEN_WIDTH  / 2) - (BOARD_WIDTH  / 2))
#define BOARD_TOP    ((SCREEN_HEIGHT / 2) - (BOARD_HEIGHT / 2))

#define CHANNEL_SELECTOR_WIDTH  15
#define CHANNEL_SELECTOR_HEIGHT 6
#define CHANNEL_SELECTOR_TOP    (BOARD_TOP - CHANNEL_SELECTOR_HEIGHT - 10)
#define CHANNEL_SELECTOR_LEFT   BOARD_LEFT


const int DEFAULT_BPM = 120;

const char STEP_KEYS[]       = "qwertyuiasdfghjk";
const char STEP_UPPER_KEYS[] = "QWERTYUIASDFGHJK";
const char CHANNEL_KEYS[]    = "12345678";

const byte CHANNEL_COLORS[CHANNELS]   = { 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
const byte CHANNEL_COLORS_B[CHANNELS] = { 0x18, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

bool  dirty = true;

uclock_t current_usecs_per_step = 0;
float    current_bpm;
int      current_channel = 0;
int      current_step = 0;

bool  metronome_on = false;

uclock_t prev_tap = NULL;

bool seq[CHANNELS][STEPS] = {};


void tick()
{
    current_step++;
    if (current_step == STEPS) {
        current_step = 0;
    }
    dirty = true;

    if (metronome_on) {
        if (current_step % 4 == 0) {
            play_metronome();
        }
    }

    int c;
    for (c = 0; c < CHANNELS; c++) {
        if (seq[c][current_step]) {
            // TODO play channel sound
        }
    }
}

void toggle_metronome()
{
    metronome_on = not(metronome_on);
    if (metronome_on) {
        //printf("Metronome enabled ");
    } else {
        //printf("Metronome disabled ");
    }
}

void set_bpm(const float value)
{
    current_bpm = value;
    current_usecs_per_step = (USECS_PER_MINUTE / value) / 4;
}

void set_bpm_from_usecs_per_beat(uclock_t usecs)
{
    current_bpm = usecs * USECS_PER_MINUTE;
    current_usecs_per_step = usecs / 4;
}

void tap_tempo()
{
    uclock_t now = uclock();
    if (prev_tap) {
        set_bpm_from_usecs_per_beat(now - prev_tap);
    }
    prev_tap = now;
}

void clear_seq(int channel)
{
    memset(seq[channel], 0, STEPS * sizeof(bool));
    dirty = true;
}

void clear_seq_all()
{
    memset(seq, 0, CHANNELS * STEPS * sizeof(bool));
    dirty = true;
}

void select_prev_channel() {
    if (current_channel > 0) {
        current_channel--;
        dirty = true;
    }
}

void select_next_channel() {
    if (current_channel < CHANNELS - 1) {
        current_channel++;
        dirty = true;
    }
}

void render_board()
{
    int color = CHANNEL_COLORS[current_channel];
    int top = BOARD_TOP;
    int i, j, z = 0;
    for (i = 0; i < BOARD_ROWS; i++) {
        int left = BOARD_LEFT;
        for (j = 0; j < BOARD_COLS; j++) {
            // if it is about to render the square for the current step,
            // use a different color.
            if (z == current_step) {
                color = CHANNEL_COLORS_B[current_channel];
            }

            // render a filled square if the step is toggled
            if (seq[current_channel][(i * BOARD_COLS) + j]) {
                square_fill(left, top, BOARD_SQUARE_SIZE, color);
            } else {
                square(left, top, BOARD_SQUARE_SIZE, color);
            }

            // restore color
            if (z == current_step) {
                color = CHANNEL_COLORS[current_channel];
            }

            left += (BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING);
            z++;
        }
        top += BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING;
    }
}

void render_channel_selector()
{
    const int top = CHANNEL_SELECTOR_TOP;
    const int bottom = top + CHANNEL_SELECTOR_HEIGHT;
    int left = CHANNEL_SELECTOR_LEFT;
    int right = left + CHANNEL_SELECTOR_WIDTH;

    int i;
    for (i = 0; i < CHANNELS; i++) {
        if (i == current_channel) {
            rect_fill(left, top, right, bottom, CHANNEL_COLORS[i]);
        } else {
            rect(left, top, right, bottom, CHANNEL_COLORS[i]);
        }
        left += (CHANNEL_SELECTOR_WIDTH + BOARD_SQUARE_PADDING);
        right = left + CHANNEL_SELECTOR_WIDTH;
    }
}

void render_hits()
{
    const int top = CHANNEL_SELECTOR_TOP - (CHANNEL_SELECTOR_HEIGHT / 2) - 3;
    const int bottom = top + (CHANNEL_SELECTOR_HEIGHT / 2);
    int left = CHANNEL_SELECTOR_LEFT;
    int right = left + CHANNEL_SELECTOR_WIDTH;

    int i;
    for (i = 0; i < CHANNELS; i++) {
        if (seq[i][current_step]) {
            rect_fill(left, top, right, bottom, CHANNEL_COLORS_B[i]);
        }
        left += (CHANNEL_SELECTOR_WIDTH + BOARD_SQUARE_PADDING);
        right = left + CHANNEL_SELECTOR_WIDTH;
    }
}

void render()
{
    clear_screen();
    render_hits();
    render_channel_selector();
    render_board();
}


int main(int argc, char* argv[])
{
    reset_sound();

    init_vga();
    set_mode(VIDEO_MODE);

    set_bpm(DEFAULT_BPM);

    render_board();

    bool is_running = true;
    uclock_t prev = uclock();

    while (is_running) {
        if (kbhit()) {
            int key = getkey();

            switch (key) {
              case K_Escape:
                is_running = false;
                break;
              case K_Shift_F9:
                toggle_metronome();
                break;
              case K_F9:
                tap_tempo();
                break;
              case K_Delete:
                clear_seq(current_channel);
                break;
              case K_Control_Delete:
                clear_seq_all();
                break;
              case K_Left:
                select_prev_channel();
                break;
              case K_Right:
                select_next_channel();
                break;
            }

            int i;
            for (i = 0; i < STEPS; i++) {
                if (key == STEP_KEYS[i] || key == STEP_UPPER_KEYS[i]) {
                    seq[current_channel][i] = not(seq[current_channel][i]);
                    dirty = true;
                }
            }

            for (i = 0; i < CHANNELS; i++) {
                if (key == CHANNEL_KEYS[i]) {
                    current_channel = i;
                    dirty = true;
                    break;
                }
            }
        }

        uclock_t now = uclock();
        if (now >= prev + current_usecs_per_step) {
            tick();
            prev = now;
        }

        if (dirty) {
            render();
            dirty = false;
        }
    }

    set_mode(TEXT_MODE);
    reset_sound();

    return EXIT_SUCCESS;
}
