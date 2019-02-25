/* Modules/Pro/low.h
	vi:ts=3 sw=3:
 */

/* $Id: low.h,v 1.3 1996/05/07 15:22:45 espie Exp espie $
 * $Log: low.h,v $
 * Revision 1.3  1996/05/07 15:22:45  espie
 * *** empty log message ***
 *
 * Revision 1.2  1996/05/06 22:48:36  espie
 * *** empty log message ***
 *
 * Revision 1.1  1996/04/12 16:31:22  espie
 * Initial revision
 *
 * Revision 1.1  1996/04/09 21:14:13  espie
 * Initial revision
 *
 */

/* start_note(ch):
 * set channel ch to play note at pitch pitch
 */
XT void start_note(struct channel *ch);
XT void stop_note(struct channel *ch);
XT void set_current_note(struct channel *ch, note note, pitch pitch);

/* set_temp_pitch(ch, pitch):
 * set ch to play at pitch pitch
 */
XT void set_temp_pitch(struct channel *ch, pitch pitch);

/* set_current_volume(ch, volume):
 * set channel ch to play at volume volume
 */
XT void set_current_volume(struct channel *ch, int volume);

/* set_temp_volume(ch, volume):
 * set channel ch to play at volume volume, but without storing it
 * (used only for tremolo)
 */
XT void set_temp_volume(struct channel *ch, int volume);

/* set_position(ch, pos):
 * set position in sample for current channel at given offset
 */
XT void set_position(struct channel *ch, size_t pos);

