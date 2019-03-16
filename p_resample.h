/* presample.h --- definitions private to the resampling engine
 */

const auto MAX_CHANNELS=8;

enum audio_state { DO_NOTHING, PLAY, REPLAY};

static struct audio_channel {
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
const auto NO_SIDE=-1;

/* Have to get some leeway for vibrato (since we don't bound pitch with
 * vibrato). This is conservative.
 */
const auto LEEWAY=150;


/* macros for fixed point arithmetic */
/* NOTE these should be used ONLY with unsigned values !!!! */

#define ACCURACY 12
#define fix_to_int(x) ((x) >> ACCURACY)
#define int_to_fix(x) ((x) << ACCURACY)
#define fractional_part(x) ((x) & (fixed_unit - 1))
#define fixed_unit	 (1 << ACCURACY)

#define C fix_to_int(ch->pointer)

