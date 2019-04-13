/* openbsd/audio.c */
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
/* sndio(7) interface */

#include <unistd.h>
#include <sndio.h>
#include <queue>
#include <iostream>
#include <limits>
#include <utility>
//#include "extern.h"
//#include "prefs.h"
#include "autoinit.h"
#include "watched_var.h"
#include "openbsd_audio.h"

// fine-tune to get the scrolling display in sync with the music
const auto ADVANCE_TAGS=20000;

/* this macro works with unsigned values !!! */
template<typename S, typename T>
inline auto
absdiff(S x, T y)
{
	return x<y ?  y-x : x-y;
}

static bool stereo;
static int32_t pps[32], pms[32];
static int dsp_samplesize = 0;


void (*add_samples)(int32_t, int32_t, int);


static void set_add_samples();

void 
set_mix(int percent)
{
	for (int i = 8; i < 32; i++) {
		pps[i] = 1 << (31 - i);
		if (i < 29)
			pms[i] = pps[i] - (percent << (29 - i) )/25;
		else
			pms[i] = pps[i] - (percent >> (i - 29) )/25;
	}
	set_add_samples();
}

template<typename T>
static T* buf;

template<> int16_t* buf<int16_t>;
template<> uint8_t* buf<uint8_t>;

template<typename T>
inline T
VALUE(int32_t x)
{
	if constexpr (!std::numeric_limits<T>::is_signed)
		return x + ((std::numeric_limits<T>::max()>>1U) +1);
	else
		return x;
}


static unsigned long idx;
static int dsize;			/* current data size */
static unsigned long samples_max;	/* number of samples in buffer */

// abstract specialization for each possibility
template<typename sample, bool stereo =true, bool mixing =stereo>
static void do_add_samples(int32_t, int32_t, int);

template<>
void 
do_add_samples<int16_t>(int32_t left, int32_t right, int n)
{
	int32_t s1 = (left+right)*pps[n];
	int32_t s2 = (left-right)*pms[n];

	buf<int16_t>[idx++] = VALUE<int16_t>( (s1 + s2) >> 16);
	buf<int16_t>[idx++] = VALUE<int16_t>( (s1 - s2) >> 16);
}

template<>
void 
do_add_samples<int16_t, true, false>(int32_t left, int32_t right, int n)
{
	if (n<16) {
		buf<int16_t>[idx++] = VALUE<int16_t>(left << (16-n) );
		buf<int16_t>[idx++] = VALUE<int16_t>(right << (16-n) );
	} else {
		buf<int16_t>[idx++] = VALUE<int16_t>(left >> (n-16) );
		buf<int16_t>[idx++] = VALUE<int16_t>(right >> (n-16) );
	}
}

template<>
void 
do_add_samples<int16_t,false>(int32_t left, int32_t right, int n)
{
	if (n<15)		/* is this possible? */
		buf<int16_t>[idx++] = VALUE<int16_t>( (left + right) << (15-n) );
	else
		buf<int16_t>[idx++] = VALUE<int16_t>( (left + right) >> (n-15) );
}

template<>
void 
do_add_samples<uint8_t>(int32_t left, int32_t right, int n)
{
	int32_t s1 = (left+right)*pps[n];
	int32_t s2 = (left-right)*pms[n];

	buf<uint8_t>[idx++] = VALUE<uint8_t>( (s1 + s2) >> 24);
	buf<uint8_t>[idx++] = VALUE<uint8_t>( (s1 - s2) >> 24);
}

template<>
void 
do_add_samples<uint8_t,true, false>(int32_t left, int32_t right, int n)
{
	/* if n<8 -> same problem as above,
	but that won't happen, right? */
	buf<uint8_t>[idx++] = VALUE<uint8_t>(left >> (n-8) );
	buf<uint8_t>[idx++] = VALUE<uint8_t>(right >> (n-8) );
}

template<>
void 
do_add_samples<uint8_t,false>(int32_t left, int32_t right, int n)
{
	buf<uint8_t>[idx++] = VALUE<uint8_t>( (left+right) >> (n-7) );
}

using audio_offset = unsigned long long;


static audio_offset realpos;
static struct sio_hdl *hdl;           	
static unsigned long current_freq;

unsigned long total;

static void
movecb(void *, int delta)
{
	realpos += delta * dsize * (stereo ? 2 : 1);
}

unsigned long 
open_audio(unsigned long f, int)
{
	hdl = sio_open(NULL, SIO_PLAY, 0);
	if (hdl == NULL)
		End() << "Error opening audio device";

	realpos = 0;
	sio_onmove(hdl, movecb, NULL);

	struct sio_par par;
	sio_initpar(&par);
	if (f)
		par.rate = f;
	par.pchan = 2;
	if (!sio_setpar(hdl, &par) || !sio_getpar(hdl, &par) || 
	    !sio_start(hdl) || (par.bits != 8 && par.bits != 16) || 
	    par.pchan > 2)
		End() << "Sorry, no audio format supported by this binary is available";

	int buf_max = par.appbufsz * par.bps * par.pchan;
	current_freq = par.rate;
	stereo = par.pchan == 2 ? true : false;

	dsp_samplesize = par.bits;
	dsize = par.bps;
	buf<uint8_t> = new uint8_t [buf_max];
	buf<int16_t> = reinterpret_cast<int16_t *>(buf<uint8_t>);
	set_add_samples();

	idx = 0;
	samples_max = buf_max / dsize / par.pchan;
	set_watched(watched::frequency, current_freq);
	total = 0;
	return current_freq;
}

static void
set_add_samples()
{
	bool mix = pps[10] == pms[10];
	if (dsp_samplesize == 16) {
		if (stereo) {
			if (mix)
				add_samples = do_add_samples<int16_t, true, false>;
			else
				add_samples = do_add_samples<int16_t>;
		} else
			add_samples = do_add_samples<int16_t, false>;
	} else {
		if (stereo) {
			if (mix)
				add_samples = do_add_samples<uint8_t, true, false>;
			else
				add_samples = do_add_samples<uint8_t>;
	    	} else
			add_samples = do_add_samples<uint8_t, false>;
	}
}

/* synchronize stuff with audio output */

class tagged {
public:
	using callback= std::function<void ()>;
	callback f;		/* function to call */
	callback f2;		/* function to call  for flush */
	audio_offset when;	/* number of bytes to let go before calling */
	tagged(callback f_, callback f2_, audio_offset when_):
	    f{f_}, f2{f2_}, when{when_}
	{
	}
};

std::queue<tagged> q;


/* flush_tags: use tags that have gone by recently */
static void 
flush_tags()
{
	while (!q.empty()) {
		if (q.front().when <= realpos + ADVANCE_TAGS) {
			q.front().f();
			q.pop();
		} else
			break;
	}
}

/* remove unused tags at end */
static void 
remove_pending_tags()
{
	while (!q.empty()) {
		q.front().f2();
		q.pop();
	}
}

void 
sync_audio(tagged::callback f, tagged::callback f2)
{
	if (hdl)
		q.emplace(f, f2, total);
	else
		f();
}

static void 
actually_flush_buffer()
{
	if (idx) {
		total += idx * dsize;
		sio_write(hdl, buf<uint8_t>, dsize * idx);
	}
	idx = 0;
}

void 
output_samples(int32_t left, int32_t right, int n)
{
	if (idx >= samples_max - 1)
		actually_flush_buffer();
	add_samples(left, right, n);
}

void 
flush_buffer()
{	
	actually_flush_buffer();
	flush_tags();
}

/*
 * Closing the sound device waits for all pending samples to play.
 */
void 
close_audio()
{
	actually_flush_buffer();
	sio_close(hdl);
	delete [] buf<uint8_t>;
}

void 
discard_buffer()
{
	remove_pending_tags();
	total = 0;
}

