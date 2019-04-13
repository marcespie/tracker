#ifndef OPENBSD_AUDIO_H
#define OPENBSD_AUDIO_H
/*
 * Copyright (c) 2019 Marc Espie <espie@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* openbsd_audio.h */
/* frequency = open_audio(f, s):
 * try to open audio with a sampling rate of f, and eventually stereo.
 * We get the real frequency back. If we ask for 0, we
 * get the ``preferred'' frequency.
 * Note: we have to close_audio() before we can open_audio() again.
 * Note: even if we don't ask for stereo, we still have to give a
 * right and left sample.
 */
extern unsigned long open_audio(unsigned long f, int s);
/* close_audio():
 * returns the audio to the system control, doing necessary
 * cleanup
 */
extern void close_audio();
/* set_mix(percent): set mix channels level.
 * 0: spatial stereo. 100: mono.
 */
extern void set_mix(int percent);

/* output_samples(l, r, n): outputs a pair of stereo samples.
 * Samples are n bits signed.
 * Output routine should be able to face anything from 16 to 25
 */
extern void output_samples(int32_t left, int32_t right, int n);

/* flush_buffer(): call from time to time, because buffering
 * is done by the program to get better (?) performance.
 */
extern void flush_buffer();

/* discard_buffer(): try to get rid of the buffer contents
 */
extern void discard_buffer();

/* sync_audio(function, f2, parameter):
 * call function(parameter) when audio finally gets to the current point
 * call f2(parameter) if flush is in effect instead
 */
extern void sync_audio(std::function<void()>, std::function<void()>);

#endif
