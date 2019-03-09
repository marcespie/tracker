/* Modules/Pro/effects.h */


enum st_effect_type {
	CH_E, A_E, NO_NOTE_CH_E, PORTA_CH_PITCH_E, CH_A_E, NOTHING
};

struct st_effect {
	enum st_effect_type type;
	union {
		void (*CH_E)(channel *ch, const event *e);
		void (*A_E)(automaton *a, const event *e);
		void (*CH_PITCH_E)(channel *ch, pitch pitch, const event *e);
		void (*CH_A_E)(channel *ch, automaton *a, const event *e);
	} f;
};

/* init_effects(): set up all data for the effects
 * can't be set up as an autoinit
 */
extern void init_effects(st_effect table[]);

/* do_nothing: this is the default behavior for an effect
 */
extern void do_nothing(channel *);
