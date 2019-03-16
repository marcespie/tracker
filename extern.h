/* extern.h */

/*--------------------------- dump_song.c ------------------------*/
/* dump_song(s): 
 * display some information pertinent to the given song s
 */
XT void dump_song(struct song *song);


/*--------------------------- display.c --------------------------*/
/* dump_event(ch, e): dump event e as occuring on channel ch
 * (some events need the current channel state for a correct dump)
 * special case: ch == 0 means current set of events done
 */
XT void dump_event(const channel *ch, const event *e);

/* dump_delimiter(): add a delimiter to the current dump, to 
 * separate left channels from right channels, for instance
 */
XT void dump_delimiter(void);


/*--------------------------- main.c -----------------------------*/
#define OLD 0
#define NEW 1
/* special new type: for when we try to read it as both types.
 */
#define BOTH 2
/* special type: does not check the signature */
#define NEW_NO_CHECK 3


/* error types. Everything is centralized,
 * and we check in some places (see st_read, player and main)
 * that there was no error. Additionally signal traps work
 * that way too.
 */
 
/* normal state */
#define NONE 0  
/* read error */
#define FILE_TOO_SHORT 1
#define CORRUPT_FILE 2
/* trap error: goto next song right now */
#define NEXT_SONG 3
/* run time problem */
#define FAULT 4
/* the song has ended */
#define ENDED 5
/* unrecoverable problem: typically, trying to 
 * jump to nowhere land.
 */
#define UNRECOVERABLE 6
/* Missing sample. Very common error, not too serious. */
#define SAMPLE_FAULT 7
/* New */
#define PREVIOUS_SONG 8
#define OUT_OF_MEM 9

/* all soundtracker feature */
#define NOT_SUPPORTED 10
XT int error;


/*--------------------------- play_list.c ------------------------*/


/*--------------------------- st_read.c --------------------------*/
/* s = read_song(f, type):
 * tries to read f as a song of type NEW/OLD/NEW_NOCHECK
 * returns NULL (and an error) if it doesn't work.
 * Returns a dynamic song structure if successful.
 */
XT struct song *read_song(struct exfile *f, int type);

/* release_song(s):
 * release all the memory song occupies.
 */
XT void release_song(struct song *song);


/*--------------------------- st_virt.c --------------------------*/
XT void compute_duration(struct automaton *a, struct song *song);


/*--------------------------- setup_audio.c ----------------------*/
/* setup_audio(ask_freq, stereo, oversample):
 * setup the audio output with these values 
 */
XT void setup_audio(unsigned long f, int s);

/*--------------------------- audio.c ----------------------------*/
/* frequency = open_audio(f, s):
 * try to open audio with a sampling rate of f, and eventually stereo.
 * We get the real frequency back. If we ask for 0, we
 * get the ``preferred'' frequency.
 * Note: we have to close_audio() before we can open_audio() again.
 * Note: even if we don't ask for stereo, we still have to give a
 * right and left sample.
 */
XT unsigned long open_audio(unsigned long f, int s);
/* close_audio():
 * returns the audio to the system control, doing necessary
 * cleanup
 */
XT void close_audio(void);
/* set_mix(percent): set mix channels level.
 * 0: spatial stereo. 100: mono.
 */
XT void set_mix(int percent);

/* output_samples(l, r, n): outputs a pair of stereo samples.
 * Samples are n bits signed.
 * Output routine should be able to face anything from 16 to 25
 */
XT void old_output_samples(long left, long right);
XT void output_samples(long left, long right, int n);

/* flush_buffer(): call from time to time, because buffering
 * is done by the program to get better (?) performance.
 */
XT void flush_buffer(void);

/* discard_buffer(): try to get rid of the buffer contents
 */
XT void discard_buffer(void);

/* new_freq = update_frequency():
 * if !0, frequency changed and playing should be updated accordingly
 */
XT unsigned long update_frequency(void);

/* bits = output_resolution()
 * returns the number of bits expected for the output.
 * Not necessary to use 16 bit samples if output is to be 8 bits
 * for instance. Return 16 by default
 */
XT int output_resolution(void);

/* sync_audio(function, f2, parameter):
 * call function(parameter) when audio finally gets to the current point
 * call f2(parameter) if flush is in effect instead
 */

XT void sync_audio
	(void (*function)(GENERIC),  void (*f2) (GENERIC), GENERIC parameter);

#ifdef SPECIAL_SAMPLE_MEMORY
XT GENERIC alloc_sample(unsigned long len);
XT void free_sample(GENERIC s);
XT int obtain_sample(GENERIC start, unsigned long l, struct exfile *f);

#else
#define alloc_sample(len)		calloc(len, 1)
#define free_sample(sample)		free(sample)
#define obtain_sample(start, l, f)	read_file(start, 1, l, f)
#endif


/*--------------------------- $(UI)/ui.c ------------------------*/
/* see unix/ui.c for the general unix implementation.
 * The old may_getchar() has been replaced by the tag-based
 * get_ui
 */
/* get_ui(): returns an array of tags that reflect the current user-interface
 * actions. Unknown tags WILL be ignored.
 * Note that get_ui will be called about once every tick, providing a poor man's
 * timer to the interface writer if needed to code multiple actions on the same
 * user-input. See unix/termio.c for a good example.
 * see amiga/ui.c for the correct way to do it when you have a real timer.
 *
 * VERY IMPORTANT: who do the tags belong to ?
 *   as a general rule, result (and their values) MUST only be considered
 *   valid between two calls to get_ui ! Be careful to call get_ui ONLY at
 *   reasonable places.
 *   One exception: structures that are dynamically allocated (like UI_LOAD_SONG
 *   values) will ONLY get freed when you ask for it !
 */
XT struct tag *get_ui(void);
#define BASE_UI 10
#define UI_NEXT_SONG	(BASE_UI)            /* load next song */
#define UI_PREVIOUS_SONG (BASE_UI + 1)    /* load previous song */
#define UI_LOAD_SONG (BASE_UI + 2)        /* load song. Name as value */
#define UI_SET_BPM (BASE_UI + 3)          /* set beat per minute to value */
#define UI_JUMP_TO_PATTERN (BASE_UI + 4)  /* jump to pattern #value.  Use 
       												 * display_pattern to keep in 
														 * sync with the player */
#define UI_RESTART (BASE_UI + 5)          /* restart current song. Not 
														 * quite jump to 0 */
#define UI_QUIT (BASE_UI + 6)             /* need I say more ? */
#define UI_DISPLAY (BASE_UI + 7)          /* status of scrolling window: 
														 * true or false */


/* st_play.c translates the get_ui() tags in a standard way.
 * Actually it doesn't translate anything right now...
 */
#define BASE_PLAY 20
#define PLAY_NEXT_SONG UI_NEXT_SONG
#define PLAY_PREVIOUS_SONG UI_PREVIOUS_SONG
#define PLAY_LOAD_SONG UI_LOAD_SONG

#define PLAY_ERROR BASE_PLAY
#define PLAY_ENDED (BASE_PLAY+1)

/* Most of these functions are information display function.
 * A correct implementation should heed run_in_fg() if needed
 */

/* notice(s, ...): important message for the user (terminal error maybe).
 * take extra pain to make it apparent even if run in background
 */
XT void notice(const char *fmt, ...);
XT void vnotice(const char *fmt, va_list al);

/* status(s): some indication of the system current status... 
 * Used for fleeing error messages too. 
 * s = 0 is valid and indicates return to the default status.
 */
XT void status(const char *s);

/* begin_info: open a logical information window.
 * returns 0 if the window couldn't be opened.
 * A NULL window shouldn't be used, but don't count on it !
 */
XT GENERIC begin_info(const char *title);
/* info(handle, line): add a line to the info window,
 * completing the current line if applicable
 */
XT void info(GENERIC handle, const char *line);
/* infos(handle, line): add to the current line of the info window
 */
XT void infos(GENERIC handle, const char *s);
/* end_info(handle): this window is complete...
 */
XT void end_info(GENERIC handle);

/* Scrolling score display:
 * new_scroll() returns a writable buffer of a suitable length for n tracks
 * in which display.c will write what it needs.
 * It can return 0 if not applicable.
 */
XT char *new_scroll(void);

/* set_number_tracks(n) sets the number of tracks for new_scroll, in order
 * to allocate room accordingly
 */
XT void set_number_tracks(int n);

/* scroll: returns this scrolling line to the program. Note that
 * scroll implies calls to new_scroll/scroll
 * are paired. After a call to scroll, the last pointer returned by new_scroll
 * should be considered invalid !
 */
XT void scroll(char *end);

/* display_pattern(current, total, real): we are at current/total(real) 
 * in the current song
 * may be used as a poor man's timer.
 */
XT void display_pattern(unsigned int current, unsigned int total, 
	unsigned int real, 
	unsigned long uptilnow, unsigned long totaltime);

XT void display_time(unsigned long time_elapsed, unsigned long check);

/* song_title(s): the current song title is s.
 * ui implementors: Don't count on this pointer remaining valid AFTER the call,
 * make a copy if needed
 */
XT void song_title(const char *s);

/* boolean checkbrk():
 * check whether a break occured and we should end right now.
 * Call it often enough (like when loading songs and stuff)
 */
XT bool checkbrk(void);


/*--------------------------- color.c ----------------------------*/
/* s = write_color(base, color):
 * write sequence to switch to color color at base, returning
 * position after the sequence
 */
XT char *write_color(char *base, unsigned int color);

XT void audio_ui(char c);
/*--------------------------- parse_options.c --------------------*/

/* add_option_set(set)
 * add a set of options to known options. 
 */
XT void add_option_set(struct option_set *options);

/* parse_options(argc, argv, what_to_do):
 * parse options according to the currently known options. Call
 * (*what_to_do)(arg) on any real argument
 */
XT void parse_options(int argc, char *argv[], void (*what_to_do) (const char *arg));

XT int string2args(char *s, char *v[]);

extern void add_play_list(const char *);
