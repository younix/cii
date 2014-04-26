/*
 * Copyright (c) 2012-2014 Jan Klemkow <j.klemkow@wemelug.de>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _XOPEN_SOURCE 500

#include <curses.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "cii.h"

struct options {
	int color;
	int show_log;
	int show_date;
	int show_time;
};

void
output(WINDOW* msgwin, FILE *fh, struct options *options)
{
	char buf[BUFSIZ];
	struct chat_msg chat_msg;
	int err;

	clearerr(fh); /* clear EOF from last reading */

	while (fgets(buf, sizeof buf, fh) != NULL) {
		if ((err = parse_msg(buf, &chat_msg)) != 0)
			continue;

		if (options->show_date)
			wprintw(msgwin, "%s ", chat_msg.date);

                if (options->show_time)
			wprintw(msgwin, "%s ", chat_msg.time);

		if (options->color)
			wcolor_set(msgwin, 2, 0);

		wprintw(msgwin, "%s: ", chat_msg.user);

		if (options->color)
			wcolor_set(msgwin, 1, 0);

		wprintw(msgwin, "%s", chat_msg.mesg);
	}

	refresh();
	wrefresh(msgwin);
}

/* handls the input from the user and print the input line out */
void
input(WINDOW* inwin)
{
	int row, col;

	/*
	 * XXX: find a better way for feeding readline or find an other lib
	 * for user input. Main problem is that you have to type backspace
	 * two time to have one.
	 */
	rl_callback_read_char();
	getmaxyx(inwin, row, col);

	/* const 3 = 2 frame + 1 cursor */
	int offset = strlen(rl_line_buffer) - col + 3;
	if (offset < 0)
		offset = 0;

	werase(inwin);
	mvwprintw(inwin, 1, 1, "%s", rl_line_buffer + offset);

	/* show cursror */
	mvwchgat(inwin, 1, rl_point - offset + 1, 1, A_REVERSE, 0, NULL);
	box(inwin, 0, 0);
	wrefresh(inwin);
}

void
write_msg(char *line)
{
	if (access("in", W_OK) == -1)
		fprintf(stderr, "unable to open \"int\" file for writing\n");

	FILE *fh = fopen("in", "w");
	if (fh == NULL)
		perror("open file 'in'");
	fprintf(fh, "%s\n", line);
	fclose(fh);
}

void
draw_screen(WINDOW *msgwin, WINDOW *inwin)
{
	int row, col;

	getmaxyx(stdscr, row, col);

	mvwin(msgwin, 0, 0);
	mvwin(inwin, row - 3, 0);
	wresize(msgwin, row - 3, col);
	wresize(inwin, 3, col);

	box(inwin, 0, 0);

	refresh();
	wrefresh(inwin);
	wrefresh(msgwin);
}

void
usage(void)
{
	fprintf(stderr, "cii [-cl] PATH\n");
	exit(EXIT_FAILURE);
}

void
debug(char *str)
{
	FILE *fh = fopen("err", "a");
	fprintf(fh, "%s", str);
	fclose(fh);
}

int
main(int argc, char**argv)
{
	WINDOW* msgwin; /* curses window for chat messages */
	WINDOW* inwin; /* curses window for user input */
	int col, row, ch;

	/* init default options */
	struct options *options = malloc(sizeof *options);
	options->color = 0;
	options->show_log = 0;
	options->show_date = 1;
	options->show_time = 1;

	while ((ch = getopt(argc, argv, "cl")) != -1) {
		switch (ch) {
		case 'c':
			options->color = 1;
			break;
		case 'l':
			options->show_log = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	/* change to directory with the in and out file from ii */
	chdir(argv[0]);

	/* initialize curses */
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	nonl();
	nodelay(stdscr, TRUE);
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(0);

	/* initialize curses windows for in- and output */
	getmaxyx(stdscr,row,col);
	msgwin = newwin(1, 1, 0, 0);
	inwin  = newwin(1, 1, 0, 0);
	scrollok(msgwin, TRUE);

	/* init colors */
	if (options->color && has_colors() == TRUE) {
		start_color();
		init_pair(1, COLOR_WHITE, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
		wcolor_set(msgwin, 1, 0);
	}

	draw_screen(msgwin, inwin);

	if (access("out", R_OK) == -1) {
		fprintf(stderr, "unable to open \"out\" file for reading\n");
		exit(EXIT_FAILURE);
	}

	FILE *fh = fopen("out", "r");
	if (fh == NULL)
		perror("open file 'out'");

	/* jump to the end of the file cause we just print new messages */
	if (options->show_log == 0)
		fseek(fh, 0, SEEK_END);

	rl_callback_handler_install("", write_msg);

	/* init select timeout for polling loop */
	struct timeval sleep_time;
	sleep_time.tv_sec = 0;
	sleep_time.tv_usec = 10;

	fd_set read_fds;

	for (;;) {
		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);
		if (select(STDIN_FILENO + 1, &read_fds, NULL, NULL,
		    &sleep_time) == -1) {
			perror("select:");
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(STDIN_FILENO, &read_fds))
			input(inwin);

		output(msgwin, fh, options);
	}

	fclose(fh);
	return EXIT_SUCCESS;
}
