/* config.h */

/* Configuration for OpenBSD */

#define IS_POSIX
#define USE_TERMIOS
#define USE_AT_EXIT
#define SCO_ANSI_COLOR

typedef void *GENERIC;

#define stricmp	strcasecmp

#define COMPRESSION_FILE "/usr/local/lib/compression_methods"
