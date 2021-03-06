PROG = tracker
SRCS = tracker.C \
    automaton.C open.C timing.C \
    dump_song.C notes.C display.C empty.C \
    ui.C prefs.C autoinit.C color.C version.C \
    play_list.C handle_options.C parse_options.C watched_var.C \
    pro_low.C pro_read.C pro_effects.C \
    pro_virt.C pro_play.C \
    openbsd_audio.C resample.C usage.C song.C audio.C

CXXFLAGS = -O2 -W -Wall -Wmissing-prototypes -std=c++17 ${DEBUG}
CPPFLAGS = -I${.CURDIR}
LDADD = -lsndio -lm


.include <bsd.prog.mk>
