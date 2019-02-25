/* open.h
	vi:ts=3 sw=3:
 */

/* $Id: open.h,v 1.1 1996/05/06 07:36:56 espie Exp espie $
 * $Log: open.h,v $
 * Revision 1.1  1996/05/06 07:36:56  espie
 * Initial revision
 *
 */

/* handle = open_file(filename, mode, path):
 * transparently open a compressed file.
 */
XT struct exfile *open_file(char *fname, char *fmode, char *path);

/* close_file(handle):
 * close a file that was opened with open_file.
 */
XT void close_file(struct exfile *file);

/* analogous of fgetc, ftell, fread, and rewind */
XT int getc_file(struct exfile *file);
XT size_t tell_file(struct exfile *file);
XT unsigned long read_file(void *p, size_t s, unsigned long n, 
	struct exfile *file);
XT void rewind_file(struct exfile *file);


