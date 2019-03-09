/* play_list.c */

#include <sys/types.h>
#include <sys/stat.h>
#ifdef OSK
#include <dir.h>
#define dirent direct
#else
#ifdef __NeXT__
#include <sys/dir.h>
#include <sys/dirent.h>
#else
#include <dirent.h>
#endif
#endif

#ifdef IS_POSIX
#include <time.h>
#else
#ifdef AMIGA
#include <time.h>
#else
#include <sys/time.h>
#endif
#endif

#include <vector>
#ifndef S_ISDIR
#define S_ISDIR(p) ((p) & S_IFDIR)
#endif
#include "defs.h"
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
	filename = name.c_str();
}

std::vector<play_entry> table;

LOCAL int 
is_dir(const char *name)
{
	struct stat buf;

	if (stat(name, &buf))
		return 0;
	return S_ISDIR(buf.st_mode);
}

LOCAL void 
expand_dir(const char *name)
{
	DIR *dir;
	struct dirent *de;
	if ( (dir = opendir(name)) ) {
		while ( (de = readdir(dir)) ) {
			if (strcmp(de->d_name, ".") == 0 || 
			    strcmp(de->d_name, "..") == 0)
				continue;
			play_entry n {name, de->d_name};
			if (is_dir(n.filename))
				expand_dir(n.filename);
			else
				table.push_back(n);
		}
		closedir(dir);
	}
}

ENTRY
obtain_play_list(void)
{
	std::cout << "Total files: " << table.size() << "\n"; 
	return begin(table);
}

void 
add_play_list(const char *name)
{
	if (is_dir(name))
		expand_dir(name);
	else
		table.emplace_back(nullptr, name);
}

int 
last_entry_index(void)
{
	return table.size() - 1;
}

void 
delete_entry(ENTRY entry)
{
	table.erase(entry);
}

void 
randomize(void)
{
	shuffle(begin(table), end(table), g);
}

