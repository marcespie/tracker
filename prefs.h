/* prefs.h */

#define BASE_PREFS      50
#define PREF_TYPE       BASE_PREFS
#define PREF_SPEED      (BASE_PREFS+1)
#define PREF_TOLERATE   (BASE_PREFS+2)
#define PREF_REPEATS    (BASE_PREFS+3)
#define PREF_IMASK      (BASE_PREFS+4)
#define PREF_BCDVOL     (BASE_PREFS+5)
#define PREF_DUMP       (BASE_PREFS+6)

#define PREF_SHOW       (BASE_PREFS+8)
#define PREF_SPEEDMODE 	(BASE_PREFS+9)
#define PREF_COLOR		(BASE_PREFS+10)
#define PREF_XTERM		(BASE_PREFS+11)
#define PREF_OUTPUT		(BASE_PREFS+12)
#define PREF_TRANSPOSE	(BASE_PREFS+13)

/* values for PREF_SPEEDMODE */
#define NORMAL_SPEEDMODE 0
#define FINESPEED_ONLY	1
#define SPEED_ONLY 2
#define OLD_SPEEDMODE 3
#define ALTER_PROTRACKER 4

#define NUMBER_PREFS    (PREF_TRANSPOSE - BASE_PREFS + 1)

extern VALUE get_pref (int index);
extern void set_pref (int index, VALUE value);
extern long get_pref_scalar (int index);
extern void set_pref_scalar (int index, long value);
extern struct tag *get_prefs (void);
