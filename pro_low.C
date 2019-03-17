/* Modules/Pro/low.c */

/* the amiga sound engine as it is used by Protracker and consorts.
 * (low-level layer for the soundtracker player, higher-level interface
 * to tracker resampling engine/Amiga audio.device control code)
 */

#include "defs.h"
#include "notes.h"
#include "channel.h"
#include "pro_low.h"
#include "resample.h"
#include "song.h"
#include "empty.h"

/* setting up a given note */
void 
start_note(channel *ch)
{
	ch->vib.offset = 0;
	ch->trem.offset = 0;
	play_note(ch->audio, ch->samp, ch->pitch);
}

void 
stop_note(channel *ch)
{
	play_note(ch->audio, empty_sample(), 0);
}

void 
set_current_note(channel *ch, note note, pitch pitch)
{
	ch->pitch = pitch;
	ch->note = note;
}

/* changing the current pitch (value may be temporary, and so is not stored
 * in channel pitch, for instance vibratos)
 */
void 
set_temp_pitch(channel *ch, pitch pitch)
{
	set_play_pitch(ch->audio, pitch);
}

/* changing the current volume, storing it in ch->volume
 */
void 
set_current_volume(channel *ch, int volume)
{
	ch->volume = MAX(MIN(volume, MAX_VOLUME), MIN_VOLUME);
	set_play_volume(ch->audio, ch->volume);
}

/* changing the current volume WITHOUT storing it
 */
void set_temp_volume(channel *ch, int volume)
{
	volume = MAX(MIN(volume, MAX_VOLUME), MIN_VOLUME);
	set_play_volume(ch->audio, volume);
}

void 
set_position(channel *ch, size_t pos)
{
	set_play_position(ch->audio, pos);
}

