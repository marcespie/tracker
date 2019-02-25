/* resample.h 
	vi:ts=3 sw=3:
 */

/* $Id: resample.h,v 1.5 1996/05/07 15:22:54 espie Exp espie $
 * $Log: resample.h,v $
 * Revision 1.5  1996/05/07 15:22:54  espie
 * *** empty log message ***
 *
 * Revision 1.4  1996/04/12 16:31:37  espie
 * *** empty log message ***
 *
 * Revision 1.3  1996/04/09 21:14:18  espie
 * *** empty log message ***
 *
 */

struct audio_channel;

/* release_audio_channels:
 * free every audio channel previously allocated
 */
XT void release_audio_channels(void);

#define LEFT_SIDE 0
#define RIGHT_SIDE 1
#define NUMBER_SIDES 2
#define BASE_AUDIO 20
#define AUDIO_SIDE (BASE_AUDIO)

/* chan = new_channel_tag_list(prop):
 * allocates a new channel for the current song
 * properties: AUDIO_SIDE LEFT_SIDE (default)/RIGHT_SIDE
 */
XT struct audio_channel *new_channel_tag_list(struct tag *prop);

/* set_data_width(side_width, sample_width):
 * accumulated data on each side will have width side_width bits,
 * and each sample will never be greater than sample_width
 */
XT void set_data_width(int side, int sample);

/* resample():
 * send out a batch of samples
 */
XT void resample(void);

/* set_resampling_beat(bpm, a, b):
 * set bpm to bpm, tempo to  a/b
 */
XT void set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b);

/* prep_sample_info(info):
 * set up secondary data structure for faster resampling
 */
XT void prep_sample_info(struct sample_info *info);
/* play_note(au, samp, pitch)
 * set audio channel au to play samp at pitch
 */
XT void play_note(struct audio_channel *au, struct sample_info *samp, 
pitch pitch);

/* set_play_pitch(au, pitch):
 * set channel au to play at pitch pitch
 */
XT void set_play_pitch(struct audio_channel *au, pitch pitch);

/* set_play_volume(au, volume):
 * set channel au to play at volume volume
 */
XT void set_play_volume(struct audio_channel *au, unsigned int volume);

/* set_play_position(au, pos):
 * set position in sample for channel au at given offset
 */
XT void set_play_position(struct audio_channel *au, size_t position);

