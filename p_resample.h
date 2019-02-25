/* presample.h --- definitions private to the resampling engine
	vi:ts=3 sw=3:
 */

/* $Id: p_resample.h,v 1.1 1996/04/12 16:31:39 espie Exp espie $
 * $Log: p_resample.h,v $
 * Revision 1.1  1996/04/12 16:31:39  espie
 * Initial revision
 *
 * Revision 1.1  1996/04/09 21:14:20  espie
 * Initial revision
 *
 * Revision 1.2  1996/03/14 18:04:22  espie
 * *** empty log message ***
 *
 * Revision 1.1  1996/03/12 22:56:12  espie
 * Initial revision
 *
 */

#define MAX_CHANNELS 8

enum audio_state { DO_NOTHING, PLAY, REPLAY};

LOCAL struct audio_channel
   {
   struct sample_info *samp;
   enum audio_state mode;
   unsigned long pointer;
   unsigned long step;
   unsigned int volume;
	unsigned int scaled_volume;
   pitch pitch;
	int side;
   } chan[MAX_CHANNELS];

/* define NO_SIDE for errors (no side specified) */
#define NO_SIDE (-1)

/* Have to get some leeway for vibrato (since we don't bound pitch with
 * vibrato). This is conservative.
 */
#define LEEWAY 150



/* macros for fixed point arithmetic */
/* NOTE these should be used ONLY with unsigned values !!!! */

#define ACCURACY 12
#define fix_to_int(x) ((x) >> ACCURACY)
#define int_to_fix(x) ((x) << ACCURACY)
#define fractional_part(x) ((x) & (fixed_unit - 1))
#define fixed_unit	 (1 << ACCURACY)

#define C fix_to_int(ch->pointer)

