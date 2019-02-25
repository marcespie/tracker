/* prefs.h 
	vi:ts=3 sw=3:
 */

/* $Id: prefs.h,v 5.0 1995/10/21 14:56:57 espie Exp espie $
 * $Log: prefs.h,v $
 * Revision 5.0  1995/10/21 14:56:57  espie
 * New
 *
 * Revision 4.17  1995/09/17 23:27:24  espie
 * *** empty log message ***
 *
 * Revision 4.16  1995/09/16 15:33:54  espie
 * *** empty log message ***
 *
 * Revision 4.15  1995/08/31 13:31:09  espie
 * NO_OUTPUT option.
 *
 * Revision 4.14  1995/05/11 12:26:15  espie
 * Corrected types.
 *
 * Revision 4.13  1995/03/17  00:32:41  espie
 * PREF_XTERM.
 *
 * Revision 4.12  1995/02/21  21:13:16  espie
 * Cleaned up source. Moved minor pieces of code around.
 *
 * Revision 4.11  1995/02/21  17:54:32  espie
 * Internal problem: buggy RCS. Fixed logs.
 *
 * Revision 4.6  1995/02/01  20:41:45  espie
 * Added color.
 *
 * Revision 4.2  1994/08/23  18:19:46  espie
 * Added speedmode option.
 */

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

XT VALUE get_pref P((int index));
XT void set_pref P((int index, VALUE value));
XT long get_pref_scalar P((int index));
XT void set_pref_scalar P((int index, long value));
XT struct tag *get_prefs P((void));
