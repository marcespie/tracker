/* randomize.c */

/* input: a series of names (as argv[1:argc - 1])
 * output: the same names, in a random order.
 * with the new database lookup facility, very useful for e.g.,
 * tracker `randomize *` (jukebox)
 */

#include "defs.h"
#include <sys/types.h>
#ifdef IS_POSIX
#include <time.h>
#else
#include <sys/time.h>
#endif

/* n = random_range(max): output a number in the range 0:max - 1.
 * For our purpose, we don't have to get a very random number,
 * so the standard generator is alright.
 */
unsigned int random_range(unsigned int max)
    {
    static int init = 0;

        /* initialize the generator to an appropriate seed eventually */
    if (!init)
        {
        srand(time(0));
        init = 1;
        }
    return rand()%max;
    }

/* output(s): output s in a suitable format. Ideally, output() should use
 * the shell quoting conventions for difficult names. Right now, it doesn't
 */
void output(char *s)
    {
    for(; *s; s++)
        switch(*s)
            {
    /*    case ' ':
        case '(':
        case ')':
        case '\\':
            putchar('\\');
            */
        default:
            putchar(*s);
            }
    putchar(' ');
    }

int main(int argc, char *argv[])
    {
    unsigned int i, k;

        /* set up everything so that our names are in argv[0 : argc - 2] */
    for (i = argc - 1, argv++; i; i--)
        {
            /* invariant: the remaining names are in argv[0: i - 1] */
        k = random_range(i);
        output(argv[k]);
        argv[k] = argv[i - 1];
        }
    exit(EXIT_SUCCESS);
    }
