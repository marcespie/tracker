/* analyzer.c */


/* read module files and output statistics on them */

#include "defs.h"

#include "extern.h"
#include "song.h"
#include "tags.h"
#include "prefs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int error;


int use_command[16];
int use_extended[16];

void analyze_block(b, n)
struct block *b;
int n;
   {
   int i, j;
   struct event *e;

   for (i = 0; i < BLOCK_LENGTH; i++)
      {
      int special;

      special = 0;
      for (j = 0; j < NUMBER_TRACKS; j++)
         {
         e = &b->e[j][i];
         switch(e->effect)
            {
#if 0
         case 13: /* skip */
            return;
         case 11: /* fastskip */
            return;
#endif
			case 9:
				if (e->note == 255)
					printf("%d %d %d\n", n, i, j);
				break;
         case 14:
				if (!use_extended[HI(e->parameters)])
					use_extended[HI(e->parameters)] = i+1;
            break;
         case 15:
            if (special != 0 && e->parameters != special)
               putchar('!');
            else
               special = e->parameters;
         default:
				if (!use_command[e->effect])
            use_command[e->effect] = i+1;
            }
         }
      }
   }


void analyze_song(song)
struct song *song;
   {
   int i;

   for (i = 1; i <= song->ninstr ; i++)
      {
      if (song->samples[i])
         {
         if (song->samples[i]->finetune)
            printf("Sample %d: finetune is %d\n", 
               i, song->samples[i]->finetune);
         }
      }
   for (i = 0; i < 16; i++)
      {
      use_command[i] = false;
      use_extended[i] = false;
      }
   for (i = 0; i < song->info.npat; i++)
      analyze_block(song->info.pblocks+i, i);
   for (i = 0; i < 16; i++)
      if (use_command[i])
         printf("%3d", i);
   for (i = 0; i < 16; i++)
      if (use_extended[i])
         printf("%3dE", i);
   printf("\n");
   }

void do_load_song(s)
char *s;
	{
	struct stat buf;

	if (stat(s, &buf))
		return;
	if (S_ISDIR(buf.st_mode))
		{
		struct dirent *de;
		DIR *dir;

		dir = opendir(s);
		printf("%s\n", s);
		chdir(s);
		while (de = readdir(dir))
			{
			if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
				continue;
			do_load_song(de->d_name);
			}
		chdir("..");
		printf("..\n");
		closedir(dir);
		}
	else
		{
		struct exfile *file;
		struct song *song;

		file = open_file(s, "r", getenv("MODPATH"));
		if (file)
			{
			song = read_song(file, NEW);
			if (!song)
				{
				rewind_file(file);
				song = read_song(file, OLD);
				}
			close_file(file);
			if (song)
				{
				puts(s);
				analyze_song(song);
				release_song(song);
				}
			}
      }
	}

int main(argc, argv)
int argc;
char **argv;
   {
   int i;

	struct exfile *file;
   struct song *song;
   int default_type;

   default_type = BOTH;
   set_pref(PREF_TOLERATE, 2);

   for (i = 1; i < argc; i++)
		do_load_song(argv[i]);
   }


void sync_audio(f, p)
void (*f) P((void *));
GENERIC p;
	{
	}

void audio_ui(c)
int c;
	{
	}

