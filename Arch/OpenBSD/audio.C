/* openbsd/audio.c 
	vi:ts=3 sw=3:
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
#define DEFAULT_SET_MIX
#define NEW_OUTPUT_SAMPLES_AWARE
#define NEW_FUNCS

/* fine-tune to get the scrolling display in sync with the music */
#define ADVANCE_TAGS 20000 

/* this macro works with unsigned values !!! */
template<typename S, typename T>
inline auto
absdiff(S x, T y) -> decltype(x-y)
{
	return x<y ?  y-x : x-y;
}

/* f' = best_frequency(f, table, def):
 * return nearest frequency in sorted table,
 * unless f == 0 -> return def
 */
LOCAL unsigned long 
best_frequency(unsigned long f, int table[], int def)
{
	unsigned long best = table[0];
	int i;

	if (f == 0)
		return def;
	for (i = 0; i < table[i]; i++)
		if (absdiff(table[i], f) < absdiff(best, f))
			best = table[i];
	return best;
}	



LOCAL short seg_end[8] = {
	0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF
};

LOCAL int 
search(int val, short *table, int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (val <= *table++)
			return i;
	}
	return size;
}

#define	BIAS		(0x84)		/* Bias for linear code. */

/*
 * linear2ulaw() - Convert a linear PCM value to u-law
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *	Biased Linear Input Code	Compressed Code
 *	------------------------	---------------
 *	00000001wxyza			000wxyz
 *	0000001wxyzab			001wxyz
 *	000001wxyzabc			010wxyz
 *	00001wxyzabcd			011wxyz
 *	0001wxyzabcde			100wxyz
 *	001wxyzabcdef			101wxyz
 *	01wxyzabcdefg			110wxyz
 *	1wxyzabcdefgh			111wxyz
 *
 * Each biased linear code has a leading 1 which identifies the segment
 * number. The value of the segment number is equal to 7 minus the number
 * of leading 0's. The quantization interval is directly available as the
 * four bits wxyz.  * The trailing bits (a - h) are ignored.
 *
 * Ordinarily the complement of the resulting code word is used for
 * transmission, and so the code word is complemented before it is returned.
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */
LOCAL unsigned char linear2ulaw(int pcm_val)
	/* 2's complement (16-bit range) */
	{
	int		mask;
	int		seg;
	unsigned char	uval;


	/* Get the sign and the magnitude of the value. */
	if (pcm_val < 0) 
		{
		pcm_val = BIAS - pcm_val;
		mask = 0x7F;
		}
	else 
		{
		pcm_val += BIAS;
		mask = 0xFF;
		}

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm_val, seg_end, 8);

	/*
	 * Combine the sign, segment, quantization bits;
	 * and complement the code word.
	 */
	if (seg >= 8)		/* out of range, return maximum value. */
		return 0x7F ^ mask;
	else {
		uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
		return uval ^ mask;
	}

}

LOCAL unsigned int cvt(int ch)
	{
	return linear2ulaw(ch * 2);
	}

#ifdef DEFAULT_SET_MIX
LOCAL int stereo;

#ifdef NEW_OUTPUT_SAMPLES_AWARE

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

#else /* NEW_OUTPUT_SAMPLES_AWARE */
/* old code: optimized away */
/* LOCAL int primary, secondary;	*/
LOCAL unsigned long pps, pms;	/* 1/2 primary+secondary, 1/2 primary-secondary */

void set_mix(int percent)
   {
	percent *= 256;
	percent /= 100;
/*
	secondary = percent;
	primary = 512 - percent;
 */
	pps = 256;
	pms = 256 - percent;
   }
#endif /* NEW_OUTPUT_SAMPLES_AWARE */

#endif /* DEFAULT_SET_MIX */

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

LOCAL int tsync = false;
#endif	/* DEFAULT_BUFFERS */

#ifdef SEPARATE_BUFFERS
LOCAL char *buffer, *buffer_l, *buffer_r;
LOCAL unsigned long idx;
#endif



#ifdef NEW_OUTPUT_SAMPLES_AWARE

LOCAL void add_samples16_stereo(long left, long right, int n)
	{
	if (pms[n] == pps[n])	/* no mixing */
		{
		if (n<16)
			{
			buffer16[idx++] = VALUE16(left << (16-n) );
			buffer16[idx++] = VALUE16(right << (16-n) );
		   }
		else
		   {
			buffer16[idx++] = VALUE16(left >> (n-16) );
			buffer16[idx++] = VALUE16(right >> (n-16) );
		   }
		}
	else
		{
		long s1, s2;

		s1 = (left+right)*pps[n];
		s2 = (left-right)*pms[n];

		buffer16[idx++] = VALUE16( (s1 + s2) >> 16);
		buffer16[idx++] = VALUE16( (s1 - s2) >> 16);
		}
	}

LOCAL void add_samples16_mono(long left, long right, int n)
	{
	if (n<15)		/* is this possible? */
		buffer16[idx++] = VALUE16( (left + right) << (15-n) );
	else
		buffer16[idx++] = VALUE16( (left + right) >> (n-15) );
	}

LOCAL void add_samples16(long left, long right, int n)
	{
	if (stereo)
		add_samples16_stereo(left, right, n);
	else
		add_samples16_mono(left, right, n);
	}

LOCAL void add_samples8_stereo(long left, long right, int n)
	{
	if (pms[n] == pps[n])	/* no mixing */
		{
		    /* if n<8 -> same problem as above,
		       but that won't happen, right? */
		buffer[idx++] = VALUE8(left >> (n-8) );
		buffer[idx++] = VALUE8(right >> (n-8) );
		}
	else
		{
		long s1, s2;

		s1 = (left+right)*pps[n];
		s2 = (left-right)*pms[n];

		buffer[idx++] = VALUE8( (s1 + s2) >> 24);
		buffer[idx++] = VALUE8( (s1 - s2) >> 24);
		}
	}

LOCAL void add_samples8_mono(long left, long right, int n)
	{
	buffer[idx++] = VALUE8( (left+right) >> (n-7) );
	}

LOCAL void add_samples8(long left, long right, int n)
	{
	if (stereo)
		add_samples8_stereo(left, right, n);
	else
		add_samples8_mono(left, right, n);
	}

#else

/* don't ask me if this code is correct then (I guess it is) ...
   anyone still using it? */
LOCAL void add_samples16_stereo(long left, long right)
	{
	if (pms == pps)	/* no mixing */
		{
		buffer16[idx++] = VALUE16(left/256);
		buffer16[idx++] = VALUE16(right/256);
		}
	else
		{
		long s1, s2;

		s1 = (left+right)*pps;
		s2 = (left-right)*pms;

		buffer16[idx++] = VALUE16( (s1 + s2)/65536 );
		buffer16[idx++] = VALUE16( (s1 - s2)/65536 );
		}
	}

LOCAL void add_samples16_mono(long left, long right)
	{
	buffer16[idx++] = VALUE16( (left + right)/256);
	}

LOCAL void add_samples16(long left, long right)
	{
	if (stereo)
		add_samples16_stereo(left, right);
	else
		add_samples16_mono(left, right);
	}

LOCAL void add_samples8_stereo(long left, long right)
	{
	if (pms == pps)	/* no mixing */
		{
		buffer[idx++] = VALUE8(left/65536);
		buffer[idx++] = VALUE8(right/65536);
		}
	else
		{
		long s1, s2;

		s1 = (left+right)*pps;
		s2 = (left-right)*pms;

		buffer[idx++] = VALUE8( (s1 + s2) >> 24);
		buffer[idx++] = VALUE8( (s1 + s2) >> 24);
		}
	}

LOCAL void add_samples8_mono(long left, long right)
	{
	buffer[idx++] = VALUE8( (left+right) >> 16);
	}

LOCAL void add_samples8(long left, long right)
	{
	if (stereo)
		add_samples8_stereo(left, right);
	else
		add_samples8_mono(left, right);
	}

#endif

#ifndef NEW_OUTPUT_SAMPLES_AWARE

XT void output_samples(long left, long right, int n);

void output_samples(long left, long right, int n)
	{
	old_output_samples(left >> (n-23), right >> (n-23));
	}
#define output_samples	old_output_samples

#endif

#ifndef NEW_FUNCS
void sync_audio(void (*function)(void *), void (*f2)(void *), void *parameter)
	{
	(*function)(parameter);
	}

void audio_ui(char c)
	{
	}

#endif


LOCAL long long realpos;
LOCAL struct sio_hdl *hdl;           	
LOCAL unsigned long current_freq;

unsigned long total;

LOCAL int dsp_samplesize = 0;

static void
movecb(void *arg, int delta)
{
	realpos += delta * dsize * (stereo ? 2 : 1);
}

unsigned long open_audio(unsigned long f, int s)
   {
	struct sio_par par;
	int buf_max;

	hdl = sio_open(NULL, SIO_PLAY, 0);
	if (hdl == NULL)
		end_all("Error opening audio device");

	realpos = 0;
	sio_onmove(hdl, movecb, NULL);

	sio_initpar(&par);
	if (f)
		par.rate = f;
	par.pchan = 2;
	if (!sio_setpar(hdl, &par) || !sio_getpar(hdl, &par) || !sio_start(hdl) ||
	    (par.bits != 8 && par.bits != 16) || par.pchan > 2)
		end_all("Sorry, no audio format supported by this binary is available");

	buf_max = par.appbufsz * par.bps * par.pchan;
	current_freq = par.rate;
	stereo = par.pchan == 2 ? 1 : 0;

	dsp_samplesize = par.bits;
	dsize = par.bps;
	buffer = (unsigned char *)malloc(buf_max);
	buffer16 = (short *)buffer;

	idx = 0;
	samples_max = buf_max / dsize / par.pchan;
	set_watched_scalar(FREQUENCY, current_freq);
	total = 0;
	return current_freq;
   }

/* synchronize stuff with audio output */
LOCAL struct tagged
	{
	struct tagged *next;	/* simply linked list */
	void (*f)(GENERIC p);	/* function to call */
	void (*f2)(GENERIC p);	/* function to call  for flush */
	GENERIC p;		/* and parameter */
	unsigned long when;	/* number of bytes to let go before calling */
	} 
	*start,	/* what still to output */
	*end;	/* where to add new tags */



/* flush_tags: use tags that have gone by recently */
LOCAL void flush_tags(void)
	{
	if (start)
		{
		while (start && start->when <= realpos + ADVANCE_TAGS)
			{
			struct tagged *tofree;

			(*start->f)(start->p);
			tofree = start;
			start = start->next;
			free(tofree);
			}
		}
	}

/* remove unused tags at end */
LOCAL void remove_pending_tags(void)
	{
	while (start)
		{
		struct tagged *tofree;

		(*start->f2)(start->p);
		tofree = start;
		start = start->next;
		free(tofree);
		}
	}

void sync_audio(void (*function)(void *p), void (*f2)(void *p), void *parameter)
	{
	struct tagged *t;

	if (hdl)
		{
		t = (struct tagged *)malloc(sizeof(struct tagged));
		if (!t)
			{
			(*function)(parameter);
			return;
			}
			/* build new tag */
		t->next = 0;
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
		}
	else
		(*function)(parameter);
	}

LOCAL void actually_flush_buffer(void)
   {
   int l,i;

	if (idx)
		{
		total += idx * dsize;
		sio_write(hdl, buffer, dsize * idx);
		}
   idx = 0;
   }

void output_samples(long left, long right, int n)
	{
	if (idx >= samples_max - 1)
		actually_flush_buffer();
	switch(dsp_samplesize)
		{
	case 16:				/*   Cool! 16 bits samples */
		add_samples16(left, right, n);
		break;
	case 8:
		add_samples8(left, right, n);
		break;
	default:	/* should not happen */
		;
	   }
   }

void flush_buffer(void)
    {	
	 actually_flush_buffer();
	 flush_tags();
    }

/*
 * Closing the Linux sound device waits for all pending samples to play.
 */
void close_audio(void)
    {
    actually_flush_buffer();
    sio_close(hdl);
    free(buffer);
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

