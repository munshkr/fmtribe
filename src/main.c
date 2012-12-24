#include <stdlib.h>
#include <curses.h>

#include "common.h"
#include "sound.h"

#define KEY_ESC 27

#define BPM 120
#define USECS_PER_MINUTE (1000000 * 60)

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

    uclock_t prev = uclock();
    while (TRUE) {
        ch = getch();
        if (ch == KEY_ESC) {
            printw(" Exit");
            break;
        }

        uclock_t now = uclock();
        if (now >= prev + (USECS_PER_MINUTE / BPM)) {
            printw(".");
            prev = now;
        }
    }

    refresh();
    endwin();

    reset_sound();

    return EXIT_SUCCESS;
}
