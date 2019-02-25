/* Modules/Pro/effects.h
	vi:ts=3 sw=3:
 */

/* $Id: effects.h,v 1.1 1996/04/12 16:31:27 espie Exp espie $
 * $Log: effects.h,v $
 * Revision 1.1  1996/04/12 16:31:27  espie
 * Initial revision
 *
 * Revision 1.1  1996/04/09 21:13:34  espie
 * Initial revision
 *
 */

enum st_effect_type
	{
	CH_E, A_E, NO_NOTE_CH_E, PORTA_CH_PITCH_E, CH_A_E, NOTHING
	};

struct st_effect
	{
	enum st_effect_type type;
	union
		{
		void (*CH_E)(struct channel *ch, struct event *e);
		void (*A_E)(struct automaton *a, struct event *e);
		void (*CH_PITCH_E)(struct channel *ch, pitch pitch, 
			struct event *e);
		void (*CH_A_E)(struct channel *ch, struct automaton *a, struct event *e);
		} f;
	};

/* init_effects(): set up all data for the effects
 * can't be set up as an autoinit
 */
XT void init_effects(struct st_effect table[]);

/* do_nothing: this is the default behavior for an effect
 */
XT void do_nothing(struct channel *ch);
