/* play_list.h 
	vi:ts=3 sw=3:
 */

/* $Id: play_list.h,v 5.0 1995/10/21 14:56:49 espie Exp espie $
 * $Log: play_list.h,v $
 * Revision 5.0  1995/10/21 14:56:49  espie
 * New
 *
 * Revision 1.1  1995/07/02 17:52:40  espie
 * Initial revision
 *
 */

#define UNKNOWN 42

typedef struct play_entry
	{
	char *filename;
	int filetype;
	char name[1];
	} *ENTRY;

