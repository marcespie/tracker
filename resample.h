/* resample.h */
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

struct audio_channel;

/* release_audio_channels:
 * free every audio channel previously allocated
 */
extern void release_audio_channels(void);

enum {LEFT_SIDE, RIGHT_SIDE, NUMBER_SIDES};

/* chan = new_channel_tag_list(prop):
 * allocates a new channel for the current song
 * LEFT_SIDE RIGHT_SIDE
 */
extern audio_channel *new_channel(int side);

/* set_data_width(side_width, sample_width):
 * accumulated data on each side will have width side_width bits,
 * and each sample will never be greater than sample_width
 */
extern void set_data_width(int side, int sample);

/* resample():
 * send out a batch of samples
 */
extern void resample(void);

/* set_resampling_beat(bpm, a, b):
 * set bpm to bpm, tempo to  a/b
 */
extern void set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b);

/* prep_sample_info(info):
 * set up secondary data structure for faster resampling
 */
extern void prep_sample_info(sample_info *info);
/* play_note(au, samp, pitch)
 * set audio channel au to play samp at pitch
 */
extern void play_note(audio_channel *au, sample_info *samp, 
pitch pitch);

/* set_play_pitch(au, pitch):
 * set channel au to play at pitch pitch
 */
extern void set_play_pitch(audio_channel *au, pitch pitch);

/* set_play_volume(au, volume):
 * set channel au to play at volume volume
 */
extern void set_play_volume(audio_channel *au, unsigned int volume);

/* set_play_position(au, pos):
 * set position in sample for channel au at given offset
 */
extern void set_play_position(audio_channel *au, size_t position);

