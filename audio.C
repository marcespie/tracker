#include <functional>
#include "audio.h"
#include "openbsd_audio.h"
#include "watched_var.h"

void
audio::set_mix(int mix)
{
	mix_percent = mix;
	if (opened)
		::set_mix(mix_percent);
}

void
audio::open()
{
	if (!opened) {
		real_freq = open_audio(ask_freq, want_stereo);
		::set_mix(mix_percent);
		set_watched(watched::frequency, real_freq);
		opened = true;
	}
}

audio::~audio()
{
	if (opened)
		close_audio();
}

void
audio::set_freq(unsigned long f)
{
	ask_freq = f;
}

void
audio::set_stereo(bool s)
{
	want_stereo = s;
}

void
audio::output(int32_t left, int32_t right, int n)
{
	output_samples(left, right, n);
}

void
audio::flush()
{
	flush_buffer();
}

void
audio::discard()
{
	discard_buffer();
}

void
audio::sync(std::function<void()> f, std::function<void()> f2)
{
	sync_audio(f, f2);
}
