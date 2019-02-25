/* watched_var.c */

#include "defs.h"
#include <assert.h>
#include <list>
#include "watched_var.h"
#include "autoinit.h"

struct watcher {
	watcher *next;
	notify_function notify_change;
	void *context;
};

LOCAL struct {
	VALUE value;
	bool set;
	watcher *first;
} variable[NUMBER_WATCHED];

LOCAL void 
notify_new(enum watched_var var)
{
	variable[var].set = true;

	watcher *w = variable[var].first;
	while (w) {
		(w->notify_change)(var, variable[var].value, w->context);
		w = w->next;
	}
}

void 
set_watched_scalar(enum watched_var var, long new_val)
{
	assert(var < NUMBER_WATCHED);
	if (!variable[var].set || variable[var].value.scalar != new_val) {
		/* first set the variable to avoid looping */
		variable[var].value.scalar = new_val;
		notify_new(var);
	}
}

void 
set_watched_real(enum watched_var var, float new_val)
{
	assert(var < NUMBER_WATCHED);
	if (!variable[var].set || variable[var].value.real != new_val) {
		/* first set the variable to avoid looping */
		variable[var].value.real = new_val;
		notify_new(var);
	}
}

void 
set_watched_pointer(enum watched_var var, GENERIC new_val)
{
	assert(var < NUMBER_WATCHED);
	if (!variable[var].set || variable[var].value.pointer != new_val) {
		/* first set the variable to avoid looping */
		variable[var].value.pointer = new_val;
		notify_new(var);
	}
}

LOCAL VALUE 
get_watched(enum watched_var var)
{
	assert(var < NUMBER_WATCHED);
	return variable[var].value;
}

long 
get_watched_scalar(enum watched_var var)
{
	return get_watched(var).scalar;
}

float 
get_watched_real(enum watched_var var)
{
	return get_watched(var).real;
}

GENERIC 
get_watched_pointer(enum watched_var var)
{
	return get_watched(var).pointer;
}
	

void 
add_notify(notify_function f, enum watched_var var, void *context)
{
	assert(var < NUMBER_WATCHED);

	watcher* n = new watcher;

	n->next = variable[var].first;
	variable[var].first = n;
	n->notify_change = f;
	n->context = context;

	if (variable[var].set)
		(*f)(var, variable[var].value, context);
}
