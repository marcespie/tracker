/* defs.h */

#define LOCAL static
/* X is too short */
#define XT extern

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef MALLOC_NOT_IN_STDLIB
#include <malloc.h>
#endif
#ifdef ONLY_BCOPY
#define memmove(a, b, n)		bcopy(b, a, n)
#endif

#ifndef EXPAND_WILDCARDS
#define EXPAND_WILDCARDS(x,y)
#endif

#ifdef BINARY_HEEDED
#define READ_ONLY    "rb"
#define WRITE_ONLY   "wb"
#else
#define READ_ONLY    "r"
#define WRITE_ONLY   "w"
#endif


/* broken ANSI compilers */
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef MIN
#define MIN(A,B) ((A)<(B) ? (A) : (B))
#endif
#ifndef MAX
#define MAX(A,B) ((A)>(B) ? (A) : (B))
#endif
     
#define D fprintf(stderr, "%d\n", __LINE__);

typedef union
   {
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
