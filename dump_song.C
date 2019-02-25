/* dump_song.c */

#include "defs.h"

#include <ctype.h>
#include <unistd.h>

#include "song.h"
#include "extern.h"
#include "notes.h"
#include "channel.h"
#include "prefs.h"

LOCAL void *handle = 0;
LOCAL char buffer[80];

extern char instname[];	/* from display.c */



/***
 ***	dump_block/dump_song:
 ***		show most of the readable info concerning a module on the screen
 ***/

#if 0
/* THIS NEED SOME UPDATING (FIXED VALUE BLOCK_LENGTH/NUMBER_TRACKS) */
LOCAL void dump_block(b)
struct block *b;
   {
   int i, j;

   for (i = 0; i < BLOCK_LENGTH; i++)
      {
      for (j = 0; j < NUMBER_TRACKS; j++)
         {
         sprintf(buffer,"%8d%5d%2d%4d", b->e[j][i].sample_number,
            b->e[j][i].effect,
            b->e[j][i].parameters);
         infos(handle, buffer);
         }
      info(handle, "");
      }
   }
#endif

/* make_readable(s):
 * transform s into a readable string 
 */
LOCAL void make_readable(char *s)
   {
   char *t, *orig;

   if (!s)
      return;

   orig = s;
   t = s;

      /* get rid of the st-xx: junk */
   if (strncmp(s, "st-", 3) == 0 || strncmp(s, "ST-", 3) == 0)
      {
      if (isdigit(s[3]) && isdigit(s[4]) && s[5] == ':')
         s += 6;
      }
   while (*s)
      {
      if (isprint(*s))
         *t++ = *s;
      s++;
      }
   *t = '\0';
   while (t != orig && isspace(t[-1]))
      *--t = '\0';
   }

void dump_song(struct song *song)
   {
   unsigned int i;
	size_t j;
   size_t maxlen;
   static char dummy[1];

   
   handle = begin_info(song->title);
   if (!handle)
      return;

   dummy[0] = '\0';
   maxlen = 0;
   for (i = 1; i <= song->ninstr; i++)
      {
		if (!song->samples[i]->name)
			song->samples[i]->name = dummy;
		make_readable(song->samples[i]->name);
		if (maxlen < strlen(song->samples[i]->name))
			maxlen = strlen(song->samples[i]->name);
      }
   for (i = 1; i <= song->ninstr; i++)
      {
		if (song->samples[i]->start || strlen(song->samples[i]->name) > 2)
			{
			static char s[15];
			char *base = s;
			
			if (get_pref_scalar(PREF_COLOR))
				{		
				base = write_color(base, song->samples[i]->color);
				}
			*base++ = instname[i];
			*base++ = ' ';
			*base++ = 0;
			infos(handle, s);
			infos(handle, song->samples[i]->name);
			for (j = strlen(song->samples[i]->name); j < maxlen + 2; j++)
				infos(handle, " ");
			if (song->samples[i]->start)
				{
				sprintf(buffer, "%6lu", song->samples[i]->length);
				infos(handle, buffer);
				if (song->samples[i]->rp_length > 2)
					{
					sprintf(buffer, "(%6lu %6lu)", 
						song->samples[i]->rp_offset, 
						song->samples[i]->rp_length);
					infos(handle, buffer);
					}
				else
					infos(handle, "             ");
				if (song->samples[i]->volume != MAX_VOLUME)
					{
					sprintf(buffer, "%3u", song->samples[i]->volume);
					infos(handle, buffer);
					}
				else 
					infos(handle, "   ");
				if (song->samples[i]->finetune)
					{
					sprintf(buffer, "%3d", song->samples[i]->finetune);
					infos(handle, buffer);
					}
				}
			base = s;
			if (get_pref_scalar(PREF_COLOR))
				base = write_color(base, 0);
			*base = 0;
			info(handle, s);
			}
      }
   end_info(handle);
   handle = 0;
   }
