/* empty.c */

#include "defs.h"

#include "extern.h"
#include "song.h"
#include "autoinit.h"
#include "empty.h"
     
static void init_empty (void);

static void (*INIT)(void) = init_empty;

static struct sample_info dummy;

static void 
init_empty(void)
{
	dummy.name = nullptr;
	dummy.length = dummy.rp_offset = dummy.rp_length = 0;
	dummy.fix_length = dummy.fix_rp_length = 0;
	dummy.volume = 0;
	dummy.finetune = 0;
	dummy.start = dummy.rp_start = NULL;
	dummy.color = 1;
	for (auto& x: dummy.volume_lookup)
		x = 0;
}

struct sample_info *
empty_sample(void)
{
	INIT_ONCE;

	return &dummy;
}
