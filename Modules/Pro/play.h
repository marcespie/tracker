/* Modules/Pro/play.h */


/* init_player(oversample, frequency):
 * sets up the player for a given oversample and
 * output frequency.
 * Note: we can call init_player again to change oversample and
 * frequency.
 */
extern void init_player(int o, unsigned long f);

/* play_song(song, start):
 * play the song.  return tags as shown in get_ui 
 */
extern tag *play_song(song *song, unsigned int start);

