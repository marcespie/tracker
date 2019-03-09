/* open.c */

/* Magic open file: path lookup and transparent decompression */

#include "defs.h"

#include <ctype.h>

#include "extern.h"
#include "autoinit.h"
#include "open.h"

/* forward declarations */
LOCAL struct exfile *do_open(struct exfile *file, const char *fname, 
    const char *path);

extern int error;



/*** extended file structure:
 *** designed to be able to rewind pipes on a small length	(BUFSIZE) 
 *** in order to be able to retry lots of formats for cheap
 ***/
#define BUFSIZE 15000
struct exfile {
	FILE *handle;		/* the real Mc Coy */
	unsigned char buffer[BUFSIZE];	/* we buffer only the file beginning */
	size_t length;		/* the length read in the buffer */
	size_t pos;		/* current pos in the buffer */ 
	/* OO methods */
	void (*close)(struct exfile *f);	
	void (*rewind)(struct exfile *f);
	unsigned long (*read)(void *ptr, size_t size, unsigned long nitems, 
	    struct exfile *f);
	int (*getcar)(struct exfile *f);
	size_t (*tell)(struct exfile *f);

	/* kludge to reopen file */
	const char *name;
	const char *path;
};

/***
 *** 	 the methods for buffered files 
 ***/
LOCAL int 
do_getchar(struct exfile *f)
{
	int c;

	if (f->pos < BUFSIZE) {
		if (f->pos >= f->length) {
			error = FILE_TOO_SHORT;
			return EOF;
		} else
			return f->buffer[f->pos++];
	}
	if ((c = fgetc(f->handle)) == EOF)
		error = FILE_TOO_SHORT;
	else
		f->pos++;
	return c;
}

LOCAL unsigned long 
do_read(void *p, size_t s, unsigned long n, struct exfile *f)
{
	size_t total = s * n;
	if (f->pos < BUFSIZE) {
		unsigned long remaining = f->length - f->pos;
		if (remaining >= total) {
			memcpy(p, &(f->buffer[f->pos]), total);
			f->pos += total;
			return n;
		} else {
			memcpy(p, &(f->buffer[f->pos]), remaining);
			total = remaining + fread((char *)p+remaining, 
				1, total - remaining, f->handle);
			f->pos += total;
			return total/s;
		}
	} else {
		total = fread(p, s, n, f->handle);
		f->pos += total * s;
		return total;
	}
}


LOCAL void 
do_rewind(struct exfile *f)
{
	if (f->pos <= f->length)
		f->pos = 0;
	else {
		(*f->close)(f);
		f = do_open(f, f->name, f->path);
	}
}

LOCAL size_t 
do_tell(struct exfile *f)
{
	return f->pos;
}


LOCAL void 
do_fclose(struct exfile *f)
{
	fclose(f->handle);
}

LOCAL struct exfile *
init_buffered(struct exfile *f)
{
	f->length = fread(f->buffer, 1, BUFSIZE, f->handle);
	f->getcar = do_getchar;
	f->tell = do_tell;
	f->read = do_read;
	f->rewind = do_rewind;
	f->pos = 0;
	return f;
}
  


/* no need for compression_methods if no pipes !!! */

LOCAL int 
exist_file(const char *fname)
{
	FILE *temp;

	temp = fopen(fname, READ_ONLY);
	if (temp) {
		fclose(temp);
		return true;
	}
	else 
		return false;
}

#ifndef MAXPATHLEN
#define MAXPATHLEN 350
#endif

/* note that find_file returns a STATIC value */
LOCAL char *
find_file(const char *fname, const char *path)
{
	const char *sep;
	static char buffer[MAXPATHLEN];
	size_t len;

	/* first, check the current directory */
	if (exist_file(fname))
		return (char *)fname;
	while(path) {
		sep = strchr(path, ':');
		if (sep)
			len = sep - path;
		else
			len = strlen(path);
		if (len < MAXPATHLEN) {
			strncpy(buffer, path, len);
			buffer[len] = '/';
			if (len + strlen(fname) < MAXPATHLEN - 5) {
				strcpy(buffer + len + 1, fname);
				puts(buffer);
				if (exist_file(buffer))
					return buffer;
			}
		}
		if (sep)
			path = sep + 1;
		else
			return NULL;
	}
	return NULL;
}

/* opening a file is bigger than it seems (much bigger) */
LOCAL struct exfile *
do_open(struct exfile *file, const char *fname, const char *path)
{

	file->name = fname;
	file->path = path;

	fname = find_file(fname, path);
	if (!fname)
		goto not_opened;
	file->close = do_fclose;
	if ( (file->handle = fopen(fname, READ_ONLY)) )
		return init_buffered(file);

not_opened:
	delete file;
	return nullptr;
}




/***
 ***	The stubs that call the actual methods
 ***/


int getc_file(struct exfile *f)
   {
   return (*f->getcar)(f);
   }

unsigned long read_file(void *p, size_t s, unsigned long n, struct exfile *f)
	{
	return (*f->read)(p, s, n, f);
	}

size_t tell_file(struct exfile *f)
   {
   return (*f->tell)(f);
   }

void rewind_file(struct exfile *f)
	{
	(*f->rewind)(f);
	}

struct exfile *
open_file(const char *fname, const char *mode, const char *path)
/* right now, only mode "r" is supported */
{

	if (mode[0] != 'r' || mode[1] != 0)
		return nullptr;

	exfile *n = new exfile;
	return do_open(n, fname, path);
}


void 
close_file(struct exfile *file)
{
    if (file) {
	    (*file->close)(file);
	    delete file;
    }
}

