/* parse_options.c */
/*
 * Copyright (c) 2019 Marc Espie <espie@openbsd.org>
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

/* stuff to put options into argc/argv form
 * (and parse them in the future)
 * not used yet...
 */
#include "parse_options.h"
#include "extern.h"
#include "autoinit.h"
#include <ctype.h>


#define LOOK_FOR_ARG 0	/* beginning of an argument, interspace */
#define QUOTED_ARG 1	/* argument with quote, looking for corresponding
                         * quote */
#define IN_ARG 2        /* inside normal argument */

int 
string2args(char *s, char *v[])
{
	char c;

	int mode = LOOK_FOR_ARG;
	int j = 0;

	while (*s) {
		switch(mode) {
		case LOOK_FOR_ARG:
			switch(*s) {
			case ' ': case '\t':
				break;
			case '\\':
				if (v != nullptr)
					v[j] = s;
				j++;
				if (! *++s)	/* check for last char */
					return j;
				mode = IN_ARG;
				break;
			case '"': case '\'':
				c = *s;
				if (v != nullptr)
					v[j] = s+1;
				j++;
				mode = QUOTED_ARG;
				break;
			default:
				if (v != nullptr)
					v[j] = s;
				j++;
				mode = IN_ARG;
				break;
			}
			break;
		case IN_ARG:
			switch(*s) {
			case ' ': case '\t':
				if (v != nullptr)
					*s = 0;
				mode = LOOK_FOR_ARG;
				break;
			case '\\':
				if (! *++s)
					return j;
				break;
			default:
				break;
			}
			break;
		case QUOTED_ARG:
			if (*s == '\\') {
				if (! *++s)
					return j;
			} else if (*s == c) {
				if (v != NULL)
					*s = 0;
				mode = LOOK_FOR_ARG;
			}
			break;
		}
		s++;
	}
	return j;
}

static struct option_set_list {
	struct option_set_list *next;
	struct option_set set;
} *first, *current;

static void 
set_up_args(struct option_set *set)
{
	for (int i = 0; i < set->number; i++) {
		switch (set->options[i].type) {
		case 's':
		case 'n':
			set->args[i] = set->options[i].def_scalar;
			break;
		case 'a':
			set->args[i] = set->options[i].def_string;
			break;
		case 'm':
			for (int j = 0; j < i; j++) {
			    if (strcmp(set->options[i].def_string, 
				set->options[j].optiontext) == 0) {
				    set->options[i].multi = j;
				    break;
			    }
			}
			break;
		default:
			notice("Internal problem with option:");
			notice(set->options[i].optiontext);
		break;
		}
	}
}

void 
add_option_set(struct option_set *set)
{
	struct option_set_list *n =
	(struct option_set_list *)malloc(sizeof(struct option_set_list));
	if (!n)
		end_all("Error: out of memory");			
	n->next = nullptr;
	n->set.options = set->options;
	n->set.number = set->number;
	n->set.args = set->args;
	set_up_args(set);
	if (current)
		current->next = n;
	else
		first = n;
	current = n;
}

static int 
do_option(char *text, char *arg)
{
	struct option_set_list *sweep;
	struct option_set *set;
	int i, j;
	const char *check;
	int argindex;
	int type;

	for (sweep = first; sweep; sweep = sweep->next) {
		set = &(sweep->set);
		for (i = 0; i < set->number; i++) {
			check = set->options[i].optiontext;

			for (j = 0; check[j] && (check[j] == tolower(text[j])); j++)
				;
			if (set->options[i].type == 'm')
				argindex = set->options[i].multi;
			else
				argindex = i;
			type = set->options[argindex].type;
			if (text[j]) {
				/* last chance for switches */
				if (type == 's'
				    && tolower(text[0]) == 'n'
				    && tolower(text[1]) == 'o') {
					for (j = 0; 
					    check[j] && 
					    check[j] == tolower(text[j+2]); j++)
						;
					if (!text[j+2]) {
						if (i == argindex)
							set->args[i] = 0;
						else
							set->args[argindex] = 1; 
						return 0;
					}
				}
			} else {
				/* found option */
				switch(type) {
				case 's':
				case 'm':
					if (i == argindex)
						set->args[argindex] = 1;
					else
						set->args[argindex] = 0;
					return 0;
				case 'n':
					if (int d; arg && sscanf(arg, "%d", &d) == 1) {
						set->args[argindex] = d;
						return 1;
					} else {
						set->args[argindex] = 
						    set->options[i].def_scalar;
						return 0;
					}
				case 'a':
					if (arg && (arg[0] != '-')) {
						set->args[argindex] = arg;
						return 1;
					} else {
						set->args[argindex] = 
						    set->options[i].def_string;
						return 0;
					}
				}
			}
		}
	}
	fprintf(stderr, "Unknown option: %s\n", text);
	return 0;
}
					

					
void 
parse_options(int argc, char *argv[], void (*what_to_do)(const char *arg))
{
	int i;
	char *arg;
	for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] != 0) {
			if (i+1 < argc)
				arg = argv[i+1];
			else
				arg = nullptr;
			if (argv[i][1] == '-') {
				(*what_to_do)(argv[i]);
				(*what_to_do)(arg);
				i++;
			} else
				i += do_option(argv[i]+1, arg);
		} else
			(*what_to_do)(argv[i]);
	}
}
