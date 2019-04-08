/* Modules/Pro/virt.c */
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

static unsigned long 
compute_pattern_duration(event *base, unsigned int plength, unsigned int ntracks, 
    automaton *a)
{
	unsigned int i, j;
	unsigned long d;
	event *e;

	a->bpm=50;
	d = 0;
	for(i = 0; i < ntracks; i++)
		loop_start[i] = 0;

	for (i = ((a->do_stuff & SET_SKIP) ? a->new_note : 0); i < plength; i++) {
		a->do_stuff = DO_SET_NOTHING;
		a->delay_counter = 1;
		for (j = 0; j < ntracks; j++) {
			e = base + j * plength + i;
			switch(e->effect) {
			case EFF_SPEED:
				if (e->parameters >= 32 && 
				    pref::get(Pref::speedmode) != OLD_SPEEDMODE) {
					a->new_finespeed = e->parameters;
					a->do_stuff |= SET_FINESPEED;
				} else if (e->parameters) {
					a->new_speed = e->parameters;
					a->do_stuff |= SET_SPEED;
				}
				break;
			case EFF_DELAY:
				a->delay_counter = (e->parameters + 1);
				break;
			case EFF_SKIP:
				a->do_stuff |= SET_SKIP;
				a->new_note = e->parameters;
				break;
			case EFF_FF:
				a->do_stuff |= SET_FASTSKIP;
				a->new_pattern = e->parameters;
				break;
			case EFF_LOOP:
				if (!e->parameters)
					loop_start[j] = d;
				break;
			default:
				break;
			}
		}

		a->update_tempo();
		d += ratio2time(NORMAL_FINESPEED * a->delay_counter * a->speed,
		    a->finespeed * 50);

		for (j = 0; j < ntracks; j++) {
			e = base + j * plength + i;
			if ( (e->effect == EFF_LOOP) && e->parameters) {
				d += (d - loop_start[j]) * e->parameters;
				break;
			}
		}

		if ((a->do_stuff & SET_SKIP) || (a->do_stuff & SET_FASTSKIP))
			break;
	}
	return d;
}
			
static void 
set_pattern(automaton *a)
{
	if ((a->pattern_num >= a->info->length) ||
	    a->gonethrough[a->pattern_num]) {
		error = ENDED;
		return;
	}
	a->gonethrough[a->pattern_num] = true;
	a->pattern = a->info->patterns+a->pattern_num;
}

void 
compute_duration(automaton *a, song *song)
{
	unsigned long duration;

	loop_start = new unsigned long [song->ntracks];
	duration = 0;
	a->pattern->total = 0;
	error = NONE;
	while (error != ENDED) {
		a->pattern->duration =
		    compute_pattern_duration(a->pattern->e,
			song->info.plength, song->ntracks, a);
		duration += a->pattern->duration;
		if (a->do_stuff & SET_FASTSKIP)
			a->pattern_num = a->new_pattern;
		else
			a->pattern_num++; 
		set_pattern(a);
		a->pattern->total = duration;
	}
	song->info.duration = duration;
	delete [] loop_start;
	error = NONE;
}
