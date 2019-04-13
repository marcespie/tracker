/* audio_channel.h */
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

class resampler;

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
	resampler& r;
public:
	const int side;
	audio_channel(int, resampler&);
	~audio_channel();
	void play(sample_info *, ::pitch);
	void set_pitch(::pitch);
	void set_volume(unsigned int);
	void set_position(size_t);
	inline void linear_value(int32_t*);
	inline void oversample_value(int32_t*);
	inline void readjust_step();
};

/* prep_sample_info(info):
 * set up secondary data structure for faster resampling
 */
extern void prep_sample_info(sample_info *info);
