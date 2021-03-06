/* pro_low.C */
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

/* the amiga sound engine as it is used by Protracker and consorts.
 * (low-level layer for the soundtracker player, higher-level interface
 * to tracker resampling engine/Amiga audio.device control code)
 */

#include <memory>

#include "notes.h"
#include "channel.h"
#include "audio_channel.h"
#include "song.h"
#include "protracker.h"
#include "empty.h"
#include "minmax.h"

/* setting up a given note */
void 
channel::start_note()
{
	vib.offset = 0;
	trem.offset = 0;
	audio->play(samp, pitch);
}

void 
channel::stop_note()
{
	audio->play(empty_sample(), 0);
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
	audio->set_pitch(pitch_);
}

/* changing the current volume, storing it in ch->volume
 */
void 
channel::set_current_volume(int volume_)
{
	volume = MAX(MIN(volume_, MAX_VOLUME), MIN_VOLUME);
	audio->set_volume(volume);
}

/* changing the current volume WITHOUT storing it
 */
void 
channel::set_temp_volume(int volume_)
{
	volume_ = MAX(MIN(volume_, MAX_VOLUME), MIN_VOLUME);
	audio->set_volume(volume_);
}

void 
channel::set_position(size_t pos)
{
	audio->set_position(pos);
}

