/* timing.c */

#include "defs.h"
#include "timing.h"
     
/* TIME_SCALE is one microsecond. Hence an unsigned long is enough
 * for a little over one hour */
const auto TIME_SCALE=1000000;

char *
time2string(char *buffer, unsigned long t)
{
	t += TIME_SCALE/2;
	t /= TIME_SCALE;
	int s = t % 60;
	t /= 60;
	sprintf(buffer, "%3d:%02d", int(t), s);
	return buffer;
}

unsigned long 
ratio2time(int n, int p)
{
	return (unsigned long)n * TIME_SCALE / (unsigned long)p;
}
