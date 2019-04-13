#ifndef PROTRACKER_H
#define PROTRACKER_H
/* protracker.h */
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

/* internal data structures for the soundtracker player routine....  */

using SAMPLE8 = signed char;

const auto MAX_NUMBER_SAMPLES=32;
const auto LAST_SAMPLE=31;
const auto ST_NUMBER_SAMPLES=15;
const auto PRO_NUMBER_SAMPLES=31;

const auto NORMAL_PLENGTH=64;
const auto NORMAL_NTRACKS=4;
const auto MAX_TRACKS=8;
const auto NUMBER_SAMPLES=32;

const auto BLOCK_LENGTH=64;
const auto NUMBER_TRACKS=4;
const auto NUMBER_PATTERNS=128;

const inline unsigned int NUMBER_EFFECTS=40;

/* some effects names */
const auto EFF_ARPEGGIO =	0;
const auto EFF_DOWN = 		1;
const auto EFF_UP =		2;
const auto EFF_PORTA =		3;
const auto EFF_VIBRATO =	4;
const auto EFF_PORTASLIDE =	5;
const auto EFF_VIBSLIDE =	6;
const auto EFF_TREMOLO =	7;

// XXX missing 8 not an error
const auto EFF_OFFSET =		9;
const auto EFF_VOLSLIDE =	10;
const auto EFF_FF =		11;
const auto EFF_VOLUME =		12;
const auto EFF_SKIP =		13;
const auto EFF_EXTENDED =	14;
const auto EFF_SPEED =		15;
const auto EFF_NONE =		16;
const auto EFF_OLD_SPEED =	17;

const auto EXT_BASE =		18;
const auto EFF_SMOOTH_DOWN = 	EXT_BASE + 1;
const auto EFF_SMOOTH_UP =  	EXT_BASE + 2;
const auto EFF_GLISS_CTRL =	EXT_BASE + 3;
const auto EFF_VIBRATO_WAVE =	EXT_BASE + 4;
const auto EFF_CHG_FTUNE =  	EXT_BASE + 5;
const auto EFF_LOOP =		EXT_BASE + 6;
const auto EFF_TREMOLO_WAVE =	EXT_BASE + 7;
const auto EFF_RETRIG =      	EXT_BASE + 9;
const auto EFF_S_UPVOL =	EXT_BASE + 10;
const auto EFF_S_DOWNVOL =   	EXT_BASE + 11;
const auto EFF_NOTECUT =	EXT_BASE + 12;
const auto EFF_LATESTART =   	EXT_BASE + 13;
const auto EFF_DELAY =	      	EXT_BASE + 14;
const auto EFF_INVERT_LOOP =	EXT_BASE + 15;

const auto SAMPLENAME_MAXLENGTH=22;
const auto TITLE_MAXLENGTH=20;

const auto MIN_PITCH=113;
const auto MAX_PITCH=856;
const auto REAL_MAX_PITCH=1050;

const auto MIN_VOLUME=0;
const auto MAX_VOLUME=64;

// the fuzz in note pitch
const auto FUZZ=5;

// we refuse to allocate more than 500000 bytes for one sample
const auto MAX_SAMPLE_LENGTH= 500000;

class sample_info {
public:
	SAMPLE8 *start, *rp_start;
	unsigned long  fix_length, fix_rp_length;

	unsigned int volume;
	unsigned short volume_lookup[MAX_VOLUME+1];
	short finetune;
	unsigned short color;
	unsigned short sample_size;			/* 8 or 16 */

	char *name;
	unsigned long  length, rp_offset, rp_length;
};

/* the actual parameters may be split in two halves occasionally */

class event {
public:
	unsigned char sample_number;
	unsigned char effect;
	unsigned char parameters;
	unsigned char note;
	auto low() const
	{
		return parameters & 15U;
	}
	auto high() const
	{
		return parameters >> 4U;
	}
};

class pattern {
public:
	event *e;
	unsigned long duration;
	unsigned long total;
	unsigned int number;
};

        
class song_info {
public:
	unsigned int length;
	unsigned int npat;
	unsigned int plength;
	unsigned long duration;
	unsigned char patnumber[NUMBER_PATTERNS];
	pattern *patterns;
	event *data;
};

const auto OLD_ST=0;
const auto PROTRACKER=1;

class resampler;

class song: public Module {
public:
	~song();
	int play(unsigned int start, resampler& r);
	void dump() const;
	void adjust_volume(unsigned long mask);
	char *title;
	/* sample 0 is always a dummy sample */
	sample_info *samples[MAX_NUMBER_SAMPLES];
	int type;
	unsigned int ntracks;
	unsigned int ninstr;
	int side_width;
	int max_sample_width;			
	song_info info;
	long samples_start;
};

const auto AMIGA_CLOCKFREQ=3575872;
#endif
