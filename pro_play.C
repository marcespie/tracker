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

#include <memory>
#include <vector>

#include "extern.h"
#include "song.h"
#include "protracker.h"
#include "notes.h"
#include "channel.h"
#include "prefs.h"
#include "pro_effects.h"
#include "automaton.h"
#include "autoinit.h"
#include "audio_channel.h"
#include "resampler.h"
#include "pro_play.h"
#include "empty.h"
     

extern short vibrato_table[3][64];

static void init_st_play();
static auto INIT = init_st_play;

/**************
 **************
 **************/

static st_effect eval[NUMBER_EFFECTS];
                    		/* the effect table */


std::vector<channel> chan;

static sample_info **voices;


int channel::side() const
{
	return audio->side;
}

void
channel::reset()
{
	samp = empty_sample();
	finetune = 0;
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

channel::channel(int side, resampler& r): audio {std::make_unique<audio_channel>(side, r)}
{
	reset();
}


static void 
init_channels(int ntracks, resampler& r)
{
	chan.clear();
	chan.emplace_back(LEFT_SIDE, r);
	chan.emplace_back(RIGHT_SIDE, r);
	chan.emplace_back(RIGHT_SIDE, r);
	chan.emplace_back(LEFT_SIDE, r);
	if (ntracks > 4) {
		chan.emplace_back(LEFT_SIDE, r);
		chan.emplace_back(RIGHT_SIDE, r);
	}
	if (ntracks > 6) {
		chan.emplace_back(RIGHT_SIDE, r);
		chan.emplace_back(LEFT_SIDE, r);
	}
}

static void
reset_channels()
{
	for (auto& ch: chan)
		ch.reset();
}

void 
init_st_play()
{
	init_effects(eval);
}


void 
automaton::dump_events() const
{
	dump_delimiter();
	for (auto s = 0;;) {
		for (auto i = 0U; i != chan.size(); ++i) {
			if (chan[i].side() == s) {
				dump_event(chan[i], &EVENT(i));
				dump_delimiter();
			}
		}
		if (++s == NUMBER_SIDES)
			break;
		dump_delimiter();
	}
	dump_event();
}

void 
automaton::setup_effect(channel& ch, const event& e)
{
	int samp, cmd;
	pitch pitch;

	/* retrieves all the parameters */
	samp = e.sample_number;

	/* load new instrument */
	if (samp)  {  
		/* note that we can change sample in the middle of a note. This 
		 * is a *feature*, not a bug (see made). Precisely: the sample 
		 * change will be taken into account for the next note, BUT the 
		 * volume change takes effect immediately.
		 */
		ch.samp = voices[samp];
		ch.finetune = voices[samp]->finetune;
		if ((1L<<samp) & pref::get(Pref::imask))
			ch.samp = empty_sample();
		ch.set_current_volume(voices[samp]->volume);
	}

	pitch = note2pitch(e.note, ch.finetune);

	cmd = e.effect;

	if (pitch >= REAL_MAX_PITCH) {
		char buffer[60];

		sprintf(buffer,"Pitch out of bounds %d", pitch);
		status(buffer);
		pitch = 0;
		error = error_type::FAULT;
	}

	ch.adjust = do_nothing;

	switch(eval[cmd].type) {
	case NOTHING:
		if (pitch) {
			ch.set_current_note(e.note, pitch);
			ch.start_note();
		}
		break;
	case CH_E:
		if (pitch)
			ch.set_current_note(e.note, pitch);
		(eval[cmd].f.CH_E)(ch, e);
		if (pitch)
			ch.start_note();
		break;
	case A_E:
		if (pitch)
			ch.set_current_note(e.note, pitch);
		(eval[cmd].f.A_E)(*this, e);
		if (pitch)
			ch.start_note();
		break;
	case NO_NOTE_CH_E:
		if (pitch)
			ch.set_current_note(e.note, pitch);
		(eval[cmd].f.CH_E)(ch, e);
		break;
	case PORTA_CH_PITCH_E:
		(eval[cmd].f.CH_PITCH_E)(ch, pitch, e);
		break;
	case CH_A_E:
		if (pitch)
			ch.set_current_note(e.note, pitch);
		(eval[cmd].f.CH_A_E)(ch, *this, e);
		if (pitch)
			ch.start_note();
		break;
	}
}


void 
automaton::play_one_tick()
{
	if (counter == 0) {	
		/* do new effects only if not in delay mode */
		if (delay_counter == 0) {
			for (auto i = 0U; i != chan.size(); ++i)
				setup_effect(chan[i], EVENT(i));
			if (pref::get(Pref::show))
				dump_events();
		}
	} else
		for (auto i = 0U; i != chan.size(); ++i) {
			/* do the effects */
			(chan[i].special)(chan[i]);
			(chan[i].adjust)(chan[i]);
		}

	update_tempo();
	/* actually output samples */
	if (pref::get(Pref::output))
		r->resample();
}

int
song::play(unsigned int start, resampler& r)
{
	INIT_ONCE;

	song_title(title);

	set_number_tracks(ntracks);
	init_channels(ntracks, r);

	auto countup = 0; 	/* keep playing the tune or not */

	voices = samples; 

	auto a = automaton{this, r};
	a.set_bpm(pref::get(Pref::speed));


	r.set_data_width(side_width, max_sample_width);

	while(true) {
		a.play_one_tick();
		a.next_tick();
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
			End();
			/* NOTREACHED */
		case UI_SET_BPM:
			a.set_bpm(val);
			break;
		case UI_RESTART:
			discard_buffer();
			a.reset_to_pattern(start);
			reset_channels();
			break;
		case UI_JUMP_TO_PATTERN:
			if (val >= 0 && val < a.info->length) {
				discard_buffer();
				a.reset_to_pattern(val);
				reset_channels();
			}
			break;
		default:
			break;
		}

		switch(error) {
		case error_type::NONE:
			break;
		case error_type::ENDED:
			countup++;
			if (auto r = pref::get(Pref::repeats); r != 0)
				if (countup >= r)
					return PLAY_ENDED;
			break;
		case error_type::SAMPLE_FAULT:
		case error_type::FAULT:
		case error_type::PREVIOUS_SONG:
		case error_type::NEXT_SONG:
		case error_type::UNRECOVERABLE:
			if ( (error == error_type::SAMPLE_FAULT && 
			    pref::get(Pref::tolerate))
			    ||(error == error_type::FAULT && 
			    pref::get(Pref::tolerate) > 1) )
				break;
			return PLAY_ERROR;
		default:
			break;
		}
		error = error_type::NONE;
	}
}

