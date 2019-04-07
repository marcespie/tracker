/* dump_song.c */
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

#include <memory>
#include <ctype.h>
#include <unistd.h>


#include "song.h"
#include "protracker.h"
#include "extern.h"
#include "notes.h"
#include "channel.h"
#include "prefs.h"

static char buffer[80];

extern char instname[];	/* from display.c */



/***
 ***	dump_block/dump_song:
 ***		show most of the readable info concerning a module on the screen
 ***/

/* make_readable(s):
 * transform s into a readable string 
 */
static void 
make_readable(const char *s)
{
	if (!s)
		return;

	auto orig = s;
	// XXX this is a logically "const" function even though it
	// tweaks the underlying buffer
	auto t = const_cast<char *>(s);

	/* get rid of the st-xx: junk */
	if (strncmp(s, "st-", 3) == 0 || strncmp(s, "ST-", 3) == 0) {
		if (isdigit(s[3]) && isdigit(s[4]) && s[5] == ':')
			s += 6;
	}
	while (*s) {
		if (isprint(*s))
			*t++ = *s;
		s++;
	}
	*t = '\0';
	while (t != orig && isspace(t[-1]))
		*--t = '\0';
}

void
song::dump() const
{
	unsigned int i;
	size_t j;
	size_t maxlen;
	static char dummy[1];


	auto handle = begin_info(title);
	if (!handle)
		return;

	dummy[0] = '\0';
	maxlen = 0;
	for (i = 1; i <= ninstr; i++) {
		if (!samples[i]->name)
			samples[i]->name = dummy;
		make_readable(samples[i]->name);
		if (maxlen < strlen(samples[i]->name))
			maxlen = strlen(samples[i]->name);
	}
	for (i = 1; i <= ninstr; i++) {
		if (samples[i]->start || 
		    strlen(samples[i]->name) > 2) {
			static char s[15];
			char *base = s;

			if (pref::get(Pref::color))
				base = write_color(base, samples[i]->color);
			*base++ = instname[i];
			*base++ = ' ';
			*base++ = 0;
			infos(handle, s);
			infos(handle, samples[i]->name);
			for (j = strlen(samples[i]->name); 
			    j < maxlen + 2; j++)
				infos(handle, " ");
			if (samples[i]->start) {
				sprintf(buffer, "%6lu", samples[i]->length);
				infos(handle, buffer);
				if (samples[i]->rp_length > 2) {
					sprintf(buffer, "(%6lu %6lu)", 
					    samples[i]->rp_offset, 
					    samples[i]->rp_length);
					infos(handle, buffer);
				} else
					infos(handle, "             ");
				if (samples[i]->volume != MAX_VOLUME) {
					sprintf(buffer, "%3u", 
					    samples[i]->volume);
					infos(handle, buffer);
				} else 
					infos(handle, "   ");
				if (samples[i]->finetune) {
					sprintf(buffer, "%3d", 
					    samples[i]->finetune);
					infos(handle, buffer);
				}
			}
			base = s;
			if (pref::get(Pref::color))
				base = write_color(base, 0);
			*base = 0;
			::info(handle, s);
		}
	}
	end_info(handle);
}
