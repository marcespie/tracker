/* tools.c */
/* standard routines for use in tracker. Used to be in main.c
 */

#include "defs.h"
#include "tools.h"
     
/* v = read_env(name, default): read the scalar value v in the environment, 
 * supply default value.
 */
int read_env(char *name, int def)
   {
   char *var;
   int value;

   var = getenv(name);
   if (!var)
      return def;
   if (sscanf(var, "%d", &value) == 1)
      return value;
   else
      return def;
   }
