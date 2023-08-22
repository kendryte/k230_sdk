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

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "mapi_sys_api.h"
#include "mapi_vdec_api.h"
#include "vo_cfg.h"
#include <stdlib.h>
#include <errno.h>

#define ENABLE_VDEC_DEBUG 1
#define BIND_VO_LAYER 1

#ifdef ENABLE_VDEC_DEBUG
#define vdec_debug printf
#else
#define vdec_debug(ARGS...)
#endif

#define MAX_WIDTH 1088
#define MAX_HEIGHT 1920
#define STREAM_BUF_SIZE MAX_WIDTH *MAX_HEIGHT
#define FRAME_BUF_SIZE MAX_WIDTH *MAX_HEIGHT * 2
#define INPUT_BUF_CNT 4
#define OUTPUT_BUF_CNT 6

static k_bool g_vb_init = K_FALSE;
static k_mapi_media_attr_t media_attr = {0};

typedef struct
{
    k_pixel_format chn_format;
    k_u32 file_size;
    k_s32 pool_id;
    pthread_t input_tid;
    pthread_t config_vo_tid;
    sem_t sem_vdec_done;
    FILE *input_file;
    k_u32 ch_id;
    char *dump_frame;
    k_u32 dump_frame_size;
    k_bool done;
    k_payload_type type;
    k_vb_blk_handle vb_handle[INPUT_BUF_CNT];
    k_pixel_format pic_format;
    k_u32 act_width;
    k_u32 act_height;
    k_u32 input_pool_id;
    k_u32 output_pool_id;
} sample_vdec_conf_t;

static sample_vdec_conf_t g_vdec_conf[VDEC_MAX_CHN_NUMS];

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

static void _show_help()
{
    printf("Please input:\n");
    printf("-i: input file name\n");
    printf("./sample_vdec -i file(*.264;*.265;*.jpg) \n");
}

static k_s32 _vb_create_pool(int ch)
{
    k_vb_pool_config pool_config;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = INPUT_BUF_CNT;
    pool_config.blk_size = STREAM_BUF_SIZE;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].input_pool_id = kd_mapi_vb_create_pool(&pool_config);
    vdec_debug("input_pool_id %d\n", g_vdec_conf[ch].input_pool_id);

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = OUTPUT_BUF_CNT;
    pool_config.blk_size = FRAME_BUF_SIZE;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].output_pool_id = kd_mapi_vb_create_pool(&pool_config);
    vdec_debug("output_pool_id %d\n", g_vdec_conf[ch].output_pool_id);

    return 0;
}

static k_s32 vb_destory_pool(int ch)
{
    vdec_debug("destory_pool input %d \n", g_vdec_conf[ch].input_pool_id);
    kd_mapi_vb_destory_pool(g_vdec_conf[ch].input_pool_id);
    vdec_debug("destory_pool output %d \n", g_vdec_conf[ch].output_pool_id);
    kd_mapi_vb_destory_pool(g_vdec_conf[ch].output_pool_id);

    return 0;
}

static k_s32 _sample_vb_init()
{
    if (g_vb_init)
    {
        vdec_debug("%s already init\n", __FUNCTION__);
        return K_SUCCESS;
    }

    k_s32 ret;
    ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS)
    {
        vdec_debug("kd_mapi_sys_init error: %x\n", ret);
        return K_FAILED;
    }

    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));
    k_vb_config *config = &media_attr.media_config.vb_config;

    memset(config, 0, sizeof(*config));
    config->max_pool_cnt = 2;

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS)
    {
        vdec_debug("kd_mapi_media_init error: %x\n", ret);
        return K_FAILED;
    }

    g_vb_init = K_TRUE;

    return ret;
}

static k_s32 _mapi_sample_vb_deinit()
{
    if (!g_vb_init)
    {
        vdec_debug("%s not init\n", __FUNCTION__);
        return K_FAILED;
    }
    k_s32 ret;
    ret = kd_mapi_media_deinit();
    if (ret != K_SUCCESS)
    {
        vdec_debug("kd_mapi_media_deinit error: %x\n", ret);
        return -1;
    }
    ret = kd_mapi_sys_deinit();
    if (ret != K_SUCCESS)
    {
        vdec_debug("kd_mapi_sys_deinit error: %x\n", ret);
        return -1;
    }

    g_vb_init = K_FALSE;

    return ret;
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
        virt_addr = (void *)((char *)mmap_addr + (phys_addr & page_mask));
    else
    {
        vdec_debug("mmap addr error: %d %s.\n", mmap_addr, strerror(errno));
        ;
    }

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phy_addr, void *virt_addr, k_u32 size)
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
    if (ret == -1)
    {
        vdec_debug("munmap error.\n");
    }

    return 0;
}

static void *vo_cfg_thread(void *arg)
{
    k_s32 ret;
    sample_vdec_conf_t *vdec_conf;
    vdec_conf = (sample_vdec_conf_t *)arg;

    k_vdec_chn_status status;
    while (1)
    {
        ret = kd_mapi_vdec_query_status(vdec_conf->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.width != 0 && status.height != 0)
        {
            break;
        }
        usleep(1000*100);
    }

    printf("vo cfg width:%d,height:%d\n",status.width,status.height);
    vo_layer_init(status.width,status.height);
    //kd_mapi_vdec_bind_vo(vdec_conf->ch_id, 0, BIND_VO_LAYER);

    return NULL;
}

static void *input_thread(void *arg)
{
    sample_vdec_conf_t *vdec_conf;
    k_vdec_stream stream;
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u8 *virt_addr;
    k_u32 blk_size, stream_len;
    k_u32 file_size;
    int i = 0;
    k_s32 ret;

    vdec_conf = (sample_vdec_conf_t *)arg;

    file_size = 0;
    blk_size = STREAM_BUF_SIZE;
    static int nseq = 0;

    int poolid = vdec_conf->input_pool_id;
    vdec_debug("%s>poolid:%d \n", __func__, poolid);
    while (file_size < vdec_conf->file_size)
    {
        memset(&stream, 0, sizeof(k_vdec_stream));

        ret = kd_mapi_sys_get_vb_block_from_pool_id(poolid, &phys_addr, blk_size, NULL);

        if (K_SUCCESS != ret)
        {
            // vdec_debug("%s>no vb\n", __func__);
            usleep(30000);
            continue;
        }

        if (i >= INPUT_BUF_CNT)
            i = 0;
        vdec_conf->pool_id = pool_id;

        virt_addr = _sys_mmap(phys_addr, blk_size);

        if (file_size + blk_size > vdec_conf->file_size)
        {
            fread(virt_addr, 1, (vdec_conf->file_size - file_size), vdec_conf->input_file);
            stream_len = vdec_conf->file_size - file_size;
            stream.end_of_stream = K_TRUE;
        }
        else
        {
            fread(virt_addr, 1, blk_size, vdec_conf->input_file);
            stream_len = blk_size;
        }

        file_size += stream_len;

        stream.phy_addr = phys_addr;
        stream.len = stream_len;

        vdec_debug("ch %d send stream: phys_addr 0x%lx, len %d\n", vdec_conf->ch_id, stream.phy_addr, stream.len);
        ret = kd_mapi_vdec_send_stream(vdec_conf->ch_id, &stream, -1);
        CHECK_RET(ret, __func__, __LINE__);
        _sys_munmap(phys_addr, virt_addr, blk_size);

        ret = kd_mapi_sys_release_vb_block(phys_addr, blk_size);
        CHECK_RET(ret, __func__, __LINE__);

        i++;
    }

    // check vdec over
    k_vdec_chn_status status;
    while (1)
    {
        ret = kd_mapi_vdec_query_status(vdec_conf->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.end_of_stream)
        {
            vdec_debug("%s>ch %d, receive eos\n", __func__, vdec_conf->ch_id);
            break;
        }
        sleep(1);
    }

    g_vdec_conf[vdec_conf->ch_id].done = K_TRUE;
    sem_post(&g_vdec_conf[vdec_conf->ch_id].sem_vdec_done);
    return arg;
}

int main(int argc, char *argv[])
{
    int ch = 0;
    k_vdec_chn_attr attr;
    k_payload_type type = K_PT_BUTT;
    int j;
    int i;
    FILE *input_file = NULL;
    k_s32 ret;

    if (argc <= 1)
    {
        _show_help();
        return -1;
    }

    for (i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            _show_help();
            return -1;
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            char *ptr = strchr(argv[i + 1], '.');

            vdec_debug("infilename: %s\n", argv[i + 1]);
            if ((input_file = fopen(argv[i + 1], "rb")) == NULL)
            {
                vdec_debug("Cannot open input file!!!\n");
                return -1;
            }

            if (strcmp(ptr, ".h264") == 0 || strcmp(ptr, ".264") == 0)
            {
                type = K_PT_H264;
                vdec_debug("file type is H264\n");
            }
            else if (strcmp(ptr, ".jpeg") == 0 || strcmp(ptr, ".mjpeg") == 0 || strcmp(ptr, ".jpg") == 0)
            {
                type = K_PT_JPEG;
                vdec_debug("file type is JPEG\n");
            }
            else if (strcmp(ptr, ".h265") == 0 || strcmp(ptr, ".hevc") == 0 || strcmp(ptr, ".265") == 0)
            {
                type = K_PT_H265;
                vdec_debug("file type is H265\n");
            }
            else
            {
                vdec_debug("Error input type\n");
                return -1;
            }
        }
        else
        {
            vdec_debug("Error :Invalid arguments %s\n", argv[i]);
            return -1;
        }
    }

    if (_sample_vb_init())
    {
        return -1;
    }

    _vb_create_pool(ch);

    g_vdec_conf[ch].ch_id = ch;

    for (j = 0; j < INPUT_BUF_CNT; j++)
    {
        g_vdec_conf[ch].vb_handle[j] = VB_INVALID_HANDLE;
    }

    g_vdec_conf[ch].input_file = input_file;
    if (g_vdec_conf[ch].input_file)
    {
        fseek(g_vdec_conf[ch].input_file, 0L, SEEK_END);
        g_vdec_conf[ch].file_size = ftell(g_vdec_conf[ch].input_file);
        fseek(g_vdec_conf[ch].input_file, 0, SEEK_SET);
    }
    else
        g_vdec_conf[ch].file_size = 0;

    vdec_debug("input file size %d\n", g_vdec_conf[ch].file_size);

    attr.pic_width = MAX_WIDTH;
    attr.pic_height = MAX_HEIGHT;
    attr.frame_buf_cnt = OUTPUT_BUF_CNT;
    attr.frame_buf_size = FRAME_BUF_SIZE;
    attr.stream_buf_size = STREAM_BUF_SIZE;
    attr.pic_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    attr.type = type;
    attr.frame_buf_pool_id = g_vdec_conf[ch].output_pool_id;
    g_vdec_conf[ch].pic_format = attr.pic_format;
    ret = kd_mapi_vdec_init(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mapi_vdec_start(ch);
    CHECK_RET(ret, __func__, __LINE__);

    kd_mapi_vdec_bind_vo(ch, 0, BIND_VO_LAYER);

    sem_init(&g_vdec_conf[ch].sem_vdec_done, 0, 0);
    pthread_create(&g_vdec_conf[ch].input_tid, NULL, input_thread, &g_vdec_conf[ch]);
    pthread_create(&g_vdec_conf[ch].config_vo_tid, NULL, vo_cfg_thread, &g_vdec_conf[ch]);

    sem_wait(&g_vdec_conf[ch].sem_vdec_done);
    if (g_vdec_conf[ch].done == K_TRUE)
    {
        kd_mapi_vdec_unbind_vo(ch, 0, BIND_VO_LAYER);
        ret = kd_mapi_vdec_stop(ch);
        CHECK_RET(ret, __func__, __LINE__);

        ret = kd_mapi_vdec_deinit(ch);
        CHECK_RET(ret, __func__, __LINE__);

        vo_layer_deinit();

        pthread_join(g_vdec_conf[ch].input_tid, NULL);
        pthread_join(g_vdec_conf[ch].config_vo_tid, NULL);

        fclose(g_vdec_conf[ch].input_file);
        vb_destory_pool(ch);
        sem_destroy(&g_vdec_conf[ch].sem_vdec_done);
        vdec_debug("kill ch %d thread done!\n", ch);
    }

    _mapi_sample_vb_deinit();

    printf("sample done\n");
    return 0;
}
