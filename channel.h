/* channel.h */
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
     
//inline const auto NUMBER_PATTERNS=128;

const auto MAX_ARP=3;
     
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

class audio_channel;

struct channel {
	std::unique_ptr<audio_channel> audio;
	channel(int side);
	int side() const;
	sample_info *samp;
	finetune finetune;
	unsigned int volume;	/* current volume of the sample (0-64) */
	::pitch pitch;          /* current pitch of the sample */
	note note;              /* we have to save the note cause */
			  	/* we can do an arpeggio without a new note */

	::pitch arp[MAX_ARP];   /* the three pitch values for an arpeggio */
	int arpindex;           /* an index to know which note the arpeggio is doing */

	sinusoid vib;
	sinusoid trem;

	int slide;              /* step size of pitch slide */

	::pitch pitchgoal;      /* pitch to slide to */
	pitch_delta pitchrate;  /* step rate for portamento */

	int volumerate;         /* step rate for volume slide */


	size_t start_offset;

	int retrig;             /* delay for extended retrig command */
	int current;

	int funk_glissando;	/* current command to adjust parameters */
	void (*adjust) (channel& ch);
	int loop_counter;
	int loop_note_num;

	int invert_speed;
	int invert_offset;
	unsigned long invert_position;
	void (*special)(channel& ch);

	// implemented in pro_low.C
	void start_note();
	void stop_note();
	void set_current_note(::note, ::pitch);
	void set_temp_pitch(::pitch);
	void set_current_volume(int volume);
	void set_temp_volume(int volume); // used for tremolo
	void set_position(size_t pos);
};

class automaton;
