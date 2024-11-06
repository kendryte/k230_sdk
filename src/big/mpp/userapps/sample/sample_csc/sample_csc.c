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
#include "mpi_nonai_2d_api.h"
#include "mpi_venc_api.h"
#include "mpi_sys_api.h"
#include "k_nonai_2d_comm.h"
#include "k_venc_comm.h"
#include "mpi_vicap_api.h"
#include "mpi_vo_api.h"
#include "vo_test_case.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080
#define VICAP_BUF_CNT 4
#define NONAI_2D_BUF_CNT 10
#define VENC_BUF_CNT  6
#define OSD_BUF_CNT   1
#define ISP_CHN0_WIDTH  (1280)
#define ISP_CHN0_HEIGHT (720)
#define OSD_MAX_WIDTH 100
#define OSD_MAX_HEIGHT 100
#define OSD_BUF_SIZE OSD_MAX_WIDTH*OSD_MAX_HEIGHT*4
#define NONAI_2D_BIND_CH     0
#define NONAI_2D_RGB888_CH   1
#define NONAI_2D_RGB565_CH   2

typedef struct
{
    k_u32 width;
    k_u32 height;
    k_vicap_sensor_type sensor_type;
    k_connector_type vo;
    pthread_t dump_tid;
    pthread_t dump_tid_1;
    pthread_t output_tid;
    char out_filename[50];
    FILE *output_file;
    k_u64 osd_phys_addr;
    k_vb_blk_handle osd_blk_handle;
} nonai_2d_conf_t;

static nonai_2d_conf_t g_nonai_2d_conf;
extern const unsigned int osd_data;
extern const int osd_data_size;
static k_u8 *dump_buf_rgb888=NULL;
static k_u8 *dump_buf_rgb565=NULL;
static k_bool vicap_started=K_FALSE;

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

static void sample_vicap_config()
{
    k_s32 ret;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));

    sensor_info.sensor_type = g_nonai_2d_conf.sensor_type;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info.sensor_type, &sensor_info);
    CHECK_RET(ret, __func__, __LINE__);

    dev_attr.acq_win.width = sensor_info.width;
    dev_attr.acq_win.height = sensor_info.height;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(VICAP_DEV_ID_0, dev_attr);
    CHECK_RET(ret, __func__, __LINE__);

    chn_attr.out_win.width = g_nonai_2d_conf.width;
    chn_attr.out_win.height = g_nonai_2d_conf.height;

    chn_attr.crop_win = chn_attr.out_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.alignment = 12;

    chn_attr.pix_format = PIXEL_FORMAT_RGB_888;
    chn_attr.buffer_num = VICAP_BUF_CNT;
    chn_attr.buffer_size = (g_nonai_2d_conf.width * g_nonai_2d_conf.height * 3 + 0xfff) & ~ 0xfff;

    ret = kd_mpi_vicap_set_chn_attr(VICAP_DEV_ID_0, VICAP_CHN_ID_0, chn_attr);
    CHECK_RET(ret, __func__, __LINE__);

    chn_attr.out_win.width = g_nonai_2d_conf.width;
    chn_attr.out_win.height = g_nonai_2d_conf.height;

    chn_attr.crop_win = chn_attr.out_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.alignment = 12;

    chn_attr.pix_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    chn_attr.buffer_num = VICAP_BUF_CNT;
    chn_attr.buffer_size = (g_nonai_2d_conf.width * g_nonai_2d_conf.height * 3 / 2 + 0xfff) & ~ 0xfff;

    ret = kd_mpi_vicap_set_chn_attr(VICAP_DEV_ID_0, VICAP_CHN_ID_1, chn_attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_vicap_init(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);

}

static k_s32 sample_connector_init(k_connector_type type)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type = type;
	k_connector_info connector_info;

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

    return 0;
}

static void sample_vo_init(k_connector_type type)
{
    layer_info info;
    k_vo_layer chn_id = K_VO_LAYER1;

    memset(&info, 0, sizeof(info));

    sample_connector_init(type);

    if(type == HX8377_V2_MIPI_4LAN_1080X1920_30FPS)
    {
        info.act_size.width = ISP_CHN0_HEIGHT ;//1080;//640;//1080;
        info.act_size.height = ISP_CHN0_WIDTH;//1920;//480;//1920;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = K_ROTATION_90;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = 0;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        vo_creat_layer_test(chn_id, &info);
    }
    else
    {
        info.act_size.width = ISP_CHN0_WIDTH ;//1080;//640;//1080;
        info.act_size.height = ISP_CHN0_HEIGHT;//1920;//480;//1920;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = 0;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = 0;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        vo_creat_layer_test(chn_id, &info);
    }
}

void sample_vicap_start()
{
    k_s32 ret;

    ret = kd_mpi_vicap_start_stream(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);

    vicap_started = K_TRUE;
}

void sample_vicap_stop()
{
    k_s32 ret;

    vicap_started = K_FALSE;

    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
}

static k_s32 sample_vb_init()
{
    k_s32 ret;
    k_vb_config config;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 7;
    config.comm_pool[0].blk_cnt = VICAP_BUF_CNT;
    config.comm_pool[0].blk_size = VICAP_ALIGN_UP(g_nonai_2d_conf.width*g_nonai_2d_conf.height*3, 0x1000);
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_cnt = VICAP_BUF_CNT;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP(g_nonai_2d_conf.width*g_nonai_2d_conf.height*3/2, 0x1000);
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[2].blk_cnt = NONAI_2D_BUF_CNT;
    config.comm_pool[2].blk_size = VICAP_ALIGN_UP(g_nonai_2d_conf.width*g_nonai_2d_conf.height*3, 0x1000);
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[3].blk_cnt = NONAI_2D_BUF_CNT;
    config.comm_pool[3].blk_size = VICAP_ALIGN_UP(g_nonai_2d_conf.width*g_nonai_2d_conf.height*3/2, 0x1000);
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[4].blk_cnt = NONAI_2D_BUF_CNT;
    config.comm_pool[4].blk_size = VICAP_ALIGN_UP(g_nonai_2d_conf.width*g_nonai_2d_conf.height*2, 0x1000);
    config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[5].blk_cnt = OSD_BUF_CNT;
    config.comm_pool[5].blk_size = OSD_BUF_SIZE;
    config.comm_pool[5].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[6].blk_cnt = VENC_BUF_CNT;
    config.comm_pool[6].blk_size = VICAP_ALIGN_UP(g_nonai_2d_conf.width*g_nonai_2d_conf.height/2, 0x1000);
    config.comm_pool[6].mode = VB_REMAP_MODE_NOCACHE;


    ret = kd_mpi_vb_set_config(&config);

    if (ret)
        printf("vb_set_config failed ret:%d\n", ret);

    ret = kd_mpi_vb_init();
    if (ret)
        printf("vb_init failed ret:%d\n", ret);

    return ret;
}

static k_s32 sample_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static k_s32 prepare_osd()
{
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u8 *virt_addr_osd;

    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, OSD_BUF_SIZE, NULL);

    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s osd get vb block error\n", __func__);
    }

    pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if (pool_id == VB_INVALID_POOLID)
    {
        printf("%s osd get pool id error\n", __func__);
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s osd get phys addr error\n", __func__);
    }

    printf("%s>phys_addr 0x%lx, blk_size %d\n", __func__, phys_addr, OSD_BUF_SIZE);
    virt_addr_osd = (k_u8 *)kd_mpi_sys_mmap_cached(phys_addr, OSD_BUF_SIZE);

    if (virt_addr_osd == NULL)
    {
        printf("%s osd mmap error\n", __func__);
    }

    memcpy(virt_addr_osd, &osd_data, osd_data_size);
    kd_mpi_sys_mmz_flush_cache(phys_addr, virt_addr_osd, OSD_BUF_SIZE);

    g_nonai_2d_conf.osd_phys_addr = phys_addr;
    g_nonai_2d_conf.osd_blk_handle = handle;

    return K_SUCCESS;
}

static void *output_thread(void *arg)
{
    k_venc_stream output;
    int out_cnt, out_frames;
    k_s32 ret;
    int i;
    k_u32 total_len = 0;

    out_cnt = 0;
    out_frames = 0;

    while (1)
    {
        k_venc_chn_status status;

        ret = kd_mpi_venc_query_status(0, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;

        output.pack = malloc(sizeof(k_venc_pack) * output.pack_cnt);

        ret = kd_mpi_venc_get_stream(0, &output, -1);
        CHECK_RET(ret, __func__, __LINE__);

        out_cnt += output.pack_cnt;
        for (i = 0; i < output.pack_cnt; i++)
        {
            if (output.pack[i].type != K_VENC_HEADER)
            {
                out_frames++;
            }

            k_u8 *pData;
            pData = (k_u8 *)kd_mpi_sys_mmap(output.pack[i].phys_addr, output.pack[i].len);
            if (g_nonai_2d_conf.output_file)
                fwrite(pData, 1, output.pack[i].len, g_nonai_2d_conf.output_file);
            kd_mpi_sys_munmap(pData, output.pack[i].len);

            total_len += output.pack[i].len;
        }

        ret = kd_mpi_venc_release_stream(0, &output);
        CHECK_RET(ret, __func__, __LINE__);

        free(output.pack);

        //insert a new osd and border
        if(out_frames >= 90)
        {
            int ch = 0;
            k_venc_2d_osd_attr venc_2d_osd_attr;
            k_venc_2d_border_attr venc_2d_border_attr;
            int index;

            index = 1;
            venc_2d_osd_attr.width  = 40;
            venc_2d_osd_attr.height = 40;
            venc_2d_osd_attr.startx = 80;
            venc_2d_osd_attr.starty = 80;
            venc_2d_osd_attr.phys_addr[0] = g_nonai_2d_conf.osd_phys_addr;
            venc_2d_osd_attr.phys_addr[1] = 0;
            venc_2d_osd_attr.phys_addr[2] = 0;
            venc_2d_osd_attr.bg_alpha = 200;
            venc_2d_osd_attr.osd_alpha = 100;
            venc_2d_osd_attr.video_alpha = 200;
            venc_2d_osd_attr.add_order = K_VENC_2D_ADD_ORDER_VIDEO_OSD;
            venc_2d_osd_attr.bg_color = (200 << 16) | (128 << 8) | (128 << 0);
            venc_2d_osd_attr.fmt = K_VENC_2D_OSD_FMT_ARGB8888;

            ret = kd_mpi_venc_set_2d_osd_param(ch, index, &venc_2d_osd_attr);
            CHECK_RET(ret, __func__, __LINE__);

            venc_2d_border_attr.width = 40;
            venc_2d_border_attr.height = 40;
            venc_2d_border_attr.line_width = 2;
            venc_2d_border_attr.color = (100 << 16) | (200 << 8) | (70 << 0);
            venc_2d_border_attr.startx = 130;
            venc_2d_border_attr.starty = 80;
            ret = kd_mpi_venc_set_2d_border_param(ch, index, &venc_2d_border_attr);
            CHECK_RET(ret, __func__, __LINE__);
        }
    }

    printf("%s>done, out_frames %d, size %d bits\n", __func__, out_frames, total_len * 8);
    return arg;
}

static void *dump_thread(void *arg)
{
    k_s32 ret;
    k_video_frame_info dumm_vf_info;
    k_video_frame_info nonai_2d_vf_info;
    k_u32 dump_cnt=0;
    k_u32 size = ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3;

    dump_buf_rgb888 = malloc(size);

    while (1)
    {
        if(!vicap_started)
        {
            usleep(30000);
            continue;
        }

        ret = kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dumm_vf_info, 1000);
        if (ret) {
            printf("sample_vicap, chn(1) dump frame failed.\n");
            continue;
        }

        ret = kd_mpi_nonai_2d_send_frame(NONAI_2D_RGB888_CH, &dumm_vf_info, 1000);
        if (ret) {
            printf("kd_mpi_nonai_2d_send_frame failed. %d\n", ret);
            kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_1, &dumm_vf_info);
            continue;
        }

        ret = kd_mpi_nonai_2d_get_frame(NONAI_2D_RGB888_CH, &nonai_2d_vf_info, 1000);
        if (ret) {
            printf("kd_mpi_nonai_2d_get_frame failed. %d\n", ret);
            kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_1, &dumm_vf_info);
            continue;
        }

        if(ret == K_SUCCESS && dump_cnt == 0)
        {
            k_u8 *pData;

            pData = (k_u8 *)kd_mpi_sys_mmap(nonai_2d_vf_info.v_frame.phys_addr[0], size);
            memcpy(dump_buf_rgb888, pData, size);
            kd_mpi_sys_munmap(pData, size);
            printf("dump 2D rgb888 output done\n");
        }

        kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_1, &dumm_vf_info);

        ret = kd_mpi_nonai_2d_release_frame(NONAI_2D_RGB888_CH, &nonai_2d_vf_info);
        if (ret) {
            printf("kd_mpi_nonai_2d_release_frame failed. %d\n", ret);
        }

        usleep(30000);
        dump_cnt++;
    }

    printf("%s>done\n", __func__);
    return arg;
}

static void *dump_thread_1(void *arg)
{
    k_s32 ret;
    k_video_frame_info dumm_vf_info;
    k_video_frame_info nonai_2d_vf_info;
    k_u32 dump_cnt=0;
    k_u32 size = ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 2;

    dump_buf_rgb565 = malloc(size);

    while (1)
    {
        if(!vicap_started)
        {
            usleep(30000);
            continue;
        }

        ret = kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_0, VICAP_DUMP_RGB, &dumm_vf_info, 1000);
        if (ret) {
            printf("sample_vicap, chn(0) dump frame failed.\n");
            continue;
        }

        ret = kd_mpi_nonai_2d_send_frame(NONAI_2D_RGB565_CH, &dumm_vf_info, 1000);
        if (ret) {
            printf("kd_mpi_nonai_2d_send_frame failed. %d\n", ret);
            kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &dumm_vf_info);
            continue;
        }

        ret = kd_mpi_nonai_2d_get_frame(NONAI_2D_RGB565_CH, &nonai_2d_vf_info, 1000);
        if (ret) {
            printf("kd_mpi_nonai_2d_get_frame failed. %d\n", ret);
            kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &dumm_vf_info);
            continue;
        }

        if(ret == K_SUCCESS && dump_cnt == 0)
        {
            k_u8 *pData;

            pData = (k_u8 *)kd_mpi_sys_mmap(nonai_2d_vf_info.v_frame.phys_addr[0], size);
            memcpy(dump_buf_rgb565, pData, size);
            kd_mpi_sys_munmap(pData, size);
            printf("dump 2D rgb565 output done\n");
        }

        kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &dumm_vf_info);

        ret = kd_mpi_nonai_2d_release_frame(NONAI_2D_RGB565_CH, &nonai_2d_vf_info);
        if (ret) {
            printf("kd_mpi_nonai_2d_release_frame failed. %d\n", ret);
        }

        usleep(30000);
        dump_cnt++;
    }

    printf("%s>done\n", __func__);
    return arg;
}

static void sample_bind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn venc_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = 0;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 0;


    ret = kd_mpi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_bind(&nonai_2d_mpp_chn, &venc_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    k_mpp_chn vo_mpp_chn;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_bind(&nonai_2d_mpp_chn, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    return;
}

static void sample_unbind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn venc_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = 0;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 0;

    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_unbind(&nonai_2d_mpp_chn, &venc_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    k_mpp_chn vo_mpp_chn;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_unbind(&nonai_2d_mpp_chn, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);
    return;
}


k_s32 sample_exit()
{
    int ch = 0;
    int ret = 0;

    sample_vicap_stop();

    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);

    kd_mpi_nonai_2d_stop_chn(NONAI_2D_RGB888_CH);
    kd_mpi_nonai_2d_destroy_chn(NONAI_2D_RGB888_CH);
    kd_mpi_nonai_2d_stop_chn(NONAI_2D_BIND_CH);
    kd_mpi_nonai_2d_destroy_chn(NONAI_2D_BIND_CH);
    kd_mpi_nonai_2d_stop_chn(NONAI_2D_RGB565_CH);
    kd_mpi_nonai_2d_destroy_chn(NONAI_2D_RGB565_CH);

    kd_mpi_venc_detach_2d(ch);
    kd_mpi_venc_stop_chn(ch);
    kd_mpi_venc_destroy_chn(ch);

    sample_unbind(ch);

    pthread_cancel(g_nonai_2d_conf.output_tid);
    pthread_join(g_nonai_2d_conf.output_tid, NULL);

    pthread_cancel(g_nonai_2d_conf.dump_tid);
    pthread_join(g_nonai_2d_conf.dump_tid, NULL);

    pthread_cancel(g_nonai_2d_conf.dump_tid_1);
    pthread_join(g_nonai_2d_conf.dump_tid_1, NULL);

    ret = kd_mpi_nonai_2d_close();
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_close_fd();
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_vb_release_block(g_nonai_2d_conf.osd_blk_handle);
    CHECK_RET(ret, __func__, __LINE__);

    sample_vb_exit();

    if (g_nonai_2d_conf.output_file)
        fclose(g_nonai_2d_conf.output_file);

    return K_SUCCESS;
}

static k_s32 sample_venc_osd_h265()
{
    int ch = 0;
    k_u32 bitrate   = 4000;   //kbps
    int width       = g_nonai_2d_conf.width;
    int height      = g_nonai_2d_conf.height;
    k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
    k_payload_type type     = K_PT_H265;
    k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
    int ret = 0;
    k_venc_chn_attr attr;
    k_venc_2d_osd_attr venc_2d_osd_attr;
    k_venc_2d_border_attr venc_2d_border_attr;
    int index;

    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    attr.venc_attr.stream_buf_size = VICAP_ALIGN_UP(g_nonai_2d_conf.width*g_nonai_2d_conf.height/2, 0x1000);
    attr.venc_attr.stream_buf_cnt = VENC_BUF_CNT;

    attr.rc_attr.rc_mode = rc_mode;
    attr.rc_attr.cbr.src_frame_rate = 30;
    attr.rc_attr.cbr.dst_frame_rate = 30;
    attr.rc_attr.cbr.bit_rate = bitrate;

    attr.venc_attr.type = type;
    attr.venc_attr.profile = profile;
    printf("payload type is H265\n");

    ret = kd_mpi_venc_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);

    kd_mpi_venc_attach_2d(ch);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_set_2d_mode(ch, K_VENC_2D_CALC_MODE_OSD_BORDER);
    CHECK_RET(ret, __func__, __LINE__);

    prepare_osd();

    index = 0;
    venc_2d_osd_attr.width  = 40;
    venc_2d_osd_attr.height = 40;
    venc_2d_osd_attr.startx = 4;
    venc_2d_osd_attr.starty = 4;
    venc_2d_osd_attr.phys_addr[0] = g_nonai_2d_conf.osd_phys_addr;
    venc_2d_osd_attr.phys_addr[1] = 0;
    venc_2d_osd_attr.phys_addr[2] = 0;
    venc_2d_osd_attr.bg_alpha = 200;
    venc_2d_osd_attr.osd_alpha = 200;
    venc_2d_osd_attr.video_alpha = 200;
    venc_2d_osd_attr.add_order = K_VENC_2D_ADD_ORDER_VIDEO_OSD;
    venc_2d_osd_attr.bg_color = (200 << 16) | (128 << 8) | (128 << 0);
    venc_2d_osd_attr.fmt = K_VENC_2D_OSD_FMT_ARGB8888;
    ret = kd_mpi_venc_set_2d_osd_param(ch, index, &venc_2d_osd_attr);
    CHECK_RET(ret, __func__, __LINE__);

    venc_2d_border_attr.width = 40;
    venc_2d_border_attr.height = 40;
    venc_2d_border_attr.line_width = 2;
    venc_2d_border_attr.color = (100 << 16) | (200 << 8) | (70 << 0);
    venc_2d_border_attr.startx = 50;
    venc_2d_border_attr.starty = 4;
    ret = kd_mpi_venc_set_2d_border_param(ch, index, &venc_2d_border_attr);
    CHECK_RET(ret, __func__, __LINE__);

    pthread_create(&g_nonai_2d_conf.output_tid, NULL, output_thread, NULL);

    return K_SUCCESS;
}

static void sample_nonai_2d()
{
    k_s32 ret;
    k_nonai_2d_chn_attr attr_2d;

    attr_2d.mode = K_NONAI_2D_CALC_MODE_CSC;
    attr_2d.dst_fmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    ret = kd_mpi_nonai_2d_create_chn(NONAI_2D_BIND_CH, &attr_2d);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_nonai_2d_start_chn(NONAI_2D_BIND_CH);
    CHECK_RET(ret, __func__, __LINE__);

    attr_2d.mode = K_NONAI_2D_CALC_MODE_CSC;
    attr_2d.dst_fmt = PIXEL_FORMAT_RGB_888_PLANAR;
    ret = kd_mpi_nonai_2d_create_chn(NONAI_2D_RGB888_CH, &attr_2d);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_nonai_2d_start_chn(NONAI_2D_RGB888_CH);
    CHECK_RET(ret, __func__, __LINE__);

    attr_2d.mode = K_NONAI_2D_CALC_MODE_CSC;
    attr_2d.dst_fmt = PIXEL_FORMAT_RGB_565;
    ret = kd_mpi_nonai_2d_create_chn(NONAI_2D_RGB565_CH, &attr_2d);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_nonai_2d_start_chn(NONAI_2D_RGB565_CH);
    CHECK_RET(ret, __func__, __LINE__);

    pthread_create(&g_nonai_2d_conf.dump_tid, NULL, dump_thread, NULL);
    pthread_create(&g_nonai_2d_conf.dump_tid_1, NULL, dump_thread_1, NULL);
}

static void sample_csc_usage()
{
    printf("Usage : -sensor [sensor_index] -o [filename] -vo [display_type]\n");
    printf("\n");
    printf("sensor_index:\n");
    printf("\t  see vicap doc\n");
    return;
}

int main(int argc, char *argv[])
{
    FILE *output_file=NULL;
    k_u32 size=0;

    memset(&g_nonai_2d_conf, 0, sizeof(nonai_2d_conf_t));
    g_nonai_2d_conf.width = 1280;
    g_nonai_2d_conf.height = 720;
    g_nonai_2d_conf.sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    g_nonai_2d_conf.vo = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0)
        {
            sample_csc_usage();
            return K_SUCCESS;
        }
        else if (strcmp(argv[i], "-sensor") == 0)
        {
            g_nonai_2d_conf.sensor_type = (k_vicap_sensor_type)atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            strcpy(g_nonai_2d_conf.out_filename, argv[i + 1]);
            if ((g_nonai_2d_conf.output_file = fopen(g_nonai_2d_conf.out_filename, "wb")) == NULL)
            {
                printf("Cannot open output file\n");
            }
            printf("out_filename: %s\n", g_nonai_2d_conf.out_filename);
        }
        else if(strcmp(argv[i], "-vo") == 0)
        {
            g_nonai_2d_conf.vo = atoi(argv[i+1]);
        }
    }

    printf("-----------csc sample test---------------------\n");

    sample_vb_init();
    sample_nonai_2d();
    sample_venc_osd_h265();
    sample_vo_init(g_nonai_2d_conf.vo);
    sample_vicap_config();
    sample_bind();
    sample_vicap_start();

    printf("press 'q' to exit application!!\n");

    while (getchar() != 'q')
    {
        usleep(50000);
    }
    sample_exit();

    if(dump_buf_rgb888)
    {
        if(g_nonai_2d_conf.output_file)
        {
            size = ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3;
            output_file = fopen("/sharefs/out_2d_rgb888.rgb", "wb");
            fwrite(dump_buf_rgb888, 1, size, output_file);
            fclose(output_file);
            printf("write 2D rgb888 output done\n");
        }
        free(dump_buf_rgb888);
    }

    if(dump_buf_rgb565)
    {
        if(g_nonai_2d_conf.output_file)
        {
            size = ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 2;
            output_file = fopen("/sharefs/out_2d_rgb565.rgb", "wb");
            fwrite(dump_buf_rgb565, 1, size, output_file);
            fclose(output_file);
            printf("write 2D rgb565 output done\n");
        }
        free(dump_buf_rgb565);
    }

    printf("sample csc done!\n");

    return 0;
}

