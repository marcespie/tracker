/* notes.h 
   vi:ts=3 sw=3:
 */

/* $Id: notes.h,v 1.2 1996/04/13 17:16:32 espie Exp espie $
 * $Log: notes.h,v $
 * Revision 1.2  1996/04/13 17:16:32  espie
 * *** empty log message ***
 *
 * Revision 1.1  1996/04/09 21:13:35  espie
 * Initial revision
 *
 */


typedef unsigned short pitch;
typedef signed short pitch_delta;

#define NO_NOTE 0
typedef unsigned char  note;

typedef unsigned char finetune;

XT pitch note2pitch(note note, finetune finetune);
XT note  pitch2note(pitch pitch);
XT pitch round_pitch(pitch pitch, finetune finetune);
XT char *note2name(note note);

