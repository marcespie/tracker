#ifndef SONG_H
#define SONG_H
// song.h
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

class exfile;
class resampler;

class Module {
public:
	virtual int play(unsigned int start, resampler& r) =0;
	virtual void dump() const =0;
	virtual void adjust_volume(unsigned long mask) =0;
	virtual ~Module()
	{
	}
};

// hint values for loading songs, to be revisited
const auto OLD=0;
const auto NEW=1;
/* special new type: for when we try to read it as both types.
 */
const auto BOTH=2;
/* special type: does not check the signature */
const auto NEW_NO_CHECK=3;


// wrapper class around all module types.
class Song {
	std::unique_ptr<Module> mod;
public:
	Song() : mod{nullptr} 
	{
	}
	bool load(exfile& file, int hint);
	Song(exfile& file, int hint);
	Song& operator=(Song&&);
	Song& operator=(const Song&) = delete;
	Song(const Song&) = delete;
	Song(Song&&);
	int play(unsigned int start, resampler& r);
	void dump() const;
	void adjust_volume(unsigned long mask);
	operator bool() const
	{
		return mod != nullptr;
	}
};

#endif
