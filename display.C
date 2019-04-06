/* display.c */
/*
 * Copyright (c) 2019 Marc Espie <espie@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "extern.h"
#include "protracker.h"
#include "notes.h"
#include <memory>
#include "channel.h"
#include "prefs.h"
#include "automaton.h"
#include "empty.h"
#include "autoinit.h"

#include <assert.h>
static void init_display(void);

static init z(init_display);

static char *base;

/* lookup tables for speed */
static const char *num[] = {
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

static const char empty[]="                       ";
char instname[] = { ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

static void 
color(unsigned int c)
{
	if (pref::get(Pref::color))
		base = write_color(base, c);
}

template<int N>
void 
copy(const char *from)
{
	*base++ = *from++;
	copy<N-1>(from);
}

template<>
void
copy<1>(const char *from)
{
	*base++ = *from;
}


static void 
stringcopy(const char *from)
{
	while (*from)
		*base++ = *from++;
}

static void 
num2(unsigned int n)
{
	assert(n < 110);

	const char *v = num[n];
	*base++ = *v++;
	*base++ = *v;
}

static void 
num3(unsigned int n)
{
	assert(n < 1000);
	if (n >= 100)
		*base++ = "0123456789"[n/100];
	else
		*base++ = ' ';
	while (n > 109)
		n -= 100;
	const char *v = num[n];
	*base++ = *v++;
	*base++ = *v;
}

using disp_function = void (*)(const channel& ch, const event& e);
static disp_function table[NUMBER_EFFECTS];

static int debug;

static void 
disp_note_name(const channel& ch, note note)
	{
	if (ch.samp->start)
		{
		copy<3>(note2name(note));
		*base++ = ' ';
		}
	else
		copy<4>(empty);
	}

/* all the various dump for the effects */
static void 
disp_default(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	copy<7>(empty);
	color(0);
}

static void 
disp_nothing(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	color(0);
	num3(debug);
	copy<4>("!!!!");
}

static void 
disp_speed(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	color(0);
	if (e.parameters < 32) {
		copy<5>("SPD  ");
		num2(e.parameters);
	} else {
		copy<4>("spd%");
		num3(e.parameters * 100/NORMAL_FINESPEED);
	}
}


static void 
disp_old_speed(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	color(0);
	copy<5>("SPD  ");
	num2(e.parameters);
}


static void 
disp_portamento(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>("-->");
		copy<3>(note2name(e.note));
		if (e.parameters) {
			*base++ = '(';
			num3(e.parameters);
			*base++ = ')';
		} else
			copy<5>(empty);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_portaslide(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>("-->");
		copy<3>(note2name(e.note));
		if (e.low()) {
			copy<2>(" -");
			num2(e.low());
		} else {
			copy<2>(" +");
			num2(e.high());
		}
		*base++ = ' ';
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_upslide(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>("    -");
		if (e.parameters)
			num3(e.parameters);
		else
			copy<3>(empty);
	}
	else
		copy<11>(empty);
	color(0);
}

static void 
disp_downslide(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>("    +");
		if (e.parameters)
			num3(e.parameters);
		else
			copy<3>(empty);
	}
	else
		copy<11>(empty);
	color(0);
}

static void 
disp_vibrato(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	if (e.parameters || ch.samp->start)
		copy<2>("vb");
	else
		copy<2>(empty);
	if (e.parameters) {
		num2(e.low());
		*base++ = '/';
		num2(e.high());
	}
	else
		copy<5>(empty);
	color(0);
}

static void 
disp_tremolo(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	if (e.parameters || ch.samp->start)
		copy<2>("tr");
	else
		copy<2>(empty);
	if (e.parameters) {
		num2(e.low());
		*base++ = '/';
		num2(e.high());
	}
	else
		copy<5>(empty);
	color(0);
}

static void 
disp_arpeggio(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		*base++=' ';
		if (e.note != NO_NOTE) {
			copy<3>(note2name(e.note + e.low()));
			*base++=' ';
			copy<3>(note2name(e.note + e.high()));
		} else if (ch.note == NO_NOTE)
			stringcopy("Arp err ");
		else {
			copy<3>(note2name(ch.note + e.low()));
			*base++=' ';
			copy<3>(note2name(ch.note + e.high()));
		}  
	} else
		copy<11>(empty);
	color(0);
}


static void 
disp_volume(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		if (e.parameters) {
			copy<5>(" vol ");
			num3(e.parameters);
		} else
			copy<8>(" silent ");
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_slidevol(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>(" vol ");
		if (e.low()) {
			*base++ = '-';
			num2(e.low());
		} else if (e.high()) {
			*base++ = '+';
			num2(e.high());
		} else
			copy<3>(empty);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_smooth_upvolume(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>("   ++");
		num3(e.parameters);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_smooth_downvolume(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>("   --");
		num3(e.parameters);
	}
	else
		copy<11>(empty);
	color(0);
}


static void 
disp_late_start(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>(" lte ");
		num3(e.parameters);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_retrig(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>(" rtg ");
		num3(e.parameters);
	}
	else
		copy<11>(empty);
	color(0);
}

static void 
disp_note_cut(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>(" cut ");
		num3(e.parameters);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_offset(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<4>(" off");
		if (ch.samp->length) {
			int percent = e.parameters * 25600/ch.samp->length;
			if (percent <= 105)
				num3(percent);
			else
				copy<3>("???");
		} else
			copy<3>(empty);
		*base++ = '%';
	} else
		copy<11>(empty);
	color(0);
}


static void 
disp_skip(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	color(0);
	if (e.parameters) {
		copy<4>("skp ");
		num3(e.parameters);
	}
	else
		copy<7>("next   ");
}

static void 
disp_loop(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	color(0);
	if (e.parameters == 0)
		copy<7>("SETLOOP");
	else {
		copy<4>("LOOP");
		num3(e.parameters+1);
	}
}

static void 
disp_vibratoslide(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>(" vibs");
		if (e.low()) {
			*base++='-';
			num2(e.low());
		} else {
			*base++ = '+';
			num2(e.high());
		}
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_delay_pattern(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	color(0);
	copy<4>("DLAY");
	num3(e.parameters);
}


static void 
disp_smooth_up(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>(" sth-");
		num3(e.parameters);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_smooth_down(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<5>(" sth+");
		num3(e.parameters);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_fastskip(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	color(0);
	copy<4>(" ff ");
	num3(e.parameters);
}

static void 
disp_invert_loop(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	if (e.parameters) {
		copy<4>("inv ");
		num3(e.parameters);
	} else
		copy<7>("inv off");
	color(0);
}

static void 
disp_change_finetune(const channel& ch, const event& e)
{
	if (ch.samp->start) {
		copy<3>(note2name(e.note));
		copy<6>(" fine ");
		num2(e.parameters);
	} else
		copy<11>(empty);
	color(0);
}

static void 
disp_vibrato_wave(const channel& ch, const event& e)
	{
	disp_note_name(ch, e.note);
	copy<3>("vb ");
	switch(e.parameters) {
	case 0:
		copy<4>("sine");
		break;
	case 1:
		copy<4>("sqre");
		break;
	case 2:
		copy<4>("ramp");
		break;
	case 4:
		copy<4>("SINE");
		break;
	case 5:
		copy<4>("SQRE");
		break;
	case 6:
		copy<4>("RAMP");
		break;
	default:
		copy<4>("????");
	}
	color(0);
}

static void 
disp_tremolo_wave(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	copy<3>("tr ");
	switch(e.parameters) {
	case 0:
		copy<4>("sine");
		break;
	case 1:
		copy<4>("sqre");
		break;
	case 2:
		copy<4>("ramp");
		break;
	case 4:
		copy<4>("SINE");
		break;
	case 5:
		copy<4>("SQRE");
		break;
	case 6:
		copy<4>("RAMP");
		break;
	default:
		copy<4>("????");
	}
	color(0);
}


static void 
disp_gliss_ctrl(const channel& ch, const event& e)
{
	disp_note_name(ch, e.note);
	if (e.parameters)
		copy<6>("gls on");
	else
		copy<6>("gls off");
}

static void 
init_display(void)
{
	for (auto& x: table)
		x = disp_nothing;
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

void 
dump_event(const channel& ch, const event *e)
{
	if (!base)
		base = new_scroll();
	if (base) {
		color(ch.samp->color);
		if (ch.samp != empty_sample())
			*base++ = instname[e->sample_number];
		else 
			*base++ = ' ';
		*base++ = ' ';
		debug = e->effect;
		(*table[e->effect])(ch, *e);
	}
}

void
dump_event()
{
	*base = 0;
	scroll(base);
	base = 0;
}

void 
dump_delimiter(void)
{
	if (base)
		*base++= '|';
}
