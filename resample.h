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

class audio_channel {
	enum class audio_state { DO_NOTHING, PLAY, REPLAY};
	sample_info *samp;
	audio_state mode;
	unsigned long pointer;
	unsigned long step;
	unsigned int volume;
	unsigned int scaled_volume;
	::pitch pitch;
	inline auto C() const;
public:
	const int side;
	audio_channel(int);
	void play(sample_info *, ::pitch);
	void set_pitch(::pitch);
	void set_volume(unsigned int);
	void set_position(size_t);
	inline void linear_value(int32_t*);
	inline void oversample_value(int32_t*);
	static void readjust_current_steps(void);
};

/* release_audio_channels:
 * free every audio channel previously allocated
 */
extern void release_audio_channels(void);

enum {LEFT_SIDE, RIGHT_SIDE, NUMBER_SIDES};

#if 0
class resampler {
public:
	void set_data_width(int side, int sample);
	void resample();
	void set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b);
};
#endif

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
