/* usage.c */
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


#include <utility>
#include <iostream>
#include "ui.h"
#include "usage.h"
#include "version.h"

const char *usage[] = {
"Usage: tracker [options] filename [...]",
"-help               Display usage information",
"-picky              Do not tolerate any faults (default is to ignore most)",
"-tolerant           Ignore all faults",
"-mono               Select single audio channel output",
"-stereo             Select dual audio channel output",
"-verbose            Show text representation of song",
"-repeats <count>    Number of repeats (0 is forever) (default 1)",
"-loop               Loops the song list (plays again and again)",
"-speed <speed>      Song speed.  Some songs want 60 (default 50)",
"-speedmode normal|finespeed|speed|old|vblank|alter",
"-mix <percent>      Percent of channel mixing. (0 = spatial, 100 = mono)",
"-new -old -both     Select default reading type (default is -both)",
"-frequency <freq>   Set playback frequency in KHz",
"-oversample <times> Set oversampling factor",
"-transpose <n>      Transpose all notes up",
"-scroll             Show what's going on",
"-color              Ansi color scrolling",
"-randomize          randomize play order",
"",
"RunTime:",
"e,x     exit program",
"n       next song",
"p       restart/previous song",
">       fast forward",
"<       rewind",
"S       NTSC tempo\t s\tPAL tempo"
};

void 
print_usage()
{
	Info handle("Usage");
	handle << "This is tracker " << VERSION << "\n";
	for (auto s: usage)
		handle << s << "\n";
}
