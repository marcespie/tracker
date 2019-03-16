/* notes.h */

typedef unsigned short pitch;
typedef signed short pitch_delta;

const auto NO_NOTE=0;
typedef unsigned char  note;

typedef unsigned char finetune;

extern pitch note2pitch(note note, finetune finetune);
extern note  pitch2note(pitch pitch);
extern pitch round_pitch(pitch pitch, finetune finetune);
extern const char *note2name(note note);

