CC ?= cc
WARN=-Wno-unused-but-set-variable
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g -fgnu89-inline $(WARN)
CFLAGS_CURSES=`pkg-config --cflags ncurses`
LIB_CURSES=`pkg-config --libs ncurses`

.PHONY: all clean install
.SUFFIXES: .o .c

all: cii

cii: cii.o parser_static.o
	$(CC) -o $@ cii.o parser_static.o -lreadline $(LIB_CURSES) $(LIBS_BSD)

.c.o:
	$(CC) $(CFLAGS) $(CFLAGS_CURSES) $(CFLAGS_BSD) $(DEFINES) -c -o $@ $<

debug:
	gdb cii cii.core

clean:
	rm -f *.o cii cii.core

install: cii
	mkdir -p ${HOME}/bin
	cp cii ${HOME}/bin
