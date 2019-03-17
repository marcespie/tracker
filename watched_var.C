/* watched_var.c */

#include "defs.h"
#include <list>
#include <functional>
#include "watched_var.h"
#include "autoinit.h"

static struct {
	long value;
	bool set;
	std::list<notify_function> l;
} variable[static_cast<size_t>(watched::number_watched)];

static void 
notify_new(watched var)
{
	const auto idx = static_cast<size_t>(var);
	variable[idx].set = true;

	for (const auto& f: variable[idx].l)
		f(var, variable[idx].value);
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
	variable[idx].l.push_back(f);
	if (variable[idx].set)
		f(var, variable[idx].value);
}
