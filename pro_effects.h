/* Modules/Pro/effects.h */
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


enum st_effect_type {
	CH_E, A_E, NO_NOTE_CH_E, PORTA_CH_PITCH_E, CH_A_E, NOTHING
};

struct st_effect {
	enum st_effect_type type;
	union {
		void (*CH_E)(channel& ch, const event& e);
		void (*A_E)(automaton& a, const event& e);
		void (*CH_PITCH_E)(channel& ch, pitch pitch, const event& e);
		void (*CH_A_E)(channel& ch, automaton& a, const event& e);
	} f;
};

/* init_effects(): set up all data for the effects
 * can't be set up as an autoinit
 */
extern void init_effects(st_effect table[]);

/* do_nothing: this is the default behavior for an effect
 */
extern void do_nothing(channel&);
