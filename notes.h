/* notes.h */

using pitch =unsigned short;
using pitch_delta=signed short;

const auto NO_NOTE = 0;
using note = unsigned char;
using finetune = unsigned char;

extern pitch note2pitch(note note, finetune finetune);
extern note  pitch2note(pitch pitch);
extern pitch round_pitch(pitch pitch, finetune finetune);
extern const char *note2name(note note);

