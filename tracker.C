/* tracker.C */
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
#include <memory>
     
#include "song.h"
#include "protracker.h"
#include "autoinit.h"

#include "prefs.h"
#include "play_list.h"
#include "open.h"
#include "fraction.h"
#include "resampler.h"
#include "usage.h"
#include "ui.h"
#include "handle_options.h"
#include "errortype.h"
#include "audio.h"
     
/* global variable to catch various types of errors and achieve the 
 * desired flow of control
 */
auto error = error_type::NONE;

/* song = load_song(namesong):
 * syntactic sugar around read_song
 *	- display the file name after stripping the path
 * - find the actual file
 * - read the song trying several formats
 * - handle errors gracefully
 */
static Song
load_song(ENTRY e)
{
	Song song;
	exfile file;

	std::cout << e->name << "...";

	/* read the song */
	if (file.open(e->name)) {
		switch(e->filetype) {
		case NEW:
			song.load(file, NEW);
			break;
		case OLD:
			song.load(file, OLD);
			break;
		case UNKNOWN:
			switch(pref::get(Pref::type)) {
			case BOTH:
				if (song.load(file, NEW)) {
					e->filetype = NEW;
					break;
				} else {
					file.rewind();
					[[fallthrough]];
				}
			case OLD:
				if (song.load(file, OLD))
					e->filetype = OLD;
				break;
			/* this is explicitly flagged as a new module,
			* so we don't need to look for a signature.
			*/
			case NEW:
				if (song.load(file, NEW_NO_CHECK))
					e->filetype = NEW;
				break;
			default:
				break;
			}
		}
	}

	if (!song)
		std::cout << "Not a song";

	std::cout << "\n";
	return song;
}


int 
main(int argc, char *argv[])
{
	audio device;
	set_default_prefs();
	if (argc == 1) {
		print_usage();
		End();
	}

	play_list list;


	// remove the program name from the options to parse !!!
	handle_options(device, argc-1, argv+1, 
	    [&list](const char *a)
	    {
	    		add_entry(list, a);
	    });
	if (trandom)
		randomize(list);

	resampler r(device);

	for (auto it = begin(list); it != end(list);) {
		auto song = load_song(it);

		if (song) {
			if (pref::get(Pref::dump))
				song.dump(); 
			if (half_mask)
				song.adjust_volume(half_mask);
			if (pref::get(Pref::output))
				device.open();
			auto result = song.play(start, r, device);
			status("");
			switch(result) {
			case PLAY_PREVIOUS_SONG:
				if (it != begin(list))
					--it;
				break;
				/* NOTREACHED */
			case PLAY_NEXT_SONG:
			case PLAY_ENDED:
				++it;
				break;
			case PLAY_ERROR:
				it = delete_entry(list, it);
			default:
				break;
			}
		} else
			it = delete_entry(list, it);

		if (it == end(list)) {
			if (loop)
				it = begin(list);
			else
				End();
		}
	}
	End();
	/* NOTREACHED */
}

