/* handle_options.c */
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

#include "extern.h"
#include <ctype.h>

#include "parse_options.h"
#include "watched_var.h"
#include "prefs.h"
#include "autoinit.h"
#include "open.h"

inline int stricmp(const char *a, const char *b)
{
	return strcasecmp(a, b);
}

extern void print_usage(void);
extern struct option_set *port_options;

unsigned long half_mask = 0;
int ask_freq;		/* parameters for setup audio */
int stereo;

int start;			/* parameters for st_play */
int trandom;

bool loop = false;	/* main automaton looping at end of argv ? */

static option opts[] = {
	{"help", 's'},				/* 0 */
	{"frequency", 'n'},			/* 1 */
	{"stereo", 's', 1},			/* 2 */
	{"loop", 's'},				/* 3 */
	{"oversample", 'n', 1},			/* 4 */
	{"randomize", 's'},			/* 5 */
	{"scroll", 's', 1},			/* 6 */
	{"picky", 'n', 1},			/* 7 */
	{"both", 'n', BOTH},			/* 8 */
	{"repeats", 'n', 1},			/* 9 */
	{"speed", 'n', 50},			/* 10 */
	{"mix", 'n', 30},			/* 11 */
	{"color", 's', 1},			/* 12 */
	{"xterm", 's'},				/* 13 */
	{"speedmode", 'a', 0, "normal"},	/* 14 */
	{"transpose", 'n'},			/* 15 */
	{"cut", 'a'},				/* 16 */
	{"add", 'a'},				/* 17 */
	{"halve", 'a'},				/* 18 */
	{"double", 'a'},			/* 19 */
	{"verbose", 's'},			/* 20 */
	{"start", 'n'},				/* 21 */
	{"list", 'a'},				/* 22 */
	{"output", 's', 1},			/* 23 */
	{"mono", 'm', 0, "stereo"},
	{"bw", 'm', 0, "color"},
	{"tolerant", 'm', 2, "picky"},
	{"old", 'm', OLD, "both"},
	{"new", 'm', NEW, "both"},
	{"pal", 'm', 50, "speed"},
	{"ntsc", 'm', 60, "speed"},
};

struct option_set args = {std::begin(opts), std::end(opts)};


/* initialize all options to default values */
void 
set_default_prefs(void)
{
	char *s;

	set_pref(Pref::imask, 0);
	set_pref(Pref::bcdvol, 0);

	/* XXX */
	s = getenv("TERM");
	if (s && (strncmp(s, "xterm", 5) == 0 || strncmp(s, "kterm", 5) == 0 
	    || strncmp(s, "cxterm", 6) == 0) )
		args.options["xterm"]->def_scalar = 1;
	else
		args.options["xterm"]->def_scalar = 0;
}

static unsigned long 
get_mask(const char *s)
{
	char c;
	unsigned long mask = 0;

	while ((c = *s++)) {			
		/* this is not ANSI and depends on the char set being
		 * ASCII-like contiguous
		 */
		if (c >= '1' && c <= '9')
			mask |= 1 << (c-'0');
		else if (c >= 'a' && c <= 'z')
			mask |= 1 << (c-'a'+10);
		else if (c >= 'A' && c <= 'Z')
			mask |= 1 << (c-'A'+10);
	}
	return mask;
}

const auto MAXLINELENGTH=200;
static char linebuf[MAXLINELENGTH+1];

static char *
read_line(exfile& f)
{
	size_t i;
	int c;

	i = 0;
	while (((c = f.getc()) != EOF) && (c != '\n')) {
		if (i < MAXLINELENGTH)
			linebuf[i++] = c;
	}
	if (c == EOF)
		return nullptr;
	else {
		char *s = (char *)malloc(i+1);
		memcpy(s, linebuf, i);
		s[i] = 0;
		return s;
	}
}

static void
set_speed_mode(const char *p)
{
	int mode;

	if (stricmp(p, "normal") == 0)
		mode = NORMAL_SPEEDMODE;
	else if (stricmp(p, "finespeed") == 0)
		mode = FINESPEED_ONLY;
	else if (stricmp(p, "speed") == 0)
		mode = SPEED_ONLY;
	else if (stricmp(p, "old") == 0)
		mode = OLD_SPEEDMODE;
	else if (stricmp(p, "vblank") == 0)
		mode = OLD_SPEEDMODE;
	else if (stricmp(p, "alter") == 0)
		mode = ALTER_PROTRACKER;
	else {
		end_all("Unknwon speedmode");
		return;
	}
	set_pref(Pref::speedmode, mode);
}

#include <iostream>
void 
handle_options(int argc, char *argv[])
{
	add_option_set(args);
	if (port_options)
		add_option_set(*port_options);
	if (auto s = getenv("TRACKER_DEFAULTS"); s != nullptr) {

		auto t = string2args(s, nullptr);
		auto v = new char *[t];
		string2args(s, v);
		parse_options(t, v, add_play_list);
		delete []v;
	}

	parse_options(argc, argv, add_play_list);
	if (args.get_long("help")) {
		print_usage();
		end_all(0);
	}
	ask_freq = args.get_long("frequency");
	if (ask_freq < 1000)
		ask_freq *= 1000;
	stereo = args.get_long("stereo");
	loop = args.get_long("loop");
	set_watched(watched::oversample, args.get_long("oversample"));
	trandom = args.get_long("randomize");
	set_pref(Pref::show, args.get_long("scroll"));
	set_pref(Pref::tolerate, args.get_long("picky"));
	set_pref(Pref::type, args.get_long("both"));
	set_pref(Pref::repeats, args.get_long("repeats"));
	set_pref(Pref::speed, args.get_long("speed"));
	set_mix(args.get_long("mix"));
	set_pref(Pref::color, args.get_long("color"));
	set_pref(Pref::xterm, args.get_long("xterm"));

	set_speed_mode(args.get_string("speedmode"));

	set_pref(Pref::transpose, args.get_long("transpose"));
	if (args.get_string("cut"))
		set_pref(Pref::imask, get_mask(args.get_string("cut")));
	else if (args.get_string("add"))
		set_pref(Pref::imask, ~get_mask(args.get_string("add")));
	if (args.get_string("halve"))
		half_mask = get_mask(args.get_string("halve"));
	else if (args.get_string("double"))
		half_mask = ~get_mask(args.get_string("double"));
	set_pref(Pref::dump, args.get_long("verbose"));
	start = args.get_long("start");
	if (args.get_string("list")) {
		char *s;
		exfile file;

		if (!file.open(args.get_string("list")))
			end_all("List file does not exist");
		while ((s = read_line(file)) != nullptr)
			add_play_list(s);
	}
	set_pref(Pref::output, args.get_long("output"));
}
