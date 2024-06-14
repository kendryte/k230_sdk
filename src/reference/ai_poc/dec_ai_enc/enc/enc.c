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
#include "mpi_venc_api.h"
#include "mpi_sys_api.h"
#include "k_venc_comm.h"
#include "mpi_vvi_api.h"
#include "k_autoconf_comm.h"

#define VENC_MAX_IN_FRAMES   30
#define ENABLE_VENC_DEBUG    1

//#define ENABLE_VDSS 1
#ifdef ENABLE_VDSS
    #include "k_vdss_comm.h"
    #include "mpi_vdss_api.h"
#else
    #include "mpi_vicap_api.h"
#endif

#ifdef ENABLE_VENC_DEBUG
    #define venc_debug  printf
#else
    #define venc_debug(ARGS...)
#endif

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1088
#define STREAM_BUF_SIZE ((MAX_WIDTH*MAX_HEIGHT/2 + 0xfff) & ~0xfff)
#define FRAME_BUF_SIZE ((MAX_WIDTH*MAX_HEIGHT*2 + 0xfff) & ~0xfff)
#define OSD_MAX_WIDTH 100
#define OSD_MAX_HEIGHT 100
#define OSD_BUF_SIZE OSD_MAX_WIDTH*OSD_MAX_HEIGHT*4
#define INPUT_BUF_CNT   6
#define OUTPUT_BUF_CNT  15
#define OSD_BUF_CNT     20

extern const unsigned int osd_data;
extern const int osd_data_size;

typedef enum
{
    VENC_SAMPLE_STATUS_IDLE = 0,
    VENC_SAMPLE_STATUS_INIT,
    VENC_SAMPLE_STATUS_START,
    VENC_SAMPLE_STATUS_BINDED,
    VENC_SAMPLE_STATUS_UNBINDED,
    VENC_SAMPLE_STATUE_RUNING,
    VENC_SAMPLE_STATUS_STOPED,
    VENC_SAMPLE_STATUS_BUTT
} VENC_SAMPLE_STATUS;

typedef struct
{
    k_u32 osd_width;
    k_u32 osd_height;
    k_u32 osd_phys_addr[VENC_MAX_IN_FRAMES][3];
    void *osd_virt_addr[VENC_MAX_IN_FRAMES][3];
    k_u32 osd_startx;
    k_u32 osd_starty;
    k_venc_2d_src_dst_fmt video_fmt;
    k_venc_2d_osd_fmt osd_fmt;
    k_u16 bg_alpha;
    k_u16 osd_alpha;
    k_u16 video_alpha;
    k_venc_2d_add_order add_order;
    k_u32 bg_color;
    k_u16 osd_coef[K_VENC_2D_COEF_NUM];
    k_u8 osd_region_num;
    k_bool osd_matrix_en;
} osd_conf_t;

typedef struct
{
    k_u16 width;
    k_u16 height;
    k_u16 line_width;
    k_u16 startx;
    k_u16 starty;
} border_conf_t;

typedef struct
{
    k_u32 ch_id;
    k_u32 output_frames;
} output_info;

typedef struct
{
    k_u32 chnum;
    pthread_t output_tid;
    k_bool osd_enable;
    osd_conf_t *osd_conf;
    k_vb_blk_handle osd_blk_handle;
    k_bool ch_done;
} venc_conf_t;

char out_filename[50] = {"\0"};
FILE *output_file = NULL;
VENC_SAMPLE_STATUS g_venc_sample_status = VENC_SAMPLE_STATUS_IDLE;
venc_conf_t g_venc_conf;

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

static void sample_vicap_config(k_u32 ch, k_u32 width, k_u32 height, k_vicap_sensor_type sensor_type)
{
#ifdef ENABLE_VDSS
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;

    mpi_vdss_rst_all(2);

    memset(&dev_attr, 0, sizeof(dev_attr));
    dev_attr.dev_num = ch;
    dev_attr.height = height;
    dev_attr.width = width;
    dev_attr.sensor_type = 1;

    dev_attr.artr.csi = CSI0;
    dev_attr.artr.type = CLOSE_3D_MODE;
    dev_attr.artr.mode = LINERA_MODE;
    dev_attr.artr.dev_format[0] = RAW10;
    dev_attr.artr.phy_attr.lan_num = MIPI_1LAN;
    dev_attr.artr.phy_attr.freq = MIPI_800M;
    dev_attr.artr.bind_dvp = DVP_CSI1_FLASE_TRIGGER0;

    kd_mpi_vdss_set_dev_attr(&dev_attr);

    memset(&chn_attr, 0, sizeof(chn_attr));
    chn_attr.format = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    chn_attr.height = height;
    chn_attr.width = width;
    chn_attr.enable = 1;

    kd_mpi_vdss_set_chn_attr(ch, ch, &chn_attr);
#else
    k_s32 ret;
    k_vicap_dev vicap_dev = VICAP_DEV_ID_0;
    k_vicap_chn vicap_chn = ch;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;


    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));

    sensor_info.sensor_type = sensor_type;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info.sensor_type, &sensor_info);
    CHECK_RET(ret, __func__, __LINE__);

    dev_attr.acq_win.width = sensor_info.width;
    dev_attr.acq_win.height = sensor_info.height;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(vicap_dev, dev_attr);
    CHECK_RET(ret, __func__, __LINE__);

    chn_attr.out_win.width = width;
    chn_attr.out_win.height = height;

    chn_attr.crop_win = chn_attr.out_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.alignment = 12;

    //chn_attr.bit_width = ISP_PIXEL_YUV_8_BIT;
    chn_attr.pix_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    chn_attr.buffer_num = INPUT_BUF_CNT;
    chn_attr.buffer_size = (width * height * 3 / 2 + 0xfff) & ~ 0xfff;
    //chn_attr.block_type = ISP_BUFQUE_TIMEOUT_TYPE;
    //chn_attr.wait_time = 500;

    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, vicap_chn, chn_attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_vicap_init(vicap_dev);
    CHECK_RET(ret, __func__, __LINE__);
#endif
}

void sample_vicap_start(k_u32 ch)
{
#ifdef ENABLE_VDSS
    k_s32 ret;
    ret = kd_mpi_vdss_start_pipe(0, ch);
    CHECK_RET(ret, __func__, __LINE__);
#else
    k_s32 ret;

    ret = kd_mpi_vicap_start_stream(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
#endif
}

void sample_vicap_stop(k_u32 ch)
{
#ifdef ENABLE_VDSS
    k_s32 ret;
    ret = kd_mpi_vdss_stop_pipe(0, ch);
    CHECK_RET(ret, __func__, __LINE__);
#else
    k_s32 ret;

    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
#endif
}

static k_s32 sample_vb_init(k_u32 ch_cnt, k_bool osd_enable)
{
    k_s32 ret;
    k_vb_config config;

    memset(&config, 0, sizeof(config));
    if (osd_enable)
    {
        config.max_pool_cnt = 3;
        config.comm_pool[2].blk_cnt = OSD_BUF_CNT * ch_cnt;
        config.comm_pool[2].blk_size = OSD_BUF_SIZE;
        config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
    }
    else
    {
        config.max_pool_cnt = 2;
    }
    config.comm_pool[0].blk_cnt = INPUT_BUF_CNT * ch_cnt;
    config.comm_pool[0].blk_size = FRAME_BUF_SIZE;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_cnt = OUTPUT_BUF_CNT * ch_cnt;
    config.comm_pool[1].blk_size = STREAM_BUF_SIZE;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    ret = kd_mpi_vb_set_config(&config);

    venc_debug("-----------venc sample test------------------------\n");

    if (ret)
        venc_debug("vb_set_config failed ret:%d\n", ret);

    ret = kd_mpi_vb_init();
    if (ret)
        venc_debug("vb_init failed ret:%d\n", ret);

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

static void *output_thread(void *arg)
{
    k_venc_stream output;
    int out_cnt, out_frames;
    k_s32 ret;
    int i;
    k_u32 total_len = 0;
    output_info *info = (output_info *)arg;

    out_cnt = 0;
    out_frames = 0;

    while (1)
    {
        k_venc_chn_status status;

        ret = kd_mpi_venc_query_status(info->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;

        output.pack = malloc(sizeof(k_venc_pack) * output.pack_cnt);

        ret = kd_mpi_venc_get_stream(info->ch_id, &output, -1);
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
            if (output_file)
                fwrite(pData, 1, output.pack[i].len, output_file);
            kd_mpi_sys_munmap(pData, output.pack[i].len);

            total_len += output.pack[i].len;
        }

        ret = kd_mpi_venc_release_stream(info->ch_id, &output);
        CHECK_RET(ret, __func__, __LINE__);

        free(output.pack);

    }

    if (output_file)
        fclose(output_file);
    venc_debug("%s>done, ch %d: out_frames %d, size %d bits\n", __func__, info->ch_id, out_frames, total_len * 8);
    return arg;
}

static void sample_vi_bind_venc(k_u32 chn_id)
{
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vi_mpp_chn;
    k_s32 ret;

#ifdef ENABLE_VDSS
    vi_mpp_chn.mod_id = K_ID_VICAP;
#else
    vi_mpp_chn.mod_id = K_ID_VI;
#endif

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_id;

    vi_mpp_chn.dev_id = chn_id;
    vi_mpp_chn.chn_id = chn_id;
    ret = kd_mpi_sys_bind(&vi_mpp_chn, &venc_mpp_chn);
    if (ret)
    {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }

    return;
}

static void sample_vi_unbind_venc(k_u32 chn_id)
{
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vi_mpp_chn;

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_id;

#ifdef ENABLE_VDSS
    vi_mpp_chn.mod_id = K_ID_VICAP;
#else
    vi_mpp_chn.mod_id = K_ID_VI;
#endif
    vi_mpp_chn.dev_id = chn_id;
    vi_mpp_chn.chn_id = chn_id;
    kd_mpi_sys_unbind(&vi_mpp_chn, &venc_mpp_chn);

    return;
}

static k_s32 prepare_osd(osd_conf_t *osd_conf, k_vb_blk_handle *osd_blk_handle)
{
    int osd_byte;
    int i;
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u8 *virt_addr_osd;
    k_s32 osd_pool_id;

    switch (osd_conf->osd_fmt)
    {
    case K_VENC_2D_OSD_FMT_ARGB8888:
        osd_byte = 4;
        break;
    case K_VENC_2D_OSD_FMT_ARGB4444:
        osd_byte = 2;
        break;
    case K_VENC_2D_OSD_FMT_ARGB1555:
        osd_byte = 2;
        break;
    default:
        osd_byte = 4;
        break;
    }
    venc_debug("osd_byte = %d\n", osd_byte);

    for (i = 0; i < 1; i++)
    {
        handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, OSD_BUF_SIZE, NULL);

        if (handle == VB_INVALID_HANDLE)
        {
            printf("%s osd get vb block error\n", __func__);
            break;
        }
        *osd_blk_handle = handle;

        pool_id = kd_mpi_vb_handle_to_pool_id(handle);
        if (pool_id == VB_INVALID_POOLID)
        {
            printf("%s osd get pool id error\n", __func__);
            break;
        }
        osd_pool_id = pool_id;
        printf("%s>osd_pool_id = %d\n", __func__, osd_pool_id);

        phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
        if (phys_addr == 0)
        {
            printf("%s osd get phys addr error\n", __func__);
            break;
        }

        printf("%s>i %d, phys_addr 0x%lx, blk_size %d\n", __func__, i, phys_addr, OSD_BUF_SIZE);
        virt_addr_osd = (k_u8 *)kd_mpi_sys_mmap_cached(phys_addr, OSD_BUF_SIZE);

        if (virt_addr_osd == NULL)
        {
            printf("%s osd mmap error\n", __func__);
            break;
        }
        printf("%s>i %d, osd virt_addr_osd %p\n", __func__, i, virt_addr_osd);

        memcpy(virt_addr_osd, &osd_data, osd_data_size);
        osd_conf->osd_phys_addr[i][0] = phys_addr;
        osd_conf->osd_virt_addr[i][0] = virt_addr_osd;
        kd_mpi_sys_mmz_flush_cache(osd_conf->osd_phys_addr[i][0], osd_conf->osd_virt_addr[i][0], OSD_BUF_SIZE);
    }
    return K_SUCCESS;
}

k_s32 sample_exit(venc_conf_t *venc_conf)
{
    int ch = 0;
    int ret = 0;

    printf("%s>g_venc_sample_status = %d\n", __FUNCTION__, g_venc_sample_status);
    switch (g_venc_sample_status)
    {
    case VENC_SAMPLE_STATUE_RUNING:
    case VENC_SAMPLE_STATUS_BINDED:
        sample_vicap_stop(ch);
        sample_vi_unbind_venc(ch);
    case VENC_SAMPLE_STATUS_START:
        kd_mpi_venc_stop_chn(ch);
        if (venc_conf->osd_enable)
            kd_mpi_venc_detach_2d(ch);
    case VENC_SAMPLE_STATUS_INIT:
        kd_mpi_venc_destroy_chn(ch);
        if (venc_conf->osd_enable)
        {
            ret = kd_mpi_vb_release_block(venc_conf->osd_blk_handle);
            CHECK_RET(ret, __func__, __LINE__);

            for (int k = 0; k < venc_conf->osd_conf->osd_region_num; k++)
            {
                printf("osd_conf->osd_virt_addr[%d][0] 0x%p\n", k, venc_conf->osd_conf->osd_virt_addr[k][0]);
                ret = kd_mpi_sys_munmap(venc_conf->osd_conf->osd_virt_addr[k][0], OSD_BUF_SIZE);
                CHECK_RET(ret, __func__, __LINE__);
            }
        }
        break;
    default:
        break;
    }

    pthread_cancel(venc_conf->output_tid);
    pthread_join(venc_conf->output_tid, NULL);

    venc_debug("kill ch %d thread done! ch_done %d, chnum %d\n", ch, g_venc_conf.ch_done, g_venc_conf.chnum);

    ret = kd_mpi_venc_close_fd();
    CHECK_RET(ret, __func__, __LINE__);

    if (output_file)
        fclose(output_file);
    sample_vb_exit();

    g_venc_conf.ch_done = K_TRUE;

    return K_SUCCESS;
}

static void *exit_app(void *arg)
{
    venc_conf_t *venc_conf = (venc_conf_t *)arg;
    while (getchar() != 'q')
    {
        usleep(10000);
    }
    sample_exit(venc_conf);
    return K_SUCCESS;
}

k_s32 sample_venc_h265(k_vicap_sensor_type sensor_type)
{
    int chnum = 1;
    int ch = 0;
    k_u32 output_frames = 10;
    k_u32 bitrate   = 4000;   //kbps
    int width       = 1920;
    int height      = 1088;
    k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
    k_payload_type type     = K_PT_H265;
    k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
    int ret = 0;

    sample_vb_init(chnum, K_FALSE);

    k_venc_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    attr.venc_attr.stream_buf_size = STREAM_BUF_SIZE;
    attr.venc_attr.stream_buf_cnt = OUTPUT_BUF_CNT;

    attr.rc_attr.rc_mode = rc_mode;
    attr.rc_attr.cbr.src_frame_rate = 30;
    attr.rc_attr.cbr.dst_frame_rate = 30;
    attr.rc_attr.cbr.bit_rate = bitrate;

    attr.venc_attr.type = type;
    attr.venc_attr.profile = profile;
    venc_debug("payload type is H265\n");

    ret = kd_mpi_venc_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_INIT;

    ret = kd_mpi_venc_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_START;

    sample_vicap_config(ch, width, height, sensor_type);
    sample_vi_bind_venc(ch);
    sample_vicap_start(ch);
    g_venc_sample_status = VENC_SAMPLE_STATUS_BINDED;

    output_info info;
    memset(&info, 0, sizeof(info));
    info.ch_id = ch;
    info.output_frames = output_frames;

    pthread_create(&g_venc_conf.output_tid, NULL, output_thread, &info);
    g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

    printf("press 'q' to exit application!!\n");
    while (!g_venc_conf.ch_done)
    {
        usleep(10000);
    }
    return K_SUCCESS;
}

k_s32 sample_venc_jpeg(k_vicap_sensor_type sensor_type)
{
    int chnum = 1;
    int ch = 0;
    k_u32 output_frames = 10;
    int width       = 1280;
    int height      = 720;
    k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_MJPEG_FIXQP;
    k_payload_type type     = K_PT_JPEG;
    k_u32 q_factor          = 45;
    int ret = 0;

    sample_vb_init(chnum, K_FALSE);

    k_venc_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    attr.venc_attr.stream_buf_size = STREAM_BUF_SIZE;
    attr.venc_attr.stream_buf_cnt = OUTPUT_BUF_CNT;

    attr.venc_attr.type = type;
    attr.rc_attr.rc_mode = rc_mode;
    attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
    attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
    attr.rc_attr.mjpeg_fixqp.q_factor = q_factor;

    venc_debug("payload type is JPEG\n");

    ret = kd_mpi_venc_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_INIT;

    ret = kd_mpi_venc_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_START;

    sample_vicap_config(ch, width, height, sensor_type);
    sample_vi_bind_venc(ch);
    sample_vicap_start(ch);
    g_venc_sample_status = VENC_SAMPLE_STATUS_BINDED;

    output_info info;
    memset(&info, 0, sizeof(info));
    info.ch_id = ch;
    info.output_frames = output_frames;

    pthread_create(&g_venc_conf.output_tid, NULL, output_thread, &info);
    g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

    printf("press 'q' to exit application!!\n");
    while (!g_venc_conf.ch_done)
    {
        usleep(10000);
    }
    return K_SUCCESS;
}

k_s32 sample_venc_osd_h264(k_vicap_sensor_type sensor_type)
{
    int chnum = 1;
    int ch = 0;
    k_u32 output_frames = 10;
    k_u32 bitrate   = 4000;   //kbps
    int width       = 1280;
    int height      = 720;
    k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
    k_payload_type type     = K_PT_H264;
    k_venc_profile profile  = VENC_PROFILE_H264_HIGH;

    osd_conf_t osd_conf =
    {
        .osd_width  = 40,
        .osd_height = 40,
        .osd_startx = 4,
        .osd_starty = 4,
        .bg_alpha   = 200,
        .osd_alpha  = 200,
        .video_alpha = 200,
        .add_order  = K_VENC_2D_ADD_ORDER_VIDEO_OSD,
        .bg_color   = (200 << 16) | (128 << 8) | (128 << 0),
        .osd_fmt    = K_VENC_2D_OSD_FMT_ARGB8888,
        .osd_region_num = 1
    };
    int ret = 0;

    sample_vb_init(chnum, K_TRUE);
    g_venc_conf.osd_conf = &osd_conf;
    g_venc_conf.osd_enable = K_TRUE;
    prepare_osd(&osd_conf, &g_venc_conf.osd_blk_handle);

    k_venc_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    attr.venc_attr.stream_buf_size = STREAM_BUF_SIZE;
    attr.venc_attr.stream_buf_cnt = OUTPUT_BUF_CNT;

    attr.rc_attr.rc_mode = rc_mode;
    attr.rc_attr.cbr.src_frame_rate = 30;
    attr.rc_attr.cbr.dst_frame_rate = 30;
    attr.rc_attr.cbr.bit_rate = bitrate;

    attr.venc_attr.type = type;
    attr.venc_attr.profile = profile;
    venc_debug("payload type is H264\n");

    ret = kd_mpi_venc_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_INIT;

    ret = kd_mpi_venc_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_START;

    kd_mpi_venc_attach_2d(ch);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_set_2d_mode(ch, K_VENC_2D_CALC_MODE_OSD);
    CHECK_RET(ret, __func__, __LINE__);

    k_venc_2d_osd_attr venc_2d_osd_attr;
    venc_2d_osd_attr.width  = osd_conf.osd_width;
    venc_2d_osd_attr.height = osd_conf.osd_height;
    venc_2d_osd_attr.startx = osd_conf.osd_startx;
    venc_2d_osd_attr.starty = osd_conf.osd_starty;
    venc_2d_osd_attr.phys_addr[0] = osd_conf.osd_phys_addr[0][0];
    venc_2d_osd_attr.phys_addr[1] = osd_conf.osd_phys_addr[0][1];
    venc_2d_osd_attr.phys_addr[2] = osd_conf.osd_phys_addr[0][2];
    venc_2d_osd_attr.bg_alpha = osd_conf.bg_alpha;
    venc_2d_osd_attr.osd_alpha = osd_conf.osd_alpha;
    venc_2d_osd_attr.video_alpha = osd_conf.video_alpha;
    venc_2d_osd_attr.add_order = osd_conf.add_order;
    venc_2d_osd_attr.bg_color = osd_conf.bg_color;
    venc_2d_osd_attr.fmt = osd_conf.osd_fmt;
    printf("%s>osd phys addr 0x%08x 0x%08x 0x%08x\n", __FUNCTION__, venc_2d_osd_attr.phys_addr[0], venc_2d_osd_attr.phys_addr[1], venc_2d_osd_attr.phys_addr[2]);

    ret = kd_mpi_venc_set_2d_osd_param(ch, 0, &venc_2d_osd_attr);
    CHECK_RET(ret, __func__, __LINE__);

    sample_vicap_config(ch, width, height, sensor_type);
    sample_vi_bind_venc(ch);
    sample_vicap_start(ch);
    g_venc_sample_status = VENC_SAMPLE_STATUS_BINDED;

    output_info info;
    memset(&info, 0, sizeof(info));
    info.ch_id = ch;
    info.output_frames = output_frames;

    pthread_create(&g_venc_conf.output_tid, NULL, output_thread, &info);
    g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

    printf("press 'q' to exit application!!\n");
    while (!g_venc_conf.ch_done)
    {
        usleep(10000);
    }
    return K_SUCCESS;
}

k_s32 sample_venc_osd_border_h265(k_vicap_sensor_type sensor_type)
{
    int chnum = 1;
    int ch = 0;
    k_u32 output_frames = 10;
    k_u32 bitrate   = 4000;   //kbps
    int width       = 1280;
    int height      = 720;
    k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
    k_payload_type type     = K_PT_H265;
    k_venc_profile profile  = VENC_PROFILE_H265_MAIN;

    osd_conf_t osd_conf =
    {
        .osd_width  = 40,
        .osd_height = 40,
        .osd_startx = 4,
        .osd_starty = 4,
        .bg_alpha   = 200,
        .osd_alpha  = 200,
        .video_alpha = 200,
        .add_order  = K_VENC_2D_ADD_ORDER_VIDEO_OSD,
        .bg_color   = (200 << 16) | (128 << 8) | (128 << 0),
        .osd_fmt    = K_VENC_2D_OSD_FMT_ARGB8888,
        .osd_region_num = 1
    };
    border_conf_t border_conf =
    {
        .width = 40,
        .height = 40,
        .line_width = 2,
        .startx = 4,
        .starty = 4
    };
    int ret = 0;

    sample_vb_init(chnum, K_TRUE);
    g_venc_conf.osd_conf = &osd_conf;
    g_venc_conf.osd_enable = K_TRUE;
    prepare_osd(&osd_conf, &g_venc_conf.osd_blk_handle);

    k_venc_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    attr.venc_attr.stream_buf_size = STREAM_BUF_SIZE;
    attr.venc_attr.stream_buf_cnt = OUTPUT_BUF_CNT;

    attr.rc_attr.rc_mode = rc_mode;
    attr.rc_attr.cbr.src_frame_rate = 30;
    attr.rc_attr.cbr.dst_frame_rate = 30;
    attr.rc_attr.cbr.bit_rate = bitrate;

    attr.venc_attr.type = type;
    attr.venc_attr.profile = profile;
    venc_debug("payload type is H264\n");

    ret = kd_mpi_venc_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_INIT;

    ret = kd_mpi_venc_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_START;

    kd_mpi_venc_attach_2d(ch);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_set_2d_mode(ch, K_VENC_2D_CALC_MODE_OSD_BORDER);
    CHECK_RET(ret, __func__, __LINE__);

    k_venc_2d_osd_attr venc_2d_osd_attr;
    venc_2d_osd_attr.width  = osd_conf.osd_width;
    venc_2d_osd_attr.height = osd_conf.osd_height;
    venc_2d_osd_attr.startx = osd_conf.osd_startx;
    venc_2d_osd_attr.starty = osd_conf.osd_starty;
    venc_2d_osd_attr.phys_addr[0] = osd_conf.osd_phys_addr[0][0];
    venc_2d_osd_attr.phys_addr[1] = osd_conf.osd_phys_addr[0][1];
    venc_2d_osd_attr.phys_addr[2] = osd_conf.osd_phys_addr[0][2];
    venc_2d_osd_attr.bg_alpha = osd_conf.bg_alpha;
    venc_2d_osd_attr.osd_alpha = osd_conf.osd_alpha;
    venc_2d_osd_attr.video_alpha = osd_conf.video_alpha;
    venc_2d_osd_attr.add_order = osd_conf.add_order;
    venc_2d_osd_attr.bg_color = osd_conf.bg_color;
    venc_2d_osd_attr.fmt = osd_conf.osd_fmt;
    printf("%s>osd phys addr 0x%08x 0x%08x 0x%08x\n", __FUNCTION__, venc_2d_osd_attr.phys_addr[0], venc_2d_osd_attr.phys_addr[1], venc_2d_osd_attr.phys_addr[2]);

    ret = kd_mpi_venc_set_2d_osd_param(ch, 0, &venc_2d_osd_attr);
    CHECK_RET(ret, __func__, __LINE__);

    k_venc_2d_border_attr venc_2d_border_attr;
    venc_2d_border_attr.width = border_conf.width;
    venc_2d_border_attr.height = border_conf.height;
    venc_2d_border_attr.line_width = border_conf.line_width;
    venc_2d_border_attr.color = (100 << 16) | (200 << 8) | (70 << 0);
    venc_2d_border_attr.startx = border_conf.startx;
    venc_2d_border_attr.starty = border_conf.starty;
    ret = kd_mpi_venc_set_2d_border_param(ch, 0, &venc_2d_border_attr);
    CHECK_RET(ret, __func__, __LINE__);

    sample_vicap_config(ch, width, height, sensor_type);
    sample_vi_bind_venc(ch);
    sample_vicap_start(ch);
    g_venc_sample_status = VENC_SAMPLE_STATUS_BINDED;

    output_info info;
    memset(&info, 0, sizeof(info));
    info.ch_id = ch;
    info.output_frames = output_frames;

    pthread_create(&g_venc_conf.output_tid, NULL, output_thread, &info);
    g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

    printf("press 'q' to exit application!!\n");
    while (!g_venc_conf.ch_done)
    {
        usleep(10000);
    }
    return K_SUCCESS;
}

void sample_venc_usage(char *sPrgNm)
{
    printf("Usage : %s [index] -sensor [sensor_index] -o [filename]\n", sPrgNm);
    printf("index:\n");
    printf("\t  0) H.265e.\n");
    printf("\t  1) JPEG encode.\n");
    printf("\t  2) OSD + H.264e.\n");
    printf("\t  3) OSD + Border + H.265e.\n");
    printf("\n");
    printf("sensor_index:\n");
    printf("\t  see vicap doc\n");
    return;
}

int main(int argc, char *argv[])
{
    k_vicap_sensor_type sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    pthread_t exit_thread_handle;
    int case_index = 0;
    int ret = 0;

    printf("argc = %d\n", argc);

    if ((argc == 1) || (!strncmp(argv[1], "-h", 2)))
    {
        sample_venc_usage(argv[0]);
        return K_SUCCESS;
    }

    if ((atoi(argv[1]) < 0) || (atoi(argv[1]) > 3))
    {
        printf("index error\n");
        sample_venc_usage(argv[0]);
        return K_FAILED;
    }

    case_index = atoi(argv[1]);
    #if defined(CONFIG_BOARD_K230_CANMV)
        sensor_type = OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR;
    #elif defined(CONFIG_BOARD_K230_CANMV_V2)
        sensor_type = OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR_V2;
    #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
        sensor_type = OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR_V2;
    #elif defined(CONFIG_BOARD_K230D_CANMV)
        sensor_type = OV_OV5647_MIPI_1920X1080_30FPS_10BIT_LINEAR;
    #else
        sensor_type = IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    #endif
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            strcpy(out_filename, argv[i + 1]);
            if ((output_file = fopen(out_filename, "wb")) == NULL)
            {
                venc_debug("Cannot open output file\n");
            }
            printf("out_filename: %s\n", out_filename);
        }
    }

    memset(&g_venc_conf, 0, sizeof(venc_conf_t));
    pthread_create(&exit_thread_handle, NULL, exit_app, &g_venc_conf);

    switch (case_index)
    {
    case 0:
        ret = sample_venc_h265(sensor_type);
        break;
    case 1:
        ret = sample_venc_jpeg(sensor_type);
        break;
    case 2:
        ret = sample_venc_osd_h264(sensor_type);
        break;
    case 3:
        ret = sample_venc_osd_border_h265(sensor_type);
        break;

    default:
        printf("the index is invaild!\n");
        sample_venc_usage(argv[0]);
        return K_FAILED;
    }

    if (K_SUCCESS == ret)
    {
        printf("sample_venc pass.\n");
    }
    else
    {
        printf("sample_venc fail.\n");
    }

    printf("sample encode done!\n");

    return 0;
}

