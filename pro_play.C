/* Modules/Pro/play.c */

#include "defs.h"
#include "song.h"
#include "notes.h"
#include "channel.h"
#include "extern.h"
#include "tags.h"
#include "prefs.h"
#include "pro_effects.h"
#include "p_automaton.h"
#include "automaton.h"
#include "autoinit.h"
#include "resample.h"
#include "pro_play.h"
#include "pro_low.h"
#include "empty.h"
     

extern short vibrato_table[3][64];

static void init_st_play(void);
static void (*INIT)(void) = init_st_play;

/**************
 **************
 **************/


static struct st_effect eval[NUMBER_EFFECTS];
                    		/* the effect table */

static struct channel chan[MAX_TRACKS];
                    /* every channel */

static unsigned int ntracks;		/* number of tracks of the current song */
static struct sample_info **voices;



/* init_channel(ch, dummy):
 * setup channel, with initially a dummy sample ready to play, and no note.
 */
static void 
init_channel(channel *ch, int side)
{
	tag tags[2];
	tags[0].type = AUDIO_SIDE;
	tags[0].data.scalar = side;
	tags[1].type = TAG_END;
	ch->samp = empty_sample();
	ch->finetune = 0;
	ch->audio = new_channel_tag_list(tags);
	ch->volume = 0; 
	ch->pitch = 0; 
	ch->note = NO_NOTE;

	/* we don't setup arpeggio values. */
	ch->vib.offset = 0; 
	ch->vib.depth = 0;
	ch->vib.rate = 0;
	ch->vib.table = vibrato_table[0];
	ch->vib.reset = false;

	ch->trem.offset = 0;
	ch->trem.depth = 0;
	ch->trem.rate = 0;
	ch->trem.table = vibrato_table[0];
	ch->trem.reset = false;

	ch->slide = 0; 

	ch->pitchgoal = 0; 
	ch->pitchrate = 0;

	ch->volumerate = 0;


	ch->funk_glissando = false;
	ch->start_offset = 0;
	ch->adjust = do_nothing;
	/* initialize loop to no loop, loop start at 0 
	 * (needed for don't you want me, for instance) */
	ch->loop_counter = -1;
	ch->loop_note_num = 0;

	ch->special = do_nothing;
	ch->invert_speed = 0;
	ch->invert_offset = 0;
	ch->invert_position = 0;
}


static void 
init_channels(void)
{
	release_audio_channels();

	init_channel(chan, LEFT_SIDE);
	init_channel(chan + 1, RIGHT_SIDE);
	init_channel(chan + 2, RIGHT_SIDE);
	init_channel(chan + 3, LEFT_SIDE);
	if (ntracks > 4) {
		init_channel(chan + 4, LEFT_SIDE);
		init_channel(chan + 5, RIGHT_SIDE);
	}
	if (ntracks > 6) {
		init_channel(chan + 6, RIGHT_SIDE);
		init_channel(chan + 7, LEFT_SIDE);
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
	dump_event(chan, EVENT(a, 0));
	dump_delimiter();
	dump_event(chan+3, EVENT(a, 3));
	dump_delimiter();
	if (ntracks > 4) {
		dump_event(chan+4, EVENT(a, 4));
		dump_delimiter();
	}
	if (ntracks > 7) {
		dump_event(chan+7, EVENT(a, 7));
		dump_delimiter();
	}
	dump_delimiter();
	dump_event(chan+1, EVENT(a, 1));
	dump_delimiter();
	dump_event(chan+2, EVENT(a, 2));
	if (ntracks > 5) {
		dump_delimiter();
		dump_event(chan+5, EVENT(a, 5));
	}
	if (ntracks > 6) {
		dump_delimiter();
		dump_event(chan+6, EVENT(a, 6));
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
				setup_effect(chan + channel, a, 
				    EVENT(a, channel));
			if (get_pref(Pref::show))
				dump_events(a);
		}
	} else
		for (unsigned channel = 0; channel < ntracks; channel++) {
			/* do the effects */
			(chan[channel].special)(chan + channel);
			(chan[channel].adjust)(chan + channel);
		}

	update_tempo(a);
	/* actually output samples */
	if (get_pref(Pref::output))
		resample();
}

static tag pres[2];

tag *
play_song(song *song, unsigned int start)
{
	int countup;      /* keep playing the tune or not */
	int r;
	automaton *a;

	INIT_ONCE;

	song_title(song->title);
	pres[1].type = TAG_END;

	ntracks = song->ntracks;
	set_number_tracks(ntracks);

	countup = 0;

	voices = song->samples; 

	a = setup_automaton(song, start);
	set_bpm(a, get_pref(Pref::speed));

	init_channels();

	set_data_width(song->side_width, song->max_sample_width);

	while(true) {
		struct tag *result;

		play_one_tick(a);
		next_tick(a);
		result = get_ui();
		while( (result = get_tag(result)) ) {
			switch(result->type) {  
			case UI_LOAD_SONG:
				if (!result->data.pointer)
					break;
				/*FALLTHRU*/
			case UI_NEXT_SONG:
			case UI_PREVIOUS_SONG:
				discard_buffer();
				pres[0].type = result->type;
				pres[0].data = result->data;
				return pres;
			case UI_QUIT:
				discard_buffer();
				end_all(0);
				/* NOTREACHED */
			case UI_SET_BPM:
				set_bpm(a, result->data.scalar);
				break;
			case UI_RESTART:
				discard_buffer();
				a = setup_automaton(song, start);
				init_channels();
				break;
			case UI_JUMP_TO_PATTERN:
				if (result->data.scalar >= 0 && 
				    result->data.scalar < a->info->length) {
					discard_buffer();
					a = setup_automaton(song, result->data.scalar);
				}
				break;
			default:
				break;
			}
			result++;
		}

		switch(error) {
		case NONE:
			break;
		case ENDED:
			countup++;
			if ( (r = get_pref(Pref::repeats)) ) {
				if (countup >= r) {
					pres[0].type = PLAY_ENDED;
					return pres;
				}
			}
			break;
		case SAMPLE_FAULT:
		case FAULT:
		case PREVIOUS_SONG:
		case NEXT_SONG:
		case UNRECOVERABLE:
			if ( (error == SAMPLE_FAULT && get_pref(Pref::tolerate))
			    ||(error == FAULT && get_pref(Pref::tolerate) > 1) )
				break;
			pres[0].type = PLAY_ERROR;
			pres[0].data.scalar = error;
			return pres;
		default:
			break;
		}
		error = NONE;
	}
}

