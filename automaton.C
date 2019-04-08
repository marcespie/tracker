/* automaton.c */
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

#include <memory>

#include "song.h"
#include "protracker.h"
#include "notes.h"
#include "automaton.h"
#include "extern.h"
#include "prefs.h"
#include "resample.h"
#include "timing.h"
     
/* set up the automaton so that I haven't got through patterns 
 * #from to #to
 */
static void 
clear_repeats(automaton *a, unsigned int from, unsigned int upto)
{
	for (unsigned i = from; i <= upto; i++)
		a->gonethrough[i] = false;
}

/* set up the automaton so that I haven't got through any patterns
 */
static void 
reset_repeats(automaton *a)
{
	clear_repeats(a, 0, a->info->length);
	a->gonethrough[a->info->length] = true;
}

/* update the pattern to play in the automaton. Checks that the pattern 
 * actually exists. Handle repetitions as well.
 */
static void 
set_pattern(automaton *a)
{
	if (a->pattern_num >= a->info->length) {
		error = UNRECOVERABLE;
		return;
	}

	if (a->gonethrough[a->pattern_num]) {
		error = ENDED;
		reset_repeats(a);
	}
	else
		a->gonethrough[a->pattern_num] = true;

	/* there is a level of indirection in the format,
	* i.e., patterns can be repeated.
	*/
	a->pattern = a->info->patterns+a->pattern_num;


	display_pattern(a->pattern_num, a->info->length, a->pattern->number, 
	    a->pattern->total, a->info->duration);
	if (error == ENDED)
		display_time(a->time_spent, a->info->duration);
}

/* initialize all the fields of the automaton necessary
 * to play a given song.
 */
static void 
init_automaton(automaton* a, const song* song, unsigned int start)
{
	a->info = &song->info;
	a->pattern_num = start;    /* first pattern */

	a->delay_counter = 0;

	reset_repeats(a);

	a->note_num = 0;           /* first note in pattern */
	a->counter = 0;            /* counter for the effect tempo */
	a->speed = NORMAL_SPEED;
	a->finespeed = NORMAL_FINESPEED;
	/* (100%=NORMAL_FINESPEED) */
	a->do_stuff = DO_SET_NOTHING;  /* some effects affect the automaton,
			       		* we keep them here.  */
	error = NONE;              /* Maybe we should not reset errors at
			       	    * this point ?  */
	a->time_spent = 0;
	set_pattern(a);
}

automaton::automaton(const song *s, unsigned int start)
{
	init_automaton(this, s, start);
}

/* get to the next pattern, and display stuff 
 */
static void 
advance_pattern(automaton *a)
{
	if (++a->pattern_num >= a->info->length) {
		error = ENDED;
		reset_repeats(a);
		a->pattern_num = 0;
	}
	set_pattern(a);
	a->note_num = 0;
}

void 
automaton::set_bpm(unsigned int bpm_)
{
	bpm = bpm_;
	set_resampling_beat(bpm, NORMAL_FINESPEED, finespeed);
}

void 
automaton::update_tempo()
{        
	/* there are three classes of speed changes:
	 * 0 does nothing. (should stop for genuine protracker)
	 * <32 is the effect speed (resets the fine speed).
	 * >=32 changes the finespeed, default 125
	 */

	switch(do_stuff & (SET_SPEED | SET_FINESPEED)) {
	case SET_SPEED | SET_FINESPEED:
		if (pref::get(Pref::speedmode) != FINESPEED_ONLY) {
			finespeed = new_finespeed;
			set_resampling_beat(bpm, NORMAL_FINESPEED, finespeed);
		}
		if (pref::get(Pref::speedmode) != SPEED_ONLY)
			speed = new_speed;
		break;
	case SET_SPEED:
		speed = new_speed;
		if (pref::get(Pref::speedmode) == ALTER_PROTRACKER) {
			finespeed = NORMAL_FINESPEED;
			set_resampling_beat(bpm, 1, 1);
		}
		break;
	case SET_FINESPEED:
		finespeed = new_finespeed;
		set_resampling_beat(bpm, NORMAL_FINESPEED, finespeed);
		break;
	default:
		break;
	}

	if (finespeed == 0) {
		status("Finespeed of 0");
		finespeed = NORMAL_FINESPEED;
		set_resampling_beat(bpm, NORMAL_FINESPEED, finespeed);
		error = FAULT;
	}
}

/* process all the stuff which we need to advance in the song,
 * including set_skip, set_fastskip, and set_loop.
 */
void 
automaton::next_tick()
{
	time_spent += ratio2time(NORMAL_FINESPEED, finespeed * 50);
	if (++counter >= speed) {
		counter = 0;
		/* if we are in delay mode, count down delay */
		if (delay_counter > 0)
			delay_counter--;
		/* get to next tick ONLY if no delay */
		if (delay_counter == 0) {
			/* loop: may change note in pattern right away */
			if (do_stuff & JUMP_PATTERN)
				note_num = loop_note_num;
			else if (do_stuff & SET_FASTSKIP) {
				pattern_num = new_pattern;
				set_pattern(this);
				note_num = 0;
			} else if (do_stuff & SET_SKIP) {
				advance_pattern(this);
				note_num = new_note;
			} else {
				if (++note_num >= info->plength) {
					advance_pattern(this);
				}
			}
			do_stuff = DO_SET_NOTHING;
		}
	}
}


event *
automaton::EVENT(int channel) const
{
	return &(pattern->e[channel * info->plength + note_num]);
}
