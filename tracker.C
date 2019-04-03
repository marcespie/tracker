/* main.c */
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

#include <signal.h>
#include <iostream>
     
#include "protracker.h"
#include "extern.h"
#include "autoinit.h"

#include "prefs.h"
#include "play_list.h"
#include "open.h"
#include "pro_play.h"
     
extern void print_usage(void);
extern unsigned long half_mask;
extern unsigned int ask_freq;
extern int stereo;
extern unsigned int start;
extern int trandom;
extern int loop;
extern int handle_options(int argc, char *argv[]);
extern void set_default_prefs(void);

/* global variable to catch various types of errors and achieve the 
 * desired flow of control
 */
int error;

/* song = load_song(namesong):
 * syntactic sugar around read_song
 *	- display the file name after stripping the path
 * - find the actual file
 * - read the song trying several formats
 * - handle errors gracefully
 */
static song *
load_song(ENTRY e)
{
	song *song = nullptr;
	exfile file;

	std::cout << e->name << "...";

	/* read the song */
	if (file.open(e->name)) {
		switch(e->filetype) {
		case NEW:
			song = read_song(file, NEW);
			break;
		case OLD:
			song = read_song(file, OLD);
			break;
		case UNKNOWN:
			switch(pref::get(Pref::type)) {
			case BOTH:
				song = read_song(file, NEW);
				if (song) {
					e->filetype = NEW;
					break;
				} else {
					file.rewind();
					[[fallthrough]];
				}
			case OLD:
				song = read_song(file, OLD);
				if (song)
					e->filetype = OLD;
				break;
			/* this is explicitly flagged as a new module,
			* so we don't need to look for a signature.
			*/
			case NEW:
				song = read_song(file, NEW_NO_CHECK);
				if (song)
					e->filetype = NEW;
				break;
			default:
				break;
			}
		}
	}

	if (!song)
		std::cout << "Not a song\n";

	return song;
}


static void 
adjust_song(song *s, unsigned long m)
{
	for (unsigned i = 1; i <= s->ninstr; i++)
		if ( (1 << i) & ~m) {
			for (unsigned j = 0; j <= MAX_VOLUME; j++)
				s->samples[i]->volume_lookup[j] *= 2;
		}
	s->side_width++;
}

int 
main(int argc, char *argv[])
{
	set_default_prefs();
	if (argc == 1) {
		print_usage();
		end_all();
	}


	// remove the program name from the options to parse !!!
	handle_options(argc-1, argv+1);
	if (trandom)
		randomize();

	auto list = obtain_play_list();

	for (auto it = begin(list); it != end(list);) {
		auto song = load_song(it);

		if (song) {
			if (pref::get(Pref::dump))
				dump_song(song); 
			if (half_mask)
				adjust_song(song, half_mask);
			setup_audio(ask_freq, stereo);
			auto result = play_song(song, start);
			release_song(song);
			status("");
			switch(result) {
			case PLAY_PREVIOUS_SONG:
				--it;
				break;
				/* NOTREACHED */
			case PLAY_NEXT_SONG:
			case PLAY_ENDED:
				++it;
				break;
			case PLAY_ERROR:
				it = delete_entry(it);
			default:
				break;
			}
		} else
			it = delete_entry(it);

		if (it == end(list)) {
			if (loop)
				it = begin(list);
			else
				end_all();
		}
	}
	end_all();
	/* NOTREACHED */
}

