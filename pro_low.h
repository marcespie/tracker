/* Modules/Pro/low.h */

/* start_note(ch):
 * set channel ch to play note at pitch pitch
 */
extern void start_note(channel *ch);
extern void stop_note(channel *ch);
extern void set_current_note(channel *ch, note note, pitch pitch);

/* set_temp_pitch(ch, pitch):
 * set ch to play at pitch pitch
 */
extern void set_temp_pitch(channel *ch, pitch pitch);

/* set_current_volume(ch, volume):
 * set channel ch to play at volume volume
 */
extern void set_current_volume(channel *ch, int volume);

/* set_temp_volume(ch, volume):
 * set channel ch to play at volume volume, but without storing it
 * (used only for tremolo)
 */
extern void set_temp_volume(channel *ch, int volume);

/* set_position(ch, pos):
 * set position in sample for current channel at given offset
 */
extern void set_position(channel *ch, size_t pos);

