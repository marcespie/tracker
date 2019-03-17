/* watched_var.h */
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

/* global values that propagates changes */

#include <functional>

enum class watched {
	oversample, 
	frequency,
	number_watched
};

using notify_function = std::function<void(watched, long)>;
/* set_watched_xxx(var, new_val):
 * set variable var to its new val, and notifies applicable clients
 */
extern void set_watched(watched var, long new_val);

/* get_watched_xxx(var):
 * get the value of var
 */
extern long get_watched(watched var);

/* add_notify(f, var, context):
 * add a notify function to var. This function will be called as
 * f(var, value, context) each time the variable is set to a new value.
 * The function f must be ready to be called at add_notify() time.
 */
extern void add_notify(notify_function f, watched var);


