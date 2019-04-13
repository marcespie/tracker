/* setup_audio.c */
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
/* higher level interface to the raw metal */

#include "extern.h"
#include "prefs.h"
#include "autoinit.h"
#include "notes.h"
#include "audio_channel.h"
#include "watched_var.h"

static void init_audio();

static auto INIT = init_audio;

static bool opened = false;
static unsigned long ask_freq, real_freq;
static int stereo;


/* forward declaration */
static void do_close_audio();

static void 
init_audio()
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
		if (pref::get(Pref::output))
			real_freq = open_audio(f, s);
		else {
			real_freq = 22050;
			set_watched(watched::oversample, real_freq);
		}
		opened = true;
	} else {
		unsigned long new_freq;

		if (s != stereo || f != ask_freq) {
			ask_freq = f;
			stereo = s;
			close_audio();
			if (pref::get(Pref::output))
				new_freq = open_audio(f, s);
			else {
				new_freq = 22050;
				set_watched(watched::frequency, real_freq);
			}
		} else
			new_freq = real_freq;
	}
}

static void 
do_close_audio()
{
	if (opened && pref::get(Pref::output))
		close_audio();
	opened = false;
}

