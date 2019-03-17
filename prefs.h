/* prefs.h */

enum class Pref {type, speed, tolerate, repeats,
    imask, bcdvol, dump, show, speedmode, color,
    xterm, output, transpose, max_prefs };
/* values for PREF_SPEEDMODE */

enum { NORMAL_SPEEDMODE, FINESPEED_ONLY, SPEED_ONLY, OLD_SPEEDMODE,
    ALTER_PROTRACKER };

extern long get_pref(Pref index);
extern void set_pref(Pref index, long value);
