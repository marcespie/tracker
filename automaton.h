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

// values for do_stuff
class song;
struct event;
struct channel;


const auto DO_SET_NOTHING=0;
const auto SET_SPEED=1;
const auto SET_SKIP=2;
const auto SET_FASTSKIP=4;
const auto SET_FINESPEED=32;

const auto JUMP_PATTERN=8;
const auto DELAY_PATTERN=16;

const auto NORMAL_SPEED=6;
                                                                               
class automaton {
	void set_pattern();
	void set_pattern_for_virt();
	void init(const song* song, unsigned int start);
	void advance_pattern();
	void clear_repeats(unsigned int, unsigned int);
	void reset_repeats();
	unsigned long compute_pattern_duration(event *base, 
	    unsigned int plength, unsigned int ntracks);

	unsigned int pattern_num;	// the pattern in the song
	pattern *pattern;		// the physical pattern

	char gonethrough[NUMBER_PATTERNS + 1];  // to check for repeats

	unsigned int counter;		// the fine position inside the effect

	unsigned int bpm;
	unsigned int speed;		// speed number of effect repeats
	unsigned int finespeed;		// finespeed, base is 100
	resampler* const r;		// only on ctor



	unsigned long time_spent;
public:
	automaton(const song* song, resampler* r);
	automaton(const song* song, resampler& r):
	    automaton(song, &r)
	{}
	automaton(const song* song):
	    automaton(song, nullptr)
	{}
	void reset_to_pattern(unsigned int start);
	void next_tick();
	void update_tempo();
	void set_bpm(unsigned int bpm);
	void play_one_tick();
	void dump_events() const;
	void setup_effect(channel&, const event&);
	const event& EVENT(int channel) const;

	void compute_duration(song* song);

	unsigned int note_num;		// the note in the pattern
	const song_info *info;		// we need the song_info
	int do_stuff;              	// keeping some stuff to do
					// ... and parameters for it:
	unsigned int new_speed, new_note, new_pattern, new_finespeed;

	unsigned int loop_note_num; 	// for command E6
	unsigned int delay_counter; 	// for command EE
				// =0 -> no delay, next pattern immediately
				// >0 -> count down
};

const auto NORMAL_FINESPEED=125;
