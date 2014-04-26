CC ?= cc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -fgnu89-inline

.PHONY: all clean install
.SUFFIXES: .o .c

all: cii

cii: cii.o parser_static.o
	$(CC) -o $@ cii.o parser_static.o -lcurses -lreadline

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

debug:
	gdb cii cii.core

clean:
	rm -f *.o cii cii.core

install: cii
	mkdir -p ${HOME}/bin
	cp cii ${HOME}/bin
