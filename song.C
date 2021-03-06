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
#include "pro_read.h"

Song::Song(exfile& file, int hint):
	mod{read_song(file, hint)}
{
}

Song& Song::operator=(Song&& o)
{
	mod.swap(o.mod);
	return *this;
}

Song::Song(Song&& o)
{
	mod.swap(o.mod);
}

bool Song::load(exfile& file, int hint)
{
	delete mod.get();
	mod = std::unique_ptr<Module>(read_song(file, hint));
	return mod != nullptr;
}

int
Song::play(unsigned int start, resampler& r, audio& d)
{
	return mod->play(start, r, d);
}

void
Song::dump() const
{
	mod->dump();
}

void
Song::adjust_volume(unsigned long mask)
{
	mod->adjust_volume(mask);
}

