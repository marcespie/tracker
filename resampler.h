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

class resampler {
	/* Have to get some leeway for vibrato (since we don't bound pitch with
	 * vibrato). This is conservative.
	 */
	static const auto LEEWAY=150;
	unsigned long step_table[REAL_MAX_PITCH + LEEWAY];  
	/* holds the increment for finding the next sampled
	 * byte at a given pitch (see resample() ).
	 */

	std::function<void(watched, long)> frequency_f, oversample_f;
	std::unordered_set<audio_channel *> allocated;
	void notify_frequency(long);
	void notify_oversample(long);
	void readjust_current_steps();
	inline void linear_resample();
	inline void over_resample();
	void readjust_beat();
	void build_step_table(
	    int oversample, 		/* use i sample for each value output */
	    unsigned long output_fr 	/* output frequency */
	);
	unsigned int oversample;
	unsigned long resampling_frequency;
	unsigned int tempo = 50;
	unsigned int num, den = 1;
	unsigned int number_samples;
	int max_side;	/* number of bits on one side */
	int max_sample;	/* number of bits for one sample */


public:
	resampler();
	~resampler();
	void set_data_width(int side, int sample);
	void resample();
	void set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b);
	inline void add(audio_channel *ch);
	inline void remove(audio_channel *ch);
	inline auto step(unsigned short p) const
	{
		return step_table[p];
	}
	//friend class audio_channel;
};

