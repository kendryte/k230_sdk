/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __AUDIO_WAV_CTRL_H__
#define __AUDIO_WAV_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#include "k_type.h"
#include "k_audio_comm.h"

typedef struct WAV_RIFF
{
    k_s8 RIFF_id[4];
    k_u32 RIFF_size;
    k_s8 RIFF_type[4];
} RIFF_t;

typedef struct WAV_FMT
{
    k_s8 FMT_id[4];
    k_u32 FMT_size;
    k_u16 FMT_auioFormat;
    k_u16 FMT_numChannels;
    k_u32 FMT_sampleRate;
    k_u32 FMT_byteRate;
    k_u16 FMT_blockAlign;
    k_u16 FMT_bitsPerSample;
} FMT_t;

typedef struct WAV_data
{
    k_s8 DATA_id[4];
    k_u32 DATA_size;
} Data_t;


typedef struct WAV_fotmat
{
    RIFF_t riff;
    FMT_t fmt;
    Data_t data;
} WAV;

void  set_wav_head(int channel,int samplerate,int bitpersample,int total_sample_size,k_u8  *audio_header);
k_s32 load_wav_head(const char* filename,int* channel,int* samplerate,int* bitpersample,int* total_sample_size);
int   read_wav_header(const char *filename, int *header_len, int *data_len,int* channel,int* samplerate,int* bitpersample);
#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif
