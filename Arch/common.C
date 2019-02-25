/* common.c */

/* common repository of code/macros for all architectures audio */
/* $Id: common.c,v 5.2 1996/05/06 07:35:48 espie Exp espie $ */
/* $Log: common.c,v $
 * Revision 5.2  1996/05/06 07:35:48  espie
 * *** empty log message ***
 *
 * Revision 5.1  1996/04/12 16:29:41  espie
 * *** empty log message ***
 *
 * Revision 5.0  1995/10/21 14:55:26  espie
 * New
 *
 * Revision 1.12  1995/09/03 15:50:02  espie
 * *** empty log message ***
 *
 * Revision 1.11  1995/09/03 14:20:11  espie
 * *** empty log message ***
 *
 * Revision 1.10  1995/09/03 13:37:11  espie
 * Corrected data sizes for frequency and left, right.
 *
 * Revision 1.9  1995/05/12  20:40:33  espie
 * Added audio_ui capability.
 *
 * Revision 1.8  1995/05/12  13:52:19  espie
 * New synchronization for sparc.
 * News ulaw conversion function.
 *
 * Revision 1.7  1995/03/17  00:30:59  espie
 * Fixed multiple stupid bugs in common.c (thanks Rolf)
 *
 * Revision 1.6  1995/02/27  14:25:37  espie
 * Rolf Grossmann patch.
 *
 * Revision 1.5  1995/02/26  23:07:14  espie
 * Changed sync to tsync.
 *
 * Revision 1.4  1995/02/23  23:33:01  espie
 * Linear resampling changed.
 *
 * Revision 1.3  1995/02/23  22:41:45  espie
 * Added # of bits.
 *
 * Revision 1.2  1995/02/23  17:03:14  espie
 * Continuing changes for a standard file.
 *
 * Revision 1.1  1995/02/23  16:42:10  espie
 * Initial revision
 *
 *
 */

/* this macro works with unsigned values !!! */
#define absdiff(x, y) ( (x) < (y) ? (y) - (x) : (x) - (y) )

/* f' = best_frequency(f, table, def):
 * return nearest frequency in sorted table,
 * unless f == 0 -> return def
 */
LOCAL unsigned long best_frequency(unsigned long f, int table[], int def)
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



LOCAL short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,
			    0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

LOCAL int search(int val, short *table, int size)
	{
	int		i;

	for (i = 0; i < size; i++) 
		{
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

LOCAL int tsync = FALSE;
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
