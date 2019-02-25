/* song.h 
	vi:ts=3 sw=3:
 */

/* internal data structures for the soundtracker player routine....
 */

/* $Id: song.h,v 5.6 1996/05/06 22:48:31 espie Exp espie $
 * $Log: song.h,v $
 * Revision 5.6  1996/05/06 22:48:31  espie
 * *** empty log message ***
 *
 * Revision 5.5  1996/05/06 14:29:11  espie
 * *** empty log message ***
 *
 * Revision 5.4  1996/04/09 21:22:37  espie
 * *** empty log message ***
 *
 * Revision 5.3  1996/04/09 21:14:03  espie
 * *** empty log message ***
 *
 * Revision 5.2  1996/03/14 18:04:03  espie
 * *** empty log message ***
 *
 * Revision 5.1  1995/12/24 03:13:08  espie
 * *** empty log message ***
 *
 * Revision 5.0  1995/10/21 14:57:00  espie
 * New
 *
 * Revision 4.27  1995/10/13 18:08:56  espie
 * Mixed up effects.
 *
 * Revision 4.21  1995/09/02 22:20:44  espie
 * Added song duration.
 *
 * Revision 4.15  1995/06/26 10:11:17  espie
 * Patterns now allocated as one big chunk.
 *
 * Revision 4.12  1995/03/01  15:24:51  espie
 * Minor changes.
 *
 * Revision 4.11  1995/02/21  17:54:32  espie
 * Internal problem: buggy RCS. Fixed logs.
 *
 * Revision 4.8  1995/02/20  16:49:58  espie
 * Added song type (protracker or old st) for further checks.
 *
 * Revision 4.6  1995/02/06  14:50:47  espie
 * Changed sample_info.
 *
 * Revision 4.5  1995/02/01  17:14:54  espie
 * Scaled volume.
 *
 * Revision 4.0  1994/01/11  17:55:59  espie
 * REAL_MAX_PITCH for better player.
 * REAL_MAX_PITCH != MAX_PITCH.
 * Added samples_start.
 *
 * Revision 2.5  1992/10/31  11:18:00  espie
 * New fields for optimized resampling.
 * Exchanged __ANSI__ to SIGNED #define.
 */

typedef signed char SAMPLE8;

#define MAX_NUMBER_SAMPLES 32
#define LAST_SAMPLE 31
#define ST_NUMBER_SAMPLES 15
#define PRO_NUMBER_SAMPLES 31

#define NORMAL_PLENGTH 64
#define NORMAL_NTRACKS 4
#define MAX_TRACKS 8
#define NUMBER_SAMPLES 32

#define BLOCK_LENGTH 64
#define NUMBER_TRACKS 4
#define NUMBER_PATTERNS 128

#define NUMBER_EFFECTS 40

/* some effects names */
#define EFF_ARPEGGIO    0
#define EFF_DOWN        1
#define EFF_UP          2
#define EFF_PORTA       3
#define EFF_VIBRATO     4
#define EFF_PORTASLIDE  5
#define EFF_VIBSLIDE    6
#define EFF_TREMOLO		7
#define EFF_OFFSET      9
#define EFF_VOLSLIDE    10
#define EFF_FF          11
#define EFF_VOLUME      12
#define EFF_SKIP        13
#define EFF_EXTENDED    14
#define EFF_SPEED       15
#define EFF_NONE        16
#define EFF_OLD_SPEED	17
#define EXT_BASE        18
#define EFF_SMOOTH_DOWN   (EXT_BASE + 1)
#define EFF_SMOOTH_UP   (EXT_BASE + 2)
#define EFF_GLISS_CTRL	(EXT_BASE + 3)
#define EFF_VIBRATO_WAVE	(EXT_BASE + 4)
#define EFF_CHG_FTUNE   (EXT_BASE + 5)
#define EFF_LOOP        (EXT_BASE + 6)
#define EFF_TREMOLO_WAVE	(EXT_BASE + 7)
#define EFF_RETRIG      (EXT_BASE + 9)
#define EFF_S_UPVOL     (EXT_BASE + 10)
#define EFF_S_DOWNVOL   (EXT_BASE + 11)
#define EFF_NOTECUT     (EXT_BASE + 12)
#define EFF_LATESTART   (EXT_BASE + 13)
#define EFF_DELAY       (EXT_BASE + 14)
#define EFF_INVERT_LOOP	(EXT_BASE + 15)

#define SAMPLENAME_MAXLENGTH 22
#define TITLE_MAXLENGTH 20

#define MIN_PITCH 113
#define MAX_PITCH 856
#define REAL_MAX_PITCH 1050

#define MIN_VOLUME 0
#define MAX_VOLUME 64

/* the fuzz in note pitch */
#define FUZZ 5

/* we refuse to allocate more than 500000 bytes for one sample */
#define MAX_SAMPLE_LENGTH 500000

struct sample_info
   {
   SAMPLE8 *start, *rp_start;
   unsigned long  fix_length, fix_rp_length;

   unsigned int volume;
	unsigned short volume_lookup[MAX_VOLUME+1];
   short finetune;
	unsigned short color;
	unsigned short sample_size;			/* 8 or 16 */

   char *name;
   unsigned long  length, rp_offset, rp_length;
   };

/* the actual parameters may be split in two halves occasionally */

#define LOW(para) ((para) & 15)
#define HI(para) ((para) >> 4)

struct event
   {
   unsigned char sample_number;
   unsigned char effect;
   unsigned char parameters;
   unsigned char note;
   };

struct pattern
	{
	struct event *e;
	unsigned long duration;
	unsigned long total;
	unsigned int number;
	};

        
struct song_info
   {
   unsigned int length;
   unsigned int npat;
	unsigned int plength;
	unsigned long duration;
   unsigned char patnumber[NUMBER_PATTERNS];
	struct pattern *patterns;
	struct event *data;
   };

#define OLD_ST 0
#define PROTRACKER 1

struct song
   {
   char *title;
      /* sample 0 is always a dummy sample */
   struct sample_info *samples[MAX_NUMBER_SAMPLES];
	int type;
	unsigned int ntracks;
	unsigned int ninstr;
	int side_width;
	int max_sample_width;			
   struct song_info info;
   long samples_start;
   };

#define AMIGA_CLOCKFREQ 3575872
