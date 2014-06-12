NAME=klik
CC=gcc
RM=rm
SOURCES=$(NAME).c io.c kbhit.c
LIBS=ncurses
CFLAGS=-DUSE_CURSES
CFLAGS+=`pkg-config --cflags $(LIBS)`
LIBFLAGS=
LIBFLAGS=`pkg-config --libs $(LIBS)`

all: klik

klik: $(SOURCES) $(NAME).h io.h kbhit.h
	$(CC) -o$(NAME) $(SOURCES) $(CFLAGS) $(LIBFLAGS)

clean:
	$(RM) *~ *.o

