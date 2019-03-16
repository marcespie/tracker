/* automaton.h */

struct automaton;
struct song;
struct event;

const auto NORMAL_FINESPEED=125;
/* a = init_automaton(song, start):
 * return an automaton set in the right state to play song from pattern #start.
 */
extern automaton *setup_automaton(song *song, unsigned int start);

/* next_tick(a):
 * set up everything for the next tick.
 */
extern void next_tick(automaton *a);

/* update_tempo(a):
 * update tempo in automaton according to parameters
 */
extern void update_tempo(automaton *a);
extern void set_bpm(automaton *a, unsigned int bpm);

extern event *EVENT(automaton *a, int channel);



