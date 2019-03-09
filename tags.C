/* tags.c */

#include "defs.h"

#include "tags.h"

/* WARNING: TAG_SUB is not supported */
struct tag *get_tag(struct tag *t)
   {
   forever
      {
      switch (t->type)
         {
      case TAG_END:
         return 0;
      case TAG_IGNORE:
         t++;
         return get_tag(t);
      case TAG_SKIP:
         t += t->data.scalar;
         break;
      case TAG_JUMP:
         t = (struct tag *)(t->data.pointer);
         break;
      default:
         return t;
         }
      }
   }
      
