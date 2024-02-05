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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vdec_api.h"
#include "mpi_vo_api.h"
#include "mpi_sys_api.h"
#include "k_vdec_comm.h"
#include "k_vvo_comm.h"
#include "vo_test_case.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#define ENABLE_VDEC_DEBUG    1
#define BIND_VO_LAYER   1

#ifdef ENABLE_VDEC_DEBUG
    #define vdec_debug  printf
#else
    #define vdec_debug(ARGS...)
#endif

#define MAX_WIDTH 1088
#define MAX_HEIGHT 1920
#define STREAM_BUF_SIZE MAX_WIDTH*MAX_HEIGHT
#define FRAME_BUF_SIZE MAX_WIDTH*MAX_HEIGHT*2
#define INPUT_BUF_CNT   4
#define OUTPUT_BUF_CNT  6

typedef struct
{
    k_pixel_format chn_format;
    k_u32 file_size;
    k_s32 pool_id;
    pthread_t input_tid;
    pthread_t output_tid;
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
    k_u32 dec_stream_frames;
} sample_vdec_conf_t;

static sample_vdec_conf_t g_vdec_conf[VDEC_MAX_CHN_NUMS];
static k_connector_type g_connector_type = LT9611_MIPI_4LAN_1920X1080_30FPS;

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

static k_s32 sample_vb_init()
{
    k_s32 ret;
    k_vb_config config;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 2;

    ret = kd_mpi_vb_set_config(&config);

    vdec_debug("-----------vdec sample test------------------------\n");

    if (ret)
        vdec_debug("vb_set_config failed ret:%d\n", ret);

    ret = kd_mpi_vb_init();
    if (ret)
        vdec_debug("vb_init failed ret:%d\n", ret);

    return ret;
}

static k_s32 vb_create_pool(int ch)
{
    k_vb_pool_config pool_config;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = INPUT_BUF_CNT;
    pool_config.blk_size = STREAM_BUF_SIZE;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].input_pool_id = kd_mpi_vb_create_pool(&pool_config);
    vdec_debug("input_pool_id %d\n", g_vdec_conf[ch].input_pool_id);

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = OUTPUT_BUF_CNT;
    pool_config.blk_size = FRAME_BUF_SIZE;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].output_pool_id = kd_mpi_vb_create_pool(&pool_config);
    vdec_debug("output_pool_id %d\n", g_vdec_conf[ch].output_pool_id);

    return 0;
}

static k_s32 vb_destory_pool(int ch)
{
    vdec_debug("destory_pool input %d \n", g_vdec_conf[ch].input_pool_id);
    kd_mpi_vb_destory_pool(g_vdec_conf[ch].input_pool_id);
    vdec_debug("destory_pool output %d \n", g_vdec_conf[ch].output_pool_id);
    kd_mpi_vb_destory_pool(g_vdec_conf[ch].output_pool_id);

    return 0;
}

static k_s32 sample_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        vdec_debug("vb_exit failed ret:%d\n", ret);
    return ret;
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

    int poolid = vdec_conf->input_pool_id;
    vdec_debug("%s>poolid:%d \n", __func__, poolid);
    while (file_size < vdec_conf->file_size)
    {
        memset(&stream, 0, sizeof(k_vdec_stream));
        handle = kd_mpi_vb_get_block(poolid, blk_size, NULL);

        if (handle == VB_INVALID_HANDLE)
        {
            //vdec_debug("%s>no vb\n", __func__);
            usleep(30000);
            continue;
        }

        pool_id = kd_mpi_vb_handle_to_pool_id(handle);
        if (pool_id == VB_INVALID_POOLID)
        {
            vdec_debug("%s get pool id error\n", __func__);
            break;
        }
        if(i >= INPUT_BUF_CNT)
            i = 0;
        vdec_conf->pool_id = pool_id;
        vdec_conf->vb_handle[i] = handle;

        phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
        if (phys_addr == 0)
        {
            vdec_debug("%s get phys addr error\n", __func__);
            break;
        }

        vdec_debug("%s>ch %d, vb_handle 0x%x, pool_id %d, blk_size %d\n", __func__,
                   vdec_conf->ch_id, vdec_conf->vb_handle[i], pool_id, blk_size);

        virt_addr = (k_u8 *)kd_mpi_sys_mmap_cached(phys_addr, blk_size);

        if (virt_addr == NULL)
        {
            vdec_debug("%s mmap error\n", __func__);
            break;
        }

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

        ret = kd_mpi_sys_mmz_flush_cache(phys_addr, virt_addr, stream_len);
        CHECK_RET(ret, __func__, __LINE__);

        file_size += stream_len;

        stream.phy_addr = phys_addr;
        stream.len = stream_len;

        vdec_debug("ch %d send stream: phys_addr 0x%lx, len %d\n", vdec_conf->ch_id, stream.phy_addr, stream.len);
        ret = kd_mpi_vdec_send_stream(vdec_conf->ch_id, &stream, -1);
        CHECK_RET(ret, __func__, __LINE__);

        ret = kd_mpi_sys_munmap((void *)virt_addr, blk_size);
        CHECK_RET(ret, __func__, __LINE__);

        ret = kd_mpi_vb_release_block(handle);
        CHECK_RET(ret, __func__, __LINE__);

        i++;

        if (vdec_conf->done)
            break;
    }

    return arg;
}

int vo_creat_layer_test(k_vo_layer chn_id, layer_info *info)
{
    // printf("The answer is: %d\n", 3);
    k_vo_video_layer_attr attr;

    // check layer
    if ((chn_id >= K_MAX_VO_LAYER_NUM) || ((info->func & K_VO_SCALER_ENABLE) && (chn_id != K_VO_LAYER0))
            || ((info->func != 0) && (chn_id == K_VO_LAYER2)))
    {
        printf("input layer num failed \n");
        return -1 ;
    }

    memset(&attr, 0, sizeof(attr));

    // set offset
    attr.display_rect = info->offset;
    // set act
    attr.img_size = info->act_size;
    // sget size
    info->size = info->act_size.height * info->act_size.width * 3 / 2;
    //set pixel format
    attr.pixel_format = info->format;
    if (info->format != PIXEL_FORMAT_YVU_PLANAR_420)
    {
        printf("input pix format failed \n");
        return -1;
    }
    // set stride
    attr.stride = (info->act_size.width / 8 - 1) + ((info->act_size.height - 1) << 16);
    // set function
    attr.func = info->func;
    // set scaler attr
    attr.scaler_attr = info->attr;

    // set video layer atrr
    kd_mpi_vo_set_video_layer_attr(chn_id, &attr);

    // enable layer
    kd_mpi_vo_enable_video_layer(chn_id);

    return 0;
}

k_s32 sample_vdec_vo_init(layer_bind_config *config)
{
    k_vo_pub_attr attr;
    layer_info info;
    k_vo_layer chn_id = config->ch;
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type = g_connector_type;
    k_connector_info connector_info;

    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));
    memset(&connector_info, 0, sizeof(k_connector_info));

    //connector get sensor info
    ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    connector_fd = kd_mpi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return K_ERR_VO_NOTREADY;
    }

    // set connect power
    kd_mpi_connector_power_set(connector_fd, K_TRUE);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    printf("%s>w %d, h %d\n", __func__, config->w, config->h);
    // config lyaer
    info.act_size.width = config->w;//1080;//640;//1080;
    info.act_size.height = config->h;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = config->ro;
    info.global_alptha = 0xff;
    info.offset.x = 0;//(1080-w)/2,
    info.offset.y = 0;//(1920-h)/2;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;
    vo_creat_layer_test(chn_id, &info);

    //exit ;
    return 0;
}

static void *output_thread(void *arg)
{
    sample_vdec_conf_t *vdec_conf;
    k_s32 ret;
    int out_cnt;
    k_video_frame_info output;
    k_vdec_supplement_info supplement;
    FILE *output_file=NULL;
    void *virt_addr = NULL;
    k_u32 frame_size=0;

    vdec_conf = (sample_vdec_conf_t *)arg;
    int first = 0;

    while (1)
    {
        k_vdec_chn_status status;

        ret = kd_mpi_vdec_query_status(vdec_conf->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.end_of_stream)
        {
            vdec_debug("%s>ch %d, receive eos\n", __func__, vdec_conf->ch_id);
            vdec_conf->dec_stream_frames = status.dec_stream_frames;
            break;
        }
        else if (status.width != vdec_conf->act_width && first == 0)
        {
            // printf("The answer is: %d\n", 1);
            layer_bind_config config;
            vdec_conf->act_width = status.width;
            vdec_conf->act_height = status.height;
            printf("status.width:%d status.height:%d \n",status.width, status.height);
            config.ch = BIND_VO_LAYER;
            if(status.width > 1080 && g_connector_type == LT9611_MIPI_4LAN_1920X1080_30FPS)
            {
                config.w = status.height;//1080;
                config.h = status.width;//1920;
                config.ro = 0;
            }
            else
            {
                config.w = status.width ;
                config.h = status.height;
                config.ro = 0;
            }
            sample_vdec_vo_init(&config);
            first = 1;
        }
        else{
            usleep(10000);
        }

    }

    vdec_conf->done = K_TRUE;

    return arg;
}

static void sample_vdec_bind_vo(k_u32 chn_id)
{
    k_mpp_chn vdec_mpp_chn;
    k_mpp_chn vvo_mpp_chn;
    k_s32 ret;

    vdec_mpp_chn.mod_id = K_ID_VDEC;
    vdec_mpp_chn.dev_id = 0;
    vdec_mpp_chn.chn_id = 0;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = 0;//VVO_DISPLAY_DEV_ID;
    vvo_mpp_chn.chn_id = chn_id;//VVO_DISPLAY_CHN_ID;
    ret = kd_mpi_sys_bind(&vdec_mpp_chn, &vvo_mpp_chn);
    if (ret)
    {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }

    return;
}

static void sample_vdec_unbind_vo(k_u32 chn_id)
{
    k_mpp_chn vdec_mpp_chn;
    k_mpp_chn vvo_mpp_chn;

    vdec_mpp_chn.mod_id = K_ID_VDEC;
    vdec_mpp_chn.dev_id = 0;
    vdec_mpp_chn.chn_id = 0;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = 0;//VVO_DISPLAY_DEV_ID;
    vvo_mpp_chn.chn_id = chn_id;//VVO_DISPLAY_CHN_ID;
    kd_mpi_sys_unbind(&vdec_mpp_chn, &vvo_mpp_chn);

    return;
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

    memset(g_vdec_conf, 0, sizeof(sample_vdec_conf_t)*VDEC_MAX_CHN_NUMS);

    for (i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            printf("Usage: ./sample_vdec.elf -i test.265 -type 0\n");
            printf("-i: input file name\n");
            printf("-type: vo type, default 0, see vo doc \n");
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
        else if (strcmp(argv[i], "-type") == 0) {
            g_connector_type = (k_connector_type)atoi(argv[i + 1]);
            printf("g_connector_type = %d...\n", g_connector_type);
        }
        else
        {
            vdec_debug("Error :Invalid arguments %s\n", argv[i]);
            return -1;
        }
    }

    if (sample_vb_init())
    {
        return -1;
    }

    vb_create_pool(ch);

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
    attr.type = type;
	attr.frame_buf_pool_id = g_vdec_conf[ch].output_pool_id;
    ret = kd_mpi_vdec_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_vdec_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);

    sample_vdec_bind_vo(BIND_VO_LAYER);

    pthread_create(&g_vdec_conf[ch].input_tid, NULL, input_thread, &g_vdec_conf[ch]);
    pthread_create(&g_vdec_conf[ch].output_tid, NULL, output_thread, &g_vdec_conf[ch]);

    while (1)
    {
        if(g_vdec_conf[ch].dec_stream_frames == 1 && type == K_PT_JPEG)
        {
            sleep(3);
            vdec_debug("one jpeg picture\n");
        }
        if (g_vdec_conf[ch].done == K_TRUE)
        {
            sample_vdec_unbind_vo(BIND_VO_LAYER);
            kd_mpi_vo_disable_video_layer(BIND_VO_LAYER);

            ret = kd_mpi_vdec_stop_chn(ch);
            CHECK_RET(ret, __func__, __LINE__);

            ret = kd_mpi_vdec_destroy_chn(ch);
            CHECK_RET(ret, __func__, __LINE__);

            pthread_kill(g_vdec_conf[ch].input_tid, SIGALRM);
            pthread_join(g_vdec_conf[ch].input_tid, NULL);

            pthread_kill(g_vdec_conf[ch].output_tid, SIGALRM);
            pthread_join(g_vdec_conf[ch].output_tid, NULL);

            fclose(g_vdec_conf[ch].input_file);
            vb_destory_pool(ch);
            vdec_debug("kill ch %d thread done!\n", ch);
            break;
        }
        else
            usleep(50000);
    }

    ret = kd_mpi_vdec_close_fd();
    CHECK_RET(ret, __func__, __LINE__);

    sample_vb_exit();

    vdec_debug("sample decode done!\n");

    return 0;
}
