/* autoinit.c */

#include "defs.h"
#include "extern.h"
#include "autoinit.h"

LOCAL struct clist {
	struct clist *next;
	void (*func)(void);
} *list = nullptr;
	

void 
at_end(void (*cleanup)(void))
{
	atexit(cleanup);
}
	
void 
end_all(const char *fmt, ...)
{
	va_list al;
	if (fmt) {
		va_start(al, fmt);
		vnotice(fmt, al);
		va_end(al);
	}
	exit(fmt ? EXIT_FAILURE : EXIT_SUCCESS);
}
