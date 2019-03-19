/* Modules/Pro/play.c */
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

#include "extern.h"
#include "song.h"
#include "notes.h"
#include "channel.h"
#include "prefs.h"
#include "pro_effects.h"
#include "p_automaton.h"
#include "automaton.h"
#include "autoinit.h"
#include "resample.h"
#include "pro_play.h"
#include "empty.h"
#include <vector>
     

extern short vibrato_table[3][64];

static void init_st_play(void);
static void (*INIT)(void) = init_st_play;

/**************
 **************
 **************/


static struct st_effect eval[NUMBER_EFFECTS];
                    		/* the effect table */


std::vector<channel> chan;

static unsigned int ntracks;		/* number of tracks of the current song */
static struct sample_info **voices;



channel::channel(int side)
{
	samp = empty_sample();
	finetune = 0;
	audio = new_channel(side);
	volume = 0; 
	pitch = 0; 
	note = NO_NOTE;

	/* we don't setup arpeggio values. */
	vib.offset = 0; 
	vib.depth = 0;
	vib.rate = 0;
	vib.table = vibrato_table[0];
	vib.reset = false;

	trem.offset = 0;
	trem.depth = 0;
	trem.rate = 0;
	trem.table = vibrato_table[0];
	trem.reset = false;

	slide = 0; 

	pitchgoal = 0; 
	pitchrate = 0;

	volumerate = 0;


	funk_glissando = false;
	start_offset = 0;
	adjust = do_nothing;
	/* initialize loop to no loop, loop start at 0 
	 * (needed for don't you want me, for instance) */
	loop_counter = -1;
	loop_note_num = 0;

	special = do_nothing;
	invert_speed = 0;
	invert_offset = 0;
	invert_position = 0;
}


static void 
init_channels(void)
{
	release_audio_channels();

	chan.clear();
	chan.emplace_back(LEFT_SIDE);
	chan.emplace_back(RIGHT_SIDE);
	chan.emplace_back(RIGHT_SIDE);
	chan.emplace_back(LEFT_SIDE);
	if (ntracks > 4) {
		chan.emplace_back(LEFT_SIDE);
		chan.emplace_back(RIGHT_SIDE);
	}
	if (ntracks > 6) {
		chan.emplace_back(RIGHT_SIDE);
		chan.emplace_back(LEFT_SIDE);
	}
}


void 
init_st_play(void)
{
	init_effects(eval);
}


static void 
dump_events(automaton *a)
{
	/* display the output in a reasonable order:
	 * LEFT1 LEFT2 || RIGHT1 RIGHT 2
	 */
	dump_event(&(chan[0]), EVENT(a, 0));
	dump_delimiter();
	dump_event(&(chan[3]), EVENT(a, 3));
	dump_delimiter();
	if (ntracks > 4) {
		dump_event(&(chan[4]), EVENT(a, 4));
		dump_delimiter();
	}
	if (ntracks > 7) {
		dump_event(&(chan[7]), EVENT(a, 7));
		dump_delimiter();
	}
	dump_delimiter();
	dump_event(&(chan[1]), EVENT(a, 1));
	dump_delimiter();
	dump_event(&(chan[2]), EVENT(a, 2));
	if (ntracks > 5) {
		dump_delimiter();
		dump_event(&(chan[5]), EVENT(a, 5));
	}
	if (ntracks > 6) {
		dump_delimiter();
		dump_event(&(chan[6]), EVENT(a, 6));
	}
	dump_event(0, 0);
}

static void 
setup_effect(channel *ch, automaton *a, event *e)
{
	int samp, cmd;
	pitch pitch;

	/* retrieves all the parameters */
	samp = e->sample_number;

	/* load new instrument */
	if (samp)  {  
		/* note that we can change sample in the middle of a note. This 
		 * is a *feature*, not a bug (see made). Precisely: the sample 
		 * change will be taken into account for the next note, BUT the 
		 * volume change takes effect immediately.
		 */
		ch->samp = voices[samp];
		ch->finetune = voices[samp]->finetune;
		if ((1L<<samp) & get_pref(Pref::imask))
			ch->samp = empty_sample();
		ch->set_current_volume(voices[samp]->volume);
	}

	pitch = note2pitch(e->note, ch->finetune);

	cmd = e->effect;

	if (pitch >= REAL_MAX_PITCH) {
		char buffer[60];

		sprintf(buffer,"Pitch out of bounds %d", pitch);
		status(buffer);
		pitch = 0;
		error = FAULT;
	}

	ch->adjust = do_nothing;

	switch(eval[cmd].type) {
	case NOTHING:
		if (pitch) {
			ch->set_current_note(e->note, pitch);
			ch->start_note();
		}
		break;
	case CH_E:
		if (pitch)
			ch->set_current_note(e->note, pitch);
		(eval[cmd].f.CH_E)(ch, e);
		if (pitch)
			ch->start_note();
		break;
	case A_E:
		if (pitch)
			ch->set_current_note(e->note, pitch);
		(eval[cmd].f.A_E)(a, e);
		if (pitch)
			ch->start_note();
		break;
	case NO_NOTE_CH_E:
		if (pitch)
			ch->set_current_note(e->note, pitch);
		(eval[cmd].f.CH_E)(ch, e);
		break;
	case PORTA_CH_PITCH_E:
		(eval[cmd].f.CH_PITCH_E)(ch, pitch, e);
		break;
	case CH_A_E:
		if (pitch)
			ch->set_current_note(e->note, pitch);
		(eval[cmd].f.CH_A_E)(ch, a, e);
		if (pitch)
			ch->start_note();
		break;
	}
}


static void 
play_one_tick(automaton *a)
{
	if (a->counter == 0) {	
		/* do new effects only if not in delay mode */
		if (a->delay_counter == 0) {
			for (unsigned channel = 0; channel < ntracks; channel++)
				setup_effect(&(chan[channel]), a, 
				    EVENT(a, channel));
			if (get_pref(Pref::show))
				dump_events(a);
		}
	} else
		for (unsigned channel = 0; channel < ntracks; channel++) {
			/* do the effects */
			(chan[channel].special)(&(chan[channel]));
			(chan[channel].adjust)(&(chan[channel]));
		}

	update_tempo(a);
	/* actually output samples */
	if (get_pref(Pref::output))
		resample();
}

int
play_song(song *song, unsigned int start)
{
	int countup;      /* keep playing the tune or not */
	int r;
	automaton *a;

	INIT_ONCE;

	song_title(song->title);

	ntracks = song->ntracks;
	set_number_tracks(ntracks);

	countup = 0;

	voices = song->samples; 

	a = setup_automaton(song, start);
	set_bpm(a, get_pref(Pref::speed));

	init_channels();

	set_data_width(song->side_width, song->max_sample_width);

	while(true) {
		play_one_tick(a);
		next_tick(a);
		auto [type, val] = get_ui();
		switch(type) {  
		case UI_NEXT_SONG:
			discard_buffer();
			return PLAY_NEXT_SONG;
		case UI_PREVIOUS_SONG:
			discard_buffer();
			return PLAY_PREVIOUS_SONG;
		case UI_QUIT:
			discard_buffer();
			end_all(0);
			/* NOTREACHED */
		case UI_SET_BPM:
			set_bpm(a, val);
			break;
		case UI_RESTART:
			discard_buffer();
			a = setup_automaton(song, start);
			init_channels();
			break;
		case UI_JUMP_TO_PATTERN:
			if (val >= 0 && val < a->info->length) {
				discard_buffer();
				a = setup_automaton(song, val);
			}
			break;
		default:
			break;
		}

		switch(error) {
		case NONE:
			break;
		case ENDED:
			countup++;
			if ( (r = get_pref(Pref::repeats)) )
				if (countup >= r)
					return PLAY_ENDED;
			break;
		case SAMPLE_FAULT:
		case FAULT:
		case PREVIOUS_SONG:
		case NEXT_SONG:
		case UNRECOVERABLE:
			if ( (error == SAMPLE_FAULT && get_pref(Pref::tolerate))
			    ||(error == FAULT && get_pref(Pref::tolerate) > 1) )
				break;
			return PLAY_ERROR;
		default:
			break;
		}
		error = NONE;
	}
}

