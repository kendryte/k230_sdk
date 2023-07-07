/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-05-09     Urey         first version
 * 2019-07-09     Zero-Free    improve device ops interface and data flows
 *
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

/* AUDIO command */
#define _AUDIO_CTL(a) (0x10 + a)

#define AUDIO_CTL_GETCAPS                   _AUDIO_CTL(1)
#define AUDIO_CTL_CONFIGURE                 _AUDIO_CTL(2)
#define AUDIO_CTL_START                     _AUDIO_CTL(3)
#define AUDIO_CTL_STOP                      _AUDIO_CTL(4)
#define AUDIO_CTL_GETBUFFERINFO             _AUDIO_CTL(5)

/* Audio Device Types */
#define AUDIO_TYPE_QUERY                    0x00
#define AUDIO_TYPE_INPUT                    0x01
#define AUDIO_TYPE_OUTPUT                   0x02
#define AUDIO_TYPE_MIXER                    0x04

/* Supported Sampling Rates */
#define AUDIO_SAMP_RATE_8K                  0x0001
#define AUDIO_SAMP_RATE_11K                 0x0002
#define AUDIO_SAMP_RATE_16K                 0x0004
#define AUDIO_SAMP_RATE_22K                 0x0008
#define AUDIO_SAMP_RATE_32K                 0x0010
#define AUDIO_SAMP_RATE_44K                 0x0020
#define AUDIO_SAMP_RATE_48K                 0x0040
#define AUDIO_SAMP_RATE_96K                 0x0080
#define AUDIO_SAMP_RATE_128K                0x0100
#define AUDIO_SAMP_RATE_160K                0x0200
#define AUDIO_SAMP_RATE_172K                0x0400
#define AUDIO_SAMP_RATE_192K                0x0800

/* Supported Bit Rates */
#define AUDIO_BIT_RATE_22K                  0x01
#define AUDIO_BIT_RATE_44K                  0x02
#define AUDIO_BIT_RATE_48K                  0x04
#define AUDIO_BIT_RATE_96K                  0x08
#define AUDIO_BIT_RATE_128K                 0x10
#define AUDIO_BIT_RATE_160K                 0x20
#define AUDIO_BIT_RATE_172K                 0x40
#define AUDIO_BIT_RATE_192K                 0x80

/* Support Dsp(input/output) Units controls */
#define AUDIO_DSP_PARAM                     0           /* get/set all params */
#define AUDIO_DSP_SAMPLERATE                1           /* samplerate */
#define AUDIO_DSP_CHANNELS                  2           /* channels */
#define AUDIO_DSP_SAMPLEBITS                3           /* sample bits width */

/* Supported Mixer Units controls */
#define AUDIO_MIXER_QUERY                   0x0000
#define AUDIO_MIXER_MUTE                    0x0001
#define AUDIO_MIXER_VOLUME                  0x0002
#define AUDIO_MIXER_BASS                    0x0004
#define AUDIO_MIXER_MID                     0x0008
#define AUDIO_MIXER_TREBLE                  0x0010
#define AUDIO_MIXER_EQUALIZER               0x0020
#define AUDIO_MIXER_LINE                    0x0040
#define AUDIO_MIXER_DIGITAL                 0x0080
#define AUDIO_MIXER_MIC                     0x0100
#define AUDIO_MIXER_VITURAL                 0x0200
#define AUDIO_MIXER_EXTEND                  0x8000    /* extend mixer command */

struct rt_audio_configure
{
    rt_uint32_t samplerate;
    rt_uint16_t channels;
    rt_uint16_t samplebits;
};

struct rt_audio_caps
{
    int main_type;
    int sub_type;

    union
    {
        rt_uint32_t mask;
        int     value;
        struct rt_audio_configure config;
    } udata;
};

#endif /* __AUDIO_H__ */
