/* mung.c */
/* Simple set of routine to log and check memory allocations */

#include <stdio.h>

static FILE *output = 0;

/* alloc_munge: allocate memory with a wall of bytes around it */
static void *alloc_munge(size_t s)
	{
	char *n;
	size_t *p;
	int i;

	n = malloc(s+64+2 * sizeof(size_t));
	if (!n)
		return 0;
	p = n;
		/* save the length of the allocation
		 * save it twice to check the size: in case it is munged,
		 * this will report an error without crashing
		 */
	p[0] = p[1] = s+64+2 * sizeof(size_t);
		/* fill the memory wall with non standard values */
	for (i = 0; i < 32; i++)
		{
		n[i+2 * sizeof(size_t)] = 11;
		n[*p - i - 1] = 11;
		}
	return n + 32 + 2 * sizeof(size_t);
	}

static void free_munge(char *n)
	{
	char *z;
	size_t *p;
	int i;

	z = n - 32 - 2 * sizeof(size_t);
	p = z;
		/* check that the size wasn't trashed */
	if (p[0] != p[1])
		fprintf(output, "Error1: %lx\n", n);
	else
		{
		/* check that the memory wall is intact */
		for (i = 0; i < 32; i++)
			{
			if (z[i + 2 * sizeof(size_t)] != 11)
				fprintf(output, "Error2: %lx\n", n);
			if (z[*p -i - 1] != 11)
				fprintf(output, "Error3: %lx\n", n);
			}
		}
		/* deadly trap for whoever will reference freed memory */
	for (i = sizeof(size_t); i < *p; i++)
		z[i] = i;
	free(z);
	}

/* replacements for malloc, free, calloc, realloc */

void *do_malloc(size_t s, char *file, unsigned int line)
	{
	char *n;

	if (!output)
		output = fopen("logfile", "w");
	
	n = alloc_munge(s);
	
	fprintf(output, "Alloc: %lx %d %s %d\n", n, s, file, line);
	fflush(output);
	return n;
	}

void do_free(char *n, char *file, unsigned int line)
	{
	if (!output)
		output = fopen("logfile", "w");
	free_munge(n);
	fprintf(output, "Free: %lx %s %d\n", n, file, line);
	fflush(output);
	}

void *do_calloc(size_t s, unsigned int n, char *file, unsigned int line)
	{
	size_t total = s * n;
	int i;
	char *p;

	if (!output)
		output = fopen("logfile", "w");
	p = alloc_munge(total);

	for (i = 0; i < total; i++)
		p[i] = 0;
	
	fprintf(output, "Calloc: %lx %d %d %s %d\n", p, n, s, file, line);
	fflush(output);
	return p;
	}

void *do_realloc(void *p, size_t s, char *file, unsigned int line)
	{
	if (!output)
		output = fopen("logfile", "w");
	do_free(p, file, line);
	return do_malloc(s, file, line);
	}

/* debugging message */
void mung_message(char *s)
	{
	if (!output)
		output = fopen("logfile", "w");
	fprintf(output, "Message: %s\n", s);
	fflush(output);
	}




