/* watched_var.h
   vi:ts=3 sw=3:
 */

/* global values that propagates changes */

/* $Id: watched_var.h,v 1.2 1996/05/07 15:22:21 espie Exp espie $
 * $Log: watched_var.h,v $
 * Revision 1.2  1996/05/07 15:22:21  espie
 * First use of watched_var protocol (oversample/frequency)
 *
 * Revision 1.1  1996/04/12 16:31:15  espie
 * Initial revision
 * 
 */

/* tracker is firmly based upon some parameters, like the resampling
 * frequency. Each time such a parameter changes, loads of modules need
 * to know about it. There actually is no natural module to put that
 * parameter in.
 *
 * For programming in the large, this can get very awkward, as each time
 * you modify such a parameter, you have to tell loads of modules about it.
 * The alternative is to define one module per such parameter, which will
 * tell every module that needs it about the parameter.
 *
 * This is not very satisfying, as it forces each module to export a new
 * function for each such parameter.
 *
 * watched variables reverse the problem. Each module can register 
 * independently, and will get notified each time the parameter changes.
 *
 * The most important gain is that modules no longer need to export new
 * functions, they just have to privately tell the watched variable they
 * need notification.
 *
 * Also this module takes care of insightly details, such as correct
 * initialization for of a given variable, or possible loops in setting
 * Specifically, the module guarantees that each client will be notified
 * at least once for a given set variable---either when asking for the
 * notification, or when the variable gets set later, and that the client
 * will be notified just once in case of spurious multiple sets with the 
 * same value.
 */

#include <functional>

enum watched_var {
	OVERSAMPLE, 
	FREQUENCY,
	NUMBER_WATCHED
};

using notify_function = std::function<void(enum watched_var, long)>;
/* set_watched_xxx(var, new_val):
 * set variable var to its new val, and notifies applicable clients
 */
extern void set_watched_scalar(enum watched_var var, long new_val);

/* get_watched_xxx(var):
 * get the value of var
 */
extern long get_watched_scalar(enum watched_var var);

/* add_notify(f, var, context):
 * add a notify function to var. This function will be called as
 * f(var, value, context) each time the variable is set to a new value.
 * The function f must be ready to be called at add_notify() time.
 */
extern void add_notify(notify_function f, enum watched_var var);


