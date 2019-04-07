/* autoinit.c */
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

#include <iostream>

#include "autoinit.h"

void 
at_end(void (*cleanup)(void))
{
	atexit(cleanup);
}
	
End::End(): errored{false}
{
}

template<typename T>
End&& operator<<(End&& o, T t)
{
	o.set_error();
	std::cerr << t;
	return std::move(o);
}

template
End&& operator<<(End&&, const char *);

template
End&& operator<<(End&&, char *);

template
End&& operator<<(End&&, int);

End::~End()
{
	if (errored) {
		std::cerr << "\n";
		exit(EXIT_FAILURE);
	} else 
		exit(EXIT_SUCCESS);
}
