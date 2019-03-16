/* main.c */

#include "defs.h"


#include <signal.h>
     
#include "song.h"
#include "extern.h"
#include "autoinit.h"

#include "tags.h"
#include "prefs.h"
#include "play_list.h"
#include "open.h"
#include "Modules/Pro/play.h"
     
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
	song *song;
	char *buffer;
	struct exfile *file;
	const char *name;
	size_t i, j;

	name = e->filename;
	/* display the file name */
	i = strlen(name);

	j = 0;

	buffer = (char*)malloc( i - j + 5);
	if (buffer) {
		sprintf(buffer, "%s...", name + j);
		status(buffer);
	}

	/* read the song */
	file = open_file(name, "r", getenv("MODPATH"));
	if (file) {
		switch(e->filetype) {
		case NEW:
			song = read_song(file, NEW);
			break;
		case OLD:
			song = read_song(file, OLD);
			break;
		case UNKNOWN:
			switch(get_pref_scalar(PREF_TYPE)) {
			case BOTH:
				song = read_song(file, NEW);
				if (song) {
					e->filetype = NEW;
					break;
				} else
					rewind_file(file);
				/* FALLTHRU */
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
				song = nullptr;
			}
			close_file(file);
		}
	} else
		song = nullptr;

	if (!song)
		notice("Not a song");
	/* remove the displayed file name */
	if (buffer) {
		status(0);
		free(buffer);
	}

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
		end_all(0);
	}


	/* remove the program name from the options to parse !!! */
	handle_options(argc-1, argv+1);
	if (trandom)
		randomize();
	auto play_list = obtain_play_list();

	int song_number = 0;
	while(true) {
		int n = last_entry_index();
		if (n < 0)
			end_all("No playable song");
		if (song_number < 0)
			song_number = n;
		if (song_number > n) {
			if (loop)
				song_number = 0;
			else
				end_all(0);
		}
		auto song = load_song(play_list+song_number);

		if (song) {
			if (get_pref_scalar(PREF_DUMP))
				dump_song(song); 
			if (half_mask)
				adjust_song(song, half_mask);
			setup_audio(ask_freq, stereo);
			auto result = play_song(song, start);
			release_song(song);
			status(0);
			while ( (result = get_tag(result)) ) {
				switch (result->type) {
				case PLAY_PREVIOUS_SONG:
					song_number--;
					break;
					/* NOTREACHED */
				case PLAY_NEXT_SONG:
				case PLAY_ENDED:
					song_number++;
					break;
				case PLAY_ERROR:
					delete_entry(play_list+song_number);
				default:
					break;
				}
				result++;
			}
		} else
			delete_entry(play_list+song_number);
	}
	end_all(0);
	/* NOTREACHED */
}

