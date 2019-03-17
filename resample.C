/* resample.c */

#include <math.h>

#include "defs.h"
#include "song.h"
#include "notes.h"
#include "channel.h"
#include "extern.h"
#include "prefs.h"
#include "resample.h"
#include "autoinit.h"
#include "empty.h"
#include "watched_var.h"
     
const auto MAX_CHANNELS=8;

enum audio_state { DO_NOTHING, PLAY, REPLAY};

static struct audio_channel {
	sample_info *samp;
	enum audio_state mode;
	unsigned long pointer;
	unsigned long step;
	unsigned int volume;
	unsigned int scaled_volume;
	pitch pitch;
	int side;
} chan[MAX_CHANNELS];

/* define NO_SIDE for errors (no side specified) */
const auto NO_SIDE=-1;

/* Have to get some leeway for vibrato (since we don't bound pitch with
 * vibrato). This is conservative.
 */
const auto LEEWAY=150;


/* macros for fixed point arithmetic */
/* NOTE these should be used ONLY with unsigned values !!!! */

const auto ACCURACY=12;
#define fix_to_int(x) ((x) >> ACCURACY)
#define int_to_fix(x) ((x) << ACCURACY)
#define fractional_part(x) ((x) & (fixed_unit - 1))
#define fixed_unit	 (1 << ACCURACY)

#define C fix_to_int(ch.pointer)

static void init_resample(void);
static void (*INIT)(void) = init_resample;


/*---------- Channels allocation mechanism -----------------------*/

/* Fixed structure for `hardware audio channels' */
static int allocated = 0;
	
/* log number of channels allocated according to side */
static int total[NUMBER_SIDES];

audio_channel *
new_channel(int side)
{
	audio_channel *n;

	INIT_ONCE;

	if (allocated >= MAX_CHANNELS)
		end_all("Maximum number of channels exceeded %d", MAX_CHANNELS);

	/* base channel setup */
	n = &chan[allocated++];
	n->mode = DO_NOTHING;
	n->pointer = 0;
	n->step = 0;
	n->pitch = 0;
	n->volume = 0;
	n->side = side;
	n->scaled_volume = 0;
	n->samp = empty_sample();

	/* checking allocation */
	if (n->side < 0 || n->side == NO_SIDE || n->side >= NUMBER_SIDES)
		end_all("Improper alloc channel call (side)");
	/* logging number of channels per side */
	total[n->side]++;
	return n;
}

void 
release_audio_channels(void)
{
	for (unsigned int i = 0; i < NUMBER_SIDES; i++)
		total[i] = 0;

	allocated = 0;
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
    int oversample, 			/* use i sample for each value output */
    unsigned long output_fr /* output frequency */
    )
{
	/* special case: oversample of 0 means linear resampling */
	if (oversample == 0)
		oversample = 1;
	step_table[0] = 0;
	auto base_freq = AMIGA_CLOCKFREQ * pow(2.0, double(get_pref(Pref::transpose))/12.0);
	// loop over amiga pitch
	for (pitch pitch = 1; pitch < REAL_MAX_PITCH + LEEWAY; pitch++) {
		auto note_fr = base_freq / pitch; // note frequency (in Hz)
		/* int_to_fix(1) is the normalizing factor */
		auto step = note_fr / output_fr * int_to_fix(1) / oversample;
		step_table[pitch] = (unsigned long)(step);
	}
}
         
static void 
readjust_current_steps(void)
{
	for (int i = 0; i < allocated; i++)
		chan[i].step = step_table[chan[i].pitch];
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
		readjust_current_steps();
		break;
	case watched::oversample:
		oversample = n;
		if (resampling_frequency) {
			build_step_table(oversample, resampling_frequency);
			readjust_current_steps();
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
set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b)
{
	tempo = bpm;
	num = a; 
	den = b;
	readjust_beat();
}

static int max_side;		/* number of bits on one side */
static int max_sample;	/* number of bits for one sample */

void 
set_data_width(int side, int sample)
{
	max_side = side;
	max_sample = sample;
}

inline void
linear_value(audio_channel& ch, long& v)
{
	switch(ch.mode) {
	case DO_NOTHING:
		break;
	case PLAY:
	/* Since we now have fix_length, we can
	 * do that check with improved performance
	 */
		if (ch.pointer < ch.samp->fix_length) {
			auto step = fractional_part(ch.pointer);
			v += 
			    (ch.samp->start[C] * (fixed_unit - step) +
			    ch.samp->start[C+1] * step)
			    * (ch.scaled_volume);
			ch.pointer += ch.step;
			break;
		} else {
		/* is there a replay ? */
			if (ch.samp->rp_start) {
				ch.mode = REPLAY;
				ch.pointer -= ch.samp->fix_length;
				[[fallthrough]];
			} else {
				ch.mode = DO_NOTHING;
				break;
			}
		}
	case REPLAY:
		while (ch.pointer >= ch.samp->fix_rp_length)
			ch.pointer -= ch.samp->fix_rp_length;
		auto step = fractional_part(ch.pointer);
		v +=
		    (ch.samp->rp_start[C] * (fixed_unit - step) +
		    ch.samp->rp_start[C+1] * step)
		    * ch.scaled_volume ;
		ch.pointer += ch.step;
		break;
	}
}

inline void
linear_resample(void)
{
	long value[NUMBER_SIDES];

	for (unsigned int i = 0; i < number_samples; i++) {
		value[LEFT_SIDE] = value[RIGHT_SIDE] = 0;
		for (int channel = 0; channel < allocated; channel++) {
			auto& ch = chan[channel];
			linear_value(ch, value[ch.side]);
		} 
		/* some assembly required... */
		output_samples(value[LEFT_SIDE], value[RIGHT_SIDE], 
		    ACCURACY+max_side);
	}
}

inline void
oversample_value(audio_channel& ch, long& v)
{
	switch(ch.mode) {
	case DO_NOTHING:
		break;
	case PLAY:
		/* Since we now have fix_length, we can
		 * do that check with improved performance
		*/
		if (ch.pointer < ch.samp->fix_length) {
			v += ch.samp->start[C] * ch.scaled_volume;
			ch.pointer += ch.step;
			break;
		} else {
			/* is there a replay ? */
			if (ch.samp->rp_start) {
				ch.mode = REPLAY;
				ch.pointer -= ch.samp->fix_length;
				[[fallthrough]];
			} else {
				ch.mode = DO_NOTHING;
				break;
			}
		}
	case REPLAY:
		while (ch.pointer >= ch.samp->fix_rp_length)
			ch.pointer -= ch.samp->fix_rp_length;
		v += ch.samp->rp_start[C] * ch.scaled_volume;
		ch.pointer += ch.step;
		break;
	}
}

inline void
over_resample(void)
{
	long value[NUMBER_SIDES];

	value[LEFT_SIDE] = value[RIGHT_SIDE] = 0;

	unsigned int i;   /* sample counter */
	unsigned int sampling;     /* oversample counter */
	i = sampling = 0;
	while(true) {
		for (int channel = 0; channel < allocated; channel++) {
			auto& ch = chan[channel];
			oversample_value(ch, value[ch.side]);
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
resample(void)
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
play_note(audio_channel *au, sample_info *samp, pitch pitch)
{
	au->pointer = 0;
	au->pitch = pitch;
	au->step = step_table[pitch];
	/* need to change the volume there: play_note is the standard
	 * mechanism used to change samples. Since the volume lookup table
	 * is not necesssarily the same for each sample, we need to adjust
	 * the scaled_volume there
	 */
	au->samp = samp;
	au->scaled_volume = au->samp->volume_lookup[au->volume];
	au->mode = PLAY;
}

/* changing the current pitch (value may be temporary, and not stored
 * in channel pitch, for instance for vibratos)
 */
void 
set_play_pitch(audio_channel *au, pitch pitch)
{
	/* save current pitch in case we want to change
	 * the step table on the run
	 */
	au->pitch = pitch;
	au->step = step_table[pitch];
}

/* changing the current volume. You HAVE to get through there so that it 
 * will work on EVERY machine.
 */
void 
set_play_volume(audio_channel *au, unsigned int volume)
{
	au->volume = volume;
	au->scaled_volume = au->samp->volume_lookup[volume];
}

void set_play_position(audio_channel *au, size_t position)
{
	au->pointer = int_to_fix(position);
	/* setting position too far must have this behavior for protracker */
	if (au->pointer >= au->samp->fix_length)
		au->mode = DO_NOTHING;
	else
		au->mode = PLAY;
}

