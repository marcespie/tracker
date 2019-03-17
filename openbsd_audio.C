/* openbsd/audio.c 
 */
/* sndio(7) interface */

#include "defs.h"
#include <unistd.h>
#include "extern.h"
#include "prefs.h"
#include "autoinit.h"
#include "watched_var.h"
#include <sndio.h>
#include <queue>
struct options_set *port_options=0;

#define UNSIGNED8

/* fine-tune to get the scrolling display in sync with the music */
#define ADVANCE_TAGS 20000 

/* this macro works with unsigned values !!! */
template<typename S, typename T>
inline auto
absdiff(S x, T y)
{
	return x<y ?  y-x : x-y;
}

static int stereo;


static unsigned long pps[32], pms[32];

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
}



#ifdef UNSIGNED16
static unsigned short *buffer16;
#define VALUE16(x)	((x)+32768)
#else
static short *buffer16;
#define VALUE16(x)	(x)
#endif

#ifdef UNSIGNED8
static unsigned char *buffer;
#define VALUE8(x)		((x)+128)
#else
static char *buffer;
#define VALUE8(x)		(x)
#endif
static unsigned long idx;
static int dsize;			/* current data size */
static unsigned long samples_max;	/* number of samples in buffer */


static void 
add_samples16_stereo(long left, long right, int n)
{
	if (pms[n] == pps[n]) {	/* no mixing */
		if (n<16) {
			buffer16[idx++] = VALUE16(left << (16-n) );
			buffer16[idx++] = VALUE16(right << (16-n) );
		} else {
			buffer16[idx++] = VALUE16(left >> (n-16) );
			buffer16[idx++] = VALUE16(right >> (n-16) );
		}
	} else {
		long s1 = (left+right)*pps[n];
		long s2 = (left-right)*pms[n];

		buffer16[idx++] = VALUE16( (s1 + s2) >> 16);
		buffer16[idx++] = VALUE16( (s1 - s2) >> 16);
	}
}

static void 
add_samples16_mono(long left, long right, int n)
{
	if (n<15)		/* is this possible? */
		buffer16[idx++] = VALUE16( (left + right) << (15-n) );
	else
		buffer16[idx++] = VALUE16( (left + right) >> (n-15) );
}

static void 
add_samples16(long left, long right, int n)
{
	if (stereo)
		add_samples16_stereo(left, right, n);
	else
		add_samples16_mono(left, right, n);
}

static void 
add_samples8_stereo(long left, long right, int n)
{
	if (pms[n] == pps[n]) {	/* no mixing */
		/* if n<8 -> same problem as above,
		but that won't happen, right? */
		buffer[idx++] = VALUE8(left >> (n-8) );
		buffer[idx++] = VALUE8(right >> (n-8) );
	} else {
		long s1 = (left+right)*pps[n];
		long s2 = (left-right)*pms[n];

		buffer[idx++] = VALUE8( (s1 + s2) >> 24);
		buffer[idx++] = VALUE8( (s1 - s2) >> 24);
	}
}

static void 
add_samples8_mono(long left, long right, int n)
{
	buffer[idx++] = VALUE8( (left+right) >> (n-7) );
}

static void 
add_samples8(long left, long right, int n)
{
	if (stereo)
		add_samples8_stereo(left, right, n);
	else
		add_samples8_mono(left, right, n);
}





static long long realpos;
static struct sio_hdl *hdl;           	
static unsigned long current_freq;

unsigned long total;

static int dsp_samplesize = 0;

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
		end_all("Error opening audio device");

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
		end_all("Sorry, no audio format supported by this binary is available");

	int buf_max = par.appbufsz * par.bps * par.pchan;
	current_freq = par.rate;
	stereo = par.pchan == 2 ? 1 : 0;

	dsp_samplesize = par.bits;
	dsize = par.bps;
	buffer = new unsigned char [buf_max];
	buffer16 = (short *)buffer;

	idx = 0;
	samples_max = buf_max / dsize / par.pchan;
	set_watched(watched::frequency, current_freq);
	total = 0;
	return current_freq;
}

/* synchronize stuff with audio output */

struct tagged {
	using callback= std::function<void ()>;
	callback f;		/* function to call */
	callback f2;		/* function to call  for flush */
	unsigned long when;	/* number of bytes to let go before calling */
	tagged(callback f_, callback f2_, unsigned long when_):
	    f{f_}, f2{f2_}, when{when_}
	{
	}
};

std::queue<tagged> q;


/* flush_tags: use tags that have gone by recently */
static void 
flush_tags(void)
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
remove_pending_tags(void)
{
	while (!q.empty()) {
		q.front().f2();
		q.pop();
	}
}

void 
sync_audio(tagged::callback f, tagged::callback f2)
{
	if (hdl) {
		q.emplace(f, f2, total);
	} else
		f();
}

static void 
actually_flush_buffer(void)
{
	if (idx) {
		total += idx * dsize;
		sio_write(hdl, buffer, dsize * idx);
	}
	idx = 0;
}

void 
output_samples(long left, long right, int n)
{
	if (idx >= samples_max - 1)
		actually_flush_buffer();
	switch(dsp_samplesize) {
	case 16:
		add_samples16(left, right, n);
		break;
	case 8:
		add_samples8(left, right, n);
		break;
	default:	/* should not happen */
		;
	}
}

void 
flush_buffer(void)
{	
	actually_flush_buffer();
	flush_tags();
}

/*
 * Closing the sound device waits for all pending samples to play.
 */
void 
close_audio(void)
{
	actually_flush_buffer();
	sio_close(hdl);
	delete [] buffer;
}

unsigned long 
update_frequency(void)
{
	return 0;
}

void 
discard_buffer(void)
{
	remove_pending_tags();
	total = 0;
}

