/* pro_effects.C */
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
#include <string>

#include "notes.h"
#include "channel.h"
#include "song.h"
#include "protracker.h"
#include "automaton.h"
#include "prefs.h"
#include "pro_effects.h"
#include "ui.h"
#include "errortype.h"
#include "minmax.h"
     
/* sine table for the vibrato effect (obtained through build_vibrato.c) */

short vibrato_table[3][64] = 
	{
   {
   0,50,100,149,196,241,284,325,362,396,426,452,473,490,502,510,512,
   510,502,490,473,452,426,396,362,325,284,241,196,149,100,50,0,-49,
   -99,-148,-195,-240,-283,-324,-361,-395,-425,-451,-472,-489,-501,
   -509,-511,-509,-501,-489,-472,-451,-425,-395,-361,-324,-283,-240,
   -195,-148,-99,-49
   },
	{
	512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,
	512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,-512,-512,
	-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512, 
	-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,-512,
	-512,-512
	},
	{
	0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240,256,272,288,
	304,320,336,352,368,384,400,416,432,448,464,480,496,-512,-496,-480,
	-464,-448,-432,-416,-400,-384,-368,-352,-336,-320,-304,-288,-272,-256,
	-240,-224,-208,-192,-176,-160,-144,-128,-112,-96,-80,-64,-48,-32,-16
	}
	};

/***	setting up effects/doing effects :
 ***		set_xxx gets called while parsing the effect,
 *** 		do_xxx gets called each tick, and update the
 ***		sound parameters while playing.
 ***/




/***
 ***	base effects
 ***/

void do_nothing(channel&)
{
}

void set_nothing();

void set_nothing()
{
}

/* slide pitch (up or down) */
static void 
do_slide(channel& ch)
{
	ch.pitch += ch.slide;
	ch.pitch = MIN(ch.pitch, MAX_PITCH);
	ch.pitch = MAX(ch.pitch, MIN_PITCH);
	ch.set_temp_pitch(ch.pitch);
}

static void 
set_upslide(channel& ch, const event& e)
{
	ch.adjust = do_slide;
	if (e.parameters)
		ch.slide = e.parameters;
}

static void 
set_downslide(channel& ch, const event& e)
{
	ch.adjust = do_slide;
	if (e.parameters)
		ch.slide = -e.parameters;
}

static int 
sinusoid_value(sinusoid& s)
{
	s.offset += s.rate;
	s.offset &= 63;
	return s.table[s.offset] * s.depth;
}

static void 
set_sinusoid(const event& e, sinusoid& s)
{
	if (e.high())
		s.rate = e.high();
	if (e.low())
		s.depth = e.low();
	if (s.reset)
		s.offset = 0;
}

/* modulate the pitch with vibrato */
static void 
do_vibrato(channel& ch)
{
	/* temporary update of only the step value,
	* note that we do not change the saved pitch.
	*/
	if (ch.pitch)
		ch.set_temp_pitch(ch.pitch + sinusoid_value(ch.vib)/256);
}

static void 
set_vibrato(channel& ch, const event& e)
{
	ch.adjust = do_vibrato;
		set_sinusoid(e, ch.vib);
}

static void 
do_tremolo(channel& ch)
{
	ch.set_temp_volume(ch.volume + sinusoid_value(ch.trem)/128);
}

static void 
set_tremolo(channel& ch, const event& e)
{
	ch.adjust = do_tremolo;
	set_sinusoid(e, ch.trem);
}

/* arpeggio looks a bit like chords: we alternate between two
 * or three notes very fast.
 */
static void 
do_arpeggio(channel& ch)
{
	if (++ch.arpindex >= MAX_ARP)
		ch.arpindex =0;
	ch.set_temp_pitch(ch.arp[ch.arpindex]);
}

static void 
set_arpeggio(channel& ch, const event& e)
{
	/* arpeggio can be installed relative to the
	* previous note, so we have to check that there
	* actually is a current(previous) note
	*/
	if (ch.note == NO_NOTE) {
		status("No note present for arpeggio");
		error = error_type::FAULT;
	} else {
		ch.arp[0] = note2pitch(ch.note, ch.finetune);
		ch.arp[1] = note2pitch(ch.note + e.high(), ch.finetune);
		if (!ch.arp[1]) {
			status("Arpeggio note out of range");
			error = error_type::FAULT;
		}
		ch.arp[2] = note2pitch(ch.note + e.low(), ch.finetune);
		if (!ch.arp[2]) {
			status("Arpeggio note out of range");
			error = error_type::FAULT;
		}
		ch.arpindex = 0;
		ch.adjust = do_arpeggio;
	}
}

/* volume slide. Mostly used to simulate waveform control.
 * (attack/decay/sustain).
 */
static void 
do_slidevol(channel& ch)
{
	ch.set_current_volume(ch.volume + ch.volumerate);
}

/* note that volumeslide does not have a ``take default''
 * behavior. If para is 0, this is truly a 0 volumeslide.
 * Issue: is the test really necessary ? Can't we do
 * a HI(para) - LOW(para). Answer: protracker does not.
 * DON'T GET the test order wrong!!! or ghouls will sound strange
 */
static void 
parse_slidevol(channel& ch, const event& e)
{
	if (e.high())
		ch.volumerate = e.high();
	else
		ch.volumerate = -e.low();
}

static void 
set_slidevol(channel& ch, const event& e)
{
	ch.adjust = do_slidevol;
	parse_slidevol(ch, e);
}

/* portamento goes from a given pitch to another.  We could optimize 
 * that effect by splitting the routine into a pitch up/pitch down 
 * part while setting up the effect.
 */
static void 
do_portamento(channel& ch)
{
	if (ch.pitchgoal) {
		if (ch.pitch < ch.pitchgoal) {
			ch.pitch += ch.pitchrate;
			if (ch.pitch >= ch.pitchgoal) {
				ch.pitch = ch.pitchgoal;
				/* ch.pitchgoal reset in protracker */
				ch.pitchgoal = 0;
			}
		} else if (ch.pitch > ch.pitchgoal) {
			if (static_cast<pitch>(ch.pitchrate) > ch.pitch) {
				ch.pitch = ch.pitchgoal;
				/* ch.pitchgoal reset in protracker */
				ch.pitchgoal = 0;		
			} else {
				ch.pitch -= ch.pitchrate;
				if (ch.pitch <= ch.pitchgoal) {
					ch.pitch = ch.pitchgoal;
					/* ch.pitchgoal reset in protracker */
					ch.pitchgoal = 0;		
				}
			}
		}
		/* funk glissando: round to the nearest note each time */
		if (ch.funk_glissando)
			ch.set_temp_pitch(
			    round_pitch(ch.pitch, ch.finetune));
		else
			ch.set_temp_pitch(ch.pitch);
	}
}

/* if para and pitch are 0, this is obviously a continuation
 * of the previous portamento.
 */
static void 
set_portamento(channel& ch, pitch pitch, const event& e)
{
	if (e.parameters)
		ch.pitchrate = e.parameters;
	if (pitch)
		ch.pitchgoal = pitch;
	if (ch.pitchgoal)
		ch.adjust = do_portamento;
}




/***
 ***	combined commands
 ***/

static void 
do_portaslide(channel& ch)
{
	do_portamento(ch);
	do_slidevol(ch);
}

static void 
set_portaslide(channel& ch, pitch pitch, const event& e)
{
	if (pitch)
		ch.pitchgoal = pitch;
	parse_slidevol(ch, e);
	if (ch.pitchgoal)
		ch.adjust = do_portaslide;
	else
		ch.adjust = do_slidevol;
}

static void 
do_vibratoslide(channel& ch)
{
	do_vibrato(ch);
	do_slidevol(ch);
}

static void 
set_vibratoslide(channel& ch, const event& e)
{
	ch.adjust = do_vibratoslide;
	parse_slidevol(ch, e);
	if (ch.vib.reset)
		ch.vib.offset = 0;
}




/***
 ***	effects that just need a setup part
 ***/

/* IMPORTANT: because of the special nature of the player, we can't 
 * process each effect independently, we have to merge effects from 
 * the four channel before doing anything about it. For instance, 
 * there can be several speed change in the same note.
 */
static void 
set_speed(automaton& a, const event& e)
{
	if (e.parameters >= 32 && 
	    pref::get(Pref::speedmode) != OLD_SPEEDMODE) {
		a.new_finespeed = e.parameters;
		a.do_stuff |= SET_FINESPEED;
	} else if (e.parameters) {
		a.new_speed = e.parameters;
		a.do_stuff |= SET_SPEED;
	}
}

/* older soundtracker speed change effect */
static void 
set_st_speed(automaton& a, const event& e)
{
	a.new_speed = e.parameters;
	a.do_stuff |= SET_SPEED;
}

static void 
set_skip(automaton& a, const event& e)
{
	// XXX BCD decoding in pro_read.C
	a.new_note = e.parameters;
	a.do_stuff |= SET_SKIP;
}

static void 
set_fastskip(automaton& a, const event& e)
{
	a.new_pattern = e.parameters;
	a.do_stuff |= SET_FASTSKIP;
}

/* immediate effect: starts the sample somewhere
 * off the start.
 */
static void 
set_offset(channel& ch, const event& e)
{
	if (e.parameters)
		ch.start_offset = e.parameters * 256;
	ch.start_note();
	ch.set_position(ch.start_offset);
}

/* change the volume of the current channel. Is effective until there 
 * is a new set_volume, slide_volume, or an instrument is reloaded 
 * explicitly by giving its number. Obviously, if you load an instrument 
 * and do a set_volume in the same note, the set_volume will take precedence.
 */
static void 
set_volume(channel& ch, const event& e)
{
	ch.set_current_volume(e.parameters);
}




/***
 ***	extended commands
 ***/

/* retrig note at a fast pace
 */
static void 
do_retrig(channel& ch)
{
	if (--ch.current <= 0) {
		ch.start_note();
		ch.current = ch.retrig;
	}
}

static void 
set_retrig(channel& ch, const event& e)
{
	ch.retrig = e.parameters;
	ch.current = ch.retrig;
	ch.adjust = do_retrig;
}

/* start note after a small delay
 */
static void 
do_latestart(channel& ch)
{
	if (--ch.current <= 0) {
		ch.set_current_note(ch.note, ch.pitch);
		ch.start_note();
		ch.adjust = do_nothing;
	}
}

static void 
set_late_start(channel& ch, const event& e)
{
	/* stop previous note if necessary */
	ch.stop_note();
	ch.current = e.parameters;
	ch.adjust = do_latestart;
}

/* cut note after some time. Note we only kill the volume, 
 * as protracker does (compatibility...)
 */
static void 
do_cut(channel& ch)
{
	if (ch.retrig) {
		if (--ch.retrig == 0)
			ch.set_current_volume(0);
	}
}

static void 
set_note_cut(channel& ch, const event& e)
{
	ch.retrig = e.parameters;
	ch.adjust = do_cut;
}


static void 
set_smooth_up(channel& ch, const event& e)
{
	ch.pitch += e.parameters;
	ch.pitch = MIN(ch.pitch, MAX_PITCH);
	ch.pitch = MAX(ch.pitch, MIN_PITCH);
	ch.set_temp_pitch(ch.pitch);
}

static void 
set_smooth_down(channel& ch, const event& e)
{
	ch.pitch -= e.parameters;
	ch.pitch = MIN(ch.pitch, MAX_PITCH);
	ch.pitch = MAX(ch.pitch, MIN_PITCH);
	ch.set_temp_pitch(ch.pitch);
}

static void 
set_change_finetune(channel& ch, const event& e)
{
	ch.finetune = e.parameters;
	if (e.note != NO_NOTE) {
		pitch pitch = note2pitch(e.note, ch.finetune);
		ch.set_current_note(e.note, pitch);
	}
	ch.start_note();
}


static void 
set_loop(channel& ch, automaton& a, const event& e)
{
	/* Note: the current implementation of protracker
	* does not allow for a jump from pattern to pattern,
	* even though it looks like a logical extension to the current 
	* format.
	*/
	if (e.parameters == 0) 
		ch.loop_note_num = a.note_num;
	else {
		if (ch.loop_counter == -1)
			ch.loop_counter = e.parameters + 1;
		/* We have to defer the actual note jump
		* to automaton.c, because some modules include several
		* loops on the same measure, which is a bit confusing
		* (see don't you want me for a good example)
		*/
		ch.loop_counter--;
		if (ch.loop_counter > 0) {
			a.do_stuff |= JUMP_PATTERN;
			a.loop_note_num = ch.loop_note_num;
		} else
			ch.loop_counter = -1;
		}
}

static void 
set_smooth_upvolume(channel& ch, const event& e)
{
	ch.set_current_volume(ch.volume + e.parameters);
}

static void 
set_smooth_downvolume(channel& ch, const event& e)
{
	ch.set_current_volume(ch.volume - e.parameters);
}


static void 
set_delay_pattern(automaton& a, const event& e)
{
	a.delay_counter = e.parameters + 1;
}



static void 
set_gliss_ctrl(channel& ch, const event& e)
{
	if (e.parameters)
		ch.funk_glissando = true;
	else
		ch.funk_glissando = false;
}

static void 
set_sine_wave(const event& e, sinusoid& s)
{
	s.table = vibrato_table[e.parameters & 3];
	if (e.parameters & 4)
		s.reset = false;
	else
		s.reset = true;
}

static void 
set_vibrato_wave(channel& ch, const event& e)
{
	set_sine_wave(e, ch.vib);
}

static void 
set_tremolo_wave(channel& ch, const event& e)
{
	set_sine_wave(e, ch.trem);
}

static void 
do_invert(channel& ch)
{
	ch.invert_offset += ch.invert_speed;
	if (ch.invert_offset >= 128) {
		ch.invert_offset = 0;
		if (ch.samp->rp_length) {
			if (++ch.invert_position >= ch.samp->rp_length)
				ch.invert_position = 0;
			ch.samp->rp_start[ch.invert_position] = -1 
			    - ch.samp->rp_start[ch.invert_position];
		}
	}
}

static void 
set_invert_loop(channel& ch, const event& e)
{
	static unsigned char funk_table[] =
	    {0, 5, 6, 7, 8, 10, 11, 13, 16, 19, 22, 26, 32, 43, 64, 128};
	if (e.parameters) {
		ch.invert_speed = funk_table[e.parameters];
		ch.special = do_invert;
	} else
		ch.special = do_nothing;
}


/* initialize the whole effect table */
void 
init_effects(st_effect table[])
{
	for (unsigned int i = 0; i < NUMBER_EFFECTS; i++)
		table[i].type = NOTHING;
	table[EFF_ARPEGGIO].f.CH_E = set_arpeggio;
	table[EFF_SPEED].f.A_E = set_speed;
	table[EFF_OLD_SPEED].f.A_E = set_st_speed;
	table[EFF_SKIP].f.A_E = set_skip;
	table[EFF_FF].f.A_E = set_fastskip;
	table[EFF_VOLUME].f.CH_E = set_volume;
	table[EFF_VOLSLIDE].f.CH_E = set_slidevol;
	table[EFF_OFFSET].f.CH_E = set_offset;
	table[EFF_PORTA].f.CH_PITCH_E = set_portamento;
	table[EFF_PORTASLIDE].f.CH_PITCH_E = set_portaslide;
	table[EFF_UP].f.CH_E = set_upslide;
	table[EFF_DOWN].f.CH_E = set_downslide;
	table[EFF_VIBRATO].f.CH_E = set_vibrato;
	table[EFF_VIBSLIDE].f.CH_E = set_vibratoslide;
	table[EFF_SMOOTH_UP].f.CH_E = set_smooth_up;
	table[EFF_SMOOTH_DOWN].f.CH_E = set_smooth_down;
	table[EFF_CHG_FTUNE].f.CH_E = set_change_finetune;
	table[EFF_LOOP].f.CH_A_E = set_loop;
	table[EFF_RETRIG].f.CH_E = set_retrig;
	table[EFF_S_UPVOL].f.CH_E = set_smooth_upvolume;
	table[EFF_S_DOWNVOL].f.CH_E = set_smooth_downvolume;
	table[EFF_NOTECUT].f.CH_E = set_note_cut;
	table[EFF_LATESTART].f.CH_E = set_late_start;
	table[EFF_DELAY].f.A_E = set_delay_pattern;
	table[EFF_TREMOLO].f.CH_E = set_tremolo;
	table[EFF_GLISS_CTRL].f.CH_E = set_gliss_ctrl;
	table[EFF_VIBRATO_WAVE].f.CH_E = set_vibrato_wave;
	table[EFF_TREMOLO_WAVE].f.CH_E = set_tremolo_wave;
	table[EFF_INVERT_LOOP].f.CH_E = set_invert_loop;

	table[EFF_ARPEGGIO].type = CH_E;
	table[EFF_SPEED].type = A_E;
	table[EFF_OLD_SPEED].type = A_E;
	table[EFF_SKIP].type = A_E;
	table[EFF_FF].type = A_E;
	table[EFF_VOLUME].type = CH_E;
	table[EFF_VOLSLIDE].type = CH_E;
	table[EFF_OFFSET].type = NO_NOTE_CH_E;
	table[EFF_PORTA].type = PORTA_CH_PITCH_E;
	table[EFF_PORTASLIDE].type = PORTA_CH_PITCH_E;
	table[EFF_UP].type = CH_E;
	table[EFF_DOWN].type = CH_E;
	table[EFF_VIBRATO].type = CH_E;
	table[EFF_VIBSLIDE].type = CH_E;
	table[EFF_SMOOTH_UP].type = CH_E;
	table[EFF_SMOOTH_DOWN].type = CH_E;
	table[EFF_CHG_FTUNE].type = NO_NOTE_CH_E;
	table[EFF_LOOP].type = CH_A_E;
	table[EFF_RETRIG].type = CH_E;
	table[EFF_S_UPVOL].type = CH_E;
	table[EFF_S_DOWNVOL].type = CH_E;
	table[EFF_NOTECUT].type = CH_E;
	table[EFF_LATESTART].type = NO_NOTE_CH_E;
	table[EFF_DELAY].type = A_E;
	table[EFF_TREMOLO].type = CH_E;
	table[EFF_GLISS_CTRL].type = CH_E;
	table[EFF_VIBRATO_WAVE].type = CH_E;
	table[EFF_TREMOLO_WAVE].type = CH_E;
	table[EFF_INVERT_LOOP].type = CH_E;
}

