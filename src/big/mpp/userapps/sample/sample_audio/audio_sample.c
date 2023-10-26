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

#include "audio_sample.h"

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"
#include "k_acodec_comm.h"

#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <list>
using namespace std;
typedef struct
{
    k_audio_stream stream;
    k_vb_blk_handle handle;
} k_audio_sample;

typedef std::list<k_audio_sample> AUDIO_STREAM_LIST;

#include <unistd.h>
#include "mpi_ai_api.h"
#include "mpi_ao_api.h"
#include "mpi_aenc_api.h"
#include "mpi_adec_api.h"
#include "audio_save_file.h"
#include "pcm_data.h"

// pdm pin cfg
#define AUDIO_PERSEC_DIV_NUM 25

// 保存pcm相关宏控制参数
#define ENABLE_SAVE_PCM 1
#define SAVE_PCM_SECOND 15

static volatile k_bool g_ao_test_start = K_FALSE;
static volatile k_bool g_ai_to_ao_test_start = K_FALSE;
static volatile k_bool g_ai_bind_ao_test_start = K_FALSE;
static volatile k_bool g_aenc_test_start = K_FALSE;
static volatile k_bool g_adec_test_start = K_FALSE;
static volatile k_bool g_audio_overall_start = K_FALSE;
static volatile k_bool g_enable_audio_codec = K_FALSE;

static volatile uint32_t g_audioout_timestamp_end = 0;
static volatile uint32_t g_audioout_timestamp_last = 0;

static int _get_sample_Byte(k_audio_bit_width bit_width)
{
    if (bit_width == KD_AUDIO_BIT_WIDTH_16)
    {
        return 2;
    }
    else if (bit_width == KD_AUDIO_BIT_WIDTH_24)
    {
        return 3;
    }
    else if (bit_width == KD_AUDIO_BIT_WIDTH_32)
    {
        return 4;
    }
    return 0;
}
static k_audio_bit_width _get_sample_bitwidth(k_u32 bitpersample)
{
    if (16 == bitpersample)
    {
        return KD_AUDIO_BIT_WIDTH_16;
    }
    else if (24 == bitpersample)
    {
        return KD_AUDIO_BIT_WIDTH_24;
    }
    else if (32 == bitpersample)
    {
        return KD_AUDIO_BIT_WIDTH_32;
    }

    return KD_AUDIO_BIT_WIDTH_32;
}
static void _test_timestamp(int sample_rate, int channel_count, k_audio_bit_width bit_width, k_u64 timestamp, k_u32 len)
{
    static k_s32 recv_size = 0;
    static k_s32 total_sec = 0;
    static k_u64 start_time = 0;
    static k_bool bstart = K_TRUE;

    recv_size += len;
    if (recv_size >= sample_rate * _get_sample_Byte(bit_width) * channel_count)
    {
        if (bstart)
        {
            start_time = timestamp;
            bstart = K_FALSE;
        }

        recv_size = 0;
        printf("[%ds] timestamp %ld us,curpts:%ld\n", total_sec++, timestamp - start_time, timestamp);
    }
}

static void _test_aenc_timestamp(int sample_rate, int channel_count, k_audio_bit_width bit_width, k_u64 timestamp, k_u32 len)
{
    static k_s32 recv_size = 0;
    static k_s32 total_sec = 0;
    static k_u64 start_time = 0;
    static k_bool bstart = K_TRUE;

    recv_size += len;
    if (recv_size >= sample_rate * _get_sample_Byte(bit_width) * channel_count / 2) // g711压缩率2倍
    {
        if (bstart)
        {
            start_time = timestamp;
            bstart = K_FALSE;
        }

        recv_size = 0;
        printf("[%ds] g711 stream timestamp %ld us,curpts:%ld\n", total_sec++, timestamp - start_time, timestamp);
    }
}

static k_s32 _sample_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static void _test_ai_i2s_in_data(const char *filename, int dev_num, int channel, k_audio_bit_width bit_width, int sample_rate,int channel_count, int nTotal_Sec)
{
    k_audio_frame audio_frame;
    int nSize = 0;
    int nSec = 0;

#if ENABLE_SAVE_PCM
    audio_save_init(filename, sample_rate, channel_count, bit_width, SAVE_PCM_SECOND);
#endif

    while (1)
    {
        if (K_SUCCESS != kd_mpi_ai_get_frame(dev_num, channel, &audio_frame, 1000))
        {
            printf("=========kd_mpi_ai_get_frame timeout\n");
            continue;
        }

        nSize += audio_frame.len;

        if (bit_width == KD_AUDIO_BIT_WIDTH_16)
        {
#if ENABLE_SAVE_PCM
            {
                k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_frame.phys_addr, audio_frame.len);
                // printf("raw_data:%p , audio_frame.len:%d\n", raw_data, audio_frame.len);
                if (0 != audio_save_pcm_memory(raw_data, audio_frame.len))
                {
                    kd_mpi_ai_release_frame(dev_num, channel, &audio_frame);
                    break;
                }
            }
#endif
            _test_timestamp(sample_rate, channel_count, bit_width, audio_frame.time_stamp, audio_frame.len);
        }
        else if (bit_width == KD_AUDIO_BIT_WIDTH_24)
        {
#if ENABLE_SAVE_PCM
            {
                k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_frame.phys_addr, audio_frame.len);
                // printf("raw_data:%p , audio_frame.len:%d\n", raw_data, audio_frame.len);
                if (0 != audio_save_pcm_memory(raw_data, audio_frame.len))
                {
                    kd_mpi_ai_release_frame(dev_num, channel, &audio_frame);
                    break;
                }
            }
#endif
            _test_timestamp(sample_rate, channel_count, bit_width, audio_frame.time_stamp, audio_frame.len);
        }
        else if (bit_width == KD_AUDIO_BIT_WIDTH_32)
        {
#if ENABLE_SAVE_PCM
            {
                k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_frame.phys_addr, audio_frame.len);
                // printf("raw_data:%p , audio_frame.len:%d\n", raw_data, audio_frame.len);
                if (0 != audio_save_pcm_memory(raw_data, audio_frame.len))
                {
                    kd_mpi_ai_release_frame(dev_num, channel, &audio_frame);
                    break;
                }
            }
#endif

            _test_timestamp(sample_rate, channel_count, bit_width, audio_frame.time_stamp, audio_frame.len);
        }

        kd_mpi_ai_release_frame(dev_num, channel, &audio_frame);

        if (nTotal_Sec > 0 && nSec >= nTotal_Sec)
        {
            printf("==========_test_ai_i2s_in_data return \n");
            return;
        }
    }
}

static void _test_ai_pdm_in_data(const char *filename, int dev_num, int channel, k_audio_bit_width bit_width, k_u32 sample_rate, int channel_count, int nTotal_Sec)
{
    k_audio_frame audio_frame;
    int nSize = 0;
    int nSec = 0;

#if ENABLE_SAVE_PCM
    audio_save_init(filename, sample_rate, channel_count, bit_width, SAVE_PCM_SECOND);
#endif

    while (1)
    {
        if (K_SUCCESS != kd_mpi_ai_get_frame(dev_num, channel, &audio_frame, 1000))
        {
            printf("=========kd_mpi_ai_get_frame timeout\n");
            continue;
        }

        nSize += audio_frame.len;

#if ENABLE_SAVE_PCM
        {
            k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_frame.phys_addr, audio_frame.len);
            if (0 != audio_save_pcm_memory(raw_data, audio_frame.len))
            {
                kd_mpi_ai_release_frame(dev_num, channel, &audio_frame);
                break;
            }
        }
#endif
        _test_timestamp(sample_rate, channel_count, bit_width, audio_frame.time_stamp, audio_frame.len);

        kd_mpi_ai_release_frame(dev_num, channel, &audio_frame);

        if (nTotal_Sec > 0 && nSec >= nTotal_Sec)
        {
            printf("==========_test_ai_pdm_in_data return \n");
            return;
        }
    }
}

static void _test_ai_to_ao(int ai_dev_num, int ai_channel, int ao_dev_num, int ao_channel, int sample_rate, k_audio_bit_width bit_width)
{
    k_audio_frame audio_frame;
    k_s32 ret = 0;
    while (g_ai_to_ao_test_start)
    {
        ret = kd_mpi_ai_get_frame(ai_dev_num, ai_channel, &audio_frame, 1000);
        if (K_SUCCESS != ret)
        {
            printf("=========kd_mpi_ai_get_frame timeout\n");
            continue;
        }

        _test_timestamp(sample_rate, 2, bit_width, audio_frame.time_stamp, audio_frame.len);

        ret = kd_mpi_ao_send_frame(ao_dev_num, ao_channel, &audio_frame, 0); // 尽快返回，防止阻塞kd_mpi_ai_get_frame
        if (K_SUCCESS != ret)
        {
            printf("=======kd_mpi_ao_send_frame failed\n");
        }

        kd_mpi_ai_release_frame(ai_dev_num, ai_channel, &audio_frame);
    }
}

static void _ai_bind_ao(int ai_dev_num, int ai_channel, int ao_dev_num, int ao_channel)
{
    k_mpp_chn ai_mpp_chn;
    k_mpp_chn ao_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev_num;
    ai_mpp_chn.chn_id = ai_channel;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev_num;
    ao_mpp_chn.chn_id = ao_channel;

    kd_mpi_sys_bind(&ai_mpp_chn, &ao_mpp_chn);
    while (g_ai_bind_ao_test_start)
    {
        sleep(1);
    }

    kd_mpi_sys_unbind(&ai_mpp_chn, &ao_mpp_chn);
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

#if 0
static void  test_get_ai_data()
{

    {
        k_u32 pool_id = -1;

        k_vb_blk_handle handle;
        handle = kd_mpi_vb_get_block(pool_id, 44100 * 2 * 4 / 25, NULL);
        printf("vb_get_block id:%d size:0x800 handle:%08x\n",
               pool_id, handle);

    }



    printf("========%s  111\n", __FUNCTION__);
    int nRet;
    k_s32 AiFd;

    fd_set read_fds;
    struct timeval TimeoutVal;

    FD_ZERO(&read_fds);
    AiFd = kd_mpi_ai_get_fd(100, 200);
    printf("=========kd_mpi_ai_get_fd get fd:%dsh \n", AiFd);
    FD_SET(AiFd, &read_fds);

    k_audio_frame audio_frame;
    //int nCount = 0;
    while (1)
    {
        TimeoutVal.tv_sec = 1;
        TimeoutVal.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(AiFd, &read_fds);

        nRet = select(AiFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (nRet < 0)
        {
            break;
        }
        else if (0 == nRet)
        {
            printf("%s: get ai frame select time out\n", __FUNCTION__);
            continue;
        }

        if (FD_ISSET(AiFd, &read_fds))
        {
            nRet = kd_mpi_ai_get_frame(100, 200, &audio_frame, 0);


            //printf("[%d]=========kd_mpi_ai_get_frame:%d\n",nCount++,nRet);
            if (nRet == K_SUCCESS)
            {
                //          printf("=========phys_addr:0x%x,virt_addr:0x%x\n",audio_frame.phys_addr,audio_frame.virt_addr);

                k_u8 *pVirtualAddr = (k_u8 *)kd_mpi_sys_mmap(audio_frame.phys_addr, audio_frame.len);
                printf("=========%d_%d_%d_%d\n", pVirtualAddr[0], pVirtualAddr[1], pVirtualAddr[2], pVirtualAddr[3]);
                kd_mpi_sys_munmap(pVirtualAddr, audio_frame.len);

                // printf("=========%d_%d_%d_%d\n",pData[0],pData[1],pData[2],pData[3]);
                kd_mpi_ai_release_frame(100, 200, &audio_frame);
            }
        }

    }
}
#endif

static k_bool g_vb_init = K_FALSE;
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

    config.comm_pool[0].blk_cnt = 150;
    config.comm_pool[0].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    config.comm_pool[0].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config.comm_pool[1].blk_cnt = 2;
    config.comm_pool[1].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2; // ao use
    config.comm_pool[1].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config.comm_pool[2].blk_cnt = 1;
    config.comm_pool[2].blk_size = sample_rate * 2 * 4 * (SAVE_PCM_SECOND + 1); // save data to memory ,申请大点(+1s)，否则mmz_userdev_mmap会崩溃,wav文件头
    config.comm_pool[2].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    int blk_total_size = 0;
    for (int i = 0; i < 3; i++)
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

k_s32 audio_sample_vb_destroy()
{
    if (!g_vb_init)
    {
        return K_FAILED;
    }
    g_vb_init = K_FALSE;

    return _sample_vb_exit();
}

k_s32 audio_sample_enable_audio_codec(k_bool enable_audio_codec)
{
    g_enable_audio_codec = enable_audio_codec;
    return K_SUCCESS;
}
k_s32 audio_sample_get_ai_i2s_data(const char *filename, k_audio_bit_width bit_width, k_u32 sample_rate,k_u32 channel_count,k_i2s_in_mono_channel  mono_channel, k_i2s_work_mode i2s_work_mode, k_bool enable_audio3a)
{
    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1 == channel_count) ? KD_AUDIO_SOUND_MODE_MONO : KD_AUDIO_SOUND_MODE_STEREO;
    if(1 == channel_count)
    {
        aio_dev_attr.kd_audio_attr.i2s_attr.mono_channel = mono_channel;
    }
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(0, &aio_dev_attr))
    {
        printf("kd_mpi_ai_set_pub_attr failed\n");
        return K_FAILED;
    }

    if (enable_audio3a)
    {
        if (K_SUCCESS != kd_mpi_ai_set_vqe_attr(0, 0, K_TRUE))
        {
            printf("kd_mpi_ai_set_vqe_attr failed\n");
            return K_FAILED;
        }
    }

    kd_mpi_ai_enable(0);
    kd_mpi_ai_enable_chn(0, 0);
    // kd_mpi_ai_enable_chn(0,1);

    _test_ai_i2s_in_data(filename, 0, 0, bit_width, sample_rate, channel_count,-1);

    // exit
    kd_mpi_ai_disable_chn(0, 0);
    kd_mpi_ai_disable(0);

    return K_SUCCESS;
}

k_s32 audio_sample_get_ai_pdm_data(const char *filename, k_audio_bit_width bit_width, k_u32 sample_rate, k_u32 channel_count)
{
    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_PDM;
    aio_dev_attr.kd_audio_attr.pdm_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.pdm_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.pdm_attr.chn_cnt = 4; // max pdm channel
    aio_dev_attr.kd_audio_attr.pdm_attr.snd_mode = (1 == channel_count) ? KD_AUDIO_SOUND_MODE_MONO : KD_AUDIO_SOUND_MODE_STEREO;
    aio_dev_attr.kd_audio_attr.pdm_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.pdm_attr.pdm_oversample = KD_AUDIO_PDM_INPUT_OVERSAMPLE_64;
    aio_dev_attr.kd_audio_attr.pdm_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.pdm_attr.frame_num;

    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(1, &aio_dev_attr))
    {
        printf("kd_mpi_ai_set_pub_attr failed\n");
        return K_FAILED;
    }
#if 0
    int channel_total = 1;
    int rec_channel = 0;
#else
    int channel_total = 1;
    int rec_channel = 0;
#endif

    kd_mpi_ai_enable(1);
    for (int i = 0; i < channel_total; i++)
    {
        kd_mpi_ai_enable_chn(1, i);
    }

    _test_ai_pdm_in_data(filename, 1, rec_channel, bit_width, sample_rate, channel_count, -1);

    // exit
    for (int i = channel_total - 1; i >= 0; i--)
    {
        kd_mpi_ai_disable_chn(1, i);
    }

    kd_mpi_ai_disable(1);

    return K_SUCCESS;
}

k_s32 audio_sample_send_ao_data(const char *filename, int nDev, int nChannel, int samplerate, k_audio_bit_width bit_width, k_i2s_work_mode i2s_work_mode)
{
    int audio_channel = 0;
    int audio_samplerate = 0;
    int audio_bitpersample = 0;
    if (0 != load_wav_info(filename, &audio_channel, &audio_samplerate, &audio_bitpersample))
    {
        return -1;
    }

    if (audio_samplerate > samplerate) // Prevent blocks without corresponding sizes
    {
        printf("error,please input: -samplerate %d\n", audio_samplerate);
        return -1;
    }

    k_audio_frame audio_frame;
    if (32 == audio_bitpersample)
    {
        _get_audio_frame(&audio_frame, audio_samplerate * 4 * 2 / AUDIO_PERSEC_DIV_NUM);
    }
    else if (24 == audio_bitpersample)
    {
        _get_audio_frame(&audio_frame, audio_samplerate * 3 * 2 / AUDIO_PERSEC_DIV_NUM);
    }
    else if (16 == audio_bitpersample)
    {
        _get_audio_frame(&audio_frame, audio_samplerate * 2 * 2 / AUDIO_PERSEC_DIV_NUM);
    }

    k_u32 *pDataBuf = (k_u32 *)audio_frame.virt_addr;
    memset(pDataBuf, 0, audio_frame.len);

    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = audio_samplerate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = _get_sample_bitwidth(audio_bitpersample);
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    ao_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1 == audio_channel) ? KD_AUDIO_SOUND_MODE_MONO : KD_AUDIO_SOUND_MODE_STEREO;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (!g_enable_audio_codec)
    {
        printf("force the i2s_mode to right justified(tm8821)\n");
        ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE; // tm8821 为i2s 右对齐
    }
    kd_mpi_ao_set_pub_attr(nDev, &ao_dev_attr);

    kd_mpi_ao_enable(nDev);
    kd_mpi_ao_enable_chn(nDev, nChannel);

    static int nCount = 0;
    k_s32 ret = 0;
    g_ao_test_start = K_TRUE;

    while (g_ao_test_start)
    {
        // 获取音频数据
        get_pcm_data_from_file(pDataBuf, audio_frame.len / 4);

#if 0
        printf("====pcm data:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x\n", \
               pDataBuf[0], pDataBuf[1], pDataBuf[2], pDataBuf[3], \
               pDataBuf[4], pDataBuf[5], pDataBuf[6], pDataBuf[7]
              );
#endif

        // 发送音频数据
        ret = kd_mpi_ao_send_frame(nDev, nChannel, &audio_frame, 1000);
        if (ret == 0)
        {
            nCount++;
        }
    }

    // exit
    printf("diable ao audio \n");
    kd_mpi_ao_disable_chn(nDev, nChannel);
    kd_mpi_ao_disable(nDev);
    _release_audio_frame();

    return K_SUCCESS;
}

k_s32 audio_sample_api_ai_to_ao(int ai_dev_num, int ai_channel, int ao_dev_num, int ao_channel, k_u32 sample_rate, k_audio_bit_width bit_width, k_i2s_work_mode i2s_work_mode, k_bool enable_audio3a)
{
    k_aio_dev_attr ai_dev_attr;
    ai_dev_attr.audio_type = (ai_dev_num == 0) ? KD_AUDIO_INPUT_TYPE_I2S : KD_AUDIO_INPUT_TYPE_PDM;
    // i2s
    if (ai_dev_num == 0)
    {
        if (!g_enable_audio_codec)
        {
            if (bit_width != KD_AUDIO_BIT_WIDTH_32) // tm8211
            {
                printf("error:tm8211 is 16bit, right justified,only 32 bit width is supported when ai to ao\n");
                return -1;
            }
        }
        if (g_enable_audio_codec)
        {
            if (0 != ai_channel)
            {
                printf("the built-in codec must use channel 0\n");
                return -1;
            }
        }
        ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
        ai_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
        ai_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
        ai_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
        ai_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
        ai_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ai_dev_attr.kd_audio_attr.i2s_attr.frame_num;
        ai_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    }
    // pdm
    else if (ai_dev_num == 1)
    {
        ai_dev_attr.kd_audio_attr.pdm_attr.sample_rate = sample_rate;
        ai_dev_attr.kd_audio_attr.pdm_attr.bit_width = bit_width;
        ai_dev_attr.kd_audio_attr.pdm_attr.chn_cnt = 4; // max pdm channel
        ai_dev_attr.kd_audio_attr.pdm_attr.snd_mode = KD_AUDIO_SOUND_MODE_STEREO;
        ai_dev_attr.kd_audio_attr.pdm_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
        ai_dev_attr.kd_audio_attr.pdm_attr.pdm_oversample = KD_AUDIO_PDM_INPUT_OVERSAMPLE_64;
        ai_dev_attr.kd_audio_attr.pdm_attr.point_num_per_frame = ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ai_dev_attr.kd_audio_attr.pdm_attr.frame_num;
    }

    if (g_enable_audio_codec)
    {
        if (0 != ao_channel)
        {
            printf("the built-in codec must use channel 0\n");
            return -1;
        }
    }

    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev_num, &ai_dev_attr))
    {
        printf("kd_mpi_ai_set_pub_attr failed\n");
        return K_FAILED;
    }

    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;

    if (K_SUCCESS != kd_mpi_ao_set_pub_attr(ao_dev_num, &ao_dev_attr))
    {
        printf("kd_mpi_ao_set_pub_attr failed\n");
        return K_FAILED;
    }

    if (enable_audio3a)
    {
        if (K_SUCCESS != kd_mpi_ai_set_vqe_attr(ai_dev_num, ai_channel, K_TRUE))
        {
            printf("kd_mpi_ai_set_vqe_attr failed\n");
            return K_FAILED;
        }
    }

    kd_mpi_ai_enable(ai_dev_num);
    kd_mpi_ai_enable_chn(ai_dev_num, ai_channel);

    kd_mpi_ao_enable(ao_dev_num);
    kd_mpi_ao_enable_chn(ao_dev_num, ao_channel);

    g_ai_to_ao_test_start = K_TRUE;

    _test_ai_to_ao(ai_dev_num, ai_channel, ao_dev_num, ao_channel, sample_rate, bit_width);

    // exit
    printf("diable ao module \n");
    kd_mpi_ao_disable_chn(ao_dev_num, ao_channel);
    kd_mpi_ao_disable(ao_dev_num);

    printf("diable ai module \n");
    kd_mpi_ai_disable_chn(ai_dev_num, ai_channel);
    kd_mpi_ai_disable(ai_dev_num);

    printf("release vb block \n");

    return K_SUCCESS;
}

k_s32 audio_sample_bind_ai_to_ao(int ai_dev_num, int ai_channel, int ao_dev_num, int ao_channel, k_u32 sample_rate, k_audio_bit_width bit_width, k_i2s_work_mode i2s_work_mode, k_bool enable_audio3a)
{
    k_aio_dev_attr ai_dev_attr;
    if (ai_dev_num == 1)
    {
        ai_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_PDM;
        ai_dev_attr.kd_audio_attr.pdm_attr.sample_rate = sample_rate;
        ai_dev_attr.kd_audio_attr.pdm_attr.bit_width = bit_width;
        ai_dev_attr.kd_audio_attr.pdm_attr.pdm_oversample = KD_AUDIO_PDM_INPUT_OVERSAMPLE_64;
        ai_dev_attr.kd_audio_attr.pdm_attr.chn_cnt = 4; // max pdm channel
        ai_dev_attr.kd_audio_attr.pdm_attr.snd_mode = KD_AUDIO_SOUND_MODE_STEREO;
        ai_dev_attr.kd_audio_attr.pdm_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
        ai_dev_attr.kd_audio_attr.pdm_attr.point_num_per_frame = ai_dev_attr.kd_audio_attr.pdm_attr.sample_rate / ai_dev_attr.kd_audio_attr.pdm_attr.frame_num;
        if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev_num, &ai_dev_attr))
        {
            printf("kd_mpi_ai_set_pub_attr failed\n");
            return K_FAILED;
        }
    }
    else if (ai_dev_num == 0)
    {
        if (!g_enable_audio_codec)
        {
            if (bit_width != KD_AUDIO_BIT_WIDTH_32) // tm8211
            {
                printf("error:tm8211 is 16bit, right justified,only 32 bit width is supported when ai to ao\n");
                return -1;
            }
        }
        if (g_enable_audio_codec)
        {
            if (0 != ai_channel)
            {
                printf("the built-in codec must use channel 0\n");
                return -1;
            }
        }
        ai_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
        ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
        ai_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
        ai_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
        ai_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
        ai_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
        ai_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ai_dev_attr.kd_audio_attr.i2s_attr.frame_num;
        ai_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
        if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev_num, &ai_dev_attr))
        {
            printf("kd_mpi_ai_set_pub_attr failed\n");
            return K_FAILED;
        }
    }

    if (g_enable_audio_codec)
    {
        if (0 != ao_channel)
        {
            printf("the built-in codec must use channel 0\n");
            return -1;
        }
    }

    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mpi_ao_set_pub_attr(ao_dev_num, &ao_dev_attr))
    {
        printf("kd_mpi_ao_set_pub_attr failed\n");
        return K_FAILED;
    }

    if (enable_audio3a)
    {
        if (K_SUCCESS != kd_mpi_ai_set_vqe_attr(ai_dev_num, ai_channel, K_TRUE))
        {
            printf("kd_mpi_ai_set_vqe_attr failed\n");
            return K_FAILED;
        }
    }

    kd_mpi_ai_enable(ai_dev_num);
    kd_mpi_ai_enable_chn(ai_dev_num, ai_channel);

    kd_mpi_ao_enable(ao_dev_num);
    kd_mpi_ao_enable_chn(ao_dev_num, ao_channel);

    g_ai_bind_ao_test_start = K_TRUE;
    _ai_bind_ao(ai_dev_num, ai_channel, ao_dev_num, ao_channel);

    // exit
    printf("diable ao module \n");
    kd_mpi_ao_disable_chn(ao_dev_num, ao_channel);
    kd_mpi_ao_disable(ao_dev_num);

    printf("diable ai module \n");
    kd_mpi_ai_disable_chn(ai_dev_num, ai_channel);
    kd_mpi_ai_disable(ai_dev_num);

    printf("release vb block \n");

    return K_SUCCESS;
}

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

static k_s32 _load_file(const char *filename, unsigned char **data, int *size)
{
    FILE *fp = fopen(filename, "rb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    *data = (unsigned char *)malloc(*size);
    if (*data == NULL)
    {
        printf("malloc size %d failed\n", *size);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);
    fread(*data, *size, 1, fp);
    fclose(fp);

    return 0;
}

static k_s32 _get_audio_stream(k_audio_stream *audio_stream, int nSize, k_vb_blk_handle *handle)
{
    *handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, nSize, NULL);
    if (*handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }
    audio_stream->len = nSize;
    audio_stream->phys_addr = kd_mpi_vb_handle_to_phyaddr(*handle);
    audio_stream->stream = kd_mpi_sys_mmap(audio_stream->phys_addr, nSize);
    // printf("_get_audio_stream virt_addr:%p\n", audio_stream->stream);

    return K_SUCCESS;
}

static k_s32 _release_audio_stream(k_vb_blk_handle handle)
{
    kd_mpi_vb_release_block(handle);
    return K_SUCCESS;
}

static void _test_file_adec_ao_api(const char *filename, int ao_dev_num, int ao_channel, int adec_channel, k_audio_bit_width bit_width, int sample_rate)
{
    k_audio_frame audio_frame;
    int nSize = 0;
    k_audio_stream audio_stream;

    unsigned char *file_data = NULL;

    unsigned char *cur_data = NULL;
    int nCur_data_index = 0;
    if (0 != _load_file(filename, &file_data, &nSize))
    {
        return;
    }

    int enc_frame_len = sample_rate * 2 * 2 / AUDIO_PERSEC_DIV_NUM / 2;

    k_vb_blk_handle handle;
    if (K_SUCCESS != _get_audio_stream(&audio_stream, enc_frame_len, &handle))
    {
        printf("_get_audio_stream failed\n");
        return;
    }

    while (g_adec_test_start)
    {
        if (nCur_data_index + enc_frame_len > nSize)
        {
            printf("read file again\n");
            nCur_data_index = 0;
        }

        cur_data = file_data + nCur_data_index;
        memcpy(audio_stream.stream, cur_data, enc_frame_len);
        audio_stream.seq++;

        if (0 != kd_mpi_adec_send_stream(adec_channel, &audio_stream, K_TRUE))
        {
            printf("kd_mpi_adec_send_stream failed\n");
            _release_audio_stream(handle);
            continue;
        }

        if (0 != kd_mpi_adec_get_frame(adec_channel, &audio_frame, 0))
        {
            printf("kd_mpi_adec_get_frame failed\n");
        }
        else
        {
            kd_mpi_ao_send_frame(ao_dev_num, ao_channel, &audio_frame, 100);

            kd_mpi_adec_release_frame(adec_channel, &audio_frame);
        }

        nCur_data_index += enc_frame_len;
    }

    _release_audio_stream(handle);

    if (file_data != NULL)
    {
        free(file_data);
        file_data = NULL;
    }
}

static void _test_file_adec_ao_sysbind(const char *filename, int ao_dev_num, int ao_channel, int adec_channel, k_audio_bit_width bit_width, int sample_rate)
{
    int nSize = 0;
    k_audio_stream audio_stream;
    unsigned char *file_data = NULL;
    unsigned char *cur_data = NULL;
    int nCur_data_index = 0;
    int enc_frame_len = sample_rate * 2 * 2 / AUDIO_PERSEC_DIV_NUM / 2;

    if (0 != _load_file(filename, &file_data, &nSize))
    {
        return;
    }

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;

    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_channel;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev_num;
    ao_mpp_chn.chn_id = ao_channel;

    if (0 != kd_mpi_sys_bind(&adec_mpp_chn, &ao_mpp_chn))
    {
        printf("kd_mpi_sys_bind failed\n");
        return;
    }

    // int nCount = 0;
    k_vb_blk_handle handle;
    if (K_SUCCESS != _get_audio_stream(&audio_stream, enc_frame_len, &handle))
    {
        return;
    }

    while (g_adec_test_start)
    {
        if (nCur_data_index + enc_frame_len > nSize)
        {
            printf("read file again\n");
            nCur_data_index = 0;
        }

        cur_data = file_data + nCur_data_index;
        memcpy(audio_stream.stream, cur_data, enc_frame_len);
        audio_stream.seq++;
        audio_stream.len = enc_frame_len;

        if (0 != kd_mpi_adec_send_stream(adec_channel, &audio_stream, K_TRUE)) // must be block to prevent fast reading of data from file
        {
            printf("kd_mpi_adec_send_stream failed\n");
            continue;
        }

        nCur_data_index += enc_frame_len;
        // printf("=====================kd_mpi_adec_send_stream:%d,size:%d\n",nCount++,audio_stream.len);
    }

    _release_audio_stream(handle);

    kd_mpi_sys_unbind(&adec_mpp_chn, &ao_mpp_chn);

    if (file_data != NULL)
    {
        free(file_data);
        file_data = NULL;
    }
}

static void _test_ai_aenc_file_api(const char *filename, int ai_dev_num, int ai_channel, int aenc_channel, k_audio_bit_width bit_width, int sample_rate)
{
    k_audio_frame audio_frame;
    k_audio_stream audio_stream;

    FILE *fp = fopen(filename, "wb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filename);
        return;
    }

    while (g_aenc_test_start)
    {
        if (K_SUCCESS != kd_mpi_ai_get_frame(ai_dev_num, ai_channel, &audio_frame, 1000))
        {
            printf("kd_mpi_ai_get_frame timeout\n");
            continue;
        }

        if (0 != kd_mpi_aenc_send_frame(aenc_channel, &audio_frame))
        {
            printf("kd_mpi_aenc_send_frame failed\n");
        }
        else
        {
            if (0 != kd_mpi_aenc_get_stream(aenc_channel, &audio_stream, 0))
            {
                printf("kd_mpi_aenc_get_stream failed\n");
            }
            else
            {
                // printf("enc audio stream len:%d,timestamp:%ld,seq:%d,phys:0x%lx\n", audio_stream.len, audio_stream.time_stamp, audio_stream.seq, audio_stream.phys_addr);

                k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_stream.phys_addr, audio_stream.len);
                fwrite(raw_data, 1, audio_stream.len, fp);
                kd_mpi_sys_munmap(raw_data, audio_stream.len);

                kd_mpi_aenc_release_stream(aenc_channel, &audio_stream);
            }
        }

        kd_mpi_ai_release_frame(ai_dev_num, ai_channel, &audio_frame);
    }

    fclose(fp);
}

static void _test_ai_aenc_file_sysbind(const char *filename, int ai_dev_num, int ai_channel, int aenc_channel, k_audio_bit_width bit_width, int sample_rate)
{
    k_audio_stream audio_stream;

    FILE *fp = fopen(filename, "wb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filename);
        return;
    }

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev_num;
    ai_mpp_chn.chn_id = ai_channel;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_channel;
    if (0 != kd_mpi_sys_bind(&ai_mpp_chn, &aenc_mpp_chn))
    {
        printf("%s kd_mpi_sys_bind failed\n", __FUNCTION__);
        return;
    }

    while (g_aenc_test_start)
    {
        if (0 != kd_mpi_aenc_get_stream(aenc_channel, &audio_stream, 1000))
        {
            printf("kd_mpi_aenc_get_stream failed\n");
            continue;
        }
        else
        {
            // printf("enc audio stream len:%d,timestamp:%ld,seq:%d,phys:0x%lx\n", audio_stream.len, audio_stream.time_stamp, audio_stream.seq, audio_stream.phys_addr);

            k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_stream.phys_addr, audio_stream.len);
            fwrite(raw_data, 1, audio_stream.len, fp);
            kd_mpi_sys_munmap(raw_data, audio_stream.len);

            kd_mpi_aenc_release_stream(aenc_channel, &audio_stream);
        }
    }

    kd_mpi_sys_unbind(&ai_mpp_chn, &aenc_mpp_chn);
    fclose(fp);
}

k_s32 audio_sample_ai_encode(k_bool use_sysbind, k_u32 samplerate, k_audio_bit_width bit_width, int encChannel, k_payload_type type, const char *filename)
{
    int sample_rate = samplerate;
    k_aenc_chn aenc_chn = encChannel;
    k_audio_dev ai_dev = 0;
    k_ai_chn ai_chn = 0;

    if (bit_width != KD_AUDIO_BIT_WIDTH_16)
    {
        bit_width = KD_AUDIO_BIT_WIDTH_16;
        printf("Force the sampling accuracy to be set to 16\n");
    }

    k_aenc_chn_attr aenc_chn_attr;
    aenc_chn_attr.type = type;
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = sample_rate / aenc_chn_attr.buf_size;

    if (0 != kd_mpi_aenc_create_chn(aenc_chn, &aenc_chn_attr))
    {
        printf("kd_mpi_aenc_create_chn faild\n");
        return -1;
    }

    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev, &aio_dev_attr))
    {
        kd_mpi_aenc_destroy_chn(aenc_chn);
        printf("kd_mpi_ai_set_pub_attr failed\n");
        return K_FAILED;
    }

    kd_mpi_ai_enable(ai_dev);
    kd_mpi_ai_enable_chn(ai_dev, ai_chn);

    g_aenc_test_start = K_TRUE;
    if (use_sysbind)
    {
        _test_ai_aenc_file_sysbind(filename, ai_dev, ai_chn, aenc_chn, bit_width, sample_rate);
    }
    else
    {
        _test_ai_aenc_file_api(filename, ai_dev, ai_chn, aenc_chn, bit_width, sample_rate);
    }
    // exit

    kd_mpi_ai_disable_chn(ai_dev, ai_chn);
    kd_mpi_ai_disable(ai_dev);
    kd_mpi_aenc_destroy_chn(aenc_chn);

    return K_SUCCESS;
}

k_s32 audio_sample_decode_ao(k_bool use_sysbind, k_u32 samplerate, k_audio_bit_width bit_width, int decChannel, k_payload_type type, const char *filename)
{
    int sample_rate = samplerate;
    k_adec_chn adec_chn = decChannel;
    k_audio_dev ao_dev = 0;
    k_ao_chn ao_chn = 0;

    if (bit_width != KD_AUDIO_BIT_WIDTH_16)
    {
        bit_width = KD_AUDIO_BIT_WIDTH_16;
        printf("Force the sampling accuracy to be set to 16\n");
    }

    k_adec_chn_attr adec_chn_attr;
    adec_chn_attr.type = type;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = sample_rate / adec_chn_attr.buf_size;

    if (0 != kd_mpi_adec_create_chn(adec_chn, &adec_chn_attr))
    {
        printf("kd_mpi_adec_create_chn faild\n");
        return -1;
    }

    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = g_enable_audio_codec ? K_STANDARD_MODE : K_RIGHT_JUSTIFYING_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mpi_ao_set_pub_attr(ao_dev, &aio_dev_attr))
    {
        printf("kd_mpi_ao_set_pub_attr failed\n");
        kd_mpi_adec_destroy_chn(adec_chn);
        return K_FAILED;
    }

    kd_mpi_ao_enable(ao_dev);
    kd_mpi_ao_enable_chn(ao_dev, ao_chn);
    kd_mpi_adec_clr_chn_buf(adec_chn);

    g_adec_test_start = K_TRUE;
    if (use_sysbind)
    {
        _test_file_adec_ao_sysbind(filename, ao_dev, ao_chn, adec_chn, bit_width, sample_rate);
    }
    else
    {
        _test_file_adec_ao_api(filename, ao_dev, ao_chn, adec_chn, bit_width, sample_rate);
    }
    // exit

    kd_mpi_ao_disable_chn(ao_dev, ao_chn);
    kd_mpi_ao_disable(ao_dev);
    kd_mpi_adec_destroy_chn(adec_chn);

    return K_SUCCESS;
}

static char *g_load_filename = NULL;
static void *sample_record_fn(void *arg)
{
    k_audio_dev ai_dev = 0;
    k_ai_chn ai_chn = 0;
    k_aenc_chn aenc_chn = 0;
    kd_mpi_ai_enable(ai_dev);
    kd_mpi_ai_enable_chn(ai_dev, ai_chn);

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;
    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev;
    ai_mpp_chn.chn_id = ai_chn;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_chn;
    kd_mpi_sys_bind(&ai_mpp_chn, &aenc_mpp_chn);

    k_audio_stream audio_stream;

    char filename[256] = {0};
    sprintf(filename, "%s_rec", g_load_filename);
    // const char *filename = "/sharefs/rec.g711a";
    FILE *fp = fopen(filename, "wb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filename);
        return NULL;
    }

    printf("=====start record thread,record file:%s\n", filename);
    while (g_audio_overall_start)
    {
        if (0 != kd_mpi_aenc_get_stream(aenc_chn, &audio_stream, 100))
        {
            printf("kd_mpi_aenc_get_stream failed\n");
            continue;
        }
        else
        {
            k_u8 *raw_data = (k_u8 *)kd_mpi_sys_mmap(audio_stream.phys_addr, audio_stream.len);
            fwrite(raw_data, 1, audio_stream.len, fp);
            kd_mpi_sys_munmap(raw_data, audio_stream.len);
        }

        kd_mpi_aenc_release_stream(aenc_chn, &audio_stream);
    }

    fclose(fp);

    kd_mpi_sys_unbind(&ai_mpp_chn, &aenc_mpp_chn);
    return NULL;
}

static void *sample_play_fn(void *arg)
{
    k_adec_chn adec_channel = 0;
    k_audio_dev ao_dev = 0;
    k_ao_chn ao_chn = 0;

    kd_mpi_ao_enable(ao_dev);
    kd_mpi_ao_enable_chn(ao_dev, ao_chn);

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;

    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_channel;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = 0;
    ao_mpp_chn.chn_id = 0;

    kd_mpi_sys_bind(&adec_mpp_chn, &ao_mpp_chn);

    unsigned char *file_data = NULL;
    k_audio_stream audio_stream;
    k_s32 sample_rate = 44100;
    int nSize = 0;
    unsigned char *cur_data = NULL;
    int nCur_data_index = 0;
    int enc_frame_len = sample_rate * 2 * 2 / AUDIO_PERSEC_DIV_NUM / 2;
    if (0 != _load_file(g_load_filename, &file_data, &nSize))
    {
        return NULL;
    }

    k_vb_blk_handle handle;
    if (K_SUCCESS != _get_audio_stream(&audio_stream, enc_frame_len, &handle))
    {
        return NULL;
    }
    printf("=====start play thread\n");
    while (g_audio_overall_start)
    {
        if (nCur_data_index + enc_frame_len > nSize)
        {
            printf("read file again\n");
            nCur_data_index = 0;
        }

        cur_data = file_data + nCur_data_index;
        memcpy(audio_stream.stream, cur_data, enc_frame_len);
        audio_stream.seq++;
        audio_stream.len = enc_frame_len;

        if (0 != kd_mpi_adec_send_stream(adec_channel, &audio_stream, K_TRUE)) // must be block to prevent fast reading of data from file
        {
            printf("kd_mpi_adec_send_stream failed\n");
            continue;
        }

        nCur_data_index += enc_frame_len;
        // printf("=====================kd_mpi_adec_send_stream:%d,size:%d\n",nCount++,audio_stream.len);
    }

    _release_audio_stream(handle);

    kd_mpi_sys_unbind(&adec_mpp_chn, &ao_mpp_chn);

    if (file_data != NULL)
    {
        free(file_data);
        file_data = NULL;
    }
    return NULL;
}

k_s32 audio_sample_ai_aenc_adec_ao(k_audio_dev ai_dev, k_ai_chn ai_chn, k_audio_dev ao_dev, k_ao_chn ao_chn, k_aenc_chn aenc_chn, k_adec_chn adec_chn, k_u32 samplerate, k_audio_bit_width bit_width, k_payload_type type, const char *load_filename)
{
    pthread_t record_thread_handle;
    pthread_t play_thread_handle;
    k_u32 sample_rate = samplerate;
    g_load_filename = (char *)load_filename;
    if (0 != access(load_filename, 0))
    {
        printf("open file:%s failed\n", g_load_filename);
        return K_FAILED;
    }

    g_enable_audio_codec = K_TRUE;
    bit_width = KD_AUDIO_BIT_WIDTH_16;
    k_i2s_work_mode i2s_work_mode = K_STANDARD_MODE;
    printf("Force the sampling accuracy to be set to 16,use inner cocdec\n");

    k_aio_dev_attr ai_dev_attr;
    ai_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ai_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    ai_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ai_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    ai_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ai_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ai_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ai_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev, &ai_dev_attr))
    {
        printf("kd_mpi_ai_set_pub_attr failed\n");
        return K_FAILED;
    }

    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;

    if (K_SUCCESS != kd_mpi_ao_set_pub_attr(ao_dev, &ao_dev_attr))
    {
        printf("kd_mpi_ao_set_pub_attr failed\n");
        return K_FAILED;
    }

    k_aenc_chn_attr aenc_chn_attr;
    aenc_chn_attr.type = type;
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = sample_rate / aenc_chn_attr.buf_size;

    if (0 != kd_mpi_aenc_create_chn(aenc_chn, &aenc_chn_attr))
    {
        printf("kd_mpi_aenc_create_chn faild\n");
        return K_FAILED;
    }

    k_adec_chn_attr adec_chn_attr;
    adec_chn_attr.type = type;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = sample_rate / adec_chn_attr.buf_size;

    if (0 != kd_mpi_adec_create_chn(adec_chn, &adec_chn_attr))
    {
        printf("kd_mpi_adec_create_chn faild\n");
        return K_FAILED;
    }

    g_audio_overall_start = K_TRUE;
    pthread_create(&play_thread_handle, NULL, sample_play_fn, NULL);
    sleep(1);
    pthread_create(&record_thread_handle, NULL, sample_record_fn, NULL);

    pthread_join(record_thread_handle, NULL);
    pthread_join(play_thread_handle, NULL);

    kd_mpi_ai_disable_chn(ai_dev, ai_chn);
    kd_mpi_ai_disable(ai_dev);
    kd_mpi_ao_disable_chn(ao_dev, ao_chn);
    kd_mpi_ao_disable(ao_dev);
    kd_mpi_aenc_destroy_chn(aenc_chn);
    kd_mpi_adec_destroy_chn(adec_chn);

    return K_SUCCESS;
}

k_s32 audio_sample_ai_aenc_adec_ao_2(k_audio_dev ai_dev, k_ai_chn ai_chn, k_audio_dev ao_dev, k_ao_chn ao_chn, k_aenc_chn aenc_chn, k_adec_chn adec_chn, k_u32 samplerate, k_audio_bit_width bit_width, k_payload_type type)
{
    g_enable_audio_codec = K_TRUE;
    bit_width = KD_AUDIO_BIT_WIDTH_16;
    k_u32 sample_rate = samplerate;
    k_i2s_work_mode i2s_work_mode = K_STANDARD_MODE;
    printf("Force the sampling accuracy to be set to 16,use inner cocdec\n");

    k_aio_dev_attr ai_dev_attr;
    ai_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ai_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    ai_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ai_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    ai_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ai_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ai_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ai_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ai_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev, &ai_dev_attr))
    {
        printf("kd_mpi_ai_set_pub_attr failed\n");
        return K_FAILED;
    }

    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;

    if (K_SUCCESS != kd_mpi_ao_set_pub_attr(ao_dev, &ao_dev_attr))
    {
        printf("kd_mpi_ao_set_pub_attr failed\n");
        return K_FAILED;
    }

    k_aenc_chn_attr aenc_chn_attr;
    aenc_chn_attr.type = type;
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = sample_rate / aenc_chn_attr.buf_size;

    if (0 != kd_mpi_aenc_create_chn(aenc_chn, &aenc_chn_attr))
    {
        printf("kd_mpi_aenc_create_chn faild\n");
        return K_FAILED;
    }

    k_adec_chn_attr adec_chn_attr;
    adec_chn_attr.type = type;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = sample_rate / adec_chn_attr.buf_size;

    if (0 != kd_mpi_adec_create_chn(adec_chn, &adec_chn_attr))
    {
        printf("kd_mpi_adec_create_chn faild\n");
        return K_FAILED;
    }

    kd_mpi_ai_enable(ai_dev);
    kd_mpi_ai_enable_chn(ai_dev, ai_chn);

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev;
    ai_mpp_chn.chn_id = ai_chn;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_chn;

    kd_mpi_sys_bind(&ai_mpp_chn, &aenc_mpp_chn);

    kd_mpi_ao_enable(ao_dev);
    kd_mpi_ao_enable_chn(ao_dev, ao_chn);

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;

    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_chn;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev;
    ao_mpp_chn.chn_id = ao_chn;

    kd_mpi_sys_bind(&adec_mpp_chn, &ao_mpp_chn);

    g_audio_overall_start = K_TRUE;

    k_audio_stream audio_stream;
    while (g_audio_overall_start)
    {
        if (K_SUCCESS != kd_mpi_aenc_get_stream(aenc_chn, &audio_stream, 100))
        {
            printf("========kd_mpi_aenc_get_stream failed\n");
            continue;
        }

        _test_aenc_timestamp(sample_rate, 2, bit_width, audio_stream.time_stamp, audio_stream.len);

        if (K_SUCCESS != kd_mpi_adec_send_stream(adec_chn, &audio_stream, K_FALSE))
        {

            printf("========kd_mpi_adec_send_stream failed\n");
        }

        kd_mpi_aenc_release_stream(aenc_chn, &audio_stream);
    }

    kd_mpi_sys_unbind(&ai_mpp_chn, &aenc_mpp_chn);
    kd_mpi_sys_unbind(&adec_mpp_chn, &ao_mpp_chn);

    kd_mpi_ai_disable_chn(ai_dev, ai_chn);
    kd_mpi_ai_disable(ai_dev);
    kd_mpi_ao_disable_chn(ao_dev, ao_chn);
    kd_mpi_ao_disable(ao_dev);
    kd_mpi_aenc_destroy_chn(aenc_chn);
    kd_mpi_adec_destroy_chn(adec_chn);

    return K_SUCCESS;
}

static k_s32 g_acodec_fd = -1;
static pthread_mutex_t g_acodec_mutex = PTHREAD_MUTEX_INITIALIZER;
static k_s32 acodec_check_open(void)
{
    pthread_mutex_lock(&g_acodec_mutex);
    if (g_acodec_fd < 0)
    {
        g_acodec_fd = open("/dev/acodec_device", O_RDWR);
        if (g_acodec_fd < 0)
        {
            perror("open err\n");
            pthread_mutex_unlock(&g_acodec_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&g_acodec_mutex);
    return 0;
}

static k_s32 _get_func_key()
{
    printf("please input 'c' key to continue,'q' key to exit\n");
    int c;
    while ((c = getchar()) != EOF)
    {
        if ('q' == c)
        {
            return -1;
        }
        else if ('c' == c)
        {
            return 0;
        }
    }
    return 0;
}

static float g_dac_hpout_gain = 0;
static k_s32 _test_dac_hpout()
{
    float gain_value = -39;
    k_s32 key_value;

    for (;;)
    {
        gain_value += 1.5;
        if (gain_value > 6)
        {
            gain_value = -39;
        }
        g_dac_hpout_gain = gain_value;
        ioctl(g_acodec_fd, k_acodec_set_gain_hpoutl, &gain_value);
        ioctl(g_acodec_fd, k_acodec_set_gain_hpoutr, &gain_value);
        printf("cur dac gain value:%.2f db\n", gain_value);

        key_value = _get_func_key();
        if (0 == key_value)
        {
            continue;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static k_s32 _test_dac_volume()
{
    k_s32 key_value;
    float volume_value = -120;
    for (;;)
    {
        volume_value += 0.5;
        if (volume_value > 7)
        {
            volume_value = -120;
        }
        ioctl(g_acodec_fd, k_acodec_set_dacl_volume, &volume_value);
        ioctl(g_acodec_fd, k_acodec_set_dacr_volume, &volume_value);
        printf("cur dac volume  value:%.2f db\n", volume_value);

        key_value = _get_func_key();
        if (0 == key_value)
        {
            continue;
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

static k_s32 _test_adc_mic()
{
    k_s32 key_value;
    k_u32 gain_value[4] = {0, 6, 20, 30};
    int cur_index = 0;
    for (;;)
    {
        if (cur_index >= 4)
        {
            cur_index = 0;
        }

        ioctl(g_acodec_fd, k_acodec_set_gain_micl, &gain_value[cur_index]);
        ioctl(g_acodec_fd, k_acodec_set_gain_micr, &gain_value[cur_index]);
        printf("cur adc mic gain value:%d db\n", gain_value[cur_index]);

        key_value = _get_func_key();
        if (0 == key_value)
        {
            cur_index++;
            continue;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static k_s32 _test_alc_mic()
{
    k_s32 key_value;
    float gain_value = -18;
    for (;;)
    {
        gain_value += 1.5;
        if (gain_value > 28.5)
        {
            gain_value = -18;
        }
        printf("k_acodec_set_alc_gain_micr:%.2f db\n", gain_value);
        ioctl(g_acodec_fd, k_acodec_set_alc_gain_micr, &gain_value);

        printf("k_acodec_set_alc_gain_micl:%.2f db\n", gain_value);
        ioctl(g_acodec_fd, k_acodec_set_alc_gain_micl, &gain_value);

        printf("cur alc mic gain value:%.2f db\n", gain_value);

        key_value = _get_func_key();
        if (0 == key_value)
        {
            continue;
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

static k_s32 _test_adc_volume()
{
    k_s32 key_value;
    float volume_value = -97;
    for (;;)
    {
        volume_value += 0.5;
        if (volume_value > 30)
        {
            volume_value = -97;
        }
        ioctl(g_acodec_fd, k_acodec_set_adcl_volume, &volume_value);
        ioctl(g_acodec_fd, k_acodec_set_adcr_volume, &volume_value);
        printf("cur adc volume  value:%.2f db\n", volume_value);

        key_value = _get_func_key();
        if (0 == key_value)
        {
            continue;
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

static k_s32 _test_adc_mic_mute()
{
    k_bool mute = K_TRUE;
    k_s32 key_value;
    for (;;)
    {
        ioctl(g_acodec_fd, k_acodec_set_micl_mute, &mute);
        ioctl(g_acodec_fd, k_acodec_set_micr_mute, &mute);
        printf("cur adc mic mute value:%d\n", mute);

        key_value = _get_func_key();
        if (0 == key_value)
        {
            mute = (k_bool)!mute;
            continue;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static k_s32 _test_dac_hpout_mute()
{
    k_bool mute = K_TRUE;
    k_s32 key_value;
    for (;;)
    {
        ioctl(g_acodec_fd, k_acodec_set_dacl_mute, &mute);
        ioctl(g_acodec_fd, k_acodec_set_dacr_mute, &mute);
        printf("cur dac hpout mute value:%d\n", mute);

        key_value = _get_func_key();
        if (0 == key_value)
        {
            mute = (k_bool)!mute;
            continue;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static k_s32 _test_get_current_db_value()
{
    k_u32 adcl_gain_value;
    k_u32 adcr_gain_value;
    float adcl_volume_value;
    float adcr_volume_value;
    float alcl_gain_value;
    float alcr_gain_value;
    float dacl_volume_value;
    float dacr_volume_value;
    float dacl_gain_value;
    float dacr_gain_value;

    ioctl(g_acodec_fd, k_acodec_get_gain_micl, &adcl_gain_value);
    ioctl(g_acodec_fd, k_acodec_get_gain_micr, &adcr_gain_value);

    ioctl(g_acodec_fd, k_acodec_get_adcl_volume, &adcl_volume_value);
    ioctl(g_acodec_fd, k_acodec_get_adcr_volume, &adcr_volume_value);

    ioctl(g_acodec_fd, k_acodec_get_alc_gain_micl, &alcl_gain_value);
    ioctl(g_acodec_fd, k_acodec_get_alc_gain_micr, &alcr_gain_value);

    ioctl(g_acodec_fd, k_acodec_get_dacl_volume, &dacl_volume_value);
    ioctl(g_acodec_fd, k_acodec_get_dacr_volume, &dacr_volume_value);

    ioctl(g_acodec_fd, k_acodec_get_gain_hpoutl, &dacl_gain_value);
    ioctl(g_acodec_fd, k_acodec_get_gain_hpoutr, &dacr_gain_value);

    printf("adcl_gain(%d),adcr_gain(%d),adcl_volume(%.1f),adcr_volume(%.1f),alcl_gain(%.1f),alcr_gain(%.1f),dacl_volume(%.1f),dacr_volume(%.1f),dacl_gain(%.1f),dacr_gain(%.1f)", \
    adcl_gain_value,adcr_gain_value,adcl_volume_value,adcr_volume_value,\
    alcl_gain_value,alcr_gain_value,dacl_volume_value,dacr_volume_value,\
    dacl_gain_value,dacr_gain_value);
    return 0;
}

static k_s32 _test_acodec_reset()
{
    return ioctl(g_acodec_fd, k_acodec_reset, NULL);

}

static void _show_acodec_func()
{
    printf("\r\n===================\n");
    printf("please input:\n");
    printf("0:adc mic gain\n");
    printf("1:adc volume\n");
    printf("2:alc mic gain\n");
    printf("3:dac hpout gain\n");
    printf("4:dac volume\n");
    printf("5:adc mute \n");
    printf("6:dac mute\n");
    printf("7:get current db values\n");
    printf("8:reset \n");
    printf("q:exit\n");
}

static k_s32 _get_menu_key(int *key)
{
    _show_acodec_func();
    int c;
    while ((c = getchar()) != EOF)
    {
        if ('q' == c)
        {
            return -1;
        }
        else
        {
            *key = c;
            return 0;
        }
    }
    return -1;
}

static void _acodec_test_func()
{

    int value;
    while (1)
    {
        _show_acodec_func();
        if (-1 == _get_menu_key(&value))
        {
            break;
        }

        if (value >= '0' && value <= '8')
        {
            if ('0' == value)
            {
                _test_adc_mic();
            }
            else if ('1' == value)
            {
                _test_adc_volume();
            }
            else if ('2' == value)
            {
                _test_alc_mic();
            }
            else if ('3' == value)
            {
                _test_dac_hpout();
            }
            else if ('4' == value)
            {
                _test_dac_volume();
            }
            else if ('5' == value)
            {
                _test_adc_mic_mute();
            }
            else if ('6' == value)
            {
                _test_dac_hpout_mute();
            }
            else if ('7' == value)
            {
                _test_get_current_db_value();
            }
            else if ('8' == value)
            {
                _test_acodec_reset();
            }
        }
        else
        {
            printf("input error:%c\n", value);
            continue;
        }
    }
}

k_s32 audio_sample_acodec()
{
    if (acodec_check_open())
        return -1;

    _acodec_test_func();

    return 0;
}
