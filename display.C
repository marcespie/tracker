/* display.c */

#include "defs.h"
#include "song.h"
#include "notes.h"
#include "channel.h"
#include "extern.h"
#include "tags.h"
#include "prefs.h"
#include "autoinit.h"
#include "automaton.h"
#include "empty.h"

#include <assert.h>
LOCAL void init_display(void);
LOCAL void (*INIT)(void) = init_display;
     
LOCAL char *base;

/* lookup tables for speed */
LOCAL char *num[] = {
" 0", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9",
"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
"30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
"40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
"50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
"60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
"70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
"80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
"90", "91", "92", "93", "94", "95", "96", "97", "98", "99",
"00", "01", "02", "03", "04", "05", "06", "07", "08", "09"};

LOCAL char empty[]="                       ";
char instname[] = { ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

LOCAL void color(unsigned int c)
	{
	if (get_pref_scalar(PREF_COLOR))
		base = write_color(base, c);
	}

/* utility functions to avoid the overhead of strncpy */
/*
LOCAL void copy1(char *from)
   {
	*base++ = *from;
   }
*/
   
LOCAL void copy2(char *from)
   {
   *base++ = *from++;
   *base++ = *from;
   }

LOCAL void copy3(char *from)
   {
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from;
   }

LOCAL void copy4(char *from)
   {
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from;
   }

LOCAL void copy5(char *from)
   {
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from;
   }

LOCAL void copy6(char *from)
   {
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from;
   }

LOCAL void copy7(char *from)
   {
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from;
   }

LOCAL void copy8(char *from)
   {
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from;
   }

LOCAL void copy11(char *from)
	{
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from++;
   *base++ = *from;
	}

LOCAL void stringcopy(char *from)
   {
   while (*from)
      *base++ = *from++;
   }

LOCAL void num2(unsigned int n)
   {
	assert(n < 110);
		{
		char *v = num[n];
		*base++ = *v++;
		*base++ = *v;
		}
   }

LOCAL void num3(unsigned int n)
   {
   char *v;

	assert(n < 1000);
   if (n >= 100)
      *base++ = "0123456789"[n/100];
	else
		*base++ = ' ';
   while (n > 109)
      n -= 100;
   v = num[n];
   *base++ = *v++;
   *base++ = *v;
   }

typedef void (*disp_function)(unsigned samp, unsigned para, note note, 
	struct channel *ch);
LOCAL disp_function table[NUMBER_EFFECTS];

LOCAL int debug;

LOCAL void disp_note_name(struct channel *ch, note note)
	{
	if (ch->samp->start)
		{
		copy3(note2name(note));
		*base++ = ' ';
		}
	else
		copy4(empty);
	}

/* all the various dump for the effects */
LOCAL void disp_default(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	copy7(empty);
	color(0);
   }

LOCAL void disp_nothing(unsigned samp, unsigned para, note note, struct channel *ch)
	{
	disp_note_name(ch, note);
	color(0);
	num3(debug);
	copy4("!!!!");
	}

LOCAL void disp_speed(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	color(0);
   if (para < 32)
      {
		copy5("SPD  ");
      num2(para);
      }
   else
      {
		copy4("spd%");
      num3(para * 100/NORMAL_FINESPEED);
      }
   }


LOCAL void disp_old_speed(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	color(0);
	copy5("SPD  ");
	num2(para);
   }


LOCAL void disp_portamento(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3("-->");
		copy3(note2name(note));
		if (para)
			{
			*base++ = '(';
			num3(para);
			*base++ = ')';
			}
		else
			copy5(empty);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_portaslide(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3("-->");
		copy3(note2name(note));
		if (LOW(para))
			{
			copy2(" -");
			num2(LOW(para));
			}
		else
			{
			copy2(" +");
			num2(HI(para));
			}
		*base++ = ' ';
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_upslide(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5("    -");
		if (para)
			num3(para);
		else
			copy3(empty);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_downslide(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5("    +");
		if (para)
			num3(para);
		else
			copy3(empty);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_vibrato(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	if (para || ch->samp->start)
		copy2("vb");
	else
		copy2(empty);
	if (para)
		{
		num2(LOW(para));
		*base++ = '/';
		num2(HI(para));
		}
	else
		copy5(empty);
	color(0);
	}

LOCAL void disp_tremolo(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	if (para || ch->samp->start)
		copy2("tr");
	else
		copy2(empty);
	if (para)
		{
		num2(LOW(para));
		*base++ = '/';
		num2(HI(para));
		}
	else
		copy5(empty);
	color(0);
	}

LOCAL void disp_arpeggio(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		*base++=' ';
		if (note != NO_NOTE)
			{
			copy3(note2name(note + LOW(para)));
			*base++=' ';
			copy3(note2name(note + HI(para)));
			}
		else
			if (ch->note == NO_NOTE)
				stringcopy("Arp err ");
			else
				{
				copy3(note2name(ch->note + LOW(para)));
				*base++=' ';
				copy3(note2name(ch->note + HI(para)));
				}  
		}
	else
		copy11(empty);
	color(0);
   }


LOCAL void disp_volume(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		if (para)
			{
			copy5(" vol ");
			num3(para);
			}
		else
			copy8(" silent ");
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_slidevol(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5(" vol ");
		if (LOW(para))
			{
			*base++ = '-';
			num2(LOW(para));
			}
		else if (HI(para))
			{
			*base++ = '+';
			num2(HI(para));
			}
		else
			copy3(empty);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_smooth_upvolume(unsigned samp, unsigned para, note note, 
	struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5("   ++");
		num3(para);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_smooth_downvolume(unsigned samp, unsigned para, note note, 
	struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5("   --");
		num3(para);
		}
	else
		copy11(empty);
	color(0);
   }


LOCAL void disp_late_start(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5(" lte ");
		num3(para);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_retrig(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5(" rtg ");
		num3(para);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_note_cut(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5(" cut ");
		num3(para);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_offset(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy4(" off");
		if (ch->samp->length)
			{
			int percent;
			percent = para * 25600/ch->samp->length;
			if (percent <= 105)
				num3(percent);
			else
				copy3("???");
			}
		else
			copy3(empty);
		*base++ = '%';
		}
	else
		copy11(empty);
	color(0);
   }


LOCAL void disp_skip(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	color(0);
   if (para)
      {
		copy4("skp ");
      num3(para);
      }
   else
		copy7("next   ");
   }

LOCAL void disp_loop(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	color(0);
   if (para == 0)
      copy7("SETLOOP");
   else
      {
		copy4("LOOP");
      num3(para+1);
      }
   }

LOCAL void disp_vibratoslide(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5(" vibs");
		if (LOW(para))
			{
			*base++='-';
			num2(LOW(para));
			}
		else
			{
			*base++ = '+';
			num2(HI(para));
			}
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_delay_pattern(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	color(0);
   copy4("DLAY");
   num3(para);
   }


LOCAL void disp_smooth_up(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5(" sth-");
		num3(para);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_smooth_down(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy5(" sth+");
		num3(para);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_fastskip(unsigned samp, unsigned para, note note, struct channel *ch)
   {
	disp_note_name(ch, note);
	color(0);
   copy4(" ff ");
   num3(para);
   }

LOCAL void disp_invert_loop(unsigned samp, unsigned para, note note, struct channel *ch)
	{
	disp_note_name(ch, note);
	if (para)
		{
		copy4("inv ");
      num3(para);
		}
	else
		copy7("inv off");
	color(0);
	}

LOCAL void disp_change_finetune(unsigned samp, unsigned para, note note, 
	struct channel *ch)
   {
	if (ch->samp->start)
		{
		copy3(note2name(note));
		copy6(" fine ");
		num2(para);
		}
	else
		copy11(empty);
	color(0);
   }

LOCAL void disp_vibrato_wave(unsigned samp, unsigned para, note note, struct channel *ch)
	{
	disp_note_name(ch, note);
	copy3("vb ");
	switch(para)
		{
	case 0:
		copy4("sine");
		break;
	case 1:
		copy4("sqre");
		break;
	case 2:
		copy4("ramp");
		break;
	case 4:
		copy4("SINE");
		break;
	case 5:
		copy4("SQRE");
		break;
	case 6:
		copy4("RAMP");
		break;
	default:
		copy4("????");
		}
	color(0);
	}

LOCAL void disp_tremolo_wave(unsigned samp, unsigned para, note note, struct channel *ch)
	{
	disp_note_name(ch, note);
	copy3("tr ");
	switch(para)
		{
	case 0:
		copy4("sine");
		break;
	case 1:
		copy4("sqre");
		break;
	case 2:
		copy4("ramp");
		break;
	case 4:
		copy4("SINE");
		break;
	case 5:
		copy4("SQRE");
		break;
	case 6:
		copy4("RAMP");
		break;
	default:
		copy4("????");
		}
	color(0);
	}


LOCAL void disp_gliss_ctrl(unsigned samp, unsigned para, note note, struct channel *ch)
	{
	disp_note_name(ch, note);
	if (para)
		copy6("gls on");
	else
		copy6("gls off");
	}

LOCAL void init_display(void)
   {
   unsigned int i;

   for (i = 0; i < NUMBER_EFFECTS; i++)
      table[i] = disp_nothing;
	table[EXT_BASE] = disp_default;
   table[EFF_SPEED] = disp_speed;
   table[EFF_OLD_SPEED] = disp_old_speed;
   table[EFF_PORTA] = disp_portamento;
   table[EFF_PORTASLIDE] = disp_portaslide;
   table[EFF_VIBRATO] = disp_vibrato;
	table[EFF_TREMOLO] = disp_tremolo;
   table[EFF_UP] = disp_upslide;
   table[EFF_DOWN] = disp_downslide;
   table[EFF_ARPEGGIO] = disp_arpeggio;
	table[EFF_NONE] = disp_default;
   table[EFF_VOLUME] = disp_volume;
   table[EFF_VOLSLIDE] = disp_slidevol;
   table[EFF_OFFSET] = disp_offset;
   table[EFF_LATESTART] = disp_late_start;
   table[EFF_S_UPVOL] = disp_smooth_upvolume;
   table[EFF_S_DOWNVOL] = disp_smooth_downvolume;
   table[EFF_SKIP] = disp_skip;
   table[EFF_LOOP] = disp_loop;
   table[EFF_VIBSLIDE] = disp_vibratoslide;
   table[EFF_RETRIG] = disp_retrig;
   table[EFF_NOTECUT] = disp_note_cut;
   table[EFF_DELAY] = disp_delay_pattern;
   table[EFF_SMOOTH_UP] = disp_smooth_up;
   table[EFF_SMOOTH_DOWN] = disp_smooth_down;
   table[EFF_FF] = disp_fastskip;
	table[EFF_INVERT_LOOP] = disp_invert_loop;
   table[EFF_CHG_FTUNE] = disp_change_finetune;
	table[EFF_VIBRATO_WAVE] = disp_vibrato_wave;
	table[EFF_TREMOLO_WAVE] = disp_tremolo_wave;
	table[EFF_GLISS_CTRL] = disp_gliss_ctrl;
   }

void dump_event(struct channel *ch, struct event *e)
   {
   INIT_ONCE;
   
	if (ch)	/* do we have a scroll line AND are we not finished */
		{
		if (!base)
			base = new_scroll();
		if (base)
			{
			color(ch->samp->color);
			if (ch->samp != empty_sample())
				*base++ = instname[e->sample_number];
			else 
				*base++ = ' ';
			*base++ = ' ';
			debug = e->effect;
			(*table[e->effect])(e->sample_number, e->parameters, e->note, ch);
			}
		}
	else
		{
		*base = 0;
		scroll(base);
		base = 0;
		}
	}

void dump_delimiter(void)
	{
	INIT_ONCE;

	if (base)
#ifdef AMIGA
		*base++ = ' ';
#else
		*base++= '|';
#endif
	}
