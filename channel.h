/* channel.h */
     
#ifndef NUMBER_PATTERNS
#define NUMBER_PATTERNS 128
#endif

#define MAX_ARP 3
     
/* there is no note in each channel initially.
 * This is defensive programming, because some
 * commands rely on the previous note. Checking
 * that there was no previous note is a way to
 * detect faulty modules.
 */

struct sinusoid {
	int offset;		/* current offset */
	int depth;		/* current depth */
	int rate;		/* current step rate */
	pitch_delta *table;	/* table to use */
	int reset;		/* flag */
};

struct channel {
	struct sample_info *samp;
	struct audio_channel *audio;
	finetune finetune;
	unsigned int volume;	/* current volume of the sample (0-64) */
	::pitch pitch;          /* current pitch of the sample */
	note note;              /* we have to save the note cause */
			  	/* we can do an arpeggio without a new note */

	::pitch arp[MAX_ARP];   /* the three pitch values for an arpeggio */
	int arpindex;           /* an index to know which note the arpeggio is doing */

	struct sinusoid vib;
	struct sinusoid trem;

	int slide;              /* step size of pitch slide */

	::pitch pitchgoal;      /* pitch to slide to */
	pitch_delta pitchrate;  /* step rate for portamento */

	int volumerate;         /* step rate for volume slide */


	size_t start_offset;

	int retrig;             /* delay for extended retrig command */
	int current;

	int funk_glissando;	/* current command to adjust parameters */
	void (*adjust) (struct channel *ch);
	int loop_counter;
	int loop_note_num;

	int invert_speed;
	int invert_offset;
	unsigned long invert_position;
	void (*special)(struct channel *ch);
};

struct automaton;
