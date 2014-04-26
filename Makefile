CFLAGS=-std=c99 -pedantic -Wall -Wextra -fgnu89-inline
LFLAGS=-lcurses -lreadline

cii: cii.c parser_static.c
	gcc ${CFLAGS} ${LFLAGS} -o $@ cii.c parser_static.c

debug:
	gdb cii cii.core

clean:
	rm -f cii cii.core

install: cii
	mkdir -p ${HOME}/bin
	cp cii ${HOME}/bin
