/* timing.h
	vi:ts=3 sw=3:
 */

/* $Id: timing.h,v 1.1 1996/04/12 16:31:33 espie Exp espie $
 * $Log: timing.h,v $
 * Revision 1.1  1996/04/12 16:31:33  espie
 * Initial revision
 *
 */


/* convert a time value to a string using a local buffer */
XT char *time2string(char *buffer, unsigned long t);

/* convert a ratio n/p seconds to a time value */
XT unsigned long ratio2time(int n, int p);



