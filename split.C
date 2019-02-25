/* split.c */



#include <signal.h>
     
#include "defs.h"
#include "song.h"
#include "extern.h"
#include "open.h"
     
     
/* global variable to catch various types of errors
 * and achieve the desired flow of control
 */
int error;

LOCAL struct song *do_scan_song(name, type)
char *name;
int type;
   {
   struct song *song;
   struct exfile *file;

   file = open_file(name, "r", getenv("MODPATH"));
   if (!file)
      return NULL;
   song = read_song(file, type); 
   close_file(file);
   return song;
   }

#define CHUNK_SIZE 32000

LOCAL char *suffix[] =
   {
   "lzh", "lha", "Z", "z", "shn", "zoo", 0
   };

LOCAL void truncate(name)
char *name;
   {
   int i;
   int last_point = 0;

   for (i = 0; name[i]; i++)
      {
      if (name[i] == '.')
         last_point = i + 1;
      }
   if (last_point)
      {
      for (i = 0; suffix[i]; i++)
      if (strcmp(name + last_point, suffix[i]) == 0)
         {
         name[last_point - 1] = 0;
         return;
         }
      }
   }

   
   
void split_module(name, cutpoint)
char *name;
long cutpoint;
   {
   char buffer[300];
   FILE *mod;
   FILE *samp;
   struct exfile *file;
   char *copy_buff;
   int chunk;

   file = open_file(name, "r", getenv("MODPATH"));
   truncate(name);
   sprintf(buffer, "%s.mod", name);
   mod = fopen(buffer, "w");
   if (!mod)
      exit(10);
   sprintf(buffer, "%s.samp", name);
   samp = fopen(buffer, "w");
   if (!samp)
      exit(10);
   copy_buff = malloc(CHUNK_SIZE);
   if (!copy_buff)
      exit(10);
   while(cutpoint >= CHUNK_SIZE)
      {
		read_file(copy_buff, 1, CHUNK_SIZE, file);
      fwrite(copy_buff, 1, CHUNK_SIZE, mod);
      cutpoint -= CHUNK_SIZE;
      }
   if (cutpoint > 0)
      {
      read_file(copy_buff, 1, cutpoint, file);
      fwrite(copy_buff, 1, cutpoint, mod);
      }
   fclose(mod);
   while ((chunk = read_file(copy_buff, 1, CHUNK_SIZE, file)) > 0)
      fwrite(copy_buff, 1, chunk, samp);
   fclose(samp);
   close_file(file);
   }
      
   


int main(argc, argv)
int argc;
char **argv;
   {
   struct song *song;
   int i;
   int default_type;


   default_type = BOTH;

   for (i = 1; i < argc; i++)
      {
      song = do_scan_song(argv[i], NEW);
      if (!song && error != NEXT_SONG)
         song = do_scan_song(argv[i], OLD);
      if (song)
         {
         dump_song(song); 
         split_module(argv[i], song->samples_start);
         release_song(song);
         }
      }
   return 0;
   }



void sync_audio(f, f2, p)
void (*f) P((GENERIC));
void (*f2) P((GENERIC));
GENERIC p;
	{
	}

void audio_ui(c)
char c;
	{
	}

void set_mix(t)
int t;
	{
	}
