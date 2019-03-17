/* setup_audio.c */
/* higher level interface to the raw metal */

#include "defs.h"
#include "extern.h"
#include "tags.h"
#include "prefs.h"
#include "autoinit.h"
#include "notes.h"
#include "resample.h"
#include "watched_var.h"

static void init_audio(void);

static void (*INIT)(void) = init_audio;

static bool opened = false;
static unsigned long ask_freq, real_freq;
static int stereo;


/* forward declaration */
static void do_close_audio(void);

static void 
init_audio(void)
{
	at_end(do_close_audio);
}

/* setup_audio(frequency, stereo):
 * try to avoid calling open_audio and other things
 * all the time
 */
void 
setup_audio(unsigned long f, int s)
{
	INIT_ONCE;

	if (!opened) {
		ask_freq = f;
		stereo = s;
		if (get_pref(Pref::output))
			real_freq = open_audio(f, s);
		else {
			real_freq = 22050;
			set_watched_scalar(FREQUENCY, real_freq);
		}
		opened = true;
	} else {
		unsigned long new_freq;

		if (s != stereo || f != ask_freq) {
			ask_freq = f;
			stereo = s;
			close_audio();
			if (get_pref(Pref::output))
				new_freq = open_audio(f, s);
			else {
				new_freq = 22050;
				set_watched_scalar(FREQUENCY, real_freq);
			}
		} else
			new_freq = real_freq;
	}
}

static void 
do_close_audio(void)
{
	if (opened && get_pref(Pref::output))
		close_audio();
	opened = false;
}

