CC ?= cc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -D_XOPEN_SOURCE=500 -g -fgnu89-inline $(WARN)
LIB_CURSES ?= -lcurses

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
        mkdir -p /usr/local/bin
        cp cii /usr/local/bin
