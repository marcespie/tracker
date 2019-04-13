/* timing.C */
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

#include <stdio.h>
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
