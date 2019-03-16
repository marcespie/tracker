/* usage.c */


#include "defs.h"
#include "extern.h"

extern char *VERSION;

const char *usage[] =
	{
"This program is NOT to be redistributed",
"         without the full documentation",
"",
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
"-mix <percent>      Percent of channel mixing. (0 = spatial, 100 = mono)",
"-new -old -both     Select default reading type (default is -both)",
"-frequency <freq>   Set playback frequency in KHz",
"-oversample <times> Set oversampling factor",
"-transpose <n>      Transpose all notes up",
"-scroll             Show what's going on",
"-color              Ansi color scrolling",
"-sync               Try to synch audio output with display",
"-randomize          randomize play order",
#ifdef VOLUME_CONTROL
"-speaker				 Output audio to internal speaker",
"-volume <n>         Set volume in dB",
#endif
"",
"RunTime:",
"e,x     exit program",
"n       next song",
"p       restart/previous song",
">       fast forward",
"<       rewind",
"S       NTSC tempo\t s\tPAL tempo",
0
	};

void print_usage()
{
	GENERIC handle;
	const char **s;

	handle = begin_info("Usage");
	infos(handle, "This is tracker ");
	info(handle, VERSION);
	for (s = usage; *s; s++)
		info(handle, *s);
	end_info(handle);
}
