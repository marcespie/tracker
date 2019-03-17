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
channel::start_note()
{
	vib.offset = 0;
	trem.offset = 0;
	play_note(audio, samp, pitch);
}

void 
channel::stop_note()
{
	play_note(audio, empty_sample(), 0);
}

void 
channel::set_current_note(::note note_, ::pitch pitch_)
{
	pitch = pitch_;
	note = note_;
}

/* changing the current pitch (value may be temporary, and so is not stored
 * in channel pitch, for instance vibratos)
 */
void 
channel::set_temp_pitch(::pitch pitch_)
{
	set_play_pitch(audio, pitch_);
}

/* changing the current volume, storing it in ch->volume
 */
void 
channel::set_current_volume(int volume_)
{
	volume = MAX(MIN(volume_, MAX_VOLUME), MIN_VOLUME);
	set_play_volume(audio, volume);
}

/* changing the current volume WITHOUT storing it
 */
void 
channel::set_temp_volume(int volume_)
{
	volume_ = MAX(MIN(volume_, MAX_VOLUME), MIN_VOLUME);
	set_play_volume(audio, volume_);
}

void 
channel::set_position(size_t pos)
{
	set_play_position(audio, pos);
}

