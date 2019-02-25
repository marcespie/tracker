/* empty.c */

#include "defs.h"

#include "extern.h"
#include "song.h"
#include "autoinit.h"
#include "empty.h"
     
LOCAL void init_empty (void);

LOCAL void (*INIT)(void) = init_empty;

LOCAL struct sample_info dummy;

LOCAL void 
init_empty(void)
{
	unsigned int i;

	dummy.name = NULL;
	dummy.length = dummy.rp_offset = dummy.rp_length = 0;
	dummy.fix_length = dummy.fix_rp_length = 0;
	dummy.volume = 0;
	dummy.finetune = 0;
	dummy.start = dummy.rp_start = NULL;
	dummy.color = 1;
	for (i = 0; i <= MAX_VOLUME; i++)
		dummy.volume_lookup[i] = 0;
}

struct sample_info *
empty_sample(void)
{
	INIT_ONCE;

	return &dummy;
}

