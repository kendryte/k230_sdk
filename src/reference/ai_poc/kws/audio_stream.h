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
#include "mpi_ai_api.h"
#include "mpi_ao_api.h"
#include "mpi_sys_api.h"
#include "k_audio_comm.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "mpi_vb_api.h"
#include "pcm_data.h"

#include <iostream>
#include <vector>


#define AUDIO_PERSEC_DIV_NUM 25
#define SAVE_PCM_SECOND 15

static volatile k_bool g_ao_test_start = K_FALSE;
static volatile k_bool g_ai_to_ao_test_start = K_FALSE;
static volatile k_bool g_ai_bind_ao_test_start = K_FALSE;
static volatile k_bool g_aenc_test_start = K_FALSE;
static volatile k_bool g_adec_test_start = K_FALSE;
static volatile k_bool g_audio_overall_start = K_FALSE;
static volatile k_bool g_enable_audio_codec = K_FALSE;
static k_bool g_vb_init = K_FALSE;

k_s32 audio_sample_exit()
{
    g_ao_test_start = K_FALSE;
    g_ai_to_ao_test_start = K_FALSE;
    g_ai_bind_ao_test_start = K_FALSE;
    g_adec_test_start = K_FALSE;
    g_aenc_test_start = K_FALSE;
    g_audio_overall_start = K_FALSE;

    return K_SUCCESS;
}

k_s32 audio_sample_vb_destroy()
{
    if (!g_vb_init)
    {
        return K_FAILED;
    }
    g_vb_init = K_FALSE;
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

k_s32 audio_sample_enable_audio_codec(k_bool enable_audio_codec)
{
    g_enable_audio_codec = enable_audio_codec;
    return K_SUCCESS;
}

k_s32 audio_sample_vb_init(k_bool enable_cache, k_u32 sample_rate)
{
    if (g_vb_init)
    {
        return K_SUCCESS;
    }
    k_s32 ret;
    k_vb_config config;
    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    config.comm_pool[0].blk_cnt = 50;
    config.comm_pool[0].blk_size = 48000 * 2 * 4;
    config.comm_pool[0].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_cnt = 2;
    config.comm_pool[1].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2; // ao use
    config.comm_pool[1].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;
    config.comm_pool[2].blk_cnt = 1;
    config.comm_pool[2].blk_size = sample_rate * 2 * 4 * (SAVE_PCM_SECOND + 1); // save data to memory ,申请大点(+1s)，否则mmz_userdev_mmap会崩溃,wav文件头
    config.comm_pool[2].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    int blk_total_size = 0;
    for (int i = 0; i < 2; i++)
    {
        blk_total_size += config.comm_pool[i].blk_cnt * config.comm_pool[i].blk_size;
    }
    printf("mmz blk total size:%.2f MB\n", blk_total_size / 1024 / 1024.0);
    ret = kd_mpi_vb_set_config(&config);
    if (ret)
    {
        printf("vb_set_config failed ret:%d\n", ret);
        return ret;
    }
    else
    {
        printf("vb_set_config ok\n");
    }
    ret = kd_mpi_vb_init();
    if (ret)
        printf("vb_init failed ret:%d\n", ret);
    else
        g_vb_init = K_TRUE;
    return ret;
}

static k_vb_blk_handle g_audio_handle;
static k_s32 _get_audio_frame(k_audio_frame *audio_frame, int nSize)
{
    g_audio_handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, nSize, NULL);
    if (g_audio_handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }
    audio_frame->len = nSize;
    audio_frame->pool_id = kd_mpi_vb_handle_to_pool_id(g_audio_handle);
    audio_frame->phys_addr = kd_mpi_vb_handle_to_phyaddr(g_audio_handle);
    audio_frame->virt_addr = kd_mpi_sys_mmap(audio_frame->phys_addr, nSize);
    printf("=======_get_audio_frame virt_addr:%p\n", audio_frame->virt_addr);

    return K_SUCCESS;
}

static k_s32 _release_audio_frame()
{
    kd_mpi_vb_release_block(g_audio_handle);
    return K_SUCCESS;
}


k_s32 start_aio_stream(bool enable_ai, bool enable_ao)
{
    k_u32 sample_rate = 16000;
    k_u32 channel_count = 1;
    k_u32 enable_audio3a = 0;

    audio_sample_enable_audio_codec(K_TRUE);
    audio_sample_vb_init(K_TRUE, sample_rate);

    if (enable_ai)
    {
        k_aio_dev_attr ai_dev_attr;
        ai_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
        ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
        ai_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
        ai_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 1;
        ai_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
        ai_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
        ai_dev_attr.kd_audio_attr.i2s_attr.frame_num = 25;
        ai_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = 4800;
        ai_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;

        if (K_SUCCESS != kd_mpi_ai_set_pub_attr(0, &ai_dev_attr))
        {
            std::cout << "kd_mpi_ai_set_pub_attr failed\n" << std::endl;
            return K_FAILED;
        }

        kd_mpi_ai_enable(0);
        kd_mpi_ai_enable_chn(0, 0);
    }

    if (enable_ao)
    {
        k_aio_dev_attr ao_dev_attr;
        ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
        ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
        ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
        ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
        ao_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
        ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
        ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = 25;
        ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
        ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;

        if (K_SUCCESS != kd_mpi_ao_set_pub_attr(0, &ao_dev_attr))
        {
            std::cout << "kd_mpi_ao_set_pub_attr failed\n" << std::endl;
            return K_FAILED;
        }

        kd_mpi_ao_enable(0);
        kd_mpi_ao_enable_chn(0, 0);
    }
    

    if (enable_audio3a)
    {
        k_ai_vqe_enable vqe_enable;
        memset(&vqe_enable, 0, sizeof(k_ai_vqe_enable));
        if (enable_audio3a&0x1)
        {
            vqe_enable.ans_enable = K_TRUE;
            printf("ans_enable\n");
        }
        if (enable_audio3a&0x2)
        {
            vqe_enable.agc_enable = K_TRUE;
            printf("agc_enable\n");
        }
        if (enable_audio3a&0x4)
        {
            vqe_enable.aec_enable = K_TRUE;
            printf("aec_enable\n");
        }

        if (K_SUCCESS != kd_mpi_ai_set_vqe_attr(0, 0, vqe_enable))
        {
            printf("kd_mpi_ai_set_vqe_attr failed\n");
            return K_FAILED;
        }
    }
    
    return K_SUCCESS;
}

k_s32 stop_aio_stream(bool disable_ai, bool disable_ao)
{
    if (disable_ai)
    {
        kd_mpi_ai_disable_chn(0, 0);
        kd_mpi_ai_disable(0);
    }

    if (disable_ao)
    {
        kd_mpi_ao_disable_chn(0, 0);
        kd_mpi_ao_disable(0);
    }

    audio_sample_exit();
    audio_sample_vb_destroy();
    return K_SUCCESS;
}

k_s32 play_wav(const char *filename)
{
    int audio_channel = 0;
    int audio_samplerate = 0;
    int audio_bitpersample = 0;

    if (0 != load_wav_info(filename, &audio_channel, &audio_samplerate, &audio_bitpersample))
    {
        return -1;
    }

    if (audio_samplerate > 16000) // Prevent blocks without corresponding sizes
    {
        printf("error,please input: -samplerate %d\n", audio_samplerate);
        return -1;
    }

    k_audio_frame audio_frame;
    _get_audio_frame(&audio_frame, audio_samplerate * 2 * 2 / AUDIO_PERSEC_DIV_NUM);


    static int nCount = 0;
    k_s32 ret = 0;
    g_ao_test_start = K_TRUE;

    while (g_ao_test_start)
    {
        // 获取音频数据
        k_s32 flag;
        flag = get_pcm_data_from_file((k_u32 *)audio_frame.virt_addr, audio_frame.len / 4);
        if (flag == 1)
        {
            g_ao_test_start = K_FALSE;
        }
        ret = kd_mpi_ao_send_frame(0, 0, &audio_frame, 1000);
        if (ret == 0)
        {
            nCount++;
        }
    }

    _release_audio_frame();
    return K_SUCCESS;
}


std::pair<k_u16 *, k_u32> get_audio_chunk()
{
    k_audio_frame audio_frame;

    // 获取音频流数据
    if (K_SUCCESS != kd_mpi_ai_get_frame(0, 0, &audio_frame, 1000))
    {
        std::cout << "=========kd_mpi_ai_get_frame timeout\n" << std::endl;
    }

    k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_frame.phys_addr, audio_frame.len);
    k_u16 *pcm_data = (k_u16 *)raw_data;
    k_u32  pcm_size = audio_frame.len / 2;

    kd_mpi_ai_release_frame(0, 0, &audio_frame);

    return std::make_pair(pcm_data, pcm_size);
}
