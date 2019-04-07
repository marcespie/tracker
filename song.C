// song.C
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

#include <memory>
#include "song.h"
#include "protracker.h"
#include "pro_play.h"
#include "extern.h"

Song::Song(exfile& file, int hint):
	mod{read_song(file, hint)}
{
}

Song& Song::operator=(Song&& o)
{
	mod.swap(o.mod);
	release_song(o.mod.get());
	return *this;
}

Song::Song(Song&& o)
{
	mod.swap(o.mod);
	release_song(o.mod.get());
}

bool Song::load(exfile& file, int hint)
{
	release_song(mod.get());
	mod = std::unique_ptr<song>(read_song(file, hint));
	return mod != nullptr;
}

Song::~Song()
{
	release_song(mod.get());
}

int
Song::play(unsigned int start)
{
	return play_song(mod.get(), start);
}

void
Song::dump() const
{
	dump_song(mod.get());
}

void
Song::adjust_volume(unsigned long mask)
{
	adjust_song(mod.get(), mask);
}

