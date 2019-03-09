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
     
XT void print_usage(void);
XT unsigned long half_mask;
XT unsigned int ask_freq;
XT int stereo;
XT unsigned int start;
XT int trandom;
XT int loop;
XT int handle_options(int argc, char *argv[]);
XT void set_default_prefs(void);

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
LOCAL struct song *load_song(ENTRY e)
   {
   struct song *song;
   char *buffer;
	struct exfile *file;
	const char *name;
   size_t i, j;
   
	name = e->filename;
		/* display the file name */
   i = strlen(name);
   
	/*
   for (j = i; j > 0; j--)
      if (name[j] == '/' || name[j] == '\\')
         {
         j++;
         break;
         }
			*/
	j = 0;
   
   buffer = (char*)malloc( i - j + 5);
   if (buffer)
      {
      sprintf(buffer, "%s...", name + j);
      status(buffer);
      }

		/* read the song */
	file = open_file(name, "r", getenv("MODPATH"));
	if (file)
		{
		switch(e->filetype)
			{
		case NEW:
			song = read_song(file, NEW);
			break;
		case OLD:
			song = read_song(file, OLD);
			break;
		case UNKNOWN:
			switch(get_pref_scalar(PREF_TYPE))
				{
			case BOTH:
				song = read_song(file, NEW);
				if (song)
					{
					e->filetype = NEW;
					break;
					}
				else
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
				song = NULL;
				}
			close_file(file);
			}
		}
	else
		song = NULL;

	if (!song)
		notice("Not a song");
		/* remove the displayed file name */
	if (buffer)
		{
		status(0);
		free(buffer);
		}

	return song;
   }


LOCAL void adjust_song(struct song *s, unsigned long m)
	{
	unsigned i, j ;

	for (i = 1; i <= s->ninstr; i++)
		if ( (1 << i) & ~m)
			{
			for (j = 0; j <= MAX_VOLUME; j++)
				s->samples[i]->volume_lookup[j] *= 2;
			}
	s->side_width++;
	}

int main(int argc, char *argv[])
   {
	int song_number, n;
   struct song *song;
	ENTRY play_list;

   struct tag *result;

	EXPAND_WILDCARDS(argc,argv);

	set_default_prefs();
   if (argc == 1)
      {
      print_usage();
      end_all(0);
      }


		/* remove the program name from the options to parse !!! */
	handle_options(argc-1, argv+1);
	if (trandom)
		randomize();
	play_list = obtain_play_list();

	song_number = 0;
	while(1)
		{
		n = last_entry_index();
		if (n < 0)
			end_all("No playable song");
		if (song_number < 0)
			song_number = n;
		if (song_number > n)
			{
			if (loop)
				song_number = 0;
			else
				end_all(0);
			}
		song = load_song(play_list+song_number);

		if (song)
			{
			if (get_pref_scalar(PREF_DUMP))
				dump_song(song); 
			if (half_mask)
				adjust_song(song, half_mask);
			setup_audio(ask_freq, stereo);
			result = play_song(song, start);
			release_song(song);
			status(0);
			while ( (result = get_tag(result)) )
				{
				switch (result->type)
					{
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
			}
		else
			delete_entry(play_list+song_number);
		}
   end_all(0);
   /* NOTREACHED */
   }


