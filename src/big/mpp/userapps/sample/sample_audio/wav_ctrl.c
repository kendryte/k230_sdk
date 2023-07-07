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

#include "wav_ctrl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_wav_header(const char *filename, int *header_len, int *data_len,int* channel,int* samplerate,int* bitpersample) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return -1;
    }

    char chunk_id[5];
    k_u32 chunk_size;
    char format[5];

    // 读取RIFF chunk
    fread(chunk_id, 1, 4, fp);
    chunk_id[4] = '\0';
    fread(&chunk_size, 4, 1, fp);
    fread(format, 1, 4, fp);
    format[4] = '\0';

    // 读取fmt chunk
    fread(chunk_id, 1, 4, fp);
    fread(&chunk_size, 4, 1, fp);
    k_u16 audio_format;
    k_u16 num_channels;
    k_u32 sample_rate;
    k_u32 byte_rate;
    k_u16 block_align;
    k_u16 bits_per_sample;
    fread(&audio_format, 2, 1, fp);
    fread(&num_channels, 2, 1, fp);
    fread(&sample_rate, 4, 1, fp);
    fread(&byte_rate, 4, 1, fp);
    fread(&block_align, 2, 1, fp);
    fread(&bits_per_sample, 2, 1, fp);

    *channel = num_channels;
    *samplerate = sample_rate;
    *bitpersample = bits_per_sample;

    // 读取附加信息
    if (chunk_size > 16) {
        fseek(fp, chunk_size - 16, SEEK_CUR);
    }

    // 读取data chunk
    while(1)
    {
        fread(chunk_id, 1, 4, fp);
        if (strcmp(chunk_id,"data") !=0)
        {
            fseek(fp, -2L, SEEK_CUR);
            continue;
        }
        fread(&chunk_size, 4, 1, fp);
       // printf("chunk_id:%s,chunk_size:%d\n",chunk_id,chunk_size);
        break;
    }

    // 计算头信息长度和数据占用长度
    *header_len = ftell(fp);
    *data_len = chunk_size;

    fclose(fp);
    return 0;
}



static WAV g_wav_head;
k_s32 load_wav_head(const char* filename,int* channel,int* samplerate,int* bitpersample,int* total_sample_size)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return -1;
    }
    fread(&g_wav_head, 1, sizeof(g_wav_head), file);
    fclose(file);

    if (0 != memcmp(g_wav_head.riff.RIFF_type, "WAVE",sizeof(g_wav_head.riff.RIFF_type)) || 0 != memcmp(g_wav_head.riff.RIFF_id, "RIFF",sizeof(g_wav_head.riff.RIFF_id)))
    {
        printf("Error: WAV file format not recognized\n");
        return -1;
    }

    *channel = g_wav_head.fmt.FMT_numChannels;
    *samplerate = g_wav_head.fmt.FMT_sampleRate;
    *bitpersample = g_wav_head.fmt.FMT_bitsPerSample;
    *total_sample_size = g_wav_head.data.DATA_size;
    return 0;
}

void set_wav_head(int channel,int samplerate,int bitpersample,int total_sample_size,k_u8  *audio_header)
{
    memcpy(g_wav_head.riff.RIFF_id,"RIFF",sizeof(g_wav_head.riff.RIFF_id));
    memcpy(g_wav_head.riff.RIFF_type,"WAVE",sizeof(g_wav_head.riff.RIFF_type));
    //g_wav_head.riff.RIFF_size = total_sample_size;
    g_wav_head.riff.RIFF_size = total_sample_size - 8;

    memcpy(g_wav_head.fmt.FMT_id,"fmt ",sizeof(g_wav_head.fmt.FMT_id));
    g_wav_head.fmt.FMT_size = 16;
    g_wav_head.fmt.FMT_auioFormat = 1;
    g_wav_head.fmt.FMT_numChannels = channel;
    g_wav_head.fmt.FMT_sampleRate = samplerate;
    g_wav_head.fmt.FMT_byteRate = samplerate*channel*bitpersample/8;
    g_wav_head.fmt.FMT_blockAlign = channel*bitpersample/8;
    g_wav_head.fmt.FMT_bitsPerSample = bitpersample;

    memcpy(g_wav_head.data.DATA_id,"data",sizeof(g_wav_head.data.DATA_id));
    g_wav_head.data.DATA_size = total_sample_size-sizeof(WAV);

    memcpy(audio_header,&g_wav_head,sizeof(g_wav_head));
}
