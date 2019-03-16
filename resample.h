/* resample.h 
 */

struct audio_channel;

/* release_audio_channels:
 * free every audio channel previously allocated
 */
extern void release_audio_channels(void);

#define LEFT_SIDE 0
#define RIGHT_SIDE 1
#define NUMBER_SIDES 2
#define BASE_AUDIO 20
#define AUDIO_SIDE (BASE_AUDIO)

/* chan = new_channel_tag_list(prop):
 * allocates a new channel for the current song
 * properties: AUDIO_SIDE LEFT_SIDE (default)/RIGHT_SIDE
 */
extern audio_channel *new_channel_tag_list(tag *prop);

/* set_data_width(side_width, sample_width):
 * accumulated data on each side will have width side_width bits,
 * and each sample will never be greater than sample_width
 */
extern void set_data_width(int side, int sample);

/* resample():
 * send out a batch of samples
 */
extern void resample(void);

/* set_resampling_beat(bpm, a, b):
 * set bpm to bpm, tempo to  a/b
 */
extern void set_resampling_beat(unsigned int bpm, unsigned int a, unsigned int b);

/* prep_sample_info(info):
 * set up secondary data structure for faster resampling
 */
extern void prep_sample_info(sample_info *info);
/* play_note(au, samp, pitch)
 * set audio channel au to play samp at pitch
 */
extern void play_note(audio_channel *au, sample_info *samp, 
pitch pitch);

/* set_play_pitch(au, pitch):
 * set channel au to play at pitch pitch
 */
extern void set_play_pitch(audio_channel *au, pitch pitch);

/* set_play_volume(au, volume):
 * set channel au to play at volume volume
 */
extern void set_play_volume(audio_channel *au, unsigned int volume);

/* set_play_position(au, pos):
 * set position in sample for channel au at given offset
 */
extern void set_play_position(audio_channel *au, size_t position);

