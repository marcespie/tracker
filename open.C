/* open.c */

/* Magic open file: path lookup and transparent decompression */

#include "defs.h"

#include <ctype.h>

#include "extern.h"
#include "autoinit.h"
#include "open.h"

extern int error;

bool
exfile::open(const char* fname, const char*)
{
	handle = fopen(fname, READ_ONLY);
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
