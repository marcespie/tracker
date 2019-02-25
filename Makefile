PROG = tracker
OBJS = tracker.o \
    automaton.o open.o timing.o \
    dump_song.o setup_audio.o notes.o display.o empty.o \
    Arch/Unix/ui.o prefs.o tags.o autoinit.o color.o version.o \
    play_list.o handle_options.o parse_options.o watched_var.o \
    Modules/Pro/low.o Modules/Pro/read.o Modules/Pro/effects.o \
    Modules/Pro/virt.o Modules/Pro/play.o \
    Arch/OpenBSD/audio.o resample.o usage.o # mung.o

CXXFLAGS = ${CPPFLAGS} -O2 -W -Wall -std=c++17
CPPFLAGS = -I.
LIBS = -lsndio -lm

.C.o:
	${CXX} -o $@ -c ${CXXFLAGS} $*.C

tracker: ${OBJS}
	${CXX} -o $@ ${CXXFLAGS} ${OBJS} ${LIBS}

analyzer.o autoinit.o automaton.o display.o dump_song.o empty.o \
handle_options.o notes.o open.o parse_options.o play_list.o prefs.o \
resample.o setup_audio.o split.o tracker.o usage.o \
Modules/Pro/effects.o Modules/Pro/play.o Modules/Pro/read.o Modules/Pro/virt.o \
Arch/OpenBSD/audio.o Arch/Unix/ui.o: extern.h defs.h

autoinit.o display.o empty.o handle_options.o notes.o open.o \
parse_options.o play_list.o prefs.o resample.o setup_audio.o \
Modules/Pro/play.o Modules/Pro/read.o \
tracker.o watched_var.o Arch/OpenBSD/audio.o Arch/Unix/ui.o: autoinit.h


automaton.o display.o Modules/Pro/effects.o Modules/Pro/play.o Modules/Pro/read.o Modules/Pro/virt.o :  automaton.h

display.o dump_song.o notes.o resample.o Modules/Pro/effects.o Modules/Pro/low.o Modules/Pro/play.o Modules/Pro/read.o Modules/Pro/virt.o :  channel.h
version.o :  config.h
analyzer.o autoinit.o automaton.o display.o dump_song.o empty.o handle_options.o notes.o open.o parse_options.o play_list.o prefs.o randomize.o resample.o setup_audio.o split.o tags.o timing.o tools.o tracker.o usage.o watched_var.o Arch/OpenBSD/audio.o Arch/Unix/ui.o Modules/Pro/effects.o Modules/Pro/low.o Modules/Pro/play.o Modules/Pro/read.o Modules/Pro/virt.o :  defs.h
display.o empty.o resample.o Modules/Pro/low.o Modules/Pro/play.o Modules/Pro/read.o :  empty.h
analyzer.o autoinit.o automaton.o display.o dump_song.o empty.o handle_options.o notes.o open.o parse_options.o play_list.o prefs.o resample.o setup_audio.o split.o tracker.o usage.o Arch/OpenBSD/audio.o Arch/Unix/ui.o Modules/Pro/effects.o Modules/Pro/play.o Modules/Pro/read.o Modules/Pro/virt.o :  extern.h
automaton.o display.o dump_song.o notes.o resample.o setup_audio.o Modules/Pro/effects.o Modules/Pro/low.o Modules/Pro/play.o Modules/Pro/read.o Modules/Pro/virt.o :  notes.h
handle_options.o open.o split.o tracker.o Modules/Pro/read.o :  open.h
handle_options.o parse_options.o :  options.h
automaton.o Modules/Pro/effects.o Modules/Pro/play.o Modules/Pro/virt.o :  p_automaton.h
resample.o :  p_resample.h
handle_options.o parse_options.o :  parse_options.h
play_list.o tracker.o :  play_list.h
analyzer.o automaton.o display.o dump_song.o handle_options.o prefs.o resample.o setup_audio.o tracker.o Arch/OpenBSD/audio.o Arch/Unix/ui.o Modules/Pro/effects.o Modules/Pro/play.o Modules/Pro/virt.o :  prefs.h
automaton.o resample.o setup_audio.o Modules/Pro/low.o Modules/Pro/play.o Modules/Pro/read.o :  resample.h
analyzer.o automaton.o display.o dump_song.o empty.o notes.o resample.o split.o tracker.o Modules/Pro/effects.o Modules/Pro/low.o Modules/Pro/play.o Modules/Pro/read.o Modules/Pro/virt.o :  song.h
analyzer.o display.o handle_options.o prefs.o resample.o setup_audio.o tags.o tracker.o Arch/Unix/ui.o Modules/Pro/play.o :  tags.h
automaton.o timing.o Arch/Unix/ui.o Modules/Pro/virt.o :  timing.h
handle_options.o resample.o setup_audio.o watched_var.o Arch/OpenBSD/audio.o :  watched_var.h
clean:
	rm -f ${OBJS}
