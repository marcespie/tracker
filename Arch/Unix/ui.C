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

#ifdef OSK
#include <sgstat.h>
typedef struct sgbug TERM_SETUP;
#else
#ifdef USE_TERMIOS
#define IOCTL_IS_REDUNDANT
#endif

#ifndef IOCTL_IS_REDUNDANT
#include <sys/ioctl.h> 
#endif
#endif

#ifdef USE_TERMIOS
typedef struct termios TERM_SETUP;
#endif

#ifdef USE_SGTTY
typedef struct sgttyb TERM_SETUP;
#endif

#ifdef USE_TERMIOS
#include <sys/types.h>
#include <sys/termios.h>	/* this should work on all posix hosts */
#include <errno.h>
#endif
#ifdef USE_SGTTY				/* only NeXt does, currently */
#include <sgtty.h>
#include <fcntl.h>
#endif


#ifdef __hpux
#include <sys/bsdtty.h>
#endif
#include <unistd.h>

LOCAL void nonblocking_io(void);
LOCAL void sane_tty(void);

LOCAL void (*INIT)(void) = nonblocking_io;


/* poor man's timer */
LOCAL unsigned int current_pattern;
LOCAL int count_pattern, count_song;
#define SMALL_DELAY 75

LOCAL TERM_SETUP sanity;
LOCAL TERM_SETUP *psanity = 0;

LOCAL int is_fg;

/* signal handler */

LOCAL void goodbye(int sig)
   {
   static char buffer[25];
	char *pter = buffer;

	if (get_pref_scalar(PREF_COLOR))
		pter = write_color(pter, 0);
	sprintf(pter, "Signal %d", sig);
   end_all(pter);
   }

LOCAL void abort_this(int sig)
   {
   end_all("Abort");
   }

#ifdef SIGTSTP
LOCAL void suspend(int sig)
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

int run_in_fg(void)
   {
#ifdef USE_TERMIOS
	pid_t val;
#else
   static int val;
#endif

		/* this should work on every unix */
	if (!isatty(fileno(stdin)) || !isatty(fileno(stdout)))
		return FALSE;

#ifdef OSK
	return TRUE;
#else
   /* real check for running in foreground */
#ifdef USE_TERMIOS
	val = tcgetpgrp(fileno(stdin));
	if (val == -1)
		{
		if (errno == ENOTTY)
			return FALSE;
		else
			return TRUE;
		}
#else
   if (ioctl(fileno(stdin), TIOCGPGRP, &val))
      return FALSE;
#endif
#ifdef IS_POSIX
   if (val == getpgrp())
#else
   if (val == getpgrp(0))
#endif
      return TRUE;
   else
      return FALSE;
#endif
   }

/* if_fg_sane_tty():
 * restore tty modes, _only_ if running in foreground
 */
LOCAL void if_fg_sane_tty(void)
	{
	if (run_in_fg())
		sane_tty();
	}


LOCAL void switch_mode(int sig)
   {
	TERM_SETUP zap;

#ifdef SIGTSTP
   (void)signal(SIGTSTP, suspend);
#endif
#ifndef OSK
   (void)signal(SIGCONT, switch_mode);
#endif
   (void)signal(SIGINT, goodbye);
   (void)signal(SIGQUIT, goodbye);
   (void)signal(SIGUSR1, abort_this);

   if (run_in_fg())
      {
#ifdef USE_SGTTY
		int tty;

      tty = fileno(stdin);
      fcntl(tty, F_SETFL, fcntl(tty, F_GETFL, 0) | FNDELAY);
      ioctl(tty, TIOCGETP, &zap);
      zap.sg_flags |= CBREAK;
      zap.sg_flags &= ~ECHO;
      ioctl(tty, TIOCSETP, &zap);
#endif
#ifdef USE_TERMIOS
		tcgetattr(fileno(stdin), &zap);
      zap.c_cc[VMIN] = 0;     /* can't work with old */
      zap.c_cc[VTIME] = 0; /* FreeBSD versions    */
      zap.c_lflag &= ~(ICANON|ECHO|ECHONL);
      tcsetattr(fileno(stdin), TCSADRAIN, &zap);
#endif
#ifdef OSK
      _gs_opt(fileno(stdin), &zap);

      zap.sg_pause = 0;    /* no pause at end of page */
      zap.sg_backsp = 0;     /* nondestructive backspace */
      zap.sg_delete = 0; /* no delete sequence */
      zap.sg_echo = 0; /* don't echo chars */
      zap.sg_dlnch = 0xff; /* ^X delete line character */
      zap.sg_rlnch = 0xff; /* ^D reprint line character */
      zap.sg_dulnch = 0xff; /* ^A duplicate line character */
      zap.sg_psch = 0xff; /* ^W pause character */
      zap.sg_eofch = 0xff; /* escape key in OS-9 */
      zap.sg_kbich = 0xff; /* ^C interrupt character */
      zap.sg_kbach = 0xff; /* ^E abort character */
      /* Note ^S and ^Q are disabled! */
      zap.sg_xon = 0xff;   /* ^Q XON */
      zap.sg_xoff = 0xff;  /* ^S XOFF */

      _ss_opt(fileno(stdin), &zap);
#endif

      is_fg = TRUE;
      }
   else
      is_fg = FALSE;
   }

/* nonblocking_io():
 * try to setup the keyboard to non blocking io
 */
LOCAL void nonblocking_io(void)
   {


#if 0 /* BROKEN */
   /* try to renice our own process to get more cpu time */
   if (nice(-15) == -1)
      nice(0);
#endif


   if (!psanity)
      {
      psanity = &sanity;
#ifdef USE_SGTTY
      ioctl(fileno(stdin), TIOCGETP, psanity);
#endif
#ifdef USE_TERMIOS
      tcgetattr(fileno(stdin), psanity);
#endif
#ifdef OSK
		_gs_opt(fileno(stdin), psanity);
#endif
      }
   switch_mode(0);
   at_end(if_fg_sane_tty);
   }


/* sane_tty():
 * restores everything to a sane state before returning to shell */
LOCAL void sane_tty(void)
   {
#ifdef USE_SGTTY
   ioctl(fileno(stdin), TIOCSETP, psanity);
#endif
#ifdef USE_TERMIOS
   tcsetattr(fileno(stdin), TCSADRAIN, psanity);
#endif
#ifdef OSK
	_ss_opt(fileno(stdin), psanity);
#endif
   }

LOCAL int may_getchar(void)
   {
   char buffer;

   INIT_ONCE;

   if (run_in_fg() && !is_fg)
      switch_mode(0);
#ifdef OSK
	if (run_in_fg() && (_gs_rdy(fileno(stdin)) > 0))
		{
		read(fileno(stdin), &buffer, 1);
		return buffer;
		}
#else
   if (run_in_fg() && read(fileno(stdin), &buffer, 1))
      return buffer;
#endif
   return EOF;
   }

LOCAL struct tag result[2];

struct tag *get_ui(void)
   {
	int c;

   result[0].type = TAG_END;
   result[1].type = TAG_END;
   count_pattern++;
   count_song++;
   switch(c = may_getchar())
      {
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
      
         


void notice(const char *fmt, ...)
   {
	va_list al;

	va_start(al, fmt);
	vfprintf(stderr, fmt, al);
	va_end(al);
   }

void vnotice(const char *fmt, va_list al)
	{
	
	vfprintf(stderr, fmt, al);
	fputc('\n', stderr);
	}

void status(const char *s)
   {
   if (run_in_fg())
      {
		if (s)
         {
         puts(s);
         }
      else
         putchar('\n');
      }
   }

LOCAL char title[25];
void song_title(const char *s)
   {
	int i;

	for (i = 0; *s && i < 24; s++)
		if (isprint(*s & 0x7f))
		    title[i++] = *s;
	title[i] = 0;
	count_song = 0;
   }



LOCAL char scroll_buffer[200];

GENERIC begin_info(const char *title)
   {
   if (run_in_fg())
      return scroll_buffer;
   else
      return 0;
   }

void infos(void *handle, const char *s)
{
	if (handle)
		fputs(s, stdout);
}

void info(void *handle, const char *line)
{
	if (handle)
		puts(line);
}

void end_info(void *handle)
{
	if (handle)
		fflush(stdout);
}

LOCAL int ntracks;

void set_number_tracks(int n)
	{
	ntracks = n;
	}

LOCAL char scroll_line[2000];

char *new_scroll(void)
   {
	scroll_line[0] = 0;
	return scroll_line;
   }
   
LOCAL void do_scroll(void *line)
{
	if (run_in_fg()) {
		puts((char *)line);
		fflush(stdout);
	}
	free(line);
}

LOCAL void free_p(void *line)
	{
	free(line);
	}

void scroll(char *end)
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

struct thingy {
	unsigned t0, t1, t2;
	unsigned long u0, u1;
};

LOCAL void do_display_pattern(void *param)
	{
	struct thingy *thingy = (struct thingy *)param;
	unsigned current, total, real;
	unsigned long uptilnow, totaltime;
	char buf0[50], buf1[50];
	current = thingy->t0;
	total = thingy->t1;
	real = thingy->t2;
	uptilnow = thingy->u0;
	totaltime =thingy->u1;
	free(thingy);

   if (run_in_fg())
		{
	  	if (get_pref_scalar(PREF_XTERM))
	  		{
			if (get_pref_scalar(PREF_SHOW))
				{
				int i;
				for (i = 0; i < ntracks; i++)
					printf("--------------");
				putchar('\n');
				fflush(stdout);
				printf("\x1b]2;V%s %3u/%3u[%3u] %s/%s %s \007", VERSION,
					current, total, real, time2string(buf0, uptilnow),
					time2string(buf1, totaltime), title);
				fflush(stdout);
				}
			else
				{
				printf("\x1b]2;V%s %3u/%3u %s/%s %s\007", VERSION, 
					current, total, time2string(buf0, uptilnow),
					time2string(buf1, totaltime), title);
				fflush(stdout);
				}
		  	}
		else
		  	{
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

void display_pattern(unsigned int current, unsigned int total, 
	unsigned int real, unsigned long uptilnow, unsigned long totaltime)
   {
	struct thingy *thingy;

	thingy = (struct thingy *)malloc(sizeof(struct thingy));
	thingy->t0 = current;
	thingy->t1 = total;
	thingy->t2 = real;
	thingy->u0 = uptilnow;
	thingy->u1 = totaltime;
	sync_audio(do_display_pattern, free_p, thingy);
	}

LOCAL void do_display_time(void *param)
	{
	char buffer[50];
	printf("%s\n", time2string(buffer, (unsigned long)param));
	}

LOCAL void do_nuts(void *p)
	{
	}

void display_time(unsigned long time, unsigned long check)
	{
	if (time/1000 != check/1000)
		{
		sync_audio(do_display_time, do_nuts, (GENERIC)check);
		if (time > check)
			sync_audio(do_display_time, do_nuts, (GENERIC)(time - check));
		else
			sync_audio(do_display_time, do_nuts, (GENERIC)(check - time));
		}
	}

int checkbrk(void)
   {
   return FALSE;
   }
