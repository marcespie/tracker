/* play_list.c */

#include <sys/types.h>
#include <sys/stat.h>
#ifdef OSK
#include <dir.h>
#define dirent direct
#else
#ifdef __NeXT__
#include <sys/dir.h>
#include <sys/dirent.h>
#else
#include <dirent.h>
#endif
#endif

#ifdef IS_POSIX
#include <time.h>
#else
#ifdef AMIGA
#include <time.h>
#else
#include <sys/time.h>
#endif
#endif

#ifndef S_ISDIR
#define S_ISDIR(p) ((p) & S_IFDIR)
#endif
#ifdef AMIGA
#include <proto/dos.h>
#endif
#include "defs.h"
#include "extern.h"
#include "play_list.h"
#include "autoinit.h"


/* n = random_range(max): output a number in the range 0:max - 1.
 * For our purpose, we don't have to get a very random number,
 * so the standard generator is alright.
 */
LOCAL unsigned int random_range(unsigned int max)
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

LOCAL struct play_entry *new_entry(char *dir, char *name)
	{
	size_t i;
	struct play_entry *n;

	if (dir)
		{
		i = strlen(name) + strlen(dir)+1; /* note + 1 only since char name[1] */
		if ( (n = (struct play_entry *)malloc(sizeof(struct play_entry) + sizeof(char) * i)) )
			{
#ifdef AMIGA
         strcpy(n->name, dir);
         if (!AddPart(n->name, name, i+1))
            return 0;
#else
			sprintf(n->name, "%s/%s", dir, name);
#endif
			n->filename = n->name;
			n->filetype = UNKNOWN;
			}
		}
	else if ( (n = (struct play_entry *)malloc(sizeof(struct play_entry))) )
		{
		n->filename = name;
		n->filetype = UNKNOWN;
		}
	return n;
	}

LOCAL ENTRY *table;
LOCAL unsigned idx;
LOCAL unsigned size;

LOCAL void free_play_list(void)
	{
	unsigned i;

	for (i = 0; i < idx; i++)
		free(table[i]);
	free(table);
	size = idx = 0;
	}

#define CHUNK 500

LOCAL void check_bounds(void)
	{
	ENTRY *oldtable;
	if (idx >= size)
		{
		oldtable = table;
		size += CHUNK;
		table = (ENTRY *)malloc(sizeof(ENTRY) * size);
		if (table)
			memcpy(table, oldtable, sizeof(ENTRY) * (size - CHUNK));
		if (oldtable)
			free(oldtable);
		}
	}

LOCAL int is_dir(char *name)
	{
	struct stat buf;

	if (stat(name, &buf))
		return 0;
	return S_ISDIR(buf.st_mode);
	}

LOCAL void expand_dir(char *name)
	{
	DIR *dir;
	struct dirent *de;
	ENTRY n;
	
	if ( (dir = opendir(name)) )
		{
		while ( (de = readdir(dir)) )
			{
			if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
				continue;
			n = new_entry(name, de->d_name);
			if (n)
				{
				if (is_dir(n->filename))
					{
					expand_dir(n->filename);
					free(n);
					}
				else
					{
					check_bounds();
					table[idx++] = n;
					}
				}
			}
		closedir(dir);
		}
	}

ENTRY *obtain_play_list(void)
	{
	at_end(free_play_list);

	check_bounds();
	table[idx] = 0;
	printf("Total files: %u\n", idx);
	return table;
	}

void add_play_list(char *name)
	{
	check_bounds();
	if (is_dir(name))
		expand_dir(name);
	else
		table[idx++] = new_entry(0, name);
	}

int last_entry_index(void)
	{
	return ((int)idx) - 1;
	}

void delete_entry(ENTRY *entry)
	{
	int n;
	ENTRY old;

	old = *entry;
	n = idx - (entry - table) - 1;

	memmove(entry, entry+1, n * sizeof(ENTRY));
	idx--;
	free(old);
	}

void randomize(void)
	{
	ENTRY e;
	unsigned i, k;

	if (idx == 0)
		return;
	for (i = idx-1; i > 0; i--)
		{
		k = random_range(i+1);
		e = table[k];
		table[k] = table[i];
		table[i] = e;
		}
	}

