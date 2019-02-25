/* Modules/Pro/play.h
	vi:ts=3 sw=3:
 */

/* $Id: play.h,v 1.2 1996/05/06 07:37:11 espie Exp espie $
 * $Log: play.h,v $
 * Revision 1.2  1996/05/06 07:37:11  espie
 * *** empty log message ***
 *
 * Revision 1.1  1996/04/12 16:31:28  espie
 * Initial revision
 *
 * Revision 1.1  1996/04/09 21:14:14  espie
 * Initial revision
 *
 */


/* init_player(oversample, frequency):
 * sets up the player for a given oversample and
 * output frequency.
 * Note: we can call init_player again to change oversample and
 * frequency.
 */
XT void init_player(int o, unsigned long f);

/* play_song(song, start):
 * play the song.  return tags as shown in get_ui 
 */
XT struct tag *play_song(struct song *song, unsigned int start);

