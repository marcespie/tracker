/* watched_var.h */

/* global values that propagates changes */

#include <functional>

enum class watched {
	oversample, 
	frequency,
	number_watched
};

using notify_function = std::function<void(watched, long)>;
/* set_watched_xxx(var, new_val):
 * set variable var to its new val, and notifies applicable clients
 */
extern void set_watched(watched var, long new_val);

/* get_watched_xxx(var):
 * get the value of var
 */
extern long get_watched(watched var);

/* add_notify(f, var, context):
 * add a notify function to var. This function will be called as
 * f(var, value, context) each time the variable is set to a new value.
 * The function f must be ready to be called at add_notify() time.
 */
extern void add_notify(notify_function f, watched var);


