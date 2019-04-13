/* pro_virt.C */
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

#include <cassert>
#include <memory>

#include "extern.h"
#include "song.h"
#include "protracker.h"
#include "notes.h"
#include "channel.h"
#include "prefs.h"
#include "automaton.h"
#include "timing.h"

static unsigned long *loop_start;

unsigned long 
automaton::compute_pattern_duration(event *base, unsigned int plength, 
    unsigned int ntracks)
{
	unsigned int i, j;
	bpm=50;
	unsigned long d = 0;
	for(i = 0; i < ntracks; i++)
		loop_start[i] = 0;

	for (i = ((do_stuff & SET_SKIP) ? new_note : 0); i < plength; i++) {
		do_stuff = DO_SET_NOTHING;
		delay_counter = 1;
		for (j = 0; j < ntracks; j++) {
			const event& e = base[j * plength + i];
			switch (e.effect) {
			case EFF_SPEED:
				if (e.parameters >= 32 && 
				    pref::get(Pref::speedmode) != OLD_SPEEDMODE) {
					new_finespeed = e.parameters;
					do_stuff |= SET_FINESPEED;
				} else if (e.parameters) {
					new_speed = e.parameters;
					do_stuff |= SET_SPEED;
				}
				break;
			case EFF_DELAY:
				delay_counter = e.parameters + 1;
				break;
			case EFF_SKIP:
				do_stuff |= SET_SKIP;
				new_note = e.parameters;
				break;
			case EFF_FF:
				do_stuff |= SET_FASTSKIP;
				new_pattern = e.parameters;
				break;
			case EFF_LOOP:
				if (!e.parameters)
					loop_start[j] = d;
				break;
			default:
				break;
			}
		}

		update_tempo();
		d += ratio2time(NORMAL_FINESPEED * delay_counter * speed,
		    finespeed * 50);

		for (j = 0; j < ntracks; j++) {
			const event& e = base[j * plength + i];
			if ( (e.effect == EFF_LOOP) && e.parameters) {
				d += (d - loop_start[j]) * e.parameters;
				break;
			}
		}

		if ((do_stuff & SET_SKIP) || (do_stuff & SET_FASTSKIP))
			break;
	}
	return d;
}
			
void 
automaton::set_pattern_for_virt()
{
	if (pattern_num >= info->length || gonethrough[pattern_num]) {
		error = error_type::ENDED;
		return;
	}
	gonethrough[pattern_num] = true;
	pattern = info->patterns + pattern_num;
}

void 
automaton::compute_duration(song *song)
{
	loop_start = new unsigned long [song->ntracks];
	unsigned long duration = 0;
	pattern->total = 0;
	error = error_type::NONE;
	while (error != error_type::ENDED) {
		pattern->duration =
		    compute_pattern_duration(pattern->e,
			song->info.plength, song->ntracks);
		duration += pattern->duration;
		if (do_stuff & SET_FASTSKIP)
			pattern_num = new_pattern;
		else
			pattern_num++; 
		set_pattern_for_virt();
		pattern->total = duration;
	}
	song->info.duration = duration;
	delete [] loop_start;
	error = error_type::NONE;
}
