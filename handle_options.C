/* handle_options.c */

#include "defs.h"
#include "extern.h"
#include <ctype.h>

#include "parse_options.h"
#include "watched_var.h"
#include "tags.h"
#include "prefs.h"
#include "autoinit.h"
#include "open.h"


extern void print_usage(void);
extern struct option_set *port_options;

unsigned long half_mask = 0;
int ask_freq;		/* parameters for setup audio */
int stereo;

int start;			/* parameters for st_play */
int trandom;

int loop = false;	/* main automaton looping at end of argv ? */

static struct option opts[] =
	{
	{"help", 's', 0},					/* 0 */
	{"frequency", 'n', 0},			/* 1 */
	{"stereo", 's', 1},				/* 2 */
	{"loop", 's', 0},					/* 3 */
	{"oversample", 'n', 1},			/* 4 */
	{"randomize", 's', 0},			/* 5 */
	{"scroll", 's', 0},				/* 6 */
	{"picky", 'n', 1},				/* 7 */
	{"both", 'n', BOTH},				/* 8 */
	{"repeats", 'n', 1},				/* 9 */
	{"speed", 'n', 50},				/* 10 */
	{"mix", 'n', 30},					/* 11 */
	{"color", 's', 0},				/* 12 */
	{"xterm", 's', 0},				/* 13 */
	{"speedmode", 'a', 0, "normal"},	/* 14 */
	{"transpose", 'n', 0},			/* 15 */
	{"cut", 'a', 0, NULL},			/* 16 */
	{"add", 'a',0 , NULL},			/* 17 */
	{"halve", 'a', 0, NULL},		/* 18 */
	{"double", 'a', 0, NULL},		/* 19 */
	{"verbose", 's', 0},				/* 20 */
	{"start", 'n', 0},				/* 21 */
	{"list", 'a', 0, NULL},			/* 22 */
	{"output", 's', 1},				/* 23 */
	{"mono", 'm', 0, "stereo"},
	{"bw", 'm', 0, "color"},
	{"tolerant", 'm', 2, "picky"},
	{"old", 'm', OLD, "both"},
	{"new", 'm', NEW, "both"},
	{"pal", 'm', 50, "speed"},
	{"ntsc", 'm', 60, "speed"},
	};





VALUE args[24];

static struct option_set set =
	{ opts, sizeof(opts)/sizeof(struct option), args};


/* initialize all options to default values */
void 
set_default_prefs(void)
{
	char *s;

	set_pref_scalar(PREF_IMASK, 0);
	set_pref_scalar(PREF_BCDVOL, 0);

	/* XXX */
	s = getenv("TERM");
	if (s && (strncmp(s, "xterm", 5) == 0 || strncmp(s, "kterm", 5) == 0 
	    || strncmp(s, "cxterm", 6) == 0) )
		opts[13].def_scalar = 1;
	else
		opts[13].def_scalar = 0;
}

static unsigned long 
get_mask(char *s)
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
read_line(exfile *f)
{
	size_t i;
	int c;

	i = 0;
	while (((c = getc_file(f)) != EOF) && (c != '\n')) {
		if (i < MAXLINELENGTH)
			linebuf[i++] = c;
	}
	if (c == EOF)
		return 0;
	else {
		char *s;

		s = (char *)malloc(i+1);
		strncpy(s, linebuf, i);
		s[i] = 0;
		return s;
	}
}

static void
set_speed_mode(void *p)
{
	char *check;
	int mode;

	check = reinterpret_cast<char *>(p);
	if (stricmp(check, "normal") == 0)
		mode = NORMAL_SPEEDMODE;
	else if (stricmp(check, "finespeed") == 0)
		mode = FINESPEED_ONLY;
	else if (stricmp(check, "speed") == 0)
		mode = SPEED_ONLY;
	else if (stricmp(check, "old") == 0)
		mode = OLD_SPEEDMODE;
	else if (stricmp(check, "vblank") == 0)
		mode = OLD_SPEEDMODE;
	else if (stricmp(check, "alter") == 0)
		mode = ALTER_PROTRACKER;
	else
		end_all("Unknwon speedmode");
		/* NOTREACHED */
	set_pref_scalar(PREF_SPEEDMODE, mode);
}

void 
handle_options(int argc, char *argv[])
{
	char *s;

	add_option_set(&set);
	if (port_options)
		add_option_set(port_options);
	if ((s = getenv("TRACKER_DEFAULTS")) != NULL) {
		int t;
		char **v;

		t = string2args(s, 0);
		v = (char **)malloc(sizeof(char *) * t);
		string2args(s, v);
		parse_options(t, v, add_play_list);
		free(v);
	}

	parse_options(argc, argv, add_play_list);
	if (args[0].scalar) {
		print_usage();
		end_all(0);
	}
	ask_freq = args[1].scalar;
	if (ask_freq < 1000)
		ask_freq *= 1000;
	stereo = args[2].scalar;
	loop = args[3].scalar;
	set_watched_scalar(OVERSAMPLE, args[4].scalar);
	trandom = args[5].scalar;
	set_pref_scalar(PREF_SHOW, args[6].scalar);
	set_pref_scalar(PREF_TOLERATE, args[7].scalar);
	set_pref_scalar(PREF_TYPE, args[8].scalar);
	set_pref_scalar(PREF_REPEATS, args[9].scalar);
	set_pref_scalar(PREF_SPEED, args[10].scalar);
	set_mix(args[11].scalar);
	set_pref_scalar(PREF_COLOR, args[12].scalar);
	set_pref_scalar(PREF_XTERM, args[13].scalar);

	set_speed_mode(args[14].pointer);

	set_pref_scalar(PREF_TRANSPOSE, args[15].scalar);
	if (args[16].pointer)
		set_pref_scalar(PREF_IMASK, get_mask((char*)args[16].pointer));
	else if (args[17].pointer)
		set_pref_scalar(PREF_IMASK, ~get_mask((char*)args[17].pointer));
	if (args[18].pointer)
		half_mask = get_mask((char*)args[18].pointer);
	else if (args[19].pointer)
		half_mask = ~get_mask((char*)args[19].pointer);
	set_pref_scalar(PREF_DUMP, args[20].scalar);
	start = args[21].scalar;
	if (args[22].pointer) {
		char *s;

		exfile *file = open_file((char*)args[22].pointer, "r", 0);
		if (!file)
			end_all("List file does not exist");
		else
			while ((s = read_line(file)))
				add_play_list(s);
		close_file(file);
	}
	set_pref_scalar(PREF_OUTPUT, args[23].scalar);
}
