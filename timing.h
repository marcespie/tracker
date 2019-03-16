/* timing.h */

/* convert a time value to a string using a local buffer */
extern char *time2string(char *buffer, unsigned long t);

/* convert a ratio n/p seconds to a time value */
extern unsigned long ratio2time(int n, int p);



