/* automaton.c */

#include "defs.h"
#include "song.h"
#include "notes.h"
#include "p_automaton.h"
#include "automaton.h"
#include "extern.h"
#include "prefs.h"
#include "resample.h"
#include "timing.h"
     
/* set up the automaton so that I haven't got through patterns 
 * #from to #to
 */
LOCAL void clear_repeats(struct automaton *a, unsigned int from, 
	unsigned int upto)
   {
   unsigned int i;

   for (i = from; i <= upto; i++)
      a->gonethrough[i] = FALSE;
   }

/* set up the automaton so that I haven't got through any patterns
 */
LOCAL void reset_repeats(struct automaton *a)
   {
   clear_repeats(a, 0, a->info->length);
   a->gonethrough[a->info->length] = TRUE;
   }

/* update the pattern to play in the automaton. Checks that the pattern 
 * actually exists. Handle repetitions as well.
 */
LOCAL void set_pattern(struct automaton *a)
   {
   if (a->pattern_num >= a->info->length)
      {
      error = UNRECOVERABLE;
      return;
      }

   if (a->gonethrough[a->pattern_num])
      {
      error = ENDED;
      reset_repeats(a);
      }
   else
      a->gonethrough[a->pattern_num] = TRUE;

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
LOCAL void init_automaton(struct automaton *a, struct song *song, 
	unsigned int start)
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

struct automaton *setup_automaton(struct song *s, unsigned int start)
	{
	static struct automaton a;

	init_automaton(&a, s, start);
	return &a;
	}

/* get to the next pattern, and display stuff 
 */
LOCAL void advance_pattern(struct automaton *a)
   {
   if (++a->pattern_num >= a->info->length)
		{
		error = ENDED;
		reset_repeats(a);
      a->pattern_num = 0;
		}
   set_pattern(a);
   a->note_num = 0;
   }

void set_bpm(struct automaton *a, unsigned int bpm)
	{
	a->bpm = bpm;
	set_resampling_beat(a->bpm, NORMAL_FINESPEED, a->finespeed);
	}

void update_tempo(struct automaton *a)
   {        
      /* there are three classes of speed changes:
       * 0 does nothing. (should stop for genuine protracker)
       * <32 is the effect speed (resets the fine speed).
       * >=32 changes the finespeed, default 125
       */

	switch(a->do_stuff & (SET_SPEED | SET_FINESPEED))
		{
	case SET_SPEED | SET_FINESPEED:
		if (get_pref_scalar(PREF_SPEEDMODE) != FINESPEED_ONLY)
			{
			a->finespeed = a->new_finespeed;
			set_resampling_beat(a->bpm, NORMAL_FINESPEED, a->finespeed);
			}
		if (get_pref_scalar(PREF_SPEEDMODE) != SPEED_ONLY)
			a->speed = a->new_speed;
		break;
	case SET_SPEED:
		a->speed = a->new_speed;
		if (get_pref_scalar(PREF_SPEEDMODE) == ALTER_PROTRACKER)
			{
			a->finespeed = NORMAL_FINESPEED;
			set_resampling_beat(a->bpm, 1, 1);
			}
		break;
	case SET_FINESPEED:
		a->finespeed = a->new_finespeed;
		set_resampling_beat(a->bpm, NORMAL_FINESPEED, a->finespeed);
		break;
	default:
		break;
		}

	if (a->finespeed == 0)
		{
		status("Finespeed of 0");
		a->finespeed = NORMAL_FINESPEED;
		set_resampling_beat(a->bpm, NORMAL_FINESPEED, a->finespeed);
		error = FAULT;
		}
   }

/* process all the stuff which we need to advance in the song,
 * including set_skip, set_fastskip, and set_loop.
 */
void next_tick(struct automaton *a)
   {
   a->time_spent += ratio2time(NORMAL_FINESPEED, a->finespeed * 50);
   if (++a->counter >= a->speed)
      {
      a->counter = 0;
				/* if we are in delay mode, count down delay */
		if (a->delay_counter > 0)
			a->delay_counter--;
			/* get to next tick ONLY if no delay */
		if (a->delay_counter == 0)
			{
				/* loop: may change note in pattern right away */
			if (a->do_stuff & JUMP_PATTERN)
				a->note_num = a->loop_note_num;
			else if (a->do_stuff & SET_FASTSKIP)
				{
				a->pattern_num = a->new_pattern;
				set_pattern(a);
				a->note_num = 0;
				}
			else if (a->do_stuff & SET_SKIP)
				{
				advance_pattern(a);
				a->note_num = a->new_note;
				}
			else
				{
            if (++a->note_num >= a->info->plength)
               {
               advance_pattern(a);
               }
				}
			a->do_stuff = DO_SET_NOTHING;
         }
      }


   }


struct event *EVENT(struct automaton *a, int channel)
	{
	return &(a->pattern->e[channel * a->info->plength + a->note_num]);
	}
