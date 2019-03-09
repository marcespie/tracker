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
struct options_set *port_options=0;

#define DEFAULT_BUFFERS
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

#define	BIAS		(0x84)		/* Bias for linear code. */

LOCAL int stereo;


LOCAL unsigned long pps[32], pms[32];

void set_mix(int percent)
	{
	int i;

	for (i = 8; i < 32; i++)
		{
		pps[i] = 1 << (31 - i);
		if (i < 29)
			pms[i] = pps[i] - (percent << (29 - i) )/25;
		else
			pms[i] = pps[i] - (percent >> (i - 29) )/25;
		}
	}



#ifdef UNSIGNED_BUFFERS
#define UNSIGNED8
#define UNSIGNED16
#endif

#ifdef DEFAULT_BUFFERS

#ifdef UNSIGNED16
LOCAL unsigned short *buffer16;
#define VALUE16(x)	((x)+32768)
#else
LOCAL short *buffer16;
#define VALUE16(x)	(x)
#endif

#ifdef UNSIGNED8
LOCAL unsigned char *buffer;
#define VALUE8(x)		((x)+128)
#else
LOCAL char *buffer;
#define VALUE8(x)		(x)
#endif
LOCAL unsigned long idx;
LOCAL int dsize;			/* current data size */
LOCAL unsigned long samples_max;	/* number of samples in buffer */

#endif	/* DEFAULT_BUFFERS */

#ifdef SEPARATE_BUFFERS
LOCAL char *buffer, *buffer_l, *buffer_r;
LOCAL unsigned long idx;
#endif




LOCAL void 
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

LOCAL void 
add_samples16_mono(long left, long right, int n)
{
	if (n<15)		/* is this possible? */
		buffer16[idx++] = VALUE16( (left + right) << (15-n) );
	else
		buffer16[idx++] = VALUE16( (left + right) >> (n-15) );
}

LOCAL void 
add_samples16(long left, long right, int n)
{
	if (stereo)
		add_samples16_stereo(left, right, n);
	else
		add_samples16_mono(left, right, n);
}

LOCAL void 
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

LOCAL void 
add_samples8_mono(long left, long right, int n)
{
	buffer[idx++] = VALUE8( (left+right) >> (n-7) );
}

LOCAL void 
add_samples8(long left, long right, int n)
{
	if (stereo)
		add_samples8_stereo(left, right, n);
	else
		add_samples8_mono(left, right, n);
}





LOCAL long long realpos;
LOCAL struct sio_hdl *hdl;           	
LOCAL unsigned long current_freq;

unsigned long total;

LOCAL int dsp_samplesize = 0;

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
	set_watched_scalar(FREQUENCY, current_freq);
	total = 0;
	return current_freq;
}

/* synchronize stuff with audio output */
LOCAL struct tagged {
	struct tagged *next;	/* simply linked list */
	void (*f)(GENERIC p);	/* function to call */
	void (*f2)(GENERIC p);	/* function to call  for flush */
	GENERIC p;		/* and parameter */
	unsigned long when;	/* number of bytes to let go before calling */
} 	*start,	/* what still to output */
	*end;	/* where to add new tags */



/* flush_tags: use tags that have gone by recently */
LOCAL void 
flush_tags(void)
{
	if (start) {
		while (start && start->when <= realpos + ADVANCE_TAGS) {

			(*start->f)(start->p);
			tagged *tofree = start;
			start = start->next;
			delete tofree;
		}
	}
}

/* remove unused tags at end */
LOCAL void 
remove_pending_tags(void)
{
	while (start) {
		(*start->f2)(start->p);
		tagged *tofree = start;
		start = start->next;
		delete tofree;
	}
}

void 
sync_audio(void (*function)(void *p), void (*f2)(void *p), void *parameter)
{
	if (hdl) {
		tagged *t = new tagged;
		if (!t) {
			(*function)(parameter);
			return;
		}
		/* build new tag */
		t->next = nullptr;
		t->f = function;
		t->f2 = f2;
		t->p = parameter;
		t->when = total;

		/* add it to list */
		if (start) 
			end->next = t;
		else
			start = t;
		end = t;

		/* set up for next tag */
	} else
		(*function)(parameter);
}

LOCAL void 
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

unsigned long update_frequency(void)
	{
	return 0;
	}

void discard_buffer(void)
	{
	remove_pending_tags();
	total = 0;
	}

void audio_ui(char c)
	{
	}

