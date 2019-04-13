// resampler.h
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
#include <unordered_set>

enum class watched;
class audio_channel;
enum {LEFT_SIDE, RIGHT_SIDE, NUMBER_SIDES};

class resampler {
	std::function<void(watched, long)> frequency_f, oversample_f;
	std::unordered_set<audio_channel *> allocated[NUMBER_SIDES];
	void notify_frequency(long);
	void notify_oversample(long);
	void readjust_current_steps();
	inline void linear_resample();
	inline void over_resample();

public:
	resampler();
	~resampler();
	void set_data_width(int side, int sample);
	void resample();
	void set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b);
	friend class audio_channel;
};

