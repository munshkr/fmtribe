#include <stdlib.h>
#include <curses.h>

#include "common.h"
#include "sound.h"

const int DEFAULT_BPM = 120;

bool metronome_on = false;
int  current_bpm;
int  current_usecs_per_beat;

uclock_t prev_tap = NULL;


void toggle_metronome() {
    metronome_on = metronome_on ? false : true;
    if (metronome_on) {
        mvprintw(24, 0, "Metronome enabled ");
    } else {
        mvprintw(24, 0, "Metronome disabled ");
    }
}

void set_bpm(int value) {
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


int main(int argc, char* argv[])
{
    reset_sound();

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    set_bpm(DEFAULT_BPM);

    printw("FMTribe");

    int ch;
    bool is_running = true;
    uclock_t prev = uclock();
    init_tick();

    while (is_running) {
        ch = getch();
        switch (ch) {
          case KEY_ESC:
            printw(" Exit");
            is_running = false;
            break;
          case KEY_F(9):
            toggle_metronome();
            break;
          case KEY_F(10):
            tap_tempo();
            break;
        }

        uclock_t now = uclock();
        if (now >= prev + current_usecs_per_beat) {
            if (metronome_on) {
                play_tick();
                printw("o");
            } else {
                printw(".");
            }
            prev = now;
        }
    }

    refresh();
    endwin();

    reset_sound();

    return EXIT_SUCCESS;
}
