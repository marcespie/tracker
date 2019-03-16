/* p_automaton.h */

#define DO_SET_NOTHING 0 
#define SET_SPEED 1
#define SET_SKIP 2
#define SET_FASTSKIP 4
#define SET_FINESPEED 32

#define JUMP_PATTERN 8
#define DELAY_PATTERN 16

#define NORMAL_SPEED 6
                                                                               
struct automaton {
   unsigned int pattern_num;           /* the pattern in the song */
   unsigned int note_num;              /* the note in the pattern */
   struct pattern *pattern;     /* the physical pattern */
   struct song_info *info;    /* we need the song_info */

   char gonethrough[NUMBER_PATTERNS + 1];  /* to check for repeats */

   unsigned int counter;               /* the fine position inside the effect */

	unsigned int bpm;
	unsigned int speed;				/* speed number of effect repeats */
	unsigned int finespeed;			/* finespeed, base is 100 */
   
   int do_stuff;              /* keeping some stuff to do */
                              /* ... and parameters for it: */
   unsigned int new_speed, new_note, new_pattern, new_finespeed;


   unsigned int loop_note_num;
                              /* for command E6 */

   unsigned int delay_counter;
                              /* for command EE */
										/* =0 -> no delay, next pattern immediately
										 * >0 -> count down */
	unsigned long time_spent;
};
