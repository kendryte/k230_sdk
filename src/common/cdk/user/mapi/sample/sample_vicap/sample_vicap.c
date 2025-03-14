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
#include <getopt.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "mapi_vo_api.h"
#include "k_vicap_comm.h"
#include "k_video_comm.h"
#include "k_vo_comm.h"
#include "k_connector_comm.h"
#include "vicap_vo_cfg.h"

// argparse
struct option long_options[] = {
    {"dev",    required_argument, NULL, 'd'},
    {"sensor", required_argument, NULL, 's'},
    {"chn",    required_argument, NULL, 'c'},
    {"ofmt",    required_argument, NULL, 'f'},
    {"width",     required_argument, NULL, 'W'},
    {"height",     required_argument, NULL, 'H'},
    {"fps",     required_argument, NULL, 'P'},
    {"work_mode",     required_argument, NULL, 'm'},
    {"dewarp", no_argument, NULL, 'e'},
    {"help",   no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

static kd_ts_sys_paga_vi_cap sys_page = {0};
static k_video_frame_info dump_pic_info = {0};
static k_s32 mmap_fd = -1;

static k_s32 sample_vicap_vo_layer_init(k_vicap_vo_layer_conf *layer_conf, k_vicap_dev arg_dev)
{
    k_s32 ret = 0;
    layer_info info[MAX_VO_LAYER_NUM];
    k_u16 margin = 0;
    k_u16 rotation = 0;
    k_u16 relative_height = 0;
    k_u16 total_height = 0;

    memset(&info, 0, sizeof(info));

    for (int i = 0; i <= arg_dev; i++) {
        rotation = layer_conf->rotation[i];
        switch (rotation) {
        case 0:
            info[i].act_size.width = layer_conf->width[i];
            info[i].act_size.height = layer_conf->height[i];
            info[i].func = K_ROTATION_0;
            break;
        case 1:
            info[i].act_size.width = layer_conf->height[i];
            info[i].act_size.height = layer_conf->width[i];
            info[i].func = K_ROTATION_90;
            break;
        case 2:
            info[i].act_size.width = layer_conf->width[i];
            info[i].act_size.height = layer_conf->height[i];
            info[i].func = K_ROTATION_180;
            break;
        case 3:
            info[i].act_size.width = layer_conf->height[i];
            info[i].act_size.height = layer_conf->width[i];
            info[i].func = K_ROTATION_270;
            break;
        case 4:
            info[i].act_size.width = layer_conf->width[i];
            info[i].act_size.height = layer_conf->height[i];
            info[i].func = 0;
            break;
        default:
            printf("invalid roation paramters.\n");
            return -1;
        }
        total_height += info[i].act_size.height;
        margin = ((DISPLAY_HEIGHT - total_height) / (i+2));
        if ((total_height > DISPLAY_HEIGHT) || (info[i].act_size.width > DISPLAY_WITDH)) {
            printf("%s, the preview window size[%dx%d] exceeds the display window size[%dx%d].\n", \
                __func__, info[i].act_size.width, total_height, DISPLAY_WITDH, DISPLAY_HEIGHT);
            return -1;
        }
        printf("%s, width(%d), height(%d), margin(%d), total_height(%d)\n", \
            __func__, info[i].act_size.width, info[i].act_size.height, margin, total_height);
    }

    for (int i = 0; i <= arg_dev; i++) {
        info[i].offset.x = (DISPLAY_WITDH - info[i].act_size.width) / 2;
        info[i].offset.y = margin + relative_height;
        printf("%s, layer(%d), offset.x(%d), offset.y(%d), relative_height(%d)\n", __func__, layer_conf->layer[i], info[i].offset.x, info[i].offset.y, relative_height);
        relative_height += info[i].act_size.height + margin;

        info[i].format = PIXEL_FORMAT_YVU_PLANAR_420;
        info[i].global_alptha = 0xff;

        sample_vo_creat_layer(layer_conf->layer[i], &info[i]);
    }

    return ret;
}

void kd_ts_mmap(k_u64 phys_addr, k_u32 size, k_u32 **v_addr)
{
    sys_page.page_size = sysconf(_SC_PAGESIZE);
    sys_page.page_mask = sys_page.page_size - 1;
#ifdef USE_RT_SMART
    *v_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
#else
    void *mmap_addr = NULL;
    k_u32 mmap_size = (((size) + (sys_page.page_size) - 1) & ~((sys_page.page_size) - 1));

    if(mmap_fd == -1)
    {
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, phys_addr & ~sys_page.page_mask);

    if(mmap_addr)
    {
        *v_addr = mmap_addr + (phys_addr & sys_page.page_mask);
    }
#endif
    if(*v_addr == NULL)
    {
        printf("mmap error\n");
        return;
    }
    printf("mmap success\n");
    return;
}

static k_vicap_sensor_type get_sensor_type(k_u16 sensor)
{
    switch (sensor)
    {
    case 0:
        return OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
    case 1:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR;
    case 2:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;
    case 3:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    case 4:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 5:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 6:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR;
    case 7:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    case 8:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    case 9:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE_V2;
    default:
        printf("unsupport sensor type %d, use default IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR\n", sensor);
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    }
    return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
}

static k_pixel_format get_pixel_format(k_u32 pix_fmt)
{
    switch (pix_fmt)
    {
    case 0:
        return PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    case 1:
        return PIXEL_FORMAT_RGB_888;
    case 2:
        return PIXEL_FORMAT_RGB_888_PLANAR;
    case 3:
        return PIXEL_FORMAT_RGB_BAYER_10BPP;
    default:
        printf("unsupported pixel format %d, use dafault YUV_SEMIPLANAR_420\n", pix_fmt);
        break;
    }
}

static k_s32 calc_buffer_size(k_pixel_format pix_fmt, k_u32 width, k_u32 height, buf_size_calc * size_calc)
{
    switch (pix_fmt)
    {
    case PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3 / 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3 / 2;
        size_calc->suffix = "yuv420sp";
        break;
    case PIXEL_FORMAT_RGB_888:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3;
        size_calc->suffix = "rgb888p";
        break;
    case PIXEL_FORMAT_RGB_888_PLANAR:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3;
        size_calc->suffix = "rgb888";
        break;
    case PIXEL_FORMAT_RGB_BAYER_10BPP:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 2;
        size_calc->suffix = "raw10";
        break;
    default:
        printf("unsupported pixel format %d, use dafault YUV_SEMIPLANAR_420 size\n", pix_fmt);
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3 / 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3 / 2;
        size_calc->suffix = "unkown";
    }
    return K_SUCCESS;
}

static k_s32 sample_vicap_vb_init(k_bool dw_en, k_u32 dev_num, k_u32 chn_num, buf_size_calc in_size, buf_size_calc out_size, buf_size_calc dw_size, k_vicap_work_mode work_mode)
{
    printf("sample_vicap_vb_init start\n");
    k_s32 ret = 0;
    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));

    media_attr.media_config.vb_config.max_pool_cnt = 64;
    int k = 0;
    for(int i = 0; i <= dev_num; i++) {

        if(work_mode == VICAP_WORK_OFFLINE_MODE)
        {
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = VICAP_INPUT_BUF_NUM;
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            printf("vb size : %d\n", in_size.buffsize_with_align);
            media_attr.media_config.vb_config.comm_pool[k].blk_size = in_size.buffsize_with_align;
            k++;
        }
        for(int j = 0; j <= chn_num; j++) {
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[k].blk_size = out_size.buffsize_with_align;
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            k++;
        }
        if(dw_en)
        {
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[k].blk_size = dw_size.buffsize_with_align;
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            k++;
        }
    }

    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }
    printf("sample_vicap_vb_init end\n");
    return 0;
}

static void sample_vicap_bind_vo(k_s32 vicap_dev, k_s32 vicap_chn, k_s32 vo_chn)
{
    k_s32 ret;

    k_mpp_chn vicap_mpp_chn, vo_mpp_chn;

    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = vicap_chn;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = vo_chn;

    ret = kd_mapi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mapi_sys_bind failed:0x%x\n", ret);
    }

    return;
}

static void sample_vicap_unbind_vo(k_s32 vicap_dev, k_s32 vicap_chn, k_s32 vo_chn)
{
    k_s32 ret;

    k_mpp_chn vicap_mpp_chn, vo_mpp_chn;

    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = vicap_chn;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = vo_chn;

    ret = kd_mapi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }

    return;
}

static void usage(char * const argv[])
{
    printf("[sample_vicap]#\n");
    printf("Usage: %s -m 0 -d 0 -s 0 -c 0 -f 0 -W 1088 -H 720 -P 30\n", argv[0]);
    printf("          -m or --mode work mode, 0: online, 1: offline, multiple sensor will use offline mode\n");
    printf("          -d or --dev device num, 0, 1, 2, set 1 will enter multiple sensor test\n");
    printf("          -s or --sensor sensor num, if device num is 1, will fix set ov9732 & ov9286\n");
    printf("               0: ov9732\n");
    printf("               1: ov9286 ir\n");
    printf("               2: ov9286 speckle\n");
    printf("               3: imx335 2LANE 1920Wx1080H\n");
    printf("               4: imx335 2LANE 2592Wx1944H\n");
    printf("               5: imx335 4LANE 2592Wx1944H\n");
    printf("               6: imx335 2LANE MCLK 7425 1920Wx1080H\n");
    printf("               7: imx335 2LANE MCLK 7425 2592Wx1944H\n");
    printf("               8: imx335 4LANE MCLK 7425 2592Wx1944H\n");
    printf("          -c or --chn channel num, 0, 1, 2\n");
    printf("          -f or --ofmt out pixel format, 0: yuv420sp, 1: rgb888, 2: rgb888p, 3: raw10\n");
    printf("          -W or --width output width\n");
    printf("          -H or --height output height\n");
    printf("          -P or --fps device bind output fps\n");
    printf("          -e or --dewarp dewarp mode, action store_true, default disable, will enable when use -e or --dewarp, \n");
    printf("               will force disable with raw10 pixel format.\n");
    printf("          -h or --help will print this usage\n");
    return;
}

int main(int argc, char * const argv[])
{

    // argparse param init
    k_u32 out_width = 1088;
    k_u32 out_height = 1920;
    k_u8 fps = 30;
    k_vicap_dev arg_dev = VICAP_DEV_ID_0; // 0
    k_vicap_chn arg_chn = VICAP_CHN_ID_0; // 0
    k_pixel_format arg_pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420; // 30
    k_vicap_sensor_type arg_sensor = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR; // 0
    k_bool dw_en = 0; // 0
    k_vicap_work_mode work_mode = VICAP_WORK_ONLINE_MODE;



    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "d:s:c:f:W:H:P:m:eh", long_options, &option_index)) != -1)
    {
        switch(c)
        {
        case 'd':
            arg_dev = (k_vicap_dev)atoi(optarg);
            printf("[sample_vicap]# dev: %d\n", arg_dev);
            break;
        case 's':
            arg_sensor = get_sensor_type(atoi(optarg));
            printf("[sample_vicap]# sensor type: %d\n", arg_sensor);
            break;
        case 'c':
            arg_chn = (k_vicap_chn)atoi(optarg);
            printf("[sample_vicap]# chn: %d\n", arg_chn);
            break;
        case 'f':
            arg_pixel_format = get_pixel_format(atoi(optarg));
            printf("[sample_vicap]# pixel format: %d\n", arg_pixel_format);
            break;
        case 'W':
            out_width = (k_u32)atoi(optarg);
            printf("[sample_vicap]# ori out width: %d, ", out_width);
            out_width = VICAP_ALIGN_UP(out_width, VICAP_ALIGN_2_BYTE);
            printf("align out width: %d\n", out_width);
            break;
        case 'H':
            out_height = (k_u32)atoi(optarg);
            printf("[sample_vicap]# out_height: %d\n", out_height);
            break;
        case 'P':
            fps = (k_u8)atoi(optarg);
            printf("[sample_vicap]# fps: %d\n", fps);
            break;
        case 'm':
            work_mode = (k_vicap_work_mode)atoi(optarg);
            printf("[sample_vicap]# work_mode: %d\n", work_mode);
            break;
        case 'e':
            printf("[sample_vicap]# dewarp enable\n");
            dw_en = 1;
            break;
        case 'h':
            usage(argv);
            return 0;
        default:
            fprintf(stderr, "Invalid option\n");
            break;
        }
    }

    // check arg
    if(arg_dev > 0 && work_mode == 0)
    {
        printf("[sample_vicap]# if -d set %d, -m need set 1\n", arg_dev, work_mode);
        usage(argv);
        return 0;
    }

    k_s32 ret = 0;

    // ipcmsg service init
    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS)
    {
        printf("kd_mapi_sys_init error: %x\n", ret);
        goto exit;
    }

    // phy2vir
    if(mmap_fd == -1)
    {
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    }
    printf("opne mem success \n");

    if(arg_dev == 1)
    {
        arg_sensor = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
    }

    // sensor info
    k_vicap_sensor_info sensor_info;
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    sensor_info.sensor_type = arg_sensor;
    ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
    if(!ret)
    {
        printf("kd_mapi_vicap_get_sensor_info ok\n");
    }

    // get resolution
    k_u32 in_width = sensor_info.width;
    k_u32 in_height = sensor_info.height;

    // calc buffer size
    buf_size_calc size_calc_in;
    buf_size_calc size_calc_out;
    buf_size_calc size_calc_dw;
    calc_buffer_size(PIXEL_FORMAT_RGB_BAYER_10BPP, in_width, in_height, &size_calc_in);
    calc_buffer_size(arg_pixel_format, out_width, out_height, &size_calc_out);
    calc_buffer_size(PIXEL_FORMAT_YUV_SEMIPLANAR_420, in_width, in_height, &size_calc_dw);

    // sensor 1 info
    k_vicap_sensor_info sensor_info_1;
    if(arg_dev == 1)
    {
        memset(&sensor_info_1, 0, sizeof(k_vicap_sensor_info));
        sensor_info_1.sensor_type = OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR;
        ret = kd_mapi_vicap_get_sensor_info(&sensor_info_1);
    }

    // vo init
    sample_vicap_vo_init();

    // init vb
    ret = sample_vicap_vb_init(dw_en, (k_u32)arg_dev, (k_u32)arg_chn, size_calc_in, size_calc_out, size_calc_dw, work_mode);
    if(ret)
    {
        ret |= kd_mapi_media_deinit();
    }

    // set dev attr
    for(k_u32 i = 0; i <= (k_u32)arg_dev; i++)
    {
        k_vicap_dev_set_info dev_attr_info;
        memset(&dev_attr_info, 0, sizeof(k_vicap_dev_set_info));
        // process working mode
        if((work_mode == VICAP_WORK_OFFLINE_MODE) || (work_mode == VICAP_WORK_LOAD_IMAGE_MODE))
        {
            dev_attr_info.mode = work_mode;
            dev_attr_info.buffer_num = VICAP_INPUT_BUF_NUM;
            dev_attr_info.buffer_size = VICAP_ALIGN_UP((in_width * in_height * 2), VICAP_ALIGN_1K);
        }
        else
        {
            dev_attr_info.mode = work_mode;
        }

        dev_attr_info.dw_en = dw_en;
        dev_attr_info.pipe_ctrl.data = 0xFFFFFFFF;
        dev_attr_info.pipe_ctrl.bits.ahdr_enable = 0;
        if(i == 1)
        {
            dev_attr_info.sensor_type = OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR;
        }
        else
        {
            dev_attr_info.sensor_type = arg_sensor;
        }
        dev_attr_info.vicap_dev = (k_vicap_dev)i;
        ret = kd_mapi_vicap_set_dev_attr(dev_attr_info);
    }

    // set chn attr
    for(k_u32 j = 0; j <= (k_u32)arg_dev; j++)
    {
        for(k_u32 i = 0; i <= (k_u32)arg_chn; i++)
        {
            k_vicap_chn_set_info chn_attr_info;
            memset(&chn_attr_info, 0, sizeof(k_vicap_chn_set_info));
            chn_attr_info.crop_en = K_FALSE;
            chn_attr_info.scale_en = K_FALSE;
            chn_attr_info.chn_en = K_TRUE;
            chn_attr_info.crop_h_start = 0;
            chn_attr_info.crop_v_start = 0;
            chn_attr_info.out_height = out_height;
            chn_attr_info.out_width = out_width;
            chn_attr_info.pixel_format = arg_pixel_format;
            chn_attr_info.buffer_num = 6;
            chn_attr_info.vicap_dev = (k_vicap_dev)j;
            // chn_attr_info.vicap_dev = arg_dev;
            chn_attr_info.vicap_chn = (k_vicap_chn)i;
            chn_attr_info.fps = fps;

            if(arg_pixel_format != PIXEL_FORMAT_RGB_BAYER_10BPP && arg_pixel_format != PIXEL_FORMAT_RGB_BAYER_12BPP)
            {
                // dewarp can enable in bayer pixel format
                if(dw_en && i == 0)
                {
                    printf("dw buf\n");
                    chn_attr_info.buf_size = size_calc_dw.buffsize_with_align;
                }
                else
                {
                    printf("dw disable buf\n");
                    chn_attr_info.buf_size = size_calc_out.buffsize_with_align;
                }
            }
            else
            {
                printf("dw is force disabled, becase of pixformat BAYER\n");
                dw_en = 0;
                chn_attr_info.buf_size = size_calc_out.buffsize_with_align;
            }
            ret = kd_mapi_vicap_set_chn_attr(chn_attr_info);
        }
    }

    // vo bind
    k_vicap_vo_layer_conf layer_conf;
    k_u8 vo_count = 0;
    for(int i = 0; i <= arg_dev; i++)
    {
        for(int j = 0 ; j <= arg_chn; j++)
        {
            k_s32 vo_chn;
            k_vo_layer layer;
            k_u16 rotation;
            rotation = 0;
            if (vo_count == 0)
            {
                vo_chn = K_VO_DISPLAY_CHN_ID1;
                layer = K_VO_LAYER1;
            }
            else if (vo_count == 1)
            {
                vo_chn = K_VO_DISPLAY_CHN_ID2;
                layer = K_VO_LAYER2;
            }
            else if (vo_count >= 2)
            {
                printf("only support bind two vo channel.\n");
                continue;
            }
            sample_vicap_bind_vo(i, j, vo_chn);
            layer_conf.enable[vo_count] = K_TRUE;
            layer_conf.width[vo_count] = out_width;
            layer_conf.height[vo_count] = out_height;
            layer_conf.rotation[vo_count] = rotation;
            layer_conf.layer[vo_count] = layer;
            vo_count++;
        }
    }

    ret = sample_vicap_vo_layer_init(&layer_conf, arg_dev);
    if (ret) {
        printf("sample_vicap, vo layer init failed.\n");
        goto vb_exit;
    }

    if(!ret)
    {
        printf("kd_mapi_vicap_set_attr ok\n");
    }

    for(int i = 0; i <= arg_dev; i++)
    {
        ret = kd_mapi_vicap_start((k_vicap_dev)i);
        if(!ret)
        {
            printf("vicap start isp %d...\n", i);
        }
    }
    // get optdate 
    // k_sensor_otp_date otp_date;

    //  memset(&otp_date, 0, sizeof(otp_date));
    // kd_mapi_sensor_otpdata_get(OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE_V2, &otp_date);
    // printf("otp date  is %s \n", otp_date.otp_date);

    // dump test
    k_video_frame dump_vf_info;
    k_u32 *dump_virt_addr = NULL;
    k_char filename[256];
    k_u32 num[VICAP_DEV_ID_MAX][VICAP_CHN_ID_MAX] = {0};
    char select;
    while(1)
    {
        printf("Press Enter to continue, q will exit!!!!\n");
        select = (char)getchar();
        if(select == 'q')
        {
            break;
        }
        for(k_u32 dev_num = 0; dev_num <= (k_u32)arg_dev; dev_num++)
        {
            for(k_u32 chn_num = 0; chn_num <= (k_u32)arg_chn; chn_num++)
            {
                ret = kd_mapi_vicap_dump_frame((k_vicap_dev)dev_num, (k_vicap_chn)chn_num, VICAP_DUMP_YUV, &dump_pic_info, 100);
                dump_vf_info = dump_pic_info.v_frame;
                printf("ret is %d dump_pic_info phy addr is %x \n", ret, dump_pic_info.v_frame.phys_addr[0]);
                printf("dump_vf_info.width: %d, dump_vf_info.height: %d\n", dump_pic_info.v_frame.width, dump_pic_info.v_frame.height);

                kd_ts_mmap(dump_vf_info.phys_addr[0], size_calc_out.buffsize, &dump_virt_addr); // map to dump_virt_addr

                memset(filename, 0, sizeof(filename));
                snprintf(filename, sizeof(filename), "dev_%02d_chn_%02d_%dx%d_%04d.%s", \
                                    dev_num, chn_num, dump_vf_info.width, dump_vf_info.height, num[dev_num][chn_num], size_calc_out.suffix);

                printf("save data file %s\n", filename);
                FILE *fd = fopen(filename, "wb");
                fwrite(dump_virt_addr, 1, size_calc_out.buffsize, fd);
                fclose(fd);

                if(ret >= 0)
                {
                    sleep(1);
                    ret = kd_mapi_vicap_release_frame(dev_num, chn_num, &dump_pic_info);
                }
                else
                {
                    sleep(1);
                }
                num[dev_num][chn_num]++;
            }
        }
    }
    printf("exit\n");

exit:
    vo_count = 0;
    for(int j = 0 ; j <= arg_dev; j++)
    {
        kd_mapi_vicap_stop((k_vicap_dev)j);
        for(int i = 0 ; i <= arg_chn; i++)
        {
            k_s32 vo_chn;
            k_vo_layer layer;
            if (vo_count == 0)
            {
                vo_chn = K_VO_DISPLAY_CHN_ID1;
                layer = K_VO_LAYER1;
            }
            else if (vo_count == 1)
            {
                vo_chn = K_VO_DISPLAY_CHN_ID2;
                layer = K_VO_LAYER2;
            }
            else
            {
                printf("only support unbind two vo chn.\n");
                continue;
            }
            sample_vicap_disable_vo_layer(layer);
            printf("sample_vicap, vo_count(%d), dev(%d) chn(%d) unbind vo chn(%d) layer(%d)\n", vo_count, j, i, vo_chn, layer);
            sample_vicap_unbind_vo(j, i, vo_chn);
            vo_count++;
        }

    }
    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
vb_exit:

    return ret;
}
