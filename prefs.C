/* prefs.c */
#include "defs.h"
#include "extern.h"
#include "prefs.h"
#include "autoinit.h"

static long preferences[static_cast<size_t>(Pref::max_prefs)];

void 
set_pref(Pref index, long value)
{
	preferences[static_cast<size_t>(index)] = value;
}

long 
get_pref(Pref index)
{
   	return preferences[static_cast<size_t>(index)];
}
