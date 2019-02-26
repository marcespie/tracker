/* watched_var.c */

#include "defs.h"
#include <assert.h>
#include <list>
#include <functional>
#include "watched_var.h"
#include "autoinit.h"

LOCAL struct {
	long value;
	bool set;
	std::list<notify_function> l;
} variable[NUMBER_WATCHED];

LOCAL void 
notify_new(enum watched_var var)
{
	variable[var].set = true;

	for (const auto& f: variable[var].l)
		f(var, variable[var].value);
}

void 
set_watched_scalar(enum watched_var var, long new_val)
{
	assert(var < NUMBER_WATCHED);
	if (!variable[var].set || variable[var].value != new_val) {
		/* first set the variable to avoid looping */
		variable[var].value = new_val;
		notify_new(var);
	}
}

long
get_watched_scalar(enum watched_var var)
{
	assert(var < NUMBER_WATCHED);
	return variable[var].value;
}


void 
add_notify(notify_function f, enum watched_var var)
{
	assert(var < NUMBER_WATCHED);

	variable[var].l.push_back(f);
	if (variable[var].set)
		f(var, variable[var].value);
}
