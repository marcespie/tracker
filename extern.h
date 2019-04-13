#ifndef EXTERN_H
#define EXTERN_H
/* extern.h */
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

#include <string.h>
#include <string>
#include <functional>
#include <utility>
#include <iosfwd>
#include <cstdint>

/* error types. Everything is centralized,
 * and we check in some places (see st_read, player and main)
 * that there was no error. Additionally signal traps work
 * that way too.
 */
enum class error_type {
/* normal state */
	NONE, 
/* read error */
	FILE_TOO_SHORT,
	CORRUPT_FILE,
/* trap error: goto next song right now */
	NEXT_SONG,
/* run time problem */
	FAULT,
/* the song has ended */
	ENDED,
/* unrecoverable problem: typically, trying to 
 * jump to nowhere land.
 */
	UNRECOVERABLE,
/* Missing sample. Very common error, not too serious. */
	SAMPLE_FAULT,
/* New */
	PREVIOUS_SONG,
	OUT_OF_MEM,
/* some weird soundtracker feature */
	NOT_SUPPORTED };
extern error_type error;


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

/* predefinitions for relevant structures */
class channel; 
class song;
class automaton;
class sample_info;
class event;
class tempo;
class play_entry;
class option_set;

#endif
