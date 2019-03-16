/* defs.h */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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
