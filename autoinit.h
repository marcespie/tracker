/* autoinit.h */
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
/* used for decentralizing initialization/termination of various
 * system routines
 */

class End {
	bool errored;
public:
	End();
	~End();
	void set_error()
	{
		errored = true;
	}
};

template<typename T>
End&& operator<<(End&& o, T t);

/* at_end(cleanup): stack cleanup to be called at program's termination
 */
extern void at_end(void (*cleanup)(void));

/* INIT_ONCE: macro for autoinitialization of routines.
 * modules that need an init routine should static void INIT = init_routine,
 * and add INIT_ONCE; at EVERY possible first entry point for their routine.
 * (I mean every, don't try to second-guess me !)
 */
#define INIT_ONCE	if (INIT){void (*func)(void) = INIT; INIT = 0; (*func)();}

// a template class that calls a function (object) on start
template<class T>
class init {
public:
	init(T f)
	{
		f();
	}
};
