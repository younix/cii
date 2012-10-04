#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>
#include <errno.h>

#include "iiview.h"

/*
 * I know, using regex for this simpel task is not very smart. But, I just want
 * to try it. So it should be posible to replace the hole module with a new one
 * without regex. Date und time have static size. The only variable thing is
 * the username and the rest is just message.
 */

#define RE_DATE "([[:digit:]]{4}-[[:digit:]]{2}-[[:digit:]]{2})"
#define RE_TIME "([[:digit:]]{2}:[[:digit:]]{2})"
#define RE_USER "[<](.*)[>]"
#define RE_MESG "(.*)"
#define MSG_REGEX RE_DATE RE_TIME RE_USER RE_MESG

#define RE_COMD "-!-"
#define EVE_REGEX RE_DATE RE_TIME RE_COMD RE_MESG

/* order of parenties */
enum {	MG_DATE,
	MG_TIME,
	MG_USER,
	MG_MESG
};

void
parse_msg(char *msg_str, struct chat_msg *chat_msg, WINDOW *win)
{
	regex_t reg;
	int result = 0;
	regmatch_t *pmatch;
	int i = 0;
	char pom[BUFSIZ];
	char *tmp;
	size_t tmp_size;

	wprintw(win, "FUCK\n");
	if (regcomp(&reg, MSG_REGEX, REG_EXTENDED) != 0) {
		fprintf(stderr, "Fehler!\n");
		return;
	}

	pmatch = calloc(reg.re_nsub, sizeof *pmatch);

	result = regexec(&reg, msg_str, reg.re_nsub, pmatch, 0);

	if (result == REG_NOMATCH) {
		goto err;
	} else if (result != 0) {
		regerror(result, &reg, pom, BUFSIZ);
		printf("Error:  %s\n", pom);
		goto err;
	}

	for (i = 0; i < reg.re_nsub; i++) {

		if (pmatch[i].rm_so == -1) {
			printf("no match!\n");
			tmp = NULL;
			continue;
		}

//		tmp = strndup(urlstr + pmatch[i].rm_so,
//			pmatch[i].rm_eo - pmatch[i].rm_so);

		switch (i) {
		case MG_DATE:
			tmp = chat_msg->date;
//			fprintf(stderr, "FUCK: %s\n", tmp);
			wprintw(win, "FUCK\n");
			fprintf(stderr, "FUCK\n");
			tmp_size = sizeof *chat_msg->date;
			break;
		case MG_TIME:
			tmp = chat_msg->time;
			tmp_size = sizeof *chat_msg->time;
			fprintf(stderr, "FUCK\n");
			break;
		case MG_USER:
			tmp = chat_msg->user;
			tmp_size = sizeof *chat_msg->user;
			fprintf(stderr, "FUCK\n");
			break;
		case MG_MESG:
			tmp = chat_msg->mesg;
			tmp_size = sizeof *chat_msg->mesg;
			fprintf(stderr, "FUCK\n");
			break;
		default:
			break;
		}
		fprintf(stderr, "FUCK\n");

		strlcpy(tmp, msg_str + pmatch[i].rm_so, tmp_size);
	}

 err:

	regfree(&reg);
}
