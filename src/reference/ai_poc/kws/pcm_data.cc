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

#include "pcm_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<wav_ctrl.h>

static k_u32         *g_pcm_file_data = NULL;
static k_u32           g_pcm_file_len = 0;
static k_u32           g_pcm_file_index = 0;


static int g_bitpersample = 0;
static int g_wav_headlen = 0;
static char g_wav_filename[256];



static k_s32  _load_file(const char*filename)
{
    unsigned char *pcm_file_data = NULL;
    FILE *fp = fopen(filename, "rb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    pcm_file_data = (unsigned char *)malloc(size);
    if (pcm_file_data == NULL)
    {
        printf("malloc size %d failed\n", size);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);
    fread(pcm_file_data, size, 1, fp);
    fclose(fp);
    g_pcm_file_len = size - g_wav_headlen;

    g_pcm_file_data = (k_u32 *)(pcm_file_data + g_wav_headlen);

    printf("open file:%s ok,file size:%d,data size:%d,wav header size:%d\n", filename, size,g_pcm_file_len,(int)g_wav_headlen);

    return 0;
}


static k_s32    _get_bit_pcm_data_from_file(const char*filename,k_u32 *pdata, k_u32 data_len)
{

    bool endfile = false;
    for (unsigned int i = 0; i < data_len; i ++)
    {
        pdata[i] = g_pcm_file_data[g_pcm_file_index++];
        if (g_pcm_file_index >= g_pcm_file_len / 4)
        {
            pdata[i] = 0;
            endfile = true;
        }
    }
    if (endfile)
    {
        g_pcm_file_index = 0;
        return 1;
    }
    return 0;
}


k_s32    load_wav_info(const char*filename,int* channel,int* samplerate,int* bitpersample)
{
    int header_len = 0;
    int data_len = 0;
    read_wav_header(filename,&header_len,&data_len,channel,samplerate,bitpersample);
    printf("========read_wav_header:headerlen:%d,channel:%d,samplerate:%d,bitpersample:%d\n",\
    header_len,*channel,*samplerate,*bitpersample);

    g_wav_headlen = header_len;
    g_bitpersample = *bitpersample;
    memcpy(g_wav_filename,filename,strlen(filename)+1);

    if (0 != _load_file(filename))
    {
        return -1;
    }
    return 0;
}

k_s32    get_pcm_data_from_file(k_u32 *pdata, k_u32 data_len)
{
    return _get_bit_pcm_data_from_file(g_wav_filename, pdata, data_len);
    return -1;
}