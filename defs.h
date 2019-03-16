/* defs.h */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef EXPAND_WILDCARDS
#define EXPAND_WILDCARDS(x,y)
#endif

const auto READ_ONLY="rb";
const auto WRITE_ONLY="wb";

#ifndef MIN
#define MIN(A,B) ((A)<(B) ? (A) : (B))
#endif
#ifndef MAX
#define MAX(A,B) ((A)>(B) ? (A) : (B))
#endif
     
#define D fprintf(stderr, "%d\n", __LINE__);

typedef union {
	long scalar;
	float real;
	GENERIC pointer;
} VALUE;

/* predefinitions for relevant structures */
struct tag;
struct channel; 
struct song;
struct automaton;
struct sample_info;
struct event;
struct tempo;
struct vauto;
struct play_entry;
struct option_set;
struct iff;
struct comm_chunk;
struct exfile;


/* memory test */
#ifdef MUNG
#define malloc(n) do_malloc(n, __FILE__, __LINE__)
#define free(n) do_free(n, __FILE__, __LINE__)
#define calloc(n, s) do_calloc(n, s, __FILE__, __LINE__)
#define realloc(p, n) do_realloc(p, n, __FILE__, __LINE__)
#else
#define mung_message(p)
#endif
