/* automaton.h
	vi:ts=3 sw=3:
 */

/* $Id: automaton.h,v 1.1 1996/04/09 21:13:26 espie Exp espie $
 * $Log: automaton.h,v $
 * Revision 1.1  1996/04/09 21:13:26  espie
 * Initial revision
 *
 */

#define NORMAL_FINESPEED 125
/* a = init_automaton(song, start):
 * return an automaton set in the right state to play song from pattern #start.
 */
XT struct automaton *setup_automaton(struct song *song, unsigned int start);

/* next_tick(a):
 * set up everything for the next tick.
 */
XT void next_tick(struct automaton *a);

/* update_tempo(a):
 * update tempo in automaton according to parameters
 */
XT void update_tempo(struct automaton *a);
XT void set_bpm(struct automaton *a, unsigned int bpm);

XT struct event *EVENT(struct automaton *a, int channel);



