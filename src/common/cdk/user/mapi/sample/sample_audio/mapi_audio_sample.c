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
#include "mapi_audio_sample.h"

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>

#include "mapi_sys_api.h"
#include "mapi_ai_api.h"
#include "mapi_aenc_api.h"
#include "mapi_ao_api.h"
#include "mapi_adec_api.h"

#define AUDIO_PERSEC_DIV_NUM 25
static k_bool g_vb_init = K_FALSE;
static k_mapi_media_attr_t media_attr = {0};
static FILE * g_fp = NULL;
static k_bool g_enable_audio_codec = K_TRUE;

static k_u32 g_save_frame_count = 0;
#define MAX_SAVE_FRAME_COUNT 25*60 //60s

static k_s32 _get_quit_key()
{
    printf("enter 'q' key to exit\n");
    int c;
    while ((c = getchar()) != EOF)
    {
        if ('q' == c)
        {
            return 0;
        }
    }
    return -1;
}

static k_s32 g_mmap_fd_tmp = 0;
static void *_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);


    if (g_mmap_fd_tmp == 0)
    {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);

    if (mmap_addr != (void *)-1)
        virt_addr = (void*)((char*)mmap_addr + (phys_addr & page_mask));
    else
    {
        printf("mmap addr error: %d %s.\n", mmap_addr, strerror(errno));;
    }

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phy_addr, void *virt_addr,k_u32 size)
{
    if (g_mmap_fd_tmp == 0)
    {
        return -1;
    }
    k_u32 ret;

    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phy_addr & page_mask) + page_mask) & ~(page_mask);
    ret = munmap((void *)((k_u64)(virt_addr) & ~page_mask), mmap_size);
    if (ret == -1) {
        printf("munmap error.\n");
    }

    return 0;
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

static k_s32 _init_ai(k_audio_bit_width bit_width, k_u32 sample_rate,k_u32 channels, k_i2s_work_mode i2s_work_mode,k_handle* ai_hdl)
{
    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==channels)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mapi_ai_init(0,0, &aio_dev_attr,ai_hdl))
    {
        printf("kd_mapi_ai_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ai_start(*ai_hdl))
    {
        printf("kd_mapi_ai_start failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32 _deinit_ai(k_handle ai_hdl)
{
    if (K_SUCCESS !=kd_mapi_ai_stop(ai_hdl))
    {
        printf("kd_mapi_ai_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ai_deinit(ai_hdl))
    {
        printf("kd_mapi_ai_deinit failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32 _init_ao(k_audio_bit_width bit_width, k_u32 sample_rate,k_u32 channels, k_i2s_work_mode i2s_work_mode,k_handle* ao_hdl)
{
    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==channels)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;

    if (!g_enable_audio_codec)
    {
        printf("force the i2s_mode to right justified(tm8821)\n");
        aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE; // tm8821 为i2s 右对齐
    }

    if (K_SUCCESS != kd_mapi_ao_init(0,0, &aio_dev_attr,ao_hdl))
    {
        printf("kd_mapi_ao_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_start(*ao_hdl))
    {
        printf("kd_mapi_ao_start failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32 _deinit_ao(k_handle ao_hdl)
{
    if (K_SUCCESS !=kd_mapi_ao_stop(ao_hdl))
    {
        printf("kd_mapi_ao_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_deinit(ao_hdl))
    {
        printf("kd_mapi_ao_deinit failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32  _init_aenc(k_handle aenc_hdl,k_u32 sample_rate)
{
    k_aenc_chn_attr aenc_chn_attr;
    aenc_chn_attr.type = K_PT_G711A;
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = sample_rate / aenc_chn_attr.buf_size;

    if (K_SUCCESS != kd_mapi_aenc_init(aenc_hdl, &aenc_chn_attr))
    {
        printf("kd_mpi_aenc_create_chn faild\n");
        return -1;
    }

    if (K_SUCCESS != kd_mapi_aenc_start(aenc_hdl))
    {
        printf("kd_mapi_aenc_start faild\n");
        return -1;
    }


    return K_SUCCESS;
}

static k_s32 _deinit_aenc(k_handle aenc_hdl)
{
    if (K_SUCCESS !=kd_mapi_aenc_stop(aenc_hdl))
    {
        printf("kd_mapi_aenc_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_aenc_deinit(aenc_hdl))
    {
        printf("kd_mapi_aenc_deinit failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32  _init_adec(k_handle adec_hdl,k_u32 sample_rate)
{
    k_adec_chn_attr adec_chn_attr;
    adec_chn_attr.type = K_PT_G711A;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = sample_rate / adec_chn_attr.buf_size;
    adec_chn_attr.mode = K_ADEC_MODE_PACK;

    if (K_SUCCESS != kd_mapi_adec_init(adec_hdl, &adec_chn_attr))
    {
        printf("kd_mapi_adec_init faild\n");
        return -1;
    }

    if (K_SUCCESS != kd_mapi_adec_start(adec_hdl))
    {
        printf("kd_mapi_adec_start faild\n");
        return -1;
    }


    return K_SUCCESS;
}

static k_s32 _deinit_adec(k_handle adec_hdl)
{
    if (K_SUCCESS !=kd_mapi_adec_stop(adec_hdl))
    {
        printf("kd_mapi_adec_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_adec_deinit(adec_hdl))
    {
        printf("kd_mapi_adec_deinit failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}



static k_s32 _mapi_sample_vb_init(k_bool enable_cache,k_u32 sample_rate)
{
    if (g_vb_init)
    {
        printf("%s already init\n",__FUNCTION__);
        return K_SUCCESS;
    }

    k_s32 ret;
    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_init error: %x\n", ret);
        return K_FAILED;
    }

    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));
    k_vb_config* config = &media_attr.media_config.vb_config;

    memset(config, 0, sizeof(*config));
    config->max_pool_cnt = 64;

    config->comm_pool[0].blk_cnt = 150;
    config->comm_pool[0].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    config->comm_pool[0].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config->comm_pool[1].blk_cnt = 2;
    config->comm_pool[1].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2; // ao use
    config->comm_pool[1].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;


    int blk_total_size = 0;
    for (int i = 0; i < 2; i++)
    {
        blk_total_size += config->comm_pool[i].blk_cnt * config->comm_pool[i].blk_size;
    }
    printf("mmz blk total size:%.2f MB\n", blk_total_size / 1024 / 1024.0);

    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
        return K_FAILED;
    }

    g_vb_init = K_TRUE;

    return ret;
}

static k_s32 _mapi_sample_vb_deinit()
{
    if (!g_vb_init)
    {
        printf("%s not init\n",__FUNCTION__);
        return K_FAILED;
    }
    k_s32 ret;
    ret = kd_mapi_media_deinit();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_deinit error: %x\n", ret);
        return -1;
    }
    ret = kd_mapi_sys_deinit();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_deinit error: %x\n", ret);
        return -1;
    }

    g_vb_init = K_FALSE;

    return ret;
}


static  k_s32 _aenc_dataproc(k_u32 chn_num, k_audio_stream* stream_data, void* p_private_data)
{
    printf("_aenc_dataproc chn_num:%d,stream data:0x%x,data len:%d,seq:%d,timestamp:%ld\n",\
    chn_num,stream_data->phys_addr,stream_data->len,stream_data->seq,stream_data->time_stamp);

#if 0
    if (g_save_frame_count <= MAX_SAVE_FRAME_COUNT)
    {
        fwrite(stream_data->stream, 1, stream_data->len, g_fp);
        if (g_save_frame_count == MAX_SAVE_FRAME_COUNT)
        {
            printf("ai -> aenc record file done\n");
        }
    }
    g_save_frame_count++;
#else
    fwrite(stream_data->stream, 1, stream_data->len, g_fp);
#endif

    return K_SUCCESS;
}

k_s32 audio_mapi_sample_ai_aenc(k_u32 sample_rate,k_u32 channels,k_bool enable_audio_codec,const char* filename)
{
    g_enable_audio_codec = enable_audio_codec;
    k_s32 ret;
    k_handle ai_hdl = 0;
    k_handle aenc_hdl = 0;

    g_fp = fopen(filename,"wb");
    if (g_fp <= 0)
    {
        printf("fopen %s failed\n",filename);
        return -1;
    }

    ret = _mapi_sample_vb_init(K_TRUE, sample_rate);
    if (ret != K_SUCCESS)
    {
        printf("_mapi_sample_vb_init failed\n");
        return -1;
    }

    ret = _init_ai(KD_AUDIO_BIT_WIDTH_16,sample_rate,channels,K_STANDARD_MODE,&ai_hdl);
    if(ret != K_SUCCESS) {
        printf("_init_ai error: %x\n", ret);
        return -1;
    }

    ret = _init_aenc(aenc_hdl,sample_rate);
    if(ret != K_SUCCESS) {
        printf("_init_aenc error: %x\n", ret);
        return -1;
    }

    g_save_frame_count = 0;

    k_aenc_callback_s  aenc_cb;
    aenc_cb.pfn_data_cb = _aenc_dataproc;
    aenc_cb.p_private_data = NULL;
    ret = kd_mapi_aenc_registercallback(aenc_hdl,&aenc_cb);

    ret = kd_mapi_aenc_bind_ai(ai_hdl,aenc_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_aenc_bind_ai error: %x\n", ret);
        return -1;
    }

    while(1)
    {
        if (0 == _get_quit_key())
        {
            break;
        }
    }

    ret = kd_mapi_aenc_unbind_ai(ai_hdl,aenc_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_aenc_unbind_ai error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ai(ai_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ai error: %x\n", ret);
        return -1;
    }

    ret = _deinit_aenc(aenc_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_aenc error: %x\n", ret);
        return -1;
    }

    ret = _mapi_sample_vb_deinit();
    if(ret != K_SUCCESS) {
        printf("_mapi_sample_vb_deinit error: %x\n", ret);
        return -1;
    }

    g_save_frame_count = 0;

    if (g_fp != NULL)
    {
        fclose(g_fp);
        g_fp = NULL;
    }
}


static k_bool g_start_play = K_FALSE;
static int g_file_totalsize = 0;
static int g_enc_frame_len = 0;
static unsigned char *g_load_file_data = NULL;
static pthread_t g_play_thread_pid;
static void *_play_local_file_thread(void *arg)
{
    k_handle adec_hdl = *(k_handle*)arg;
    k_s32 ret;
    int nCur_data_index = 0;
    unsigned char * cur_data = NULL;
    k_audio_stream audio_stream;
    memset(&audio_stream,0,sizeof(audio_stream));
    audio_stream.len = g_enc_frame_len;

    k_u32 pool_id;
    ret =  kd_mapi_sys_get_vb_block(&pool_id, &audio_stream.phys_addr, g_enc_frame_len, NULL);

#if 1
    audio_stream.stream = _sys_mmap(audio_stream.phys_addr,g_enc_frame_len);
#endif

    while(g_start_play)
    {
        if (nCur_data_index + g_enc_frame_len > g_file_totalsize)
        {
            printf("read file again\n");
            nCur_data_index = 0;
        }

        cur_data = g_load_file_data + nCur_data_index;

        memcpy(audio_stream.stream, cur_data, g_enc_frame_len);
        audio_stream.seq++;
        //printf("audio_stream.phys_addr:0x%x,audio_stream len:%d,viraddr:0x%x\n",audio_stream.phys_addr,audio_stream.len,audio_stream.stream);

        kd_mapi_adec_send_stream(adec_hdl,&audio_stream);

        nCur_data_index += g_enc_frame_len;
    }


    kd_mapi_sys_release_vb_block(audio_stream.phys_addr, g_enc_frame_len);
#if 1
    _sys_munmap(audio_stream.phys_addr,audio_stream.stream,g_enc_frame_len);
#endif
    return NULL;
}

k_s32 audio_mapi_sample_adec_ao(k_u32 sample_rate,k_u32 channels,k_bool enable_audio_codec,const char* filename)
{
    g_enable_audio_codec = enable_audio_codec;
    k_s32 ret;
    k_handle ao_hdl = 0;
    k_handle adec_hdl = 0;

    g_enc_frame_len = sample_rate * 2 * 2 / AUDIO_PERSEC_DIV_NUM / 2;
    ret = _load_file(filename, &g_load_file_data, &g_file_totalsize);
    if(ret != K_SUCCESS) {
        printf("_load_file(%s) error: %x\n",filename, ret);
        return -1;
    }

    ret = _mapi_sample_vb_init(K_TRUE, sample_rate);
    if (ret != K_SUCCESS)
    {
        printf("audio_sample_vb_init failed\n");
        return -1;
    }

    ret = _init_ao(KD_AUDIO_BIT_WIDTH_16,sample_rate,channels,K_STANDARD_MODE,&ao_hdl);
    if(ret != K_SUCCESS) {
        printf("_init_ao error: %x\n", ret);
        return -1;
    }

    ret = _init_adec(adec_hdl,sample_rate);
    if(ret != K_SUCCESS) {
        printf("_init_adec error: %x\n", ret);
        return -1;
    }

    ret = kd_mapi_adec_bind_ao(ao_hdl,adec_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_adec_bind_ao error: %x\n", ret);
        return -1;
    }

    g_start_play = K_TRUE;
    pthread_create(&g_play_thread_pid, NULL, _play_local_file_thread, &adec_hdl);

    while(1)
    {
        if (0 == _get_quit_key())
        {
            break;
        }
    }

    g_start_play = K_FALSE;
    pthread_join(g_play_thread_pid,NULL);

    ret = kd_mapi_adec_unbind_ao(ao_hdl,adec_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_adec_unbind_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ao(ao_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_adec(adec_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_adec error: %x\n", ret);
        return -1;
    }

    ret = _mapi_sample_vb_deinit();
    if(ret != K_SUCCESS) {
        printf("_mapi_sample_vb_deinit error: %x\n", ret);
        return -1;
    }

    return 0;
}


static k_audio_stream g_audio_stream;
static k_bool g_start_loopback = K_FALSE;
static pthread_t g_loopback_thread_pid;
pthread_cond_t g_loopback_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t g_loopback_mutex = PTHREAD_MUTEX_INITIALIZER;
static k_s32 _init_audio_stream(k_u32 sample_rate,k_u32 channels)
{
    g_enc_frame_len = sample_rate * 2 * channels / AUDIO_PERSEC_DIV_NUM / 2;
    memset(&g_audio_stream,0,sizeof(g_audio_stream));
    g_audio_stream.len = g_enc_frame_len;

    k_u32 pool_id;
    kd_mapi_sys_get_vb_block(&pool_id, &g_audio_stream.phys_addr, g_enc_frame_len, NULL);

#if 1
    g_audio_stream.stream = _sys_mmap(g_audio_stream.phys_addr,g_enc_frame_len);
#endif

    return 0;
}

static  k_s32 _loopback_aenc_dataproc(k_u32 chn_num, k_audio_stream* stream_data, void* p_private_data)
{
    pthread_mutex_lock(&g_loopback_mutex);
    memcpy(g_audio_stream.stream,stream_data->stream,stream_data->len);
    pthread_mutex_unlock(&g_loopback_mutex);
    pthread_cond_signal(&g_loopback_cond);
    return 0;
}

static void *_loopback_thread(void *arg)
{
    k_handle adec_hdl = *(k_handle*)arg;
    struct timespec tv;
    int ret = 0;
    while(g_start_loopback)
    {
        pthread_mutex_lock(&g_loopback_mutex);
        clock_gettime(CLOCK_MONOTONIC, &tv);
        tv.tv_sec += 1;
        ret = pthread_cond_timedwait(&g_loopback_cond,&g_loopback_mutex,&tv);
        if (0 == ret)
        {
            kd_mapi_adec_send_stream(adec_hdl,&g_audio_stream);
        }
        pthread_mutex_unlock(&g_loopback_mutex);
    }

    return NULL;
}


static k_s32 _deinit_audio_stream()
{
    kd_mapi_sys_release_vb_block(g_audio_stream.phys_addr, g_enc_frame_len);
#if 1
    _sys_munmap(g_audio_stream.phys_addr,g_audio_stream.stream,g_enc_frame_len);
#endif

    return 0;
}

k_s32 audio_mapi_sample_audio_loopback(k_u32 sample_rate,k_u32 channels)
{
    k_s32 ret;
    k_handle ai_hdl = 0;
    k_handle aenc_hdl = 0;
    k_handle ao_hdl = 0;
    k_handle adec_hdl = 0;

    ret = _mapi_sample_vb_init(K_TRUE, sample_rate);
    if (ret != K_SUCCESS)
    {
        printf("audio_sample_vb_init failed\n");
        return -1;
    }

    _init_audio_stream(sample_rate,channels);

    ret = _init_aenc(aenc_hdl,sample_rate);
    if(ret != K_SUCCESS) {
        printf("_init_aenc error: %x\n", ret);
        return -1;
    }

    ret = _init_adec(adec_hdl,sample_rate);
    if(ret != K_SUCCESS) {
        printf("_init_adec error: %x\n", ret);
        return -1;
    }

    ret = _init_ai(KD_AUDIO_BIT_WIDTH_16,sample_rate,channels,K_STANDARD_MODE,&ai_hdl);
    if(ret != K_SUCCESS) {
        printf("_init_ai error: %x\n", ret);
        return -1;
    }

    ret = _init_ao(KD_AUDIO_BIT_WIDTH_16,sample_rate,channels,K_STANDARD_MODE,&ao_hdl);
    if(ret != K_SUCCESS) {
        printf("_init_ao error: %x\n", ret);
        return -1;
    }

    k_aenc_callback_s  aenc_cb;
    aenc_cb.pfn_data_cb = _loopback_aenc_dataproc;
    aenc_cb.p_private_data = NULL;
    ret = kd_mapi_aenc_registercallback(aenc_hdl,&aenc_cb);

    ret = kd_mapi_aenc_bind_ai(ai_hdl,aenc_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_aenc_bind_ai error: %x\n", ret);
        return -1;
    }

    ret = kd_mapi_adec_bind_ao(ao_hdl,adec_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_adec_bind_ao error: %x\n", ret);
        return -1;
    }

    g_start_loopback = K_TRUE;
    pthread_create(&g_loopback_thread_pid, NULL, _loopback_thread, &adec_hdl);

    while(1)
    {
        if (0 == _get_quit_key())
        {
            break;
        }
    }

    g_start_loopback = K_FALSE;
    pthread_join(g_loopback_thread_pid,NULL);

    ret = kd_mapi_aenc_unbind_ai(ai_hdl,aenc_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_aenc_bind_ai error: %x\n", ret);
        return -1;
    }

    ret = kd_mapi_adec_unbind_ao(ao_hdl,adec_hdl);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_adec_unbind_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ai(ai_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ai error: %x\n", ret);
        return -1;
    }

    ret = _deinit_aenc(aenc_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_aenc error: %x\n", ret);
        return -1;
    }


    ret = _deinit_ao(ao_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_adec(adec_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_adec error: %x\n", ret);
        return -1;
    }

    _deinit_audio_stream();

    ret = _mapi_sample_vb_deinit();
    if(ret != K_SUCCESS) {
        printf("_mapi_sample_vb_deinit error: %x\n", ret);
        return -1;
    }

    return 0;
}

k_s32 audio_mapi_sample_audio_double_loopback(k_u32 sample_rate,k_u32 channels,k_bool enable_audio_codec)
{
    k_s32 ret;

    ret = _mapi_sample_vb_init(K_TRUE, sample_rate);
    if (ret != K_SUCCESS)
    {
        printf("audio_sample_vb_init failed\n");
        return -1;
    }

    //pdm in 1 channel enable
    k_handle ai_pdm_hdl = 0;
    k_aio_dev_attr ai_pdm_dev_attr;
    ai_pdm_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_PDM;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.sample_rate = sample_rate;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.chn_cnt = 4; // max pdm channel
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.snd_mode = (1 == channels) ? KD_AUDIO_SOUND_MODE_MONO : KD_AUDIO_SOUND_MODE_STEREO;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.pdm_oversample = KD_AUDIO_PDM_INPUT_OVERSAMPLE_64;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.point_num_per_frame = sample_rate / ai_pdm_dev_attr.kd_audio_attr.pdm_attr.frame_num;
    if (K_SUCCESS != kd_mapi_ai_init(1,0, &ai_pdm_dev_attr,&ai_pdm_hdl))
    {
        printf("kd_mapi_ai_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ai_start(ai_pdm_hdl))
    {
        printf("pdm kd_mapi_ai_start failed\n");
        return K_FAILED;
    }

    //i2s in 1 channel enable
    k_handle ai_i2s_hdl = 0;
    k_aio_dev_attr ai_i2s_dev_attr;
    ai_i2s_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==channels)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / ai_i2s_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mapi_ai_init(0,0, &ai_i2s_dev_attr,&ai_i2s_hdl))
    {
        printf("kd_mapi_ai_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ai_start(ai_i2s_hdl))
    {
        printf("i2s kd_mapi_ai_start failed\n");
        return K_FAILED;
    }

    //i2s out 2 channel enable
    k_handle ao_i2s_hdl = 0;
    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==channels)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;


    if (K_SUCCESS != kd_mapi_ao_init(0,2/*all channel*/, &ao_dev_attr,&ao_i2s_hdl))
    {
        printf("kd_mapi_ao_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_start(ao_i2s_hdl))
    {
        printf("kd_mapi_ao_start failed\n");
        return K_FAILED;
    }

    //sysbind
    //ai_0(i2s) bind ao_1(i2s)
    ret = kd_mapi_ai_bind_ao(ai_i2s_hdl,(((0) << 16) | ((1) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("i2s->i2s kd_mapi_ai_bind_ao error: %x\n", ret);
        return -1;
    }

    //ai_1(pdm) bind ao_0(i2s)
    ret = kd_mapi_ai_bind_ao(ai_pdm_hdl,(((0) << 16) | ((0) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("pdm->i2s kd_mapi_ai_bind_ao error: %x\n", ret);
        return -1;
    }

    //modify ao audio volume
    kd_mapi_ao_set_volume(ao_i2s_hdl,6);
    //modify ai audio volume
    kd_mapi_ai_set_volume(ai_i2s_hdl,-3);

    printf("=======audio_double_loop ok\n");
    while(1)
    {
        if (0 == _get_quit_key())
        {
            break;
        }
    }

    //sys unbind
    //ai_0(i2s) unbind ao_1(i2s)
    ret = kd_mapi_ai_unbind_ao(ai_i2s_hdl,(((0) << 16) | ((1) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("i2s->i2s kd_mapi_ai_unbind_ao error: %x\n", ret);
        return -1;
    }

    //ai_1(pdm) unbind ao_0(i2s)
    ret = kd_mapi_ai_unbind_ao(ai_pdm_hdl,(((0) << 16) | ((0) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("pdm->i2s kd_mapi_ai_unbind_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ao(ao_i2s_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ai(ai_i2s_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ai error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ai(ai_pdm_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ai error: %x\n", ret);
        return -1;
    }

    //reset audio codec
    kd_mapi_acodec_reset();

    ret = _mapi_sample_vb_deinit();
    if(ret != K_SUCCESS) {
        printf("_mapi_sample_vb_deinit error: %x\n", ret);
        return -1;
    }

    return 0;
}


#include "pcm_data.h"
#include "wav_ctrl.h"
static k_s32 _get_audio_frame(k_audio_frame *audio_frame, int nSize)
{
    k_u32 pool_id;
    kd_mapi_sys_get_vb_block(&pool_id,&audio_frame->phys_addr,nSize,NULL);

    audio_frame->virt_addr = _sys_mmap(audio_frame->phys_addr,nSize);
    audio_frame->len = nSize;

    return K_SUCCESS;
}

static k_s32 _release_audio_frame(k_audio_frame *audio_frame)
{
    kd_mapi_sys_release_vb_block(audio_frame->phys_addr, audio_frame->len);
#if 1
    _sys_munmap(audio_frame->phys_addr,audio_frame->virt_addr,audio_frame->len);
#endif

    return 0;
}

k_s32 audio_mapi_sample_play_wav(const char* filename)
{
    k_s32 ret;
    k_handle ao_hdl = 0;
    int audio_channel = 0;
    int audio_samplerate = 0;
    int audio_bitpersample = 0;


    if (0 != load_wav_info(filename, &audio_channel, &audio_samplerate, &audio_bitpersample))
    {
        return -1;
    }

    ret = _mapi_sample_vb_init(K_TRUE, 48000);
    if (ret != K_SUCCESS)
    {
        printf("audio_sample_vb_init failed\n");
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

    g_enable_audio_codec = K_TRUE;
    ret = _init_ao(KD_AUDIO_BIT_WIDTH_16,audio_samplerate,audio_channel,K_STANDARD_MODE,&ao_hdl);
    if(ret != K_SUCCESS) {
        printf("_init_ao error: %x\n", ret);
        return -1;
    }

    while (1)
    {
        // 获取音频数据
        if (get_pcm_data_from_file(pDataBuf, audio_frame.len / 4) < 0)
        {
            break;
        }

        // 发送音频数据
        ret = kd_mapi_ao_send_frame(ao_hdl, &audio_frame);
    }
    usleep(1000*100);

    ret = _deinit_ao(ao_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    _release_audio_frame(&audio_frame);

    _mapi_sample_vb_deinit();
    return 0;
}