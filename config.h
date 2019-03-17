#include <cstring>
using GENERIC = void *;

inline int stricmp(const char *a, const char *b)
{
	return strcasecmp(a, b);
}
