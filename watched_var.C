/* watched_var.c */
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

#include <unordered_set>
#include <functional>
#include "watched_var.h"
#include "autoinit.h"

static struct {
	long value;
	bool set;
	std::unordered_set<notify_function*> l;
} variable[static_cast<size_t>(watched::number_watched)];

static void 
notify_new(watched var)
{
	const auto idx = static_cast<size_t>(var);
	variable[idx].set = true;

	for (const auto f: variable[idx].l)
		(*f)(var, variable[idx].value);
}

void 
set_watched(watched var, long new_val)
{
	const auto idx = static_cast<size_t>(var);
	if (!variable[idx].set || variable[idx].value != new_val) {
		/* first set the variable to avoid looping */
		variable[idx].value = new_val;
		notify_new(var);
	}
}

long
get_watched(watched var)
{
	const auto idx = static_cast<size_t>(var);
	return variable[idx].value;
}


void 
add_notify(notify_function f, watched var)
{
	const auto idx = static_cast<size_t>(var);
	variable[idx].l.insert(&f);
	if (variable[idx].set)
		f(var, variable[idx].value);
}

void 
remove_notify(notify_function f, watched var)
{
	const auto idx = static_cast<size_t>(var);
	variable[idx].l.erase(&f);
}
