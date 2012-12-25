#include <conio.h>
#include <keys.h>

#include "common.h"
#include "sound.h"
#include "vga.h"

#define BOARD_SQUARE_SIZE 25
#define BOARD_SQUARE_PADDING 10
#define BOARD_ROWS 2
#define BOARD_COLS 8

#define BOARD_WIDTH  ((BOARD_SQUARE_SIZE * BOARD_COLS) + (BOARD_SQUARE_PADDING * (BOARD_COLS - 1)))
#define BOARD_HEIGHT ((BOARD_SQUARE_SIZE * BOARD_ROWS) + (BOARD_SQUARE_PADDING * (BOARD_ROWS - 1)))
#define BOARD_SIZE   (BOARD_WIDTH * BOARD_HEIGHT)

#define BOARD_LEFT   ((SCREEN_WIDTH  / 2) - (BOARD_WIDTH  / 2))
#define BOARD_TOP    ((SCREEN_HEIGHT / 2) - (BOARD_HEIGHT / 2))


const int DEFAULT_BPM = 120;

const char STEP_KEYS[]       = "qwertyuiasdfghjk";
const char STEP_UPPER_KEYS[] = "QWERTYUIASDFGHJK";

bool  metronome_on = false;
float current_bpm;
int   current_usecs_per_beat;
bool  dirty = true;

uclock_t prev_tap = NULL;

bool seq[BOARD_ROWS][BOARD_COLS] = {};


void toggle_metronome() {
    metronome_on = metronome_on ? false : true;
    if (metronome_on) {
        //printf("Metronome enabled ");
    } else {
        //printf("Metronome disabled ");
    }
}

void set_bpm(const float value) {
    current_bpm = value;
    current_usecs_per_beat = USECS_PER_MINUTE / value;
}

void set_bpm_from_usecs_per_beat(uclock_t usecs) {
    current_bpm = usecs * USECS_PER_MINUTE;
    current_usecs_per_beat = usecs;
}

void tap_tempo() {
    uclock_t now = uclock();
    if (prev_tap) {
        set_bpm_from_usecs_per_beat(now - prev_tap);
    }
    prev_tap = now;
}

void clear_seq()
{
    memset(seq, 0, BOARD_ROWS * BOARD_COLS * sizeof(bool));
}

void render_board()
{
    int i, j;
    int top = BOARD_TOP;
    for (i = 0; i < BOARD_ROWS; i++) {
        int left = BOARD_LEFT;
        for (j = 0; j < BOARD_COLS; j++) {
            if (seq[i][j]) {
                square_fill(left, top, BOARD_SQUARE_SIZE, 10);
            } else {
                square(left, top, BOARD_SQUARE_SIZE, 10);
            }
            left += (BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING);
        }
        top += BOARD_SQUARE_SIZE + BOARD_SQUARE_PADDING;
    }
}

void render()
{
    clear_screen();
    //render_channel_selector();
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
    init_tick();

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
                clear_seq();
                break;
            }

            int i;
            for (i = 0; i < 16; i++) {
                if (key == STEP_KEYS[i] || key == STEP_UPPER_KEYS[i]) {
                    seq[i / BOARD_COLS][i % BOARD_COLS] = not(seq[i / BOARD_COLS][i % BOARD_COLS]);
                    dirty = true;
                }
            }
        }

        uclock_t now = uclock();
        if (now >= prev + current_usecs_per_beat) {
            if (metronome_on) {
                play_tick();
                //printf("o");
            } else {
                //printf(".");
            }
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
