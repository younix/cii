/*
 * Copyright (c) 2012 Jan Klemkow <j.klemkow@wemelug.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "iiview.h"

int
parse_msg(char *line, struct chat_msg *chat_msg)
{
	if (line == NULL || chat_msg == NULL)
		return -1;

	size_t msg_len = strlen(line);

	if (msg_len < 20)
		return -2;

	/* seperate chat message from user status change */
	if (line[17] == '-')
		return -3;

	char *user_start = strptime(line, "%F %R <", &chat_msg->tm);
	char *user_end = strchr(user_start, '>');

	if (user_start > user_end ||
	    user_end - user_start > sizeof chat_msg->user)
		return -4;

	strlcpy(chat_msg->user, user_start, (user_end - user_start) + 1);
	strlcpy(chat_msg->mesg, user_end + 2, sizeof chat_msg->mesg);

	return 0;
}
