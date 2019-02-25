/* parse_options.h 
	vi:ts=3 sw=3:
 */

/* $Id: parse_options.h,v 5.1 1996/05/06 22:48:28 espie Exp espie $
 * $Log: parse_options.h,v $
 * Revision 5.1  1996/05/06 22:48:28  espie
 * *** empty log message ***
 *
 * Revision 5.0  1995/10/21 14:56:36  espie
 * New
 *
 * Revision 1.2  1995/09/16 15:33:13  espie
 * *** empty log message ***
 *
 * Revision 1.1  1995/09/07 14:33:32  espie
 * Initial revision
 *
 */

struct option
	{
	char *optiontext;
	char type;
	unsigned long def_scalar;
	char *def_string;
	int multi;
	};

struct option_set
	{
	struct option *options;
	unsigned int number;
	VALUE *args;
	};

