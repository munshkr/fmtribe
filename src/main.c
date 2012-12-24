#include <stdlib.h>
#include <curses.h>

#include "common.h"
#include "sound.h"

#define KEY_ESC 27

#define BPM 120
#define USECS_PER_MINUTE (1000000 * 60)

bool metronome_on = false;

void toggle_metronome() {
    metronome_on = metronome_on ? false : true;
    if (metronome_on) {
        mvprintw(24, 0, "Metronome enabled ");
    } else {
        mvprintw(24, 0, "Metronome disabled ");
    }
}

int main(int argc, char* argv[])
{
    reset_sound();

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

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
        }

        uclock_t now = uclock();
        if (now >= prev + (USECS_PER_MINUTE / BPM)) {
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
