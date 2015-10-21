#include <stdio.h>
#include "klik.h"
#include "io.h"
#ifdef __MSDOS__
#include <conio.h>
#endif
#ifdef USE_CURSES
#include <curses.h>
#include "kbhit.h"
#endif

char output_buffer[2048];
int  buffer_end = 0;

enum modes guess_screen_mode(enum modes type) {
    if (type != NONE) {
        return type;
    }
#ifdef USE_CURSES
    return CURSES;
#elif defined(__MSDOS__)
    return ANSI;
#else
    return FLAT;
#endif
}

void initialize_screen(enum modes type, unsigned long *width, unsigned long *height) {
    int index;

    for (index=0;index<sizeof(output_buffer);index++) {
        output_buffer[index] = '\000';
    }

    if (type == NONE) {
        /* Do nothing */;
#ifdef __MSDOS__
    } else if (type == ANSI) {
        /* Do nothing */;
#endif
#ifdef USE_CURSES
    } else if (type == CURSES) {
        initscr();
        cbreak();
        noecho();
        getmaxyx(stdscr, *height, *width);
#endif
    } else {
        /* Do nothing */;
    }

    if (!*width) {
        *width = 80;
    }
    if (!*height) {
        *height = 22;
    }
}

void clear_screen(enum modes type) {
    if (type == NONE) {
        /* Do nothing */;
#ifdef __MSDOS__
    } else if (type == ANSI) {
        printf("%c[22;1H", 27);
#endif
#ifdef USE_CURSES
    } else if (type == CURSES) {
        clear();
        refresh();
#endif
    } else {
        /* Do nothing */;
    }
}

void show_character(enum modes type, struct thing **screen, int xpos, int ypos) {
    char next = screen[xpos][ypos].display;
    int  idle = screen[xpos][ypos].idle;

    if (type == NONE) {
        /* Do nothing */;
#ifdef USE_CURSES
    } else if (type == CURSES) {
        move(ypos - 1, xpos - 1);
        addch(next);
        refresh();
#endif
    } else {
        /* Includes ANSI */
        printf("%c", next);
        fflush(stdout);
    }
}

int input_character(enum modes type) {
    int value = -1;

    if (type == NONE) {
        /* Do nothing */;
#ifdef __MSDOS__
    } else if (type == ANSI) {
        if (kbhit()) {
            value = getch();
        }
#endif
#ifdef USE_CURSES
    } else if (type == CURSES) {
        /* NCurses lets us use the same function names as
         * DOS, but it's too irritating to merge one line
         * of code.
         */
        if (kbhit()) {
            value = getch();
        }
#endif
    } else {
        /* No support */
    }

    if (value < 1) {
        /* Possibly of interest */
    }
}

void dump_output(enum modes type, char c) {
    /* Add character to the list or print the buffer to the screen */
    if (c) {
        output_buffer[buffer_end] = c;
        ++ buffer_end;
        return;
    }
    if (type == NONE) {
        /* Do nothing */;
#ifdef __MSDOS__
    } else if (type == ANSI) {
        printf("\n%s", output_buffer);
#endif
#ifdef USE_CURSES
    } else if (type == CURSES) {
        int width, height, index;

        getmaxyx(stdscr, height, width);
        for (index=0;index<buffer_end;index++) {
            move(height - 1, index);
            addch(output_buffer[index]);
            refresh();
        }
#endif
    } else {
        printf("\n%s", output_buffer);
    }
}

void dump_number(enum modes type, int number) {
    /* Add a (small) number to the output buffer */
    if (number > 9) {
        dump_output(type, '1');
        number -= 10;
    }
    dump_output(type, number + '0');
}

void close_window(enum modes type) {
#ifdef USE_CURSES
    if (type == CURSES) {
        endwin();
    }
#endif
}

