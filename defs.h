/* defs.h */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

const auto READ_ONLY="rb";
const auto WRITE_ONLY="wb";

template<typename S, typename T>
inline auto
MIN(S x, T y)
{
	return x<y ?  x : y;
}

template<typename S, typename T>
inline auto
MAX(S x, T y)
{
	return x>y ?  x : y;
}

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
