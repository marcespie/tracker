#ifndef UI_H
#define UI_H
/* ui.h */
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

#include <iosfwd>
class Info {
public:
	const bool fg;
	std::ostream& out;
	Info(const char* title =nullptr);
};

// XXX iomanip objects are of unspecified types, so I can't write template
// specializations


template<typename T>
Info& operator<<(Info& o, T t)
{
	if (o.fg)
		o.out << t;
    	return o;
}
extern std::pair<int, unsigned long> get_ui();
const auto BASE_UI=10;
const auto UI_NEXT_SONG=BASE_UI;	/* load next song */
const auto UI_PREVIOUS_SONG=BASE_UI+1;	/* load previous song */
const auto UI_LOAD_SONG=BASE_UI+2;	/* load song. Name as value */
const auto UI_SET_BPM=BASE_UI+3;	/* set beat per minute to value */
const auto UI_JUMP_TO_PATTERN=BASE_UI+4;/* jump to pattern #value.  Use
       					 * display_pattern to keep in 
					 * sync with the player */
const auto UI_RESTART=BASE_UI+5; 	/* restart current song. Not 
					 * quite jump to 0 */
const auto UI_QUIT=BASE_UI+6;
const auto UI_DISPLAY=BASE_UI+7; 	/* status of scrolling window: 
					 * true or false */ 

/* st_play.c translates the get_ui() tags in a standard way.
 * Actually it doesn't translate anything right now...
 */
const auto BASE_PLAY=20;
const auto PLAY_NEXT_SONG=UI_NEXT_SONG;
const auto PLAY_PREVIOUS_SONG=UI_PREVIOUS_SONG;
const auto PLAY_LOAD_SONG=UI_LOAD_SONG;

const auto PLAY_ERROR=BASE_PLAY;
const auto PLAY_ENDED=BASE_PLAY+1;

/* Most of these functions are information display function.
 * A correct implementation should heed run_in_fg() if needed
 */

/* status(s): some indication of the system current status... 
 * Used for fleeing error messages too. 
 * s = 0 is valid and indicates return to the default status.
 */
extern void status(const std::string&);

/* Scrolling score display:
 * new_scroll() returns a writable buffer of a suitable length for n tracks
 * in which display.c will write what it needs.
 * It can return 0 if not applicable.
 */
extern char *new_scroll();

class audio;
/* set_number_tracks(n) sets the number of tracks for new_scroll, in order
 * to allocate room accordingly
 */
extern void set_number_tracks(int n, audio& a);

/* scroll: returns this scrolling line to the program. Note that
 * scroll implies calls to new_scroll/scroll
 * are paired. After a call to scroll, the last pointer returned by new_scroll
 * should be considered invalid !
 */
extern void scroll(char *end);

/* display_pattern(current, total, real): we are at current/total(real) 
 * in the current song
 * may be used as a poor man's timer.
 */
extern void display_pattern(unsigned int current, unsigned int total, 
	unsigned int real, 
	unsigned long uptilnow, unsigned long totaltime);

extern void display_time(unsigned long time_elapsed, unsigned long check);

/* song_title(s): the current song title is s.
 * ui implementors: Don't count on this pointer remaining valid AFTER the call,
 * make a copy if needed
 */
extern void song_title(const char *s);

extern void audio_ui(char c);
#endif
