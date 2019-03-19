/* empty.c */
/*
 * Copyright (c) 2019 Marc Espie <espie@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "extern.h"
#include "song.h"
#include "autoinit.h"
#include "empty.h"
     
static void init_empty (void);

static void (*INIT)(void) = init_empty;

static sample_info dummy;

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

sample_info *
empty_sample(void)
{
	INIT_ONCE;

	return &dummy;
}
