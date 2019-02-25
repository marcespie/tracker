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

clean:
	rm -f ${OBJS}
