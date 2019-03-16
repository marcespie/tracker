/* tags.h */

#define forever for(;;)

/* a tag is a very simple data structure */

struct tag
   {
   unsigned long type;
   VALUE data;
   };

#ifndef TAG_END
#define TAG_END 0
#endif
#ifndef TAG_IGNORE
#define TAG_IGNORE 1    /* ignore this tag */
#endif
#ifndef TAG_SKIP
#define TAG_SKIP 3      /* skip <n> tags in the list */
#endif
#ifndef TAG_SUB
#define TAG_SUB 2       /* sub to <taglist> */
#endif
#ifndef TAG_JUMP
#define TAG_JUMP 4      /* jump to <taglist> */
#endif

struct tag *get_tag (struct tag *t);

