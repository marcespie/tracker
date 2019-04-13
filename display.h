#ifndef DISPLAY_H
#define DISPLAY_H
/* display.h */
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

class channel;
class event;

/* dump_event(ch, e): dump event e as occuring on channel ch
 * (some events need the current channel state for a correct dump)
 * special case: ch == 0 means current set of events done
 */
extern void dump_event(const channel& ch, const event *e);
// finish the line if necessary
extern void dump_event();

/* dump_delimiter(): add a delimiter to the current dump, to 
 * separate left channels from right channels, for instance
 */
extern void dump_delimiter();

#endif
