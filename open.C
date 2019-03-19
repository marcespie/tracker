/* open.c */
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

/* Magic open file: path lookup and transparent decompression */

#include <ctype.h>

#include "extern.h"
#include "autoinit.h"
#include "open.h"

extern int error;

bool
exfile::open(const char* fname)
{
	handle = fopen(fname, READ_ONLY);
	return handle != nullptr;
		
}

bool
exfile::open(const std::string& fname)
{
	handle = fopen(fname.c_str(), READ_ONLY);
	return handle != nullptr;
		
}
exfile::~exfile()
{
	if (handle)
		fclose(handle);
}

int 
exfile::getc()
{
	int c = fgetc(handle);
	if (c == EOF)
		error = FILE_TOO_SHORT;
	return c;
}

unsigned long
exfile::read(void *p, size_t sz, unsigned long n)
{
	return fread(p, sz, n, handle);
}

void
exfile::rewind()
{
	::rewind(handle);
}

size_t
exfile::tell() const
{
	return ::ftell(handle);
}
