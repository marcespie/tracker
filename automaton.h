/* automaton.h */
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

struct automaton;
struct song;
struct event;

const auto NORMAL_FINESPEED=125;
/* a = init_automaton(song, start):
 * return an automaton set in the right state to play song from pattern #start.
 */
extern automaton *setup_automaton(song *song, unsigned int start);

/* next_tick(a):
 * set up everything for the next tick.
 */
extern void next_tick(automaton *a);

/* update_tempo(a):
 * update tempo in automaton according to parameters
 */
extern void update_tempo(automaton *a);
extern void set_bpm(automaton *a, unsigned int bpm);

extern event *EVENT(automaton *a, int channel);



