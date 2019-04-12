/* play_list.c */
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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <time.h>

#include <vector>
#include "extern.h"
#include "play_list.h"
#include "autoinit.h"
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

static std::random_device rd;
std::mt19937 g(rd());



play_entry::play_entry(const char* dir, const char* f): 
    filetype{UNKNOWN}
{
	if (dir)
		name = std::string(dir) + '/' + f;
	else
		name = f;
}

static int 
is_dir(const char *name)
{
	struct stat buf;

	if (stat(name, &buf))
		return 0;
	return S_ISDIR(buf.st_mode);
}

static void 
expand_dir(play_list&p, const char *name)
{
	DIR *dir;
	struct dirent *de;
	if ( (dir = opendir(name)) ) {
		while ( (de = readdir(dir)) ) {
			if (strcmp(de->d_name, ".") == 0 || 
			    strcmp(de->d_name, "..") == 0)
				continue;
			play_entry n {name, de->d_name};
			if (is_dir(n.filename()))
				expand_dir(p, n.filename());
			else
				p.push_back(n);
		}
		closedir(dir);
	}
}

void
add_entry(play_list& p, const char *name)
{
	if (is_dir(name))
		expand_dir(p, name);
	else
		p.emplace_back(nullptr, name);
}

ENTRY
delete_entry(play_list& p, ENTRY entry)
{
	return p.erase(entry);
}

void 
randomize(play_list& p)
{
	shuffle(begin(p), end(p), g);
}

