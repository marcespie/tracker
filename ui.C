/* ui.C */
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

/* Set terminal discipline to non blocking io and such.
 */

#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/termios.h>	/* this should work on all posix hosts */
#include <cerrno>
#include <unistd.h>

#include "prefs.h"
#include "autoinit.h"
#include "timing.h"
#include "ui.h"
#include "color.h"
#include "audio.h"

extern char *VERSION;

using TERM_SETUP=termios;


static void nonblocking_io();
static void sane_tty(void);

static auto INIT = nonblocking_io;


/* poor man's timer */
static unsigned int current_pattern;
static int count_pattern, count_song;
const auto SMALL_DELAY=75;

static TERM_SETUP sanity;
static TERM_SETUP *psanity = nullptr;

static bool is_fg;

/* signal handler */

static void 
goodbye(int sig)
{
	static char buffer[25];
	char *pter = buffer;

	if (pref::get(Pref::color))
		pter = write_color(pter, 0);
	*pter = 0;
	End() << pter << "Signal " << sig;;
}

static void 
abort_this(int)
{
	End() << "Abort";
}

#ifdef SIGTSTP
static void 
suspend(int)
{
	static char buffer[25];
	char *buf = buffer;

	if (pref::get(Pref::color))
		buf = write_color(buf, 0);
	*buf = 0;
	puts(buf);
	fflush(stdout);
	sane_tty();
	(void)signal(SIGTSTP, SIG_DFL);
	kill(0, SIGTSTP);
}
#endif

bool run_in_fg();

bool 
run_in_fg()
{
	pid_t val;

	if (!isatty(0) || !isatty(1))
		return false;

	val = tcgetpgrp(0);
	if (val == -1) {
		if (errno == ENOTTY)
			return false;
		else
			return true;
	}
	if (val == getpgrp())
		return true;
	else
		return false;
}

/* if_fg_sane_tty():
 * restore tty modes, _only_ if running in foreground
 */
static void 
if_fg_sane_tty()
{
	if (run_in_fg())
		sane_tty();
}


static void switch_mode(int)
{
	TERM_SETUP zap;

#ifdef SIGTSTP
	(void)signal(SIGTSTP, suspend);
#endif
#ifdef SIGCONT
	(void)signal(SIGCONT, switch_mode);
#endif
	(void)signal(SIGINT, goodbye);
	(void)signal(SIGQUIT, goodbye);
	(void)signal(SIGUSR1, abort_this);

	is_fg = run_in_fg();
	if (is_fg) {
		tcgetattr(0, &zap);
		zap.c_cc[VMIN] = 0;     /* can't work with old */
		zap.c_cc[VTIME] = 0; /* FreeBSD versions    */
		zap.c_lflag &= ~(ICANON|ECHO|ECHONL);
		tcsetattr(0, TCSADRAIN, &zap);

	}
}

/* nonblocking_io():
 * try to setup the keyboard to non blocking io
 */
static void 
nonblocking_io()
{
	if (!psanity) {
		psanity = &sanity;
		tcgetattr(0, psanity);
	}
	switch_mode(0);
	at_end(if_fg_sane_tty);
}


/* sane_tty():
 * restores everything to a sane state before returning to shell */
static void 
sane_tty()
{
	tcsetattr(0, TCSADRAIN, psanity);
}

static int 
may_getchar()
{
	char buffer;

	INIT_ONCE;

	if (run_in_fg() && !is_fg)
		switch_mode(0);
	if (run_in_fg() && read(fileno(stdin), &buffer, 1))
		return buffer;
	return EOF;
}

static int ntracks;
audio* audio_p;

void 
set_number_tracks(int n, audio& a)
{
	audio_p = &a;
	ntracks = n;
}

inline auto 
result(int type =0, unsigned long value =0)
{
	return std::pair(type, value);
}

std::pair<int, unsigned long>
get_ui()
{
	int c;

	count_pattern++;
	count_song++;
	switch(c = may_getchar()) {
	case 'n':
		return result(UI_NEXT_SONG);
	case 'p':
		int t;
		if (count_song > SMALL_DELAY)
			t = UI_RESTART;
		else
			t = UI_PREVIOUS_SONG;
		count_song = 0;
		return result(t);
	case 'x':
	case 'e':
	case 'q':
		return result(UI_QUIT);
	case 'r':
		if (pref::get(Pref::repeats))
			pref::set(Pref::repeats, 0);
		else
			pref::set(Pref::repeats, 1);
		break;

	case 'm':
		audio_p->set_mix(100);
		break;
	case 'M':
		audio_p->set_mix(30);
		break;
	case 's':
		return result(UI_SET_BPM, 50);
	case 'S':
		return result(UI_SET_BPM, 60);
	case '>':
		return result(UI_JUMP_TO_PATTERN, current_pattern + 1);
	case '<':
		{
		int p = current_pattern;
		if (count_song < SMALL_DELAY)
			p--;
		count_song = 0;
		return result(UI_JUMP_TO_PATTERN, p);
		}
	case '?':
		pref::set(Pref::show, !pref::get(Pref::show));
		if (pref::get(Pref::show))
			putchar('\n');
		break;
	default:
		break;
	}
	return result();
}
      
void 
status(const std::string& s)
{
	if (run_in_fg()) {
		std::cout << s << "\n";
	}
}

static char title[25];
void 
song_title(const char *s)
{
	int i;

	for (i = 0; *s && i < 24; s++)
		if (isprint(*s & 0x7f))
			title[i++] = *s;
	title[i] = 0;
	count_song = 0;
}


Info::Info(const char*): fg{run_in_fg()}, out{std::cout}
{
}


static char scroll_line[2000];

char *
new_scroll()
{
	scroll_line[0] = 0;
	return scroll_line;
}
   
static void 
do_scroll(char *line)
{
	if (run_in_fg()) {
		puts(line);
		fflush(stdout);
	}
	delete[] line;
}

void 
scroll(char *end)
{
	if (run_in_fg()) {
		size_t t;
		char *p;

		t = end - scroll_line;
		if (t > 0) {
			p = new char[t+1];
			strncpy(p, scroll_line, t);
			p[t] = 0;
			audio_p->sync(
			    [p](){do_scroll(p);},
			    [p](){delete[] p;}
			);
		}
	}
}

static void 
do_display_pattern(unsigned int current, unsigned int total, 
    unsigned int real, unsigned long uptilnow, unsigned long totaltime)
{
	char buf0[50], buf1[50];

	if (run_in_fg()) {
		if (pref::get(Pref::xterm)) {
			if (pref::get(Pref::show)) {
				printf("\x1b]2;V%s %3u/%3u[%3u] %s/%s %s \007", VERSION,
				    current, total, real, time2string(buf0, uptilnow),
				    time2string(buf1, totaltime), title);
				for (int i = 0; i < ntracks; i++)
					std::cout << "--------------";
				if (ntracks != 0)
					std::cout << "--";
				std::cout << "\n";
			} else {
				printf("\x1b]2;V%s %3u/%3u %s/%s %s\007", VERSION, 
				    current, total, time2string(buf0, uptilnow),
				    time2string(buf1, totaltime), title);
				fflush(stdout);
			}
		} else {
			if (pref::get(Pref::show))
				printf("\n%3u/%3u[%3u] %s\n", current, total, real, title);
			else
				printf("%3u/%3u\b\b\b\b\b\b\b", current, total);
			fflush(stdout); 
		}
	}
	current_pattern = current;
	count_pattern = 0;
}

void 
display_pattern(unsigned int current, unsigned int total, 
    unsigned int real, unsigned long uptilnow, unsigned long totaltime)
{
	audio_p->sync(
	    [=]() 
	    {
		do_display_pattern(current, total, real, 
		    uptilnow, totaltime);
	    }, 
	    []() {});
}

static void 
do_display_time(unsigned long param)
{
	char buffer[50];
	printf("%s\n", time2string(buffer, param));
}

void 
display_time(unsigned long time, unsigned long check)
{
	if (time/1000 != check/1000) {
		audio_p->sync(
		   [check]() {do_display_time(check); },
		   []() {});
		if (time > check)
			audio_p->sync(
			    [time,check]() {do_display_time(time-check);},
			    []() {});
		else
			audio_p->sync(
			    [time,check]() {do_display_time(check-time);},
			    []() {});
	}
}
