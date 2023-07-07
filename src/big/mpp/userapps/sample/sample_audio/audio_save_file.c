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

#include "audio_save_file.h"
#include<stdio.h>
#include<pthread.h>
#include <time.h>
#include <string.h>

#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"


#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "wav_ctrl.h"


static k_u32  *g_save_32_data = NULL;
static k_u16  *g_save_16_data = NULL;
static k_u8  *g_save_24_data = NULL;//Assignment in byte
static k_u32    g_save_data_index = 0;
static k_bool g_save_over = K_FALSE;
static k_u64  g_file_phys_addr;
static k_u32  g_total_size = 0;
static int    g_sample_bit_size = 0;
static int    g_sample_rate = 0;
static int    g_sample_channel = 0;
static k_vb_blk_handle g_vb_handle;
static int    g_save_count = 0;
static char   g_filename[256];
k_s32 audio_save_init(const char* filename,int sample_rate, int channel_count, k_audio_bit_width bit_width, int total_sec)
{
    strcpy(g_filename,filename);
    g_save_data_index = 0;
    int sample_bit_size = 0;
    int channel = channel_count;
    if (KD_AUDIO_BIT_WIDTH_16 == bit_width)
    {
        sample_bit_size = 16;
    }
    else if (KD_AUDIO_BIT_WIDTH_24 == bit_width)
    {
        sample_bit_size = 24;
    }
    else if (KD_AUDIO_BIT_WIDTH_32 == bit_width)
    {
        sample_bit_size = 32;
    }

    int nTotalSize = sample_rate * channel * sample_bit_size / 8 * total_sec + sizeof(WAV);
    printf("%s get vb block size:%d\n", __FUNCTION__, nTotalSize);
    g_vb_handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, nTotalSize, NULL);
    if (VB_INVALID_HANDLE == g_vb_handle)
    {
        printf("get vb block size:%d failed\n", nTotalSize);
        return -1;
    }

    g_total_size = nTotalSize;
    g_sample_bit_size = sample_bit_size;
    g_sample_rate = sample_rate;
    g_sample_channel = channel;
    g_file_phys_addr = kd_mpi_vb_handle_to_phyaddr(g_vb_handle);

    if (KD_AUDIO_BIT_WIDTH_16 == bit_width)
    {
        g_save_16_data = (k_u16 *)kd_mpi_sys_mmap(g_file_phys_addr, nTotalSize);
    }
    else if (KD_AUDIO_BIT_WIDTH_32 == bit_width)
    {
        g_save_32_data = (k_u32 *)kd_mpi_sys_mmap(g_file_phys_addr, nTotalSize);
    }
    else if (KD_AUDIO_BIT_WIDTH_24 == bit_width)
    {
        g_save_24_data = (k_u8 *)kd_mpi_sys_mmap(g_file_phys_addr, nTotalSize);
    }
    printf("======kd_mpi_sys_mmap total size:%d\n", g_total_size);

    return 0;
}

k_s32 _save_file(const char*filename,unsigned char*pdata,int len)
{
    FILE *fp = fopen(filename, "wb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filename);
        return -1;
    }

    fwrite(pdata,len,1,fp);

    fclose(fp);
    return 0;
}

k_s32 audio_save_pcm_memory(k_u8 *data, int size)
{
    if (g_save_over)
    {
        return -1;
    }


    if (16 == g_sample_bit_size)
    {
        k_u16 *pcm_data = (k_u16 *)data;
        k_u32  pcm_size = size / 2;

        for (unsigned int i = 0; i < pcm_size; i ++)
        {
            g_save_16_data[sizeof(WAV)/2+g_save_data_index++] = pcm_data[i];
            if (g_save_data_index >= (g_total_size-sizeof(WAV)) / 2)
            {
                set_wav_head(g_sample_channel,g_sample_rate,g_sample_bit_size,g_total_size,(k_u8*)g_save_16_data);
                printf("dump binary memory test%d.wav %p %p\n",++g_save_count, (k_u32 *)g_file_phys_addr, (k_u8*)g_file_phys_addr + g_total_size);
                _save_file(g_filename,(unsigned char*)g_save_16_data,g_total_size);
                g_save_over = K_TRUE;
                kd_mpi_sys_munmap(g_save_16_data,g_total_size);
                kd_mpi_vb_release_block(g_vb_handle);
                g_save_data_index = 0;
                return 0;
            }
        }
    }
    else if (24 == g_sample_bit_size)
    {
        k_u8 *pcm_data = (k_u8 *)data;
        k_u32  pcm_size = size;

        for (unsigned int i = 0; i < pcm_size; i ++)
        {

            g_save_24_data[sizeof(WAV)+g_save_data_index++] = pcm_data[i];
            if (g_save_data_index >= g_total_size-sizeof(WAV))
            {
                set_wav_head(g_sample_channel,g_sample_rate,g_sample_bit_size,g_total_size,(k_u8*)g_save_24_data);
                printf("dump binary memory test%d.wav %p %p\n",++g_save_count, (k_u32 *)g_file_phys_addr, (k_u8*)g_file_phys_addr + g_total_size);
                _save_file(g_filename,(unsigned char*)g_save_24_data,g_total_size);
                g_save_over = K_TRUE;
                kd_mpi_sys_munmap(g_save_24_data,g_total_size);
                kd_mpi_vb_release_block(g_vb_handle);
                g_save_data_index = 0;
                return 0;
            }
        }
    }
    else if (32 == g_sample_bit_size)
    {
        k_u32 *pcm_data = (k_u32 *)data;
        k_u32  pcm_size = size / 4;

        for (unsigned int i = 0; i < pcm_size; i ++)
        {

            g_save_32_data[sizeof(WAV)/4+g_save_data_index++] = pcm_data[i];
            if (g_save_data_index >= (g_total_size-sizeof(WAV)) / 4)
            {
                set_wav_head(g_sample_channel,g_sample_rate,g_sample_bit_size,g_total_size,(k_u8*)g_save_32_data);
                printf("dump binary memory test%d.wav %p %p\n",++g_save_count, (k_u32 *)g_file_phys_addr, (k_u8*)g_file_phys_addr + g_total_size);
                _save_file(g_filename,(unsigned char*)g_save_32_data,g_total_size);
                g_save_over = K_TRUE;
                kd_mpi_sys_munmap(g_save_32_data,g_total_size);
                kd_mpi_vb_release_block(g_vb_handle);
                g_save_data_index = 0;
                return 0;
            }
        }
    }
    return 0;
}
