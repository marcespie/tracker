/* notes.c */

#include "defs.h"

#include <ctype.h>
#include <assert.h>
#include <math.h>

#include "song.h"
#include "extern.h"
#include "notes.h"
#include "channel.h"
#include "autoinit.h"

#define NUMBER_NOTES 120
#define NUMBER_FINETUNES 17

static void create_notes_table(void);
static void (*INIT)(void) = create_notes_table;


/* the musical notes correspond to some specific pitch.
 * It's useful to be able to find them back, at least for
 * arpeggii.
 */

/* pitch values are stored in the range 1 <= n <= NUMBER_NOTES
 * note 0 is NO_NOTE, with corresponding null pitch
 */
static pitch pitch_table[NUMBER_NOTES+1][NUMBER_FINETUNES];

static const char *note_template = "C-C#D-D#E-F-F#G-G#A-A#B-";


/* note = pitch2note(pitch): 
 * find note corresponding to the stated pitch 
 */
note pitch2note(pitch pitch)
   {
   note a, b, i;
   
   INIT_ONCE;

   if (pitch == 0)
      return NO_NOTE;
   a = 1;
   b = NUMBER_NOTES;
   while(b-a > 1)
      {
      i = (a+b)/2;
      if (pitch_table[i][0] == pitch)
         return i;
      if (pitch_table[i][0] > pitch)
         a = i;
      else
         b = i;
      }
   if (pitch_table[a][0] - FUZZ <= pitch)
      return a;
   if (pitch_table[b][0] + FUZZ >= pitch)
      return b;
	error = CORRUPT_FILE;
   return NO_NOTE;
   }

/* pitch = round_pitch(pitch, finetune):
 * return the pitch corresponding to the nearest note for the given
 * finetune
 */
pitch round_pitch(pitch pitch, finetune finetune)
	{
   note a, b, i;
   
   INIT_ONCE;

   if (pitch == 0)
      return pitch;
   a = 1;
   b = NUMBER_NOTES;
   while(b-a > 1)
      {
      i = (a+b)/2;
      if (pitch_table[i][finetune] == pitch)
         return pitch;
      if (pitch_table[i][finetune] > pitch)
         a = i;
      else
         b = i;
      }
			/* need some check for the actual nearest note ? */
	return pitch_table[i][finetune];
   }


static void create_notes_table(void)
   {
   double base, pitch;
   int i, j, k;

   for (j = -8; j < 8; j++)
      {
      k = j < 0 ? j + 16 : j;
      base = AMIGA_CLOCKFREQ/440.0/4.0 / pow(2.0, j/96.0);
			/* relies on NO_NOTE being 0 */
		pitch_table[NO_NOTE][k] = 0;

      for (i = 0; i < NUMBER_NOTES;)
         {
         pitch = base / pow(2.0, i/12.0);
				/* take care of offset between i and stored table */
         pitch_table[++i][k] = floor(pitch + 0.5);
         }
      }
    }

const char *
note2name(note i)
{
	static char name[4];

	switch(i) {
	case NO_NOTE:
		return "   ";
	default:
		name[0] = note_template[(i+8)%12 * 2];
		name[1] = note_template[(i+8)%12 * 2 +1];
		name[2] = '0' + (i-4)/12;
		name[3] = 0;
		return const_cast<const char *>(name);
	}
}
   
pitch note2pitch(note note, finetune finetune)
	{
	INIT_ONCE;

	if (note < NUMBER_NOTES)
		return pitch_table[note][finetune];
	else
		return 0;
	}
