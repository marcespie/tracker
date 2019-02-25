/* autoinit.c */

/* emulates atexit functionality, but much more reliable */

#include "defs.h"
#include "extern.h"
#include "autoinit.h"

LOCAL struct clist
	{
	struct clist *next;
	void (*func)(void);
	} *list = 0;
	

void at_end(void (*cleanup)(void))
	{
#ifdef USE_AT_EXIT
	atexit(cleanup);
#else
	struct clist *new;

	new = (struct clist *)malloc(sizeof(struct clist));
	if (!new)
		{ /* don't forget to call the latest function ! */
		(*cleanup)();	
		end_all("Allocation problem");
		}
	new->next = list;
	new->func = cleanup;
	list = new;
#endif
	}
	
void end_all(char *fmt, ...)
	{
	va_list al;
#ifndef USE_AT_EXIT
	struct clist *p;
#endif
	if (fmt)
		{
		va_start(al, fmt);
		vnotice(fmt, al);
		va_end(al);
		}
#ifndef USE_AT_EXIT
	for (p = list; p; p = p->next)
		(p->func)();			/* don't bother freeing (malloc) */
#endif
	exit(fmt ? EXIT_FAILURE : EXIT_SUCCESS);
	}
