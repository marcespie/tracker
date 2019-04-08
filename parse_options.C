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
#include <list>
#include <iostream>

#include "parse_options.h"
#include "extern.h"
#include "autoinit.h"


enum class state {
	LOOK_FOR_ARG, 	// beginning of an argument, interspace
	QUOTED_ARG, 	// argument with quote, 
			// looking for corresponding quote
	IN_ARG }; 	// inside normal argument

int 
string2args(char *s, char *v[])
{
	char c;

	auto mode = state::LOOK_FOR_ARG;
	int j = 0;

	while (*s) {
		switch(mode) {
		case state::LOOK_FOR_ARG:
			switch(*s) {
			case ' ': case '\t':
				break;
			case '\\':
				if (v != nullptr)
					v[j] = s;
				j++;
				if (! *++s)	/* check for last char */
					return j;
				mode = state::IN_ARG;
				break;
			case '"': case '\'':
				c = *s;
				if (v != nullptr)
					v[j] = s+1;
				j++;
				mode = state::QUOTED_ARG;
				break;
			default:
				if (v != nullptr)
					v[j] = s;
				j++;
				mode = state::IN_ARG;
				break;
			}
			break;
		case state::IN_ARG:
			switch(*s) {
			case ' ': case '\t':
				if (v != nullptr)
					*s = 0;
				mode = state::LOOK_FOR_ARG;
				break;
			case '\\':
				if (! *++s)
					return j;
				break;
			default:
				break;
			}
			break;
		case state::QUOTED_ARG:
			if (*s == '\\') {
				if (! *++s)
					return j;
			} else if (*s == c) {
				if (v != NULL)
					*s = 0;
				mode = state::LOOK_FOR_ARG;
			}
			break;
		}
		s++;
	}
	return j;
}

void
option::finish_setup(const char* optiontext)
{
	switch (type) {
	case 's':
	case 'n':
		arg = def_scalar;
		break;
	case 'a':
		arg = def_string;
		break;
	case 'm':
		multi = def_string;
		break;
	default:
		std::cerr << "Internal problem with option:" << 
		    optiontext << "\n";
	break;
	}
}

int 
option_set::do1(const char *text, const char *arg)
{
	int j;
	const char *argindex;
	int type;

	auto& set = *this;

	for (auto& [key, opt]: set.options) {
		for (j = 0; key[j] && (key[j] == tolower(text[j])); j++)
			;
		if (opt.type == 'm')
			argindex = opt.multi;
		else
			argindex = key;
		type = opt.type;
		if (text[j]) {
			/* last chance for switches */
			if (type == 's'
			    && tolower(text[0]) == 'n'
			    && tolower(text[1]) == 'o') {
				for (j = 0; 
				    key[j] && 
				    key[j] == tolower(text[j+2]); j++)
					;
				if (!text[j+2]) {
					if (key == argindex)
						opt.arg = 0;
					else
						set[argindex].arg = 1; 
					return 0;
				}
			}
		} else {
			/* found option */
			switch(type) {
			case 's':
			case 'm':
				if (key == argindex)
					opt.arg = 1;
				else
					set[argindex].arg = 0; 
				return 0;
			case 'n':
				if (int d; arg && sscanf(arg, "%d", &d) == 1) {
					set[argindex].arg = d;
					return 1;
				} else {
					set[argindex].arg = opt.def_scalar;
					return 0;
				}
			case 'a':
				if (arg && (arg[0] != '-')) {
					set[argindex].arg = arg;
					return 1;
				} else {
					set[argindex].arg = opt.def_string;
					return 0;
				}
			}
		}
	}
	fprintf(stderr, "Unknown option: %s\n", text);
	return 0;
}
					

					
void 
option_set::parse(int argc, char *argv[], void (*what_to_do)(const char *arg))
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
				what_to_do(argv[i]);
				what_to_do(arg);
				i++;
			} else
				i += do1(argv[i]+1, arg);
		} else
			what_to_do(argv[i]);
	}
}

template<class iterator>
void 
option_set::parse(iterator b, iterator e, void (*what_to_do)(const char *arg))
{
	const char *arg = nullptr;
	for (auto i = b; i != e; ++i) {
		const char *s = *i;
		if (s[0] == '-' && s[1] != 0) {
			if (i+1 != e)
				arg = *(i+1);
			else
				arg = nullptr;
			if (s[1] == '-') {
				what_to_do(s);
				what_to_do(arg);
				i++;
			} else
				i += do1(s+1, arg);
		} else
			what_to_do(s);
	}
}

template
void option_set::parse<char**>(char **b, char **e, void (*what_to_do)(const char *args));
