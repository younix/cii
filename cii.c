/*
 * Copyright (c) 2012-2016 Jan Klemkow <j.klemkow@wemelug.de>
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

#include <curses.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>

#ifdef USE_LIBBSD
#	include <bsd/string.h>
#else
#	include <string.h>
#endif

#include <readline/history.h>
#include <readline/readline.h>

#include "cii.h"

struct options {
	int color;
	int show_log;
	int show_date;
	int show_time;
};

static void
exit_handler(void)
{
	if (isendwin() == FALSE)
		endwin();
}

static void
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
static void
input(WINDOW* inwin)
{
	int row, col;

	/*
	 * XXX: find a better way for feeding readline or find an other lib
	 * for user input.  Main problem is that you have to type backspace
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

static void
write_msg(char *line)
{
	if (access("in", W_OK) == -1)
		fprintf(stderr, "unable to open \"in\" file for writing\n");

	FILE *fh = fopen("in", "w");
	if (fh == NULL)
		perror("open file 'in'");
	fprintf(fh, "%s", line);
	fclose(fh);
}

static void
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

static void
usage(void)
{
	fprintf(stderr, "cii [-cl] PATH\n");
	exit(EXIT_FAILURE);
}

#if 0
static void
debug(char *str)
{
	FILE *fh = fopen("err", "a");
	fprintf(fh, "%s", str);
	fclose(fh);
}
#endif

int
main(int argc, char *argv[])
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
	if (chdir(argv[0]) == -1)
		err(EXIT_FAILURE, "chdir");

	if (atexit(exit_handler) == -1)
		err(EXIT_FAILURE, "atexit");

	/* initialize curses */
	if (setlocale(LC_ALL, "") == NULL) errx(EXIT_FAILURE, "setlocale");
	if (initscr() == NULL) errx(EXIT_FAILURE, "initscr");
	if (cbreak() == ERR) errx(EXIT_FAILURE, "cbreak");
	if (noecho() == ERR) errx(EXIT_FAILURE, "noecho");
	nonl();
	nodelay(stdscr, TRUE);
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(0);

	/* initialize curses windows for in- and output */
	getmaxyx(stdscr, row, col);
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

	/* out file */
	if (access("out", R_OK) == -1)
		errx(EXIT_FAILURE, "unable to open \"out\" file\n");

	/* open external source */
	char tail_cmd[BUFSIZ];
	snprintf(tail_cmd, sizeof tail_cmd, "exec tail -n 0 -f out");

	FILE *tail_fh = NULL;
	if ((tail_fh = popen(tail_cmd, "r")) == NULL)
		err(EXIT_FAILURE, "unable to open pipe to tail command");

//XXX: fix this feature
//	/* jump to the end of the file cause we just print new messages */
//	if (options->show_log == 0)
//		fseek(fh, 0, SEEK_END);

	struct pollfd pfd[2];
	pfd[0].fd = STDIN_FILENO;
	pfd[0].events = POLLIN;

	pfd[1].fd = fileno(tail_fh);
	pfd[1].events = POLLIN;

	rl_callback_handler_install("", write_msg);

	for (;;) {
		poll(pfd, 2, -1);

		/* handle tail command error and its broken pipe */
		if (pfd[1].revents & POLLHUP)
			break;

		/* handle keyboard input */
		if (pfd[0].revents & POLLIN)
			input(inwin);

		/* handle out file input */
		if (pfd[1].revents & POLLIN)
			output(msgwin, tail_fh, options);
	}
	fclose(tail_fh);

	return EXIT_SUCCESS;
}
