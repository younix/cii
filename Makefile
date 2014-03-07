CFLAGS=-std=c99 -pedantic -Wall -Wextra -fgnu89-inline
LFLAGS=-lcurses -lreadline

iiview: iiview.c parser_static.c
	gcc ${CFLAGS} ${LFLAGS} -o $@ iiview.c parser_static.c

debug:
	gdb iiview iiview.core

clean:
	rm -f iiview iiview.core

install: iiview
	mkdir -p ${HOME}/bin
	cp $> ${HOME}/bin
