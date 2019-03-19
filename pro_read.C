/* Modules/Pro/read.c */
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

#include <ctype.h>
#include <assert.h>
#include <string.h>

#include "extern.h"
#include "song.h"
#include "notes.h"
#include "channel.h"
#include "autoinit.h"
#include "automaton.h"
#include "empty.h"
#include "resample.h"
#include "open.h"

static unsigned int patsize;

#define NOT_YET (-1)

static unsigned char *buffer;		/* Buffer to read everything */
static unsigned int bufsize;

static void 
setup_buffer(unsigned int size)
{
	bufsize = size;
	if (!buffer)
		buffer = (unsigned char *)malloc(bufsize);
	else
		buffer = (unsigned char *)realloc(buffer, bufsize);
	if (!buffer)
		end_all("Memory allocation");
}

/***
 ***	Low level st-file access routines 
 ***/

/* s = getstring(f, len):
 * gets a soundtracker string from file f.
 * I.e, it is a fixed length string terminated
 * by a 0 if too short. 
 */
static char *
getstring(exfile& f, unsigned int len)
{
	assert(len < bufsize);
	f.read(buffer, len);
	buffer[len] = '\0';
	char *n = (char *)malloc(strlen((const char *)buffer)+1);
	if (!n) 
		return nullptr;

	return strcpy(n, (char *)buffer);
}

/* byteskip(f, len)
 * same as fseek, except it works on stdin
 */
static void 
byteskip(exfile& f, unsigned long int len)
{
	while (len > bufsize) {
		f.read(buffer, bufsize);
		len -= bufsize;
	}
	f.read(buffer, len);
}

/* v = getushort(f)
 * reads an unsigned short from f
 */
static unsigned int 
getushort(exfile& f)
{
	/* order dependent !!! */
	unsigned int i = f.getc() << 8;
	return i | f.getc();
}

/***
 ***  Sample handling
 ***/

/* info = fill_sample_info(f):
 * allocate and fill sample info with the information at current position of
 * file f. Allocate memory for storing the sample, also.  fill_sample_info 
 * is guaranteed to give you an accurate snapshot of what sample should 
 * be like. In particular, length, rp_length, start, rp_start, fix_length, 
 * fix_rp_length will have the values you can expect if part of the sample 
 * is missing.
 */
static sample_info *
fill_sample_info(exfile& f)
{
	/* New method: instead of allocating/freeing sample infos,
	* we keep one in reserve */
	static struct sample_info *info = nullptr;
	struct sample_info *result;

	if (!info)
		info = new sample_info;
	info->finetune = 0;
	info->name = nullptr;
	info->length = 0;
	info->start = nullptr;
	info->sample_size = 8;
	info->rp_start = nullptr;

	info->name = getstring(f, SAMPLENAME_MAXLENGTH);
	if (!info->name) {
		error = OUT_OF_MEM;
		return nullptr;
	}
	info->length = getushort(f);
	info->finetune = f.getc();
	if (info->finetune > 15)
		info->finetune = 0;
	info->volume = f.getc();
	info->volume = MIN(info->volume, MAX_VOLUME);
	info->rp_offset = getushort(f);
	info->rp_length = getushort(f);

	/* the next check is for old modules for which
	 * the sample data types are a bit confused, so
	 * that what we were expecting to be #words is #bytes.
	 */
	/* not sure I understand the -1 myself, though it's
	 * necessary to play kawai-k1 correctly 
	 */
	if (info->rp_length + info->rp_offset - 1 > info->length)
		info->rp_offset /= 2;

	if (info->rp_length + info->rp_offset > info->length)
		info->rp_length = info->length - info->rp_offset;

	info->length *= 2;
	info->rp_offset *= 2;
	info->rp_length *= 2;
	/* in all logic, a 2-sized sample could exist, but this is 
	 * not the case, and even so, some flavors of soundtracker 
	 * output empty instruments as being 2-sized.
	*/
	if (info->rp_length <= 2)
		info->rp_length = 0;

	if (info->length <= 2) {
		if (info->name)
			free(info->name);
		return nullptr;
	}
	if (info->length > MAX_SAMPLE_LENGTH)
		error = CORRUPT_FILE;
	result = info;
	info = nullptr;
	return result;
}

	
static void 
fill_sample_infos(song *song, exfile& f)
{
	for (unsigned int i = 1; i <= song->ninstr; i++) {
		song->samples[i] = fill_sample_info(f);
		if (error != NONE)
			return;
	}
}

static void 
free_sample_info(sample_info *sample)
{
	if (sample) {
		if (sample->start)
			delete [] sample->start;
		if (sample->name)
			free(sample->name);
		delete sample;
	}
}

static void 
setup_used_samples(song *song, unsigned char used[])
{
	for (unsigned int i = 1; i <= song->ninstr; i++)
		used[i] = false;
	for (unsigned int i = 0; 
	    i < song->info.plength * song->ntracks * song->info.npat; i++) {
		assert(song->info.data[i].sample_number < MAX_NUMBER_SAMPLES);
		used[song->info.data[i].sample_number] = true;
	}
}
	
static unsigned long 
compress_samples(song *song, unsigned char used[])
{
	unsigned char map_sample[MAX_NUMBER_SAMPLES];
	unsigned int i, j;
	unsigned long won = 0;

	j = 0;
	for (i = 1; i <= song->ninstr; i++) {
		if (used[i]) {
		if (song->samples[i]) {
			map_sample[i] = ++j;
			song->samples[j] = song->samples[i];
			song->samples[j]->color = j%15+1;
		} else
			/* empty samples are remapped to the last sample.
			 * If there are empty samples, then the last sample 
			 * is free, after the remapping.
			 */
			map_sample[i] = LAST_SAMPLE;
		} else {
			map_sample[i] = 0;
			won += sizeof(struct sample_info);
			if (song->samples[i]) {
				won += song->samples[i]->length;
			}
			free_sample_info(song->samples[i]);
			song->samples[i] = NULL;
		}
	}
	/* don't forget to map `no sample' to `no sample' */
	map_sample[0] = 0;
	/* don't bother removing stuff after song->ninstr */
	song->ninstr = j;
	/* set the last sample to be the empty sample, see above */
	if (j < LAST_SAMPLE)
		song->samples[LAST_SAMPLE] = empty_sample();

	/* effectively remap samples around */

	for (i = 0; 
	    i < song->info.plength * song->ntracks * song->info.npat; i++)
		song->info.data[i].sample_number = 
		    map_sample[song->info.data[i].sample_number];

	return won;
}


static void 
read_sample(sample_info *info, exfile& f)
{
	/* add one byte for resampling */
	info->start = new SAMPLE8 [info->length+1];

	if (info->rp_length)
		info->rp_start = info->start + info->rp_offset;

	f.read(info->start, info->length);
}


static void 
read_samples(song *song, exfile& f, char used[])
{
	/* read samples from the file if they are needed */
	for (unsigned int i = 1; i <= song->ninstr; i++)
		if (song->samples[i]) {	/* does the sample exist ? */
			if (used[i]) 	/* do we need it ? */
				read_sample(song->samples[i], f);
			else
				byteskip(f, song->samples[i]->length);
		}
}

/***
*** Pattern information handling
***/

static void 
fill_pattern_numbers(song_info *info, exfile& f)
{
	for (unsigned int i = 0; i < NUMBER_PATTERNS; i++) {
		unsigned int p = f.getc();
		if (p >= NUMBER_PATTERNS)
			p = 0;
		if (p+1 > info->npat)
			info->npat = p+1;
		info->patnumber[i] = p;
	}
}

static void 
mark_used_pattern_numbers(song_info *info, unsigned char used[])
{
	unsigned int i;

	for (i = 0; i < NUMBER_PATTERNS; i++)
		used[i] = 0;
	for (i = 0; i < info->length; i++)
		used[info->patnumber[i]] = 1;
}

/* n = compress_patterns(info, used):
 * -> n:      number of actually used patterns
 *    used[]: boolean array: for each pattern number, is it used ?
 */
static unsigned int 
compress_patterns(song_info *info, unsigned char used[])
{
	unsigned char remap[NUMBER_PATTERNS];
	unsigned int i, j;

	mark_used_pattern_numbers(info, used);

	/* build correspondence table */
	j = 0;
	for (i = 0; i < info->npat; i++) {
		remap[i] = j;
		if (used[i])
			j++;
	}

	/* compress unused pattern numbers out */
	for (i = 0; i < NUMBER_PATTERNS; i++)
		info->patnumber[i] = remap[info->patnumber[i]];

	return j;
}

/* pattern_size = setup_patterns(info, ntracks, number)
 */
static unsigned int 
setup_patterns(song_info *info, unsigned int ntracks, unsigned int used)
{
	/* allocate the memory */
	info->patterns = (pattern *)malloc(sizeof(pattern) * info->length);
	info->data = (event *)malloc(sizeof(event) * info->plength * 
	    ntracks * used);
	if (!info->patterns ||!info->data) {
		error = OUT_OF_MEM;
		return 0;
	}
	/* setup the pointers to various patterns. Note we allocate
	* all the patterns as one data block:
	* - to prevent memory fragmentation
	* - to allow uniform pointer access to all events in the 
	*   virtual player */
	for (unsigned i = 0; i < info->length; i++) {
		info->patterns[i].e = info->data 
		    + info->patnumber[i] * info->plength * ntracks;
		info->patterns[i].number = info->patnumber[i];
	}

	return info->plength * ntracks * 4;
}

static void 
fill_event(event *e, unsigned char *p, int *current_instrument, song *song)
{
	pitch pitch;

	e->sample_number = (p[0] & 0x10) | (p[2] >> 4);
	assert(e->sample_number < MAX_NUMBER_SAMPLES);
	if (e->sample_number)
		*current_instrument = e->sample_number;
	e->effect = p[2] & 0xf;
	e->parameters = p[3];
	/* remove some weirdness from protracker events */
	switch(e->effect) {
	case EFF_EXTENDED:
		e->effect = EXT_BASE + e->high();
		e->parameters = e->low();
		break;
	case 0:
		e->effect = e->parameters ? EFF_ARPEGGIO : EFF_NONE;
		break;
	case EFF_SKIP:
		e->parameters = e->high() * 10 + e->low();
		break;
	case EFF_SPEED:
		if (song->type == OLD_ST)
			e->effect = EFF_OLD_SPEED;
		break;
	}
	pitch = ( (p[0] & 15) << 8 ) | p[1];
	e->note = pitch2note(pitch);
}


static event *
fill_pattern(exfile& f, song *song, event *e)
{
	f.read(buffer, patsize);

	for (unsigned j = 0; j < song->ntracks; j++) {
		int current_instrument = NOT_YET;
		for (unsigned i = 0; i < song->info.plength; i++)
			fill_event(e+i, buffer+4*(i*song->ntracks+j), 
			    &current_instrument, song);
		e += song->info.plength;
	}
	return e;
}


/* winned = fill_patterns(song, f, used):
 * read patterns from file f into song, keeping only patterns tagged by
 * used, return memory gain.
 */
static unsigned 
fill_patterns(song *song, exfile& f, unsigned char used[])
{
	unsigned int won = 0;

	event *e = song->info.data;
	for (unsigned i = 0; i < song->info.npat; i++) {
		if (used[i]) {
			e = fill_pattern(f, song, e);
			if (error != NONE)
				return 0;
		} else {
			byteskip(f, patsize);
			won += patsize;
		}
	}
	return won;
}


/***
 *** End song setup
 ***/

static void 
adjust_volumes(song *song)
{
	for (unsigned i = 1; i <= song->ninstr; i++)
	for (unsigned j = 0; j <= MAX_VOLUME; j++)	/* note <= not a bug */
		song->samples[i]->volume_lookup[j] = 
		    (song->ntracks == 6) ? (4 * j) / 3 : j;
}

void 
setup_song(song *song)
{
	for (unsigned i = 1; i <= song->ninstr; i++)
		prep_sample_info(song->samples[i]);
	adjust_volumes(song);

	auto a = setup_automaton(song, 0);

	compute_duration(a, song);
}

/***
 *** Actual song loading
 ***/

static void 
fill_song_info(song_info *info, exfile& f)
{
	info->length = f.getc();
	f.getc();
	info->npat = 0;
	fill_pattern_numbers(info, f);
	if (info->npat == 0 || 
	    info->length == 0 || info->length >= NUMBER_PATTERNS)
		error = CORRUPT_FILE;
}

/* new_song: allocate a new structure for a song.
 *  clear each and every field as appropriate.
 */
static song *
new_song(void)
{
	song *n = new song;
	n->title = nullptr;
	n->info.length = 0;
	n->info.npat = 0;
	n->info.patterns = nullptr;
	n->info.data = nullptr;
	for (unsigned i = 1; i < MAX_NUMBER_SAMPLES; i++)
		n->samples[i] = nullptr;
	return n;
}

/* release_song(song): give back all memory occupied by song. Assume 
 * that each structure has been correctly allocated by a call to the
 * corresponding new_XXX function.
 */
void 
release_song(song *song)
{
	if (!song)
		return;
	/* Since sample compression, structure is INVALID after song->ninstr */
	for (unsigned i = 1; i <= song->ninstr; i++)
		free_sample_info(song->samples[i]);
	if (song->info.patterns)
		free(song->info.patterns);
	if (song->info.data)
		free(song->info.data);
	if (song->title)
		free(song->title);
	delete song;
}

/* error_song(song): what we should return if there was an error. 
 * Actually, is mostly useful for its side effects.
 */
static song *
error_song(song *song)
{
	release_song(song);
	return nullptr;
}

/* bad_sig(f, song): read the signature on file f and returns true if 
 * it is not a known sig. Set some parameters of song as a side effect
 */
static bool 
bad_sig(exfile& f, song *song)
{
	char a = f.getc();
	char b = f.getc();
	char c = f.getc();
	char d = f.getc();
	if (a == 'M' && b == '.' && c == 'K' && d == '.')
		return false;
	if (a == 'M' && b == '&' && c == 'K' && d == '!')
		return false;
	if (a == 'F' && b == 'L' && c == 'T' && d == '4')
		return false;
	if (a == '6' && b == 'C' && c == 'H' && d == 'N') {
		song->ntracks = 6;
		return false;
	}
	if (a == '8' && b == 'C' && c == 'H' && d == 'N') {
		song->ntracks= 8;
		return false;
	}
	return true;
}

/* s = read_song(f, type): try to read a song s of type NEW/OLD/NEW_NOCHECK
 * from file f. Might fail, i.e., return NULL if file is not a mod file of 
 * the correct type.
 */
song *
read_song(exfile& f, int type)
{
	unsigned char pattern_used[NUMBER_PATTERNS];
	unsigned char sample_used[MAX_NUMBER_SAMPLES];

	error = NONE;

	if (!buffer)
		setup_buffer(1024);

	auto song = new_song();
	if (!song)
		return error_song(song);

	song->ntracks = 4;
	if (type == NEW || type == NEW_NO_CHECK) {
		song->type = PROTRACKER;
		song->ninstr = PRO_NUMBER_SAMPLES;
	} else {
		song->type = OLD_ST;
		song->ninstr = ST_NUMBER_SAMPLES;
	}

	song->ntracks = NORMAL_NTRACKS;
	song->info.plength = NORMAL_PLENGTH;

	song->title = getstring(f, TITLE_MAXLENGTH);
	if (error != NONE)
		return error_song(song);

	fill_sample_infos(song, f);

	fill_song_info(&song->info, f);

	if (error != NONE)
		return error_song(song);

	if (type == NEW && bad_sig(f, song))
		return error_song(song);

	if (type == NEW_NO_CHECK)
		byteskip(f, 4);

	song->max_sample_width = 8;			/* temporary */

	if (song->ntracks == 4)
		song->side_width = song->max_sample_width + 6 + 1;
	else
		song->side_width = song->max_sample_width + 6 + 2;

	auto used_patterns = compress_patterns(&song->info, pattern_used);

	patsize = setup_patterns(&song->info, song->ntracks, used_patterns);
	if (error != NONE)
		return error_song(song);

	setup_buffer(patsize);
	if (!buffer) {
		error = OUT_OF_MEM;
		return error_song(song);
	}

	auto won = fill_patterns(song, f, pattern_used);
	if (error != NONE)
		return error_song(song);

	song->info.npat = used_patterns;

	setup_used_samples(song, sample_used);

	song->samples_start = f.tell();

	read_samples(song, f, (char *)sample_used);
	if (error != NONE)
		return error_song(song);

	/* remap samples around */
	won += compress_samples(song, sample_used);

	setup_song(song);
	return song;
}

