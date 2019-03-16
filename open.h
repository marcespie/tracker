/* open.h */

/* handle = open_file(filename, mode, path):
 * transparently open a compressed file.
 */
extern struct exfile *open_file(const char *fname, const char *fmode, 
	const char *path);

/* close_file(handle):
 * close a file that was opened with open_file.
 */
extern void close_file(struct exfile *file);

/* analogous of fgetc, ftell, fread, and rewind */
extern int getc_file(struct exfile *file);
extern size_t tell_file(struct exfile *file);
extern unsigned long read_file(void *p, size_t s, unsigned long n, 
	struct exfile *file);
extern void rewind_file(struct exfile *file);


