#include <stdlib.h>
#include <curses.h>

#include "common.h"
#include "sound.h"

int main(int argc, char* argv[])
{
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

    return EXIT_SUCCESS;
}
