/* color.c */

char *write_color(char *base, unsigned int color)
	{
#ifdef __OS2__
	// The first 7 colors are totally useless
	if(color>0)
		color = color % 8 + 8;
#endif
	*base++ = '\033';
	*base++ = '[';
#ifdef AMIGA
#else
#ifdef SCO_ANSI_COLOR
	if (color == 0)
		{
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
#endif
	return base;
	}
