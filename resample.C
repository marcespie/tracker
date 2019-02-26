/* resample.c */

#include <math.h>

#include "defs.h"
#include "song.h"
#include "notes.h"
#include "channel.h"
#include "tags.h"
#include "extern.h"
#include "prefs.h"
#include "resample.h"
#include "p_resample.h"
#include "autoinit.h"
#include "empty.h"
#include "watched_var.h"
     
LOCAL void init_resample(void);
LOCAL void (*INIT)(void) = init_resample;


/*---------- Channels allocation mechanism -----------------------*/

/* Fixed structure for `hardware audio channels' */
LOCAL int allocated = 0;
	
/* log number of channels allocated according to side */
LOCAL int total[NUMBER_SIDES];

struct audio_channel *new_channel_tag_list(struct tag *prop)
   {
   struct audio_channel *n;

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
	n->side = NO_SIDE;
	n->scaled_volume = 0;
   n->samp = empty_sample();

		/* handling allocation tags (only tag right now is SIDE) */
	while ( (prop = get_tag(prop)) )
		{
		switch(prop->type)
			{
		case AUDIO_SIDE:
			n->side = prop->data.scalar;
			break;
		default:
			break;
			}
		prop++;
		}

		/* checking allocation */
	if (n->side < 0 || n->side == NO_SIDE || n->side >= NUMBER_SIDES)
		end_all("Improper alloc channel call (side)");
		/* logging number of channels per side */
	total[n->side]++;
   return n;
   }

void release_audio_channels(void)
   {
	unsigned int i;
	
	for (i = 0; i < NUMBER_SIDES; i++)
		total[i] = 0;

   allocated = 0;
   }






/*---------- Resampling engine ------------------------------------*/

LOCAL unsigned long step_table[REAL_MAX_PITCH + LEEWAY];  
                  /* holds the increment for finding the next sampled
                   * byte at a given pitch (see resample() ).
                   */

LOCAL unsigned int oversample;
LOCAL unsigned long resampling_frequency;
LOCAL unsigned int tempo = 50;
LOCAL unsigned int num, den = 1;
LOCAL unsigned int number_samples;

void prep_sample_info(struct sample_info *info)
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
LOCAL void build_step_table(
	int oversample, 			/* use i sample for each value output */
	unsigned long output_fr /* output frequency */
	)
   {
   double note_fr; 			/* note frequency (in Hz) */
   double step;
	double base_freq;
   pitch pitch;      			/* actual amiga pitch */

		/* special case: oversample of 0 means linear resampling */
	if (oversample == 0)
		oversample = 1;
   step_table[0] = 0;
	base_freq = AMIGA_CLOCKFREQ * pow(2.0, 
		(double)get_pref_scalar(PREF_TRANSPOSE)/12.0);
   for (pitch = 1; pitch < REAL_MAX_PITCH + LEEWAY; pitch++)
      {
      note_fr = base_freq / pitch;
         /* int_to_fix(1) is the normalizing factor */
      step = note_fr / output_fr * int_to_fix(1) / oversample;
      step_table[pitch] = (unsigned long)step;
      }
   }
         
LOCAL void readjust_current_steps(void)
   {
   int i;

   for (i = 0; i < allocated; i++)
      chan[i].step = step_table[chan[i].pitch];
   }

LOCAL void readjust_beat(void)
	{
	number_samples = resampling_frequency * num / tempo / den;
	}

LOCAL void 
notify_resample(enum watched_var var, long n)
{
	switch(var) {
	case FREQUENCY:
		resampling_frequency = n;
		build_step_table(oversample, resampling_frequency);
		readjust_beat();
		readjust_current_steps();
		break;
	case OVERSAMPLE:
		oversample = n;
		if (resampling_frequency) {
			build_step_table(oversample, resampling_frequency);
			readjust_current_steps();
		}
		break;
	}
}

LOCAL void 
init_resample(void)
{
	add_notify(notify_resample, FREQUENCY);
	add_notify(notify_resample, OVERSAMPLE);
}

void set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b)
	{
	tempo = bpm;
	num = a; 
	den = b;
	readjust_beat();
	}

LOCAL int max_side;		/* number of bits on one side */
LOCAL int max_sample;	/* number of bits for one sample */

void set_data_width(int side, int sample)
	{
	max_side = side;
	max_sample = sample;
	}

/* The resampling mechanism itself.
 * According to the current channel automaton,
 * we resample the instruments in real time to
 * generate output.
 */
void resample(void)
   {
   int i;            /* sample counter */
   int channel;      /* channel counter */
   int sampling;     /* oversample counter */
	int step;         /* fractional part for linear resampling */
   long value[NUMBER_SIDES];
                     /* recombinations of the various data */
   struct audio_channel *ch;


      /* safety check: we can't have a segv there, provided
       * chan points to a valid sample.
       * For `empty' samples, what is needed is fix_length == 0
       * and rp_start == NULL
       */

		/* do the resampling, i.e., actually play sounds */
		/* code unfolding for special cases */
	switch(oversample)
		{
	case 0:	/* linear resampling */
      for (i = 0; i < number_samples; i++) 
         {
			value[LEFT_SIDE] = value[RIGHT_SIDE] = 0;
         for (channel = 0; channel < allocated; channel++)
            {
            ch = chan + channel;
            switch(ch->mode)
               {
            case DO_NOTHING:
               break;
            case PLAY:
                  /* Since we now have fix_length, we can
                   * do that check with improved performance
                   */
               if (ch->pointer < ch->samp->fix_length)
                  {
						step = fractional_part(ch->pointer);
						value[ch->side] += 
							 (ch->samp->start[C] * (fixed_unit - step) +
							   ch->samp->start[C+1] * step)
							 * (ch->scaled_volume);
                  ch->pointer += ch->step;
                  break;
                  }
               else
                  {
                     /* is there a replay ? */
						if (ch->samp->rp_start)
							{
							ch->mode = REPLAY;
							ch->pointer -= ch->samp->fix_length;
							/* FALLTHRU */
							}
						else
							{
							ch->mode = DO_NOTHING;
							break;
							}
                  }
            case REPLAY:
               while (ch->pointer >= ch->samp->fix_rp_length)
                  ch->pointer -= ch->samp->fix_rp_length;
					step = fractional_part(ch->pointer);
					value[ch->side] += 
					 	(ch->samp->rp_start[C] * (fixed_unit - step) +
							ch->samp->rp_start[C+1] * step)
						 * ch->scaled_volume ;
               ch->pointer += ch->step;
               break;
               }
            } 
				/* some assembly required... */
         output_samples(value[LEFT_SIDE], value[RIGHT_SIDE], ACCURACY+max_side);
         }
		break;
	default:		/* standard oversampling code */
		value[LEFT_SIDE] = value[RIGHT_SIDE] = 0;
		i = sampling = 0;
		while(true)
         {
         for (channel = 0; channel < allocated; channel++)
				{
				ch = chan + channel;
				switch(ch->mode)
					{
				case DO_NOTHING:
					break;
				case PLAY:
						/* Since we now have fix_length, we can
						 * do that check with improved performance
						 */
					if (ch->pointer < ch->samp->fix_length)
						{
						value[ch->side] += ch->samp->start[C] * ch->scaled_volume;
						ch->pointer += ch->step;
						break;
						}
					else
						{
							/* is there a replay ? */
						if (ch->samp->rp_start)
							{
							ch->mode = REPLAY;
							ch->pointer -= ch->samp->fix_length;
								/* FALLTHRU */
							}
						else
							{
							ch->mode = DO_NOTHING;
							break;
							}
						}
				case REPLAY:
					while (ch->pointer >= ch->samp->fix_rp_length)
						ch->pointer -= ch->samp->fix_rp_length;
					value[ch->side] += ch->samp->rp_start[C] * ch->scaled_volume;
					ch->pointer += ch->step;
					break;
					}
				}
			if (++sampling >= oversample)
				{
				sampling = 0;
				switch(oversample)
					{
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
   flush_buffer();
   }





/*--------------------- Low level note player -------------------*/

/* setting up a given note */
void play_note(struct audio_channel *au, 
	struct sample_info *samp, 
	pitch pitch)
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
void set_play_pitch(struct audio_channel *au, pitch pitch)
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
void set_play_volume(struct audio_channel *au, unsigned int volume)
   {
   au->volume = volume;
	au->scaled_volume = au->samp->volume_lookup[volume];
   }

void set_play_position(struct audio_channel *au, size_t position)
   {
   au->pointer = int_to_fix(position);
			/* setting position too far must have this behavior for protracker */
	if (au->pointer >= au->samp->fix_length)
		au->mode = DO_NOTHING;
	else
		au->mode = PLAY;
   }

