#ifndef _IO__H_
#define _IO__H_

enum modes {
    ANSI,
    CURSES,
    FLAT,
    NONE            /* Never use NONE */
};

extern int use_color;

enum modes guess_screen_mode(enum modes);
void initialize_screen(enum modes, unsigned long *, unsigned long *);
void clear_screen(enum modes);
void show_character(enum modes, struct thing **, int, int);
int input_character(enum modes);
void dump_output(enum modes, char);
void dump_number(enum modes, int);
void close_window(enum modes);

#endif

