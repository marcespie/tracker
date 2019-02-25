/* open.c */

/* Magic open file: path lookup and transparent decompression */

#include "defs.h"

#include <ctype.h>

#include "extern.h"
#include "autoinit.h"
#include "open.h"

/* forward declarations */
LOCAL struct exfile *do_open(struct exfile *file, char *fname, char *path);



/***
 *** 	Stuff for compression methods 
 ***/

/* Max buffer length for reading compression methods */
#define MAX_LENGTH 90


/* automaton */
#define END_OF_LINE 0
#define BEGIN_OF_LINE 1
#define IN_SPEC 2
#define BEGIN_OF_COMMAND 3
#define IN_COMMAND 4


#ifndef NO_PIPES
LOCAL void init_compression P((void));
LOCAL void (*INIT)P((void)) = init_compression;
#endif

extern int error;



/*** extended file structure:
 ***		designed to be able to rewind pipes on a small length	(BUFSIZE) 
 ***		in order to be able to retry lots of formats for cheap
 ***/
#define BUFSIZE 15000
struct exfile
   {
   FILE *handle;							/* the real Mc Coy */
	unsigned char buffer[BUFSIZE];	/* we buffer only the file beginning */
	size_t length;								/* the length read in the buffer */
	size_t pos;									/* current pos in the buffer */

		/* OO methods */
   void (*close)(struct exfile *f);	
   void (*rewind)(struct exfile *f);
	unsigned long (*read)(void *ptr, size_t size, unsigned long nitems, 
		struct exfile *f);
   int (*getcar)(struct exfile *f);
   size_t (*tell)(struct exfile *f);

		/* kludge to reopen file */
	char *name;
	char *path;
   };

/***
 *** 	 the methods for buffered files 
 ***/
LOCAL int do_getchar(struct exfile *f)
   {
   int c;

	if (f->pos < BUFSIZE)
		{
		if (f->pos >= f->length)
			{
			error = FILE_TOO_SHORT;
			return EOF;
			}
		else
			return f->buffer[f->pos++];
		}
   if ((c = fgetc(f->handle)) == EOF)
      error = FILE_TOO_SHORT;
   else
      f->pos++;
   return c;
   }

LOCAL unsigned long do_read(void *p, size_t s, unsigned long n, 
	struct exfile *f)
	{
	size_t total = s * n;
	if (f->pos < BUFSIZE)
		{
		unsigned long remaining = f->length - f->pos;
		if (remaining >= total)
			{
			memcpy(p, &(f->buffer[f->pos]), total);
			f->pos += total;
			return n;
			}
		else
			{
			memcpy(p, &(f->buffer[f->pos]), remaining);
			total = remaining + 
					fread((char *)p+remaining, 1, total - remaining, f->handle);
			f->pos += total;
			return total/s;
			}
		}
	else
		{
		total = fread(p, s, n, f->handle);
		f->pos += total * s;
		return total;
		}
	}


LOCAL void do_rewind(struct exfile *f)
	{
	if (f->pos <= f->length)
		f->pos = 0;
	else
		{
		(*f->close)(f);
		f = do_open(f, f->name, f->path);
		}
	}

LOCAL size_t do_tell(struct exfile *f)
   {
   return f->pos;
   }

#ifndef NO_PIPES
LOCAL void do_pclose(struct exfile *f)
   {
   pclose(f->handle);
   }
#endif

LOCAL void do_fclose(struct exfile *f)
   {
   fclose(f->handle);
   }

LOCAL struct exfile *init_buffered(struct exfile *f)
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
#ifndef NO_PIPES
/***
 ***	Compression methods database handling
 ***/

/* compression methods we do know about.
 * Important restriction: for the time being, the output
 * must be a single module.
 */
struct compression_method
   {
   struct compression_method *next;
   char *extension;
   char *command;
   };

LOCAL struct compression_method *read_method(FILE *f)
   {
   static char buffer[MAX_LENGTH + 1];
   
   while (fgets(buffer, MAX_LENGTH, f))
      {
      int state = BEGIN_OF_LINE;
      size_t start, i;
      char *spec = NULL, *command = NULL;
      
      for (i = 0; state != END_OF_LINE; i++)
         {
         switch(state)
            {
         case BEGIN_OF_LINE:
            switch(buffer[i])
               {
            case ' ':
            case '\t':
               break;
            case 0:
            case '\n':
            case '#':
               state = END_OF_LINE;
               break;
            default:
               start = i;
               state = IN_SPEC;
               }
            break;
         case IN_SPEC:
            switch(buffer[i])
               {
            case ' ':
            case '\t':
               spec = (char *)malloc(i - start + 1);
               strncpy(spec, buffer + start, i - start);
               spec[i - start] = 0;
               state = BEGIN_OF_COMMAND;
               break;
            case 0:
            case '\n':
               state = END_OF_LINE;
               break;
            default:
               break;
               }
            break;
         case BEGIN_OF_COMMAND:
            switch (buffer[i])
               {
            case ' ':
            case '\t':
               break;
            case 0:
            case '\n':
               state = END_OF_LINE;
               free(spec);
               break;
            default:
               state = IN_COMMAND;
               start = i;
               }
            break;
         case IN_COMMAND:
            switch (buffer[i])
               {
            case 0:
            case '\n':
               command = (char *)malloc(i - start + 1);
               strncpy(command, buffer + start, i - start);
               command[i-start] = 0;
               state = END_OF_LINE;
            default:
               break;
               }
            }
         }      
      if (command && spec)
         {
         struct compression_method *n;
         
         n = (struct compression_method *)malloc(sizeof(struct compression_method));
         n->next = 0;
         n->extension = spec;
         n->command = command;
         return n;
         }
      }
   return 0;
   }
      

LOCAL struct compression_method **read_methods(
	struct compression_method **previous,
	FILE *f)
   {
   struct compression_method *method;
   
   if (f)
      {
      while ( (method = read_method(f)) )
         {
         *previous = method;
         previous = &(method->next);
         }
      fclose(f);
      }
   return previous;
   }
      

LOCAL struct compression_method *comp_list;

LOCAL void free_compression(void)
	{
	struct compression_method *temp;

	while (comp_list)
		{
		free(comp_list->extension);
		free(comp_list->command);
		temp = comp_list;
		comp_list = comp_list->next;
		free(temp);
		}
	}

LOCAL void init_compression(void)
   {
   char *fname;
   FILE *f;
   struct compression_method **previous;

	at_end(free_compression);
   f = 0;
   fname = getenv("TRACKER_COMPRESSION");
   if (fname)
      f = fopen(fname, "r");
   if (!f)
      {
      fname = COMPRESSION_FILE;
      f = fopen(fname, "r");
      }
	if (!f)
		notice(COMPRESSION_FILE);
   previous = read_methods(&comp_list, f);
   }
      
/* Handling extensions.
 */
LOCAL int check_ext(char *s, char *ext)
   {
   size_t ext_len, s_len;
   char *c;

   ext_len = strlen(ext);
   s_len = strlen(s);
   if (s_len < ext_len)
      return FALSE;
   for (c = s + s_len - ext_len; *c; c++, ext++)
      if (tolower(*c) != tolower(*ext))
         return FALSE;
   return TRUE;
   }

#endif

LOCAL int exist_file(char *fname)
   {
   FILE *temp;

   temp = fopen(fname, READ_ONLY);
   if (temp)
      {
      fclose(temp);
      return TRUE;
      }
   else
      return FALSE;
   }

#ifndef MAXPATHLEN
#define MAXPATHLEN 350
#endif

/* note that find_file returns a STATIC value */
LOCAL char *find_file(char *fname, char *path)
   {
   char *sep;
   static char buffer[MAXPATHLEN];
   size_t len;

      /* first, check the current directory */
   if (exist_file(fname))
      return fname;
   while(path)
      {
      sep = strchr(path, ':');
      if (sep)
         len = sep - path;
      else
         len = strlen(path);
      if (len < MAXPATHLEN)
         {
         strncpy(buffer, path, len);
         buffer[len] = '/';
         if (len + strlen(fname) < MAXPATHLEN - 5)
            {
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
LOCAL struct exfile *do_open(struct exfile *file, char *fname, char *path)
   {
#ifndef NO_PIPES
   struct compression_method *comp;

   INIT_ONCE;
#endif
    
	file->name = fname;
	file->path = path;

   fname = find_file(fname, path);
   if (!fname)
      goto not_opened;
#ifndef NO_PIPES
         /* check for extension */
   for (comp = comp_list; comp; comp = comp->next)
      if (check_ext(fname, comp->extension))
         {
         char *pipe;
         
         pipe = (char *)malloc(strlen(comp->command) + strlen(fname) + 25);
         if (!pipe)
            goto not_opened;

         sprintf(pipe, comp->command, fname);
         file->close = do_pclose;
         file->handle = popen(pipe, READ_ONLY);
         free(pipe);
         if (file->handle)
            return init_buffered(file);
         else
            goto not_opened;
         }
#endif
   file->close = do_fclose;
   if ( (file->handle = fopen(fname, READ_ONLY)) )
      return init_buffered(file);

not_opened:
   free(file);
   return NULL;
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

struct exfile *open_file(char *fname, char *mode, char *path)
/* right now, only mode "r" is supported */
{
	struct exfile *n;

	if (mode[0] != 'r' || mode[1] != 0)
		return nullptr;
	n = (struct exfile *)malloc(sizeof(struct exfile));
	if (n)
		return do_open(n, fname, path);
	else
		return nullptr;
}


void close_file(struct exfile *file)
   {
	if (file)
		{
		(*file->close)(file);
		free(file);
		}
   }

