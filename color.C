/* color.c */

char *
write_color(char *base, unsigned int color)
{
	*base++ = '\033';
	*base++ = '[';
#ifdef SCO_ANSI_COLOR
	if (color == 0) {
		*base++ = 'x';
		return base;
	}
#endif
	*base++ = '0' + color / 8;
	*base++=';';
	*base++='3';
	if (color == 0)		/* color == 0 means reset */
		*base++ = '9';
	else
		*base++ = '0' + color % 8;
	*base++ = 'm';
	return base;
}
