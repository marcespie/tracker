/* unix/ui.c */

/* Set terminal discipline to non blocking io and such.
 */

#include <signal.h>
#include <ctype.h>

#include "defs.h"
#include "extern.h"
#include "tags.h"
#include "prefs.h"
#include "autoinit.h"
#include "timing.h"

extern char *VERSION;

#include <sys/types.h>
#include <sys/termios.h>	/* this should work on all posix hosts */
#include <errno.h>

using TERM_SETUP=termios;

#include <unistd.h>

static void nonblocking_io(void);
static void sane_tty(void);

static void (*INIT)(void) = nonblocking_io;


/* poor man's timer */
static unsigned int current_pattern;
static int count_pattern, count_song;
#define SMALL_DELAY 75

static TERM_SETUP sanity;
static TERM_SETUP *psanity = nullptr;

static bool is_fg;

/* signal handler */

static void 
goodbye(int sig)
{
	static char buffer[25];
	char *pter = buffer;

	if (get_pref_scalar(PREF_COLOR))
		pter = write_color(pter, 0);
	sprintf(pter, "Signal %d", sig);
	end_all(pter);
}

static void 
abort_this(int)
{
	end_all("Abort");
}

#ifdef SIGTSTP
static void 
suspend(int)
{
	static char buffer[25];
	char *buf = buffer;

	if (get_pref_scalar(PREF_COLOR))
		buf = write_color(buf, 0);
	*buf = 0;
	puts(buf);
	fflush(stdout);
	sane_tty();
	(void)signal(SIGTSTP, SIG_DFL);
	kill(0, SIGTSTP);
}
#endif

bool 
run_in_fg(void)
{
	pid_t val;

	/* this should work on every unix */
	if (!isatty(fileno(stdin)) || !isatty(fileno(stdout)))
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
if_fg_sane_tty(void)
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
nonblocking_io(void)
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
sane_tty(void)
{
	tcsetattr(0, TCSADRAIN, psanity);
}

static int 
may_getchar(void)
{
	char buffer;

	INIT_ONCE;

	if (run_in_fg() && !is_fg)
		switch_mode(0);
	if (run_in_fg() && read(fileno(stdin), &buffer, 1))
		return buffer;
	return EOF;
}

static tag result[2];

tag *
get_ui(void)
{
	int c;

	result[0].type = TAG_END;
	result[1].type = TAG_END;
	count_pattern++;
	count_song++;
	switch(c = may_getchar()) {
	case 'n':
		result[0].type = UI_NEXT_SONG;
		break;
	case 'p':
		if (count_song > SMALL_DELAY)
			result[0].type = UI_RESTART;
		else
			result[0].type = UI_PREVIOUS_SONG;
		count_song = 0;
		break;
	case 'x':
	case 'e':
	case 'q':
		result[0].type = UI_QUIT;
		break;
	case 'r':
		if (get_pref_scalar(PREF_REPEATS))
			set_pref_scalar(PREF_REPEATS, 0);
		else
			set_pref_scalar(PREF_REPEATS, 1);
		break;

	case 'm':
		set_mix(100);
		break;
	case 'M':
		set_mix(30);
		break;
	case 's':
		result[0].type = UI_SET_BPM;
		result[0].data.scalar = 50;
		break;
	case 'S':
		result[0].type = UI_SET_BPM;
		result[0].data.scalar = 60;
		break;
	case '>':
		result[0].type = UI_JUMP_TO_PATTERN;
		result[0].data.scalar = current_pattern + 1;
		break;
	case '<':
		result[0].type = UI_JUMP_TO_PATTERN;
		result[0].data.scalar = current_pattern;
		if (count_pattern < SMALL_DELAY)
			result[0].data.scalar--;
		break;
	case '?':
		set_pref_scalar(PREF_SHOW, !get_pref_scalar(PREF_SHOW));
		if (get_pref_scalar(PREF_SHOW))
			putchar('\n');
		break;
	default:
		audio_ui(c);
		break;
	}
	return result;
}
      
void 
notice(const char *fmt, ...)
{
	va_list al;

	va_start(al, fmt);
	vfprintf(stderr, fmt, al);
	va_end(al);
}

void 
vnotice(const char *fmt, va_list al)
{
	vfprintf(stderr, fmt, al);
	fputc('\n', stderr);
}

void 
status(const char *s)
{
	if (run_in_fg()) {
		if (s) {
			puts(s);
		} else
			putchar('\n');
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



static char scroll_buffer[200];

GENERIC 
begin_info(const char *)
{
	if (run_in_fg())
		return scroll_buffer;
	else
		return nullptr;
}

void infos(void *handle, const char *s)
{
	if (handle)
		fputs(s, stdout);
}

void 
info(void *handle, const char *line)
{
	if (handle)
		puts(line);
}

void 
end_info(void *handle)
{
	if (handle)
		fflush(stdout);
}

static int ntracks;

void 
set_number_tracks(int n)
{
	ntracks = n;
}

static char scroll_line[2000];

char *
new_scroll(void)
{
	scroll_line[0] = 0;
	return scroll_line;
}
   
static void 
do_scroll(void *line)
{
	if (run_in_fg()) {
		puts((char *)line);
		fflush(stdout);
	}
	free(line);
}

static void free_p(void *line)
{
	free(line);
}

void 
scroll(char *end)
{
	if (run_in_fg()) {
		size_t t;
		char *p;

		t = end - scroll_line;
		if (t > 0) {
			p = (char *)malloc(t+1);
			strncpy(p, scroll_line, t);
			p[t] = 0;
			sync_audio(do_scroll, free_p, p);
		}
	}
}

struct Thingy {
	unsigned t0, t1, t2;
	unsigned long u0, u1;
};

static void 
do_display_pattern(void *param)
{
	Thingy *thingy = (Thingy *)param;
	unsigned current, total, real;
	unsigned long uptilnow, totaltime;
	char buf0[50], buf1[50];
	current = thingy->t0;
	total = thingy->t1;
	real = thingy->t2;
	uptilnow = thingy->u0;
	totaltime =thingy->u1;
	free(thingy);

	if (run_in_fg()) {
		if (get_pref_scalar(PREF_XTERM)) {
			if (get_pref_scalar(PREF_SHOW)) {
				for (int i = 0; i < ntracks; i++)
					printf("--------------");
				putchar('\n');
				printf("\x1b]2;V%s %3u/%3u[%3u] %s/%s %s \007", VERSION,
				    current, total, real, time2string(buf0, uptilnow),
				    time2string(buf1, totaltime), title);
				fflush(stdout);
			} else {
				printf("\x1b]2;V%s %3u/%3u %s/%s %s\007", VERSION, 
				    current, total, time2string(buf0, uptilnow),
				    time2string(buf1, totaltime), title);
				fflush(stdout);
			}
		} else {
			if (get_pref_scalar(PREF_SHOW))
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
	Thingy *thingy;

	thingy = (Thingy *)malloc(sizeof(Thingy));
	thingy->t0 = current;
	thingy->t1 = total;
	thingy->t2 = real;
	thingy->u0 = uptilnow;
	thingy->u1 = totaltime;
	sync_audio(do_display_pattern, free_p, thingy);
}

static void 
do_display_time(void *param)
{
	char buffer[50];
	printf("%s\n", time2string(buffer, (unsigned long)param));
}

static void 
do_nuts(void *)
{
}

void 
display_time(unsigned long time, unsigned long check)
{
	if (time/1000 != check/1000) {
		sync_audio(do_display_time, do_nuts, (GENERIC)check);
		if (time > check)
			sync_audio(do_display_time, do_nuts, (GENERIC)(time - check));
		else
			sync_audio(do_display_time, do_nuts, (GENERIC)(check - time));
	}
}

bool 
checkbrk(void)
{
	return false;
}