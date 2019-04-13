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
#include "resampler.h"
#include "timing.h"
     
/* set up the automaton so that I haven't got through patterns 
 * #from to #to
 */
void 
automaton::clear_repeats(unsigned int from, unsigned int upto)
{
	for (unsigned i = from; i <= upto; i++)
		gonethrough[i] = false;
}

/* set up the automaton so that I haven't got through any patterns
 */
void 
automaton::reset_repeats()
{
	clear_repeats(0, info->length);
	gonethrough[info->length] = true;
}

/* update the pattern to play in the automaton. Checks that the pattern 
 * actually exists. Handle repetitions as well.
 */
void 
automaton::set_pattern()
{
	if (pattern_num >= info->length) {
		error = UNRECOVERABLE;
		return;
	}

	if (gonethrough[pattern_num]) {
		error = ENDED;
		reset_repeats();
	}
	else
		gonethrough[pattern_num] = true;

	/* there is a level of indirection in the format,
	* i.e., patterns can be repeated.
	*/
	pattern = info->patterns+pattern_num;


	display_pattern(pattern_num, info->length, pattern->number, 
	    pattern->total, info->duration);
	if (error == ENDED)
		display_time(time_spent, info->duration);
}

void
automaton::reset_to_pattern(unsigned int start)
{
	pattern_num = start;    /* first pattern */

	delay_counter = 0;

	reset_repeats();

	note_num = 0;           /* first note in pattern */
	counter = 0;            /* counter for the effect tempo */
	speed = NORMAL_SPEED;
	finespeed = NORMAL_FINESPEED;
	/* (100%=NORMAL_FINESPEED) */
	do_stuff = DO_SET_NOTHING;  /* some effects affect the automaton,
			       		* we keep them here.  */
	error = NONE;              /* Maybe we should not reset errors at
			       	    * this point ?  */
	time_spent = 0;
	set_pattern();
}

automaton::automaton(const song *s, resampler* r_):
    r{r_}, info{&s->info}
{
	reset_to_pattern(0);
}

/* get to the next pattern, and display stuff 
 */
void 
automaton::advance_pattern()
{
	if (++pattern_num >= info->length) {
		error = ENDED;
		reset_repeats();
		pattern_num = 0;
	}
	set_pattern();
	note_num = 0;
}

void 
automaton::set_bpm(unsigned int bpm_)
{
	bpm = bpm_;
	set_beat(bpm, NORMAL_FINESPEED, finespeed);
}

void 
automaton::set_beat(unsigned int bpm, unsigned int a, unsigned int b)
{
	if (r)
		r->set_resampling_beat(bpm, a, b);
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
			set_beat(bpm, NORMAL_FINESPEED, finespeed);
		}
		if (pref::get(Pref::speedmode) != SPEED_ONLY)
			speed = new_speed;
		break;
	case SET_SPEED:
		speed = new_speed;
		if (pref::get(Pref::speedmode) == ALTER_PROTRACKER) {
			finespeed = NORMAL_FINESPEED;
			set_beat(bpm, 1, 1);
		}
		break;
	case SET_FINESPEED:
		finespeed = new_finespeed;
		set_beat(bpm, NORMAL_FINESPEED, finespeed);
		break;
	default:
		break;
	}

	if (finespeed == 0) {
		status("Finespeed of 0");
		finespeed = NORMAL_FINESPEED;
		set_beat(bpm, NORMAL_FINESPEED, finespeed);
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
				set_pattern();
				note_num = 0;
			} else if (do_stuff & SET_SKIP) {
				advance_pattern();
				note_num = new_note;
			} else {
				if (++note_num >= info->plength) {
					advance_pattern();
				}
			}
			do_stuff = DO_SET_NOTHING;
		}
	}
}


const event&
automaton::EVENT(int channel) const
{
	return pattern->e[channel * info->plength + note_num];
}
