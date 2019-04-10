/* resample.c */
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

#include <cmath>
#include <memory>
#include <vector>
#include <iostream>

#include "song.h"
#include "protracker.h"
#include "notes.h"
#include "channel.h"
#include "extern.h"
#include "prefs.h"
#include "resample.h"
#include "resampler.h"
#include "autoinit.h"
#include "empty.h"
#include "watched_var.h"
     
/* macros for fixed point arithmetic */
/* NOTE these should be used ONLY with unsigned values !!!! */

const auto ACCURACY=12U;
const auto fixed_unit = 1U << ACCURACY;

template<typename T>
auto inline fix_to_int(T x)
{
	return x >> ACCURACY;
}

template<typename T>
auto inline int_to_fix(T x)
{
	return x << ACCURACY;
}

inline auto
audio_channel::C() const
{
		return fix_to_int(pointer);
}

template<typename T>
auto inline fractional_part(T x)
{
	return x & (fixed_unit - 1);
}


/* Have to get some leeway for vibrato (since we don't bound pitch with
 * vibrato). This is conservative.
 */
const auto LEEWAY=150;



static void init_resample(void);
static void (*INIT)(void) = init_resample;


/*---------- Channels allocation mechanism -----------------------*/

static std::vector<audio_channel* > allocated;
static int total[NUMBER_SIDES];


audio_channel::audio_channel(int side_):
    samp{empty_sample()}, mode{audio_state::DO_NOTHING}, pointer{0}, 
    step{0}, volume{0}, scaled_volume{0}, pitch{0},
    side{side_}
{
	INIT_ONCE;

	/* checking allocation */
	if (side < 0 || side >= NUMBER_SIDES)
		End() << "Improper alloc channel call side: " << side;
	/* logging number of channels per side */
	total[side]++;
	allocated.push_back(this); // XXX  we don't really track those objects
	// so we depend on releasing audio channels correctly
}

void 
release_audio_channels(void)
{
	allocated.clear();
	for (unsigned int i = 0; i < NUMBER_SIDES; i++)
		total[i] = 0;
}

/*---------- Resampling engine ------------------------------------*/

static unsigned long step_table[REAL_MAX_PITCH + LEEWAY];  
                  /* holds the increment for finding the next sampled
                   * byte at a given pitch (see resample() ).
                   */

static unsigned int oversample;
static unsigned long resampling_frequency;
static unsigned int tempo = 50;
static unsigned int num, den = 1;
static unsigned int number_samples;

void 
prep_sample_info(sample_info *info)
{
	info->fix_length = int_to_fix(info->length);
	info->fix_rp_length = int_to_fix(info->rp_length);
}

/* create a table for converting ``amiga'' pitch
 * to a step rate at a given resampling frequency.
 * For accuracy, we don't use floating point, but
 * instead fixed point ( << ACCURACY).
 * IMPORTANT NOTES:
 * - we need to make it fit within 32 bits (long), which must be enough 
 * for ACCURACY + log2(max sample length)
 * - for linear resampling to work correctly, we need 
 * sample size (8 bit) + volume size (6 bit) + ACCURACY to fit within a
 * long. Appropriate steps will have to be taken when we switch to 16 bit
 * samples...
 * - never forget that samples are SIGNED numbers. If you have unsigned 
 * samples, you have to convert them SOMEWHERE.
 */
static void 
build_step_table(
    int oversample, 		/* use i sample for each value output */
    unsigned long output_fr 	/* output frequency */
    )
{
	/* special case: oversample of 0 means linear resampling */
	if (oversample == 0)
		oversample = 1;
	step_table[0] = 0;
	auto base_freq = AMIGA_CLOCKFREQ * pow(2.0, double(pref::get(Pref::transpose))/12.0);
	// loop over amiga pitch
	for (pitch pitch = 1; pitch < REAL_MAX_PITCH + LEEWAY; pitch++) {
		auto note_fr = base_freq / pitch; // note frequency (in Hz)
		/* int_to_fix(1) is the normalizing factor */
		auto step = note_fr / output_fr * int_to_fix(1) / oversample;
		step_table[pitch] = (unsigned long)(step);
	}
}
         
void 
audio_channel::readjust_current_steps(void)
{
	for (auto ch: allocated)
		ch->step = step_table[ch->pitch];
}

static void 
readjust_beat(void)
{
	number_samples = resampling_frequency * num / tempo / den;
}

static void 
notify_resample(watched var, long n)
{
	switch(var) {
	case watched::frequency:
		resampling_frequency = n;
		build_step_table(oversample, resampling_frequency);
		readjust_beat();
		audio_channel::readjust_current_steps();
		break;
	case watched::oversample:
		oversample = n;
		if (resampling_frequency) {
			build_step_table(oversample, resampling_frequency);
			audio_channel::readjust_current_steps();
		}
		break;
	default:
		break;
	}
}

static void 
init_resample(void)
{
	add_notify(notify_resample, watched::frequency);
	add_notify(notify_resample, watched::oversample);
}

void 
resampler::set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b)
{
	tempo = bpm;
	num = a; 
	den = b;
	readjust_beat();
}

static int max_side;	/* number of bits on one side */
static int max_sample;	/* number of bits for one sample */

void 
resampler::set_data_width(int side, int sample)
{
	max_side = side;
	max_sample = sample;
}

inline void
audio_channel::linear_value(int32_t* t)
{
	auto& v = t[side];

	switch(mode) {
	case audio_state::DO_NOTHING:
		break;
	case audio_state::PLAY:
	/* Since we now have fix_length, we can
	 * do that check with improved performance
	 */
		if (pointer < samp->fix_length) {
			auto step = fractional_part(pointer);
			v += (samp->start[C()] * (fixed_unit - step) +
			    samp->start[C()+1] * step) * (scaled_volume);
			pointer += step;
			break;
		} else {
		/* is there a replay ? */
			if (samp->rp_start) {
				mode = audio_state::REPLAY;
				pointer -= samp->fix_length;
				[[fallthrough]];
			} else {
				mode = audio_state::DO_NOTHING;
				break;
			}
		}
	case audio_state::REPLAY:
		while (pointer >= samp->fix_rp_length)
			pointer -= samp->fix_rp_length;
		auto step = fractional_part(pointer);
		v +=
		    (samp->rp_start[C()] * (fixed_unit - step) +
		    samp->rp_start[C()+1] * step) * scaled_volume ;
		pointer += step;
		break;
	}
}

inline void
linear_resample(void)
{
	int32_t value[NUMBER_SIDES];

	for (unsigned int i = 0; i < number_samples; i++) {
		value[LEFT_SIDE] = value[RIGHT_SIDE] = 0;
		for (auto ch: allocated)
			ch->linear_value(value);
		/* some assembly required... */
		output_samples(value[LEFT_SIDE], value[RIGHT_SIDE], 
		    ACCURACY+max_side);
	}
}

inline void
audio_channel::oversample_value(int32_t* t)
{
	auto& v = t[side];

	switch(mode) {
	case audio_state::DO_NOTHING:
		break;
	case audio_state::PLAY:
		/* Since we now have fix_length, we can
		 * do that check with improved performance
		*/
		if (pointer < samp->fix_length) {
			v += samp->start[C()] * scaled_volume;
			pointer += step;
			break;
		} else {
			/* is there a replay ? */
			if (samp->rp_start) {
				mode = audio_state::REPLAY;
				pointer -= samp->fix_length;
				[[fallthrough]];
			} else {
				mode = audio_state::DO_NOTHING;
				break;
			}
		}
	case audio_state::REPLAY:
		while (pointer >= samp->fix_rp_length)
			pointer -= samp->fix_rp_length;
		v += samp->rp_start[C()] * scaled_volume;
		pointer += step;
		break;
	}
}

inline void
over_resample(void)
{
	int32_t value[NUMBER_SIDES];

	value[LEFT_SIDE] = value[RIGHT_SIDE] = 0;

	unsigned int i;   /* sample counter */
	unsigned int sampling;     /* oversample counter */
	i = sampling = 0;
	while(true) {
		for (auto ch: allocated) {
			ch->oversample_value(value);
		}
		if (++sampling >= oversample) {
			sampling = 0;
			switch(oversample) {
			case 1:
				output_samples(value[LEFT_SIDE],
				    value[RIGHT_SIDE], max_side);
				break;
			case 2:
				output_samples(value[LEFT_SIDE], 
				    value[RIGHT_SIDE], max_side+1);
				break;
			case 4:
				output_samples(value[LEFT_SIDE],
				    value[RIGHT_SIDE], max_side+2);
				break;
			default:
				output_samples(value[LEFT_SIDE]/oversample,
				    value[RIGHT_SIDE]/oversample, max_side);
			}

			value[LEFT_SIDE] = value[RIGHT_SIDE] = 0;
			if (++i >= number_samples) 
				break;
		}
	}   
}

/* The resampling mechanism itself.
 * According to the current channel automaton,
 * we resample the instruments in real time to
 * generate output.
 */
void 
resampler::resample(void)
{
	/* do the resampling, i.e., actually play sounds */
	/* code unfolding for special cases */
	switch(oversample) {
	case 0:
		linear_resample();
	default:
		over_resample();
	}
	flush_buffer();
}


/*--------------------- Low level note player -------------------*/

/* setting up a given note */
void 
audio_channel::play(sample_info *samp_, ::pitch pitch_)
{
	pointer = 0;
	pitch = pitch_;
	step = step_table[pitch];
	/* need to change the volume there: play_note is the standard
	 * mechanism used to change samples. Since the volume lookup table
	 * is not necesssarily the same for each sample, we need to adjust
	 * the scaled_volume there
	 */
	samp = samp_;
	scaled_volume = samp->volume_lookup[volume];
	mode = audio_state::PLAY;
}

/* changing the current pitch (value may be temporary, and not stored
 * in channel pitch, for instance for vibratos)
 */
void 
audio_channel::set_pitch(::pitch pitch_)
{
	/* save current pitch in case we want to change
	 * the step table on the run
	 */
	pitch = pitch_;
	step = step_table[pitch];
}

/* changing the current volume. You HAVE to get through there so that it 
 * will work on EVERY machine.
 */
void 
audio_channel::set_volume(unsigned int volume_)
{
	volume = volume_;
	scaled_volume = samp->volume_lookup[volume];
}

void 
audio_channel::set_position(size_t position)
{
	pointer = int_to_fix(position);
	/* setting position too far must have this behavior for protracker */
	if (pointer >= samp->fix_length)
		mode = audio_state::DO_NOTHING;
	else
		mode = audio_state::PLAY;
}

