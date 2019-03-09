/* play_list.h */

#include <string>
#include <vector>
const int UNKNOWN=42;

struct play_entry {
	play_entry(const char*, const char *);
	const char *filename;
	int filetype;
	std::string name;
};


using ENTRY = std::vector<play_entry>::iterator;

extern ENTRY obtain_play_list(void);

extern  int last_entry_index(void);
extern void randomize(void);
extern void delete_entry(ENTRY);
