/* prefs.c */
#include "defs.h"
#include "extern.h"
#include "prefs.h"
#include "tags.h"
#include "autoinit.h"

static void init_prefs (void);

static void (*INIT)(void) = init_prefs;

static struct tag preferences[NUMBER_PREFS];

static void init_prefs(void)
   {
   int i;
   
   for (i = 0; i < NUMBER_PREFS; i++)
      preferences[i].type = BASE_PREFS + i;
   }



VALUE get_pref(int index)
   {
   INIT_ONCE;

   return preferences[index-BASE_PREFS].data;
   }

void set_pref(int index, VALUE value)
   {
   preferences[index-BASE_PREFS].data = value;
   }

void set_pref_scalar(int index, long value)
   {
   VALUE temp;
   
   temp.scalar = value;
   set_pref(index, temp);
   }

long get_pref_scalar(int index)
   {
   return get_pref(index).scalar;
   }

struct tag *get_prefs(void)
   {
   INIT_ONCE;

   return preferences;
   }
