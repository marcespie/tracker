/* autoinit.h */
/* used for decentralizing initialization/termination of various
 * system routines
 */

/* end_all(s): the program must exit now, after displaying s to the user, 
 * usually through notice and calling all stacked at_end() functions. 
 * s may be 0 for normal exit. DO NOT use exit() anywhere in tracker 
 * but end_all() instead.
 */
extern void end_all(const char *fmt, ...);

/* at_end(cleanup): stack cleanup to be called at program's termination
 */
extern void at_end(void (*cleanup)(void));

/* INIT_ONCE: macro for autoinitialization of routines.
 * modules that need an init routine should static void INIT = init_routine,
 * and add INIT_ONCE; at EVERY possible first entry point for their routine.
 * (I mean every, don't try to second-guess me !)
 */
#define INIT_ONCE	if (INIT){void (*func)(void) = INIT; INIT = 0; (*func)();}

// a template class that calls a function (object) on start
template<class T>
class init {
public:
	init(T f)
	{
		f();
	}
};
