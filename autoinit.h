/* autoinit.h
   vi:ts=3 sw=3
 */

/* $Id: autoinit.h,v 1.2 1996/05/06 14:28:41 espie Exp espie $
 * $Log: autoinit.h,v $
 * Revision 1.2  1996/05/06 14:28:41  espie
 * *** empty log message ***
 *
 * Revision 1.1  1996/04/09 21:13:22  espie
 * Initial revision
 *
 */
/* used for decentralizing initialization/termination of various
 * system routines
 */

/* end_all(s): the program must exit now, after displaying s to the user, 
 * usually through notice and calling all stacked at_end() functions. 
 * s may be 0 for normal exit. DO NOT use exit() anywhere in tracker 
 * but end_all() instead.
 */
XT /*@exits@*/ void end_all(char *fmt, ...);

/* at_end(cleanup): stack cleanup to be called at program's termination
 */
XT void at_end(void (*cleanup)(void));

/* INIT_ONCE: macro for autoinitialization of routines.
 * modules that need an init routine should LOCAL void INIT = init_routine,
 * and add INIT_ONCE; at EVERY possible first entry point for their routine.
 * (I mean every, don't try to second-guess me !)
 */
#define INIT_ONCE	if (INIT){void (*func)(void) = INIT; INIT = 0; (*func)();}


