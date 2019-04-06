/* extern.h */
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

#include <string.h>
#include <string>
#include <functional>
#include <utility>
#include <iosfwd>
#include <cstdint>

const auto READ_ONLY="rb";
const auto WRITE_ONLY="wb";

template<typename S, typename T>
inline auto
MIN(S x, T y)
{
	return x<y ?  x : y;
}

template<typename S, typename T>
inline auto
MAX(S x, T y)
{
	return x>y ?  x : y;
}

/* predefinitions for relevant structures */
struct channel; 
struct song;
struct automaton;
struct sample_info;
struct event;
struct tempo;
struct play_entry;
struct option_set;
/*--------------------------- dump_song.c ------------------------*/
/* dump_song(s): 
 * display some information pertinent to the given song s
 */
extern void dump_song(struct song *song);


/*--------------------------- display.c --------------------------*/
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
extern void dump_delimiter(void);


/*--------------------------- main.c -----------------------------*/
const auto OLD=0;
const auto NEW=1;
/* special new type: for when we try to read it as both types.
 */
const auto BOTH=2;
/* special type: does not check the signature */
const auto NEW_NO_CHECK=3;


/* error types. Everything is centralized,
 * and we check in some places (see st_read, player and main)
 * that there was no error. Additionally signal traps work
 * that way too.
 */
 
/* normal state */
const auto NONE=0;
/* read error */
const auto FILE_TOO_SHORT=1;
const auto CORRUPT_FILE=2;
/* trap error: goto next song right now */
const auto NEXT_SONG=3;
/* run time problem */
const auto FAULT=4;
/* the song has ended */
const auto ENDED=5;
/* unrecoverable problem: typically, trying to 
 * jump to nowhere land.
 */
const auto UNRECOVERABLE=6;
/* Missing sample. Very common error, not too serious. */
const auto SAMPLE_FAULT=7;
/* New */
const auto PREVIOUS_SONG=8;
const auto OUT_OF_MEM=9;

/* all soundtracker feature */
const auto NOT_SUPPORTED=10;
extern int error;


/*--------------------------- play_list.c ------------------------*/


/*--------------------------- st_read.c --------------------------*/
/* s = read_song(f, type):
 * tries to read f as a song of type NEW/OLD/NEW_NOCHECK
 * returns NULL (and an error) if it doesn't work.
 * Returns a dynamic song structure if successful.
 */
extern struct song *read_song(class exfile& f, int type);

/* release_song(s):
 * release all the memory song occupies.
 */
extern void release_song(struct song *song);


/*--------------------------- st_virt.c --------------------------*/
extern void compute_duration(struct automaton *a, struct song *song);


/*--------------------------- setup_audio.c ----------------------*/
/* setup_audio(ask_freq, stereo, oversample):
 * setup the audio output with these values 
 */
extern void setup_audio(unsigned long f, int s);

/*--------------------------- audio.c ----------------------------*/
/* frequency = open_audio(f, s):
 * try to open audio with a sampling rate of f, and eventually stereo.
 * We get the real frequency back. If we ask for 0, we
 * get the ``preferred'' frequency.
 * Note: we have to close_audio() before we can open_audio() again.
 * Note: even if we don't ask for stereo, we still have to give a
 * right and left sample.
 */
extern unsigned long open_audio(unsigned long f, int s);
/* close_audio():
 * returns the audio to the system control, doing necessary
 * cleanup
 */
extern void close_audio(void);
/* set_mix(percent): set mix channels level.
 * 0: spatial stereo. 100: mono.
 */
extern void set_mix(int percent);

/* output_samples(l, r, n): outputs a pair of stereo samples.
 * Samples are n bits signed.
 * Output routine should be able to face anything from 16 to 25
 */
extern void output_samples(int32_t left, int32_t right, int n);

/* flush_buffer(): call from time to time, because buffering
 * is done by the program to get better (?) performance.
 */
extern void flush_buffer(void);

/* discard_buffer(): try to get rid of the buffer contents
 */
extern void discard_buffer(void);

/* sync_audio(function, f2, parameter):
 * call function(parameter) when audio finally gets to the current point
 * call f2(parameter) if flush is in effect instead
 */
extern void sync_audio(std::function<void()>, std::function<void()>);

/*--------------------------- ui.c ------------------------*/
/* get_ui(): returns a user-interface action + optional parameter
 */

extern std::pair<int, unsigned long> get_ui(void);
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

class Info;
/* begin_info: open a logical information window.
 * returns nullptr if the window couldn't be opened.
 * A NULL window shouldn't be used, but don't count on it !
 */
extern Info *begin_info(const char *title);
/* info(handle, line): add a line to the info window,
 * completing the current line if applicable
 */
extern void info(Info *handle, const char *line);
/* infos(handle, line): add to the current line of the info window
 */
extern void infos(Info *handle, const char *s);
/* end_info(handle): this window is complete...
 */
extern void end_info(Info *handle);

/* Scrolling score display:
 * new_scroll() returns a writable buffer of a suitable length for n tracks
 * in which display.c will write what it needs.
 * It can return 0 if not applicable.
 */
extern char *new_scroll(void);

/* set_number_tracks(n) sets the number of tracks for new_scroll, in order
 * to allocate room accordingly
 */
extern void set_number_tracks(int n);

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

/*--------------------------- color.c ----------------------------*/
/* s = write_color(base, color):
 * write sequence to switch to color color at base, returning
 * position after the sequence
 */
extern char *write_color(char *base, unsigned int color);

extern void audio_ui(char c);
/*--------------------------- parse_options.c --------------------*/

extern int string2args(char *s, char *v[]);

extern void add_play_list(const char *);
