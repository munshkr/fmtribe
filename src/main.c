#include <stdlib.h>
#include <curses.h>

#include "common.h"
#include "sound.h"

int main(int argc, char* argv[])
{
    reset_sound();

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    printw("FMTribe");
    int ch;

    while (TRUE) {
        ch = getch();
        if (ch == 27) {
            printw(" Exit");
            break;
        }
    }

    refresh();
    endwin();

    reset_sound();

    return EXIT_SUCCESS;
}
