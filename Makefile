CFLAGS=-std=c99 -pedantic -Wall -fgnu89-inline
LFLAGS=-lcurses -lreadline

iiview: iiview.c parser_static.c
	gcc ${CFLAGS} ${LFLAGS} -o $@ $>

debug:
	gdb iiview iiview.core

clean:
	rm -f iiview iiview.core

install: iiview
	mkdir -p ${HOME}/bin
	cp $> ${HOME}/bin
