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
#include <errno.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "mpi_sys_api.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"

#include "vo_test_case.h"
#include "mapi_sys_api.h"

extern k_vo_display_resolution hx8399[20];


#define DISPLAY_WITDH  1088
#define DISPLAY_HEIGHT 1920

typedef struct {
    k_vicap_dev dev_num;
    k_bool dev_enable;
    k_vicap_sensor_type sensor_type;
    k_vicap_sensor_info sensor_info;

    k_u16 in_width;
    k_u16 in_height;

    k_bool ae_enable;
    k_bool awb_enable;
    k_bool dnr3_enable;
    k_bool hdr_enable;

    k_vicap_chn chn_num[VICAP_CHN_ID_MAX];

    k_bool chn_enable[VICAP_CHN_ID_MAX];
    k_pixel_format out_format[VICAP_CHN_ID_MAX];
    k_bool crop_enable[VICAP_CHN_ID_MAX];

    k_vicap_window out_win[VICAP_CHN_ID_MAX];

    k_u32 buf_size[VICAP_CHN_ID_MAX];

    k_video_frame_info dump_info[VICAP_CHN_ID_MAX];

    k_bool preview[VICAP_CHN_ID_MAX];
    k_u16 rotation[VICAP_CHN_ID_MAX];
    k_bool dw_enable;
} vicap_device_obj;

#define MAX_VO_LAYER_NUM 2

typedef struct {
    k_u16 width[MAX_VO_LAYER_NUM];
    k_u16 height[MAX_VO_LAYER_NUM];
    k_u16 rotation[MAX_VO_LAYER_NUM];
    k_vo_layer layer[MAX_VO_LAYER_NUM];
    k_bool enable[MAX_VO_LAYER_NUM];
} k_vicap_vo_layer_conf;

static void sample_vicap_vo_init(void)
{
    k_vo_display_resolution *resolution = &hx8399[0];
    k_vo_pub_attr attr;

    kd_display_set_backlight();
    kd_display_reset();
    dwc_dsi_init();

    memset(&attr, 0, sizeof(attr));

    attr.bg_color = 0x808000;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    kd_mpi_vo_init();
    kd_mpi_vo_set_dev_param(&attr);
}

static k_s32 sample_vicap_vo_layer_init(k_vicap_vo_layer_conf *layer_conf)
{
    k_s32 ret = 0;
    layer_info info[MAX_VO_LAYER_NUM];
    k_u16 margin = 0;
    k_u16 rotation = 0;
    k_u16 relative_height = 0;
    k_u16 total_height = 0;

    memset(&info, 0, sizeof(info));

    for (int i = 0; i < MAX_VO_LAYER_NUM; i++) {
        if (layer_conf->enable[i]) {
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
    }

    for (int i = 0; i < MAX_VO_LAYER_NUM; i++) {
        if (layer_conf->enable[i]) {
            info[i].offset.x = (DISPLAY_WITDH - info[i].act_size.width)/2;
            info[i].offset.y = margin + relative_height;
            printf("%s, layer(%d), offset.x(%d), offset.y(%d), relative_height(%d)\n", __func__, layer_conf->layer[i], info[i].offset.x, info[i].offset.y, relative_height);
            relative_height += info[i].act_size.height + margin;

            info[i].format = PIXEL_FORMAT_YVU_PLANAR_420;
            info[i].global_alptha = 0xff;

            vo_creat_layer_test(layer_conf->layer[i], &info[i]);
        }
    }

    return ret;
}

static void sample_vicap_vo_enable(void)
{
    kd_mpi_vo_enable();
}

static void sample_vicap_disable_vo_layer(k_vo_layer layer)
{
    kd_mpi_vo_disable_video_layer(layer);
}

static k_s32 sample_vicap_vb_init(vicap_device_obj *dev_obj)
{
    k_s32 ret = 0;
    k_vb_config config;
    k_vb_supplement_config supplement_config;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    for (int i = 0, k =0; i < VICAP_DEV_ID_MAX; i++) {
        if (!dev_obj[i].dev_enable)
            continue;
        printf("%s, enable dev(%d)\n", __func__, i);
        for (int j = 0; j < VICAP_CHN_ID_MAX; j++) {
            if (!dev_obj[i].chn_enable[j])
                continue;
            printf("%s, enable chn(%d), k(%d)\n", __func__, j, k);
            config.comm_pool[k].blk_cnt = VICAP_MIN_FRAME_COUNT * 2;
            config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;

            k_pixel_format pix_format = dev_obj[i].out_format[j];
            k_u16 out_width = dev_obj[i].out_win[j].width;
            k_u16 out_height = dev_obj[i].out_win[j].height;
            k_u16 in_width = dev_obj[i].in_width;
            k_u16 in_height = dev_obj[i].in_height;

            switch (pix_format) {
            case PIXEL_FORMAT_YUV_SEMIPLANAR_420:
                config.comm_pool[k].blk_size = VICAP_ALIGN_UP((out_width * out_height * 3 / 2), VICAP_ALIGN_1K);
                break;
            case PIXEL_FORMAT_RGB_888:
            case PIXEL_FORMAT_RGB_888_PLANAR:
                config.comm_pool[k].blk_size = VICAP_ALIGN_UP((out_width * out_height * 3), VICAP_ALIGN_1K);
                break;
            case PIXEL_FORMAT_RGB_BAYER_10BPP:
                config.comm_pool[k].blk_size = VICAP_ALIGN_UP((in_width * in_height * 2), VICAP_ALIGN_1K);
                break;
            default:
                dev_obj[i].out_format[j] = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
                config.comm_pool[k].blk_size = VICAP_ALIGN_UP((out_width * out_height * 3 / 2), VICAP_ALIGN_1K);
                break;
            }
            dev_obj[i].buf_size[j] = config.comm_pool[k].blk_size;
            printf("%s, dev(%d) chn(%d) pool(%d) buf_size(%d)\n", __func__, i, j, k ,dev_obj[i].buf_size[j]);
            k++;
        }
    }

    ret = kd_mpi_vb_set_config(&config);
    if (ret) {
        printf("vb_set_config failed ret:%d\n", ret);
        return ret;
    }

    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;

    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    if (ret) {
        printf("vb_set_supplement_config failed ret:%d\n", ret);
        return ret;
    }

    ret = kd_mpi_vb_init();
    if (ret) {
        printf("vb_init failed ret:%d\n", ret);
        return ret;
    }

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

    ret = kd_mpi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
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

    ret = kd_mpi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }

    return;
}

#define VICAP_MIN_PARAMETERS (7)

static void usage(void)
{
    printf("usage: ./sample_vicap -dev 0 -sensor 0 -chn 0 -chn 1 -ow 640 -oh 480 -preview 1 -rotation 1\n");
    printf("Options:\n");
    printf(" -dev:          vicap device id[0,1,2]\tdefault 0\n");
    printf(" -sensor:       sensor type[0: ov9732@1280x720, 1: ov9286_ir@1280x720], 2: ov9286_speckle@1280x720]\n");
    printf(" -ae:           ae status[0: disable AE, 1: enable AE]\tdefault enable\n");
    printf(" -awb:          awb status[0: disable AWB, 1: enable AWb]\tdefault enable\n");
    printf(" -chn:          vicap output channel id[0,1,2]\tdefault 0\n");
    printf(" -ow:           the output image width, default same with input width\n");
    printf(" -oh:           the output image height, default same with input height\n");
    printf(" -ox:           the output image start position of x\n");
    printf(" -oy:           the output image start position of y\n");
    printf(" -crop:         crop enable[0: disable, 1: enable]\n");
    printf(" -ofmt:         the output pixel format[0: yuv, 1: rgb888, 2: rgb888p, 3: raw], only channel 0 support raw data, default yuv\n");
    printf(" -preview:      the output preview enable[0: disable, 1: enable], only support 2 output channel preview\n");
    printf(" -rotation:     display rotaion[0: degree 0, 1: degree 90, 2: degree 270, 3: degree 180, 4: unsupport rotaion]\n");
    printf(" -help:         print this help\n");

    exit(1);
}

int main(int argc, char *argv[])
{
    k_s32 ret = 0;

    vicap_device_obj device_obj[VICAP_DEV_ID_MAX];
    memset(&device_obj, 0 , sizeof(device_obj));

    k_vicap_vo_layer_conf layer_conf;
    memset(&layer_conf, 0 , sizeof(k_vicap_vo_layer_conf));

    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;

    k_video_frame_info dump_info;

    k_u8 dev_count = 0, cur_dev = 0;
    k_u8 chn_count = 0, cur_chn = 0;
    k_u8 vo_count = 0, preview_count = 0;

    k_u32 pipe_ctrl = 0xFFFFFFFF;
    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));

    if (argc < VICAP_MIN_PARAMETERS) {
        printf("sample_vicap requires some necessary parameters:\n");
        usage();
    }

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            usage();
        }
        else if (strcmp(argv[i], "-dev") == 0)
        {
dev_parse:
            chn_count = 0;
            if ((i + 1) >= argc) {
                printf("dev parameters missing.\n");
                return -1;
            }
            cur_dev = atoi(argv[i + 1]);
            if (cur_dev >= VICAP_DEV_ID_MAX)
            {
                printf("unsupported vicap device, the valid device num is:0, 1, 2!\n");
                return -1;
            }
            dev_count++;
            printf("cur_dev(%d), dev_count(%d)\n", cur_dev, dev_count);

            if (dev_count >= VICAP_DEV_ID_MAX)
            {
                printf("the vicap device number exceeds the limit!\n");
                return -1;
            }
            //TODO
            if (dev_count > 1) {
                printf("current version only support one vicap device!!!\n");
                return -1;
            }

            device_obj[cur_dev].dev_num = cur_dev;
            device_obj[cur_dev].dev_enable = K_TRUE;
            device_obj[cur_dev].ae_enable = K_TRUE;//default enable ae
            device_obj[cur_dev].awb_enable = K_TRUE;//default enable awb
            device_obj[cur_dev].dnr3_enable = K_FALSE;//default disable 3ndr
            device_obj[cur_dev].hdr_enable = K_FALSE;//default disable hdr
            //parse dev paramters
            for (i = i + 2; i < argc; i += 2)
            {
chn_parse:
                if (strcmp(argv[i], "-chn") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("chn parameters missing.\n");
                        return -1;
                    }
                    cur_chn = atoi(argv[i + 1]);
                    if (cur_chn >= VICAP_CHN_ID_MAX) {
                        printf("unsupported vicap channel, the valid channel num is:0, 1, 2!\n");
                        return -1;
                    }

                    chn_count++;
                    if (chn_count > VICAP_CHN_ID_MAX) {
                        printf("the vicap channel number exceeds the limit!\n");
                        return -1;
                    }

                    printf("cur_chn(%d), chn_count(%d)\n", cur_chn ,chn_count);
                    device_obj[cur_dev].chn_num[cur_chn] = cur_chn;
                    device_obj[cur_dev].chn_enable[cur_chn] = K_TRUE;
                    device_obj[cur_dev].preview[cur_chn] = K_TRUE;//default enable preview
                    //parse chn parameters
                    for (i = i + 2; i < argc; i += 2)
                    {
                        if ((i + 1) >= argc) {
                            printf("chn parameters(%s) error.\n", argv[i]);
                            usage();
                        }

                        if (strcmp(argv[i], "-ox") == 0)
                        {
                            k_u16 x_start = atoi(argv[i + 1]);
                            device_obj[cur_dev].out_win[cur_chn].h_start = x_start;
                        }
                        else if (strcmp(argv[i], "-oy") == 0)
                        {
                            k_u16 y_start = atoi(argv[i + 1]);
                            device_obj[cur_dev].out_win[cur_chn].v_start = y_start;
                        }
                        else if (strcmp(argv[i], "-ow") == 0)
                        {
                            k_u16 out_width = atoi(argv[i + 1]);
                            out_width = VICAP_ALIGN_UP(out_width, 16);
                            device_obj[cur_dev].out_win[cur_chn].width = out_width;
                        }
                        else if (strcmp(argv[i], "-oh") == 0)
                        {
                            k_u16 out_height = atoi(argv[i + 1]);
                            out_height = VICAP_ALIGN_UP(out_height, 16);
                            device_obj[cur_dev].out_win[cur_chn].height = out_height;
                        }
                        else if (strcmp(argv[i], "-rotation") == 0)
                        {
                            k_u16 rotation = atoi(argv[i + 1]);
                            device_obj[cur_dev].rotation[cur_chn] = rotation;
                        }
                        else if (strcmp(argv[i], "-crop") == 0)
                        {
                            k_u16 crop = atoi(argv[i + 1]);
                            if (crop == 0)
                                device_obj[cur_dev].crop_enable[cur_chn] = K_FALSE;
                            else if (crop == 1)
                                device_obj[cur_dev].crop_enable[cur_chn] = K_TRUE;
                            else {
                                printf("invalid crop paramters.\n");
                                usage();
                            }
                        }
                        else if (strcmp(argv[i], "-ofmt") == 0)
                        {
                            k_u16 out_format = atoi(argv[i + 1]);
                            if (out_format > 3) {
                                printf("unsupported out format\n");
                                return -1;
                            } else if ((out_format == 3) && (cur_chn != 0)) {
                                printf("only channel 0 supoorted raw data output\n");
                                return -1;
                            }
                            switch (out_format) {
                            case 0://yuv
                                device_obj[cur_dev].out_format[cur_chn] = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
                                break;
                            case 1://rgb888
                                device_obj[cur_dev].out_format[cur_chn] = PIXEL_FORMAT_RGB_888;
                                break;
                            case 2://rgb888p
                                device_obj[cur_dev].out_format[cur_chn] = PIXEL_FORMAT_RGB_888_PLANAR;
                                break;
                            case 3://raw
                                device_obj[cur_dev].out_format[cur_chn] = PIXEL_FORMAT_RGB_BAYER_10BPP;
                                break;
                            default:
                                printf("unsupported pixel format\n");
                                return -1;
                            }
                        }
                        else if (strcmp(argv[i], "-preview") == 0)
                        {
                            k_u16 preview = atoi(argv[i + 1]);
                            if (preview == 0)
                                device_obj[cur_dev].preview[cur_chn] = K_FALSE;
                            else if (preview == 1)
                                device_obj[cur_dev].preview[cur_chn] = K_TRUE;
                            else {
                                printf("invalid preview paramters.\n");
                                usage();
                            }
                            if (preview_count > 2) {
                                printf("only support two output channel for preview\n");
                                return -1;
                            }
                        }
                        else if (strcmp(argv[i], "-chn") == 0)
                        {
                            goto chn_parse;
                        }
                        else if (strcmp(argv[i], "-dev") == 0)
                        {
                            goto dev_parse;
                        }
                        else
                        {
                            printf("invalid chn paramters.\n");
                            usage();
                        }
                    }
                }
                else if (strcmp(argv[i], "-sensor") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("sensor parameters missing.\n");
                        return -1;
                    }
                    k_u16 sensor = atoi(argv[i + 1]);
                    if (sensor == 0) {
                        device_obj[cur_dev].sensor_type = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
                    } else if (sensor == 1) {
                        device_obj[cur_dev].sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR;
                    } else if (sensor == 2) {
                        device_obj[cur_dev].sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;
                    } else if (sensor == 3) {
                        device_obj[cur_dev].sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
                    } else if (sensor == 4) {
                        device_obj[cur_dev].sensor_type = IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
                    } else if (sensor == 5) {
                        device_obj[cur_dev].sensor_type = IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR;
                    } else {
                        printf("unsupport sensor type.\n");
                        return -1;
                    }
                }
                else if (strcmp(argv[i], "-pipe") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("pipe parameters missing.\n");
                        return -1;
                    }
                    pipe_ctrl = atoi(argv[i + 1]);
                }
                else if (strcmp(argv[i], "-ae") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("ae parameters missing.\n");
                        return -1;
                    }
                    k_s32 ae_status = atoi(argv[i + 1]);
                    if (ae_status == 0) {
                        device_obj[cur_dev].ae_enable = K_FALSE;
                    } else if (ae_status == 1) {
                        device_obj[cur_dev].ae_enable = K_TRUE;
                    } else {
                        printf("unsupport ae parameters.\n");
                        return -1;
                    }
                }
                else if (strcmp(argv[i], "-awb") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("awb parameters missing.\n");
                        return -1;
                    }
                    k_s32 awb_status = atoi(argv[i + 1]);
                    if (awb_status == 0) {
                        device_obj[cur_dev].awb_enable = K_FALSE;
                    } else if (awb_status == 1) {
                        device_obj[cur_dev].awb_enable = K_TRUE;
                    } else {
                        printf("unsupport awb parameters.\n");
                        return -1;
                    }
                }
                else if (strcmp(argv[i], "-hdr") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("hdr parameters missing.\n");
                        return -1;
                    }
                    k_s32 hdr_status = atoi(argv[i + 1]);
                    if (hdr_status == 0) {
                        device_obj[cur_dev].hdr_enable = K_FALSE;
                    } else if (hdr_status == 1) {
                        device_obj[cur_dev].hdr_enable = K_TRUE;
                    } else {
                        printf("unsupport hdr parameters.\n");
                        return -1;
                    }
                }
                else if (strcmp(argv[i], "-dnr") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("3ndr parameters missing.\n");
                        return -1;
                    }
                    k_s32 dnr3_status = atoi(argv[i + 1]);
                    if (dnr3_status == 0) {
                        device_obj[cur_dev].dnr3_enable = K_FALSE;
                    } else if (dnr3_status == 1) {
                        device_obj[cur_dev].dnr3_enable = K_TRUE;
                    } else {
                        printf("unsupport 3dnr parameters.\n");
                        return -1;
                    }
                }
                else if (strcmp(argv[i], "-dw") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("dw parameters missing.\n");
                        return -1;
                    }
                    // enable dewarp
                    dev_attr.dw_enable = atoi(argv[i + 1]);
                    device_obj[cur_dev].dw_enable = dev_attr.dw_enable;
                }
                else
                {
                    printf("invalid dev paramters.\n");
                    usage();
                }
            }
        }
        else
        {
            printf("invalid paramters.\n");
            usage();
        }
    }

    printf("sample_vicap: dev_count(%d), chn_count(%d)\n", dev_count, chn_count);
        // ipcmsg init
    kd_mapi_sys_init();
    sample_vicap_vo_init();

    printf("sample_vicap ...kd_mpi_vicap_get_sensor_info\n");

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        //vicap get sensor info
        ret = kd_mpi_vicap_get_sensor_info(device_obj[dev_num].sensor_type, &device_obj[dev_num].sensor_info);
        if (ret) {
            printf("sample_vicap, the sensor type not supported!\n");
            return ret;
        }
        device_obj[dev_num].in_width = device_obj[dev_num].sensor_info.width;
        device_obj[dev_num].in_height = device_obj[dev_num].sensor_info.height;
        printf("sample_vicap, dev[%d] in size[%dx%d]\n", \
            dev_num, device_obj[dev_num].in_width, device_obj[dev_num].in_height);

        //vicap device attr set
        dev_attr.acq_win.h_start = 0;
        dev_attr.acq_win.v_start = 0;
        dev_attr.acq_win.width = device_obj[dev_num].in_width;
        dev_attr.acq_win.height = device_obj[dev_num].in_height;
        dev_attr.mode = VICAP_WORK_ONLINE_MODE;

        dev_attr.pipe_ctrl.data = pipe_ctrl;
        dev_attr.pipe_ctrl.bits.af_enable = 0;
        dev_attr.pipe_ctrl.bits.ae_enable = device_obj[dev_num].ae_enable;
        dev_attr.pipe_ctrl.bits.awb_enable = device_obj[dev_num].awb_enable;
        dev_attr.pipe_ctrl.bits.dnr3_enable = device_obj[dev_num].dnr3_enable;
        dev_attr.pipe_ctrl.bits.ahdr_enable = device_obj[dev_num].hdr_enable;

        dev_attr.cpature_frame = 0;

        memcpy(&dev_attr.sensor_info, &device_obj[dev_num].sensor_info, sizeof(k_vicap_sensor_info));

        ret = kd_mpi_vicap_set_dev_attr(dev_num, dev_attr);
        if (ret) {
            printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
            return ret;
        }

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            //set default value
            if (!device_obj[dev_num].out_format[chn_num]) {
                device_obj[dev_num].out_format[chn_num] = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
            }

            if (!device_obj[dev_num].out_win[chn_num].width) {
                device_obj[dev_num].out_win[chn_num].width = device_obj[dev_num].in_width;
            }

            if (!device_obj[dev_num].out_win[chn_num].height) {
                device_obj[dev_num].out_win[chn_num].height = device_obj[dev_num].in_height;
            }

            if ( device_obj[dev_num].out_win[chn_num].h_start || device_obj[dev_num].out_win[chn_num].v_start) {
                device_obj[dev_num].crop_enable[chn_num] = K_TRUE;
            }

            if ((device_obj[dev_num].out_win[chn_num].width > DISPLAY_WITDH)
                && (device_obj[dev_num].out_win[chn_num].height > DISPLAY_HEIGHT)) {
                device_obj[dev_num].preview[chn_num] = K_FALSE;
            }

            if (!device_obj[dev_num].rotation[chn_num]
                && ((device_obj[dev_num].out_win[chn_num].width > DISPLAY_WITDH)
                && (device_obj[dev_num].out_win[chn_num].width < DISPLAY_HEIGHT))) {
                device_obj[dev_num].rotation[chn_num] = 1;
            }

            printf("sample_vicap, dev_num(%d), chn_num(%d), in_size[%dx%d], out_offset[%d:%d], out_size[%dx%d]\n", \
                dev_num, chn_num, device_obj[dev_num].in_width, device_obj[dev_num].in_height, \
                device_obj[dev_num].out_win[chn_num].h_start, device_obj[dev_num].out_win[chn_num].v_start, \
                device_obj[dev_num].out_win[chn_num].width, device_obj[dev_num].out_win[chn_num].height);
        }
    }

    ret = sample_vicap_vb_init(device_obj);
    if (ret) {
        printf("sample_vicap_vb_init failed\n");
        return -1;
    }

    //vicap channel attr set
    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
            if (device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_RGB_BAYER_10BPP) {
                chn_attr.out_win.width = device_obj[dev_num].in_width;
                chn_attr.out_win.height = device_obj[dev_num].in_height;
            } else {
                chn_attr.out_win.width = device_obj[dev_num].out_win[chn_num].width;
                chn_attr.out_win.height = device_obj[dev_num].out_win[chn_num].height;
            }

            if (device_obj[dev_num].crop_enable[chn_num]) {
                chn_attr.crop_win = chn_attr.out_win;
            } else {
                chn_attr.crop_win.width = device_obj[dev_num].in_width;
                chn_attr.crop_win.height = device_obj[dev_num].in_height;
            }

            chn_attr.scale_win = chn_attr.out_win;
            chn_attr.crop_enable = K_FALSE;
            chn_attr.scale_enable = K_FALSE;
            chn_attr.chn_enable = K_TRUE;

            chn_attr.pix_format = device_obj[dev_num].out_format[chn_num];
            chn_attr.buffer_num = VICAP_MIN_FRAME_COUNT * 2;//at least 3 buffers for isp
            chn_attr.buffer_size = device_obj[dev_num].buf_size[chn_num];

            printf("sample_vicap, set dev(%d) chn(%d) attr, buffer_size(%d), out size[%dx%d]\n", \
                dev_num, chn_num, chn_attr.buffer_size, chn_attr.out_win.width, chn_attr.out_win.height);
            ret = kd_mpi_vicap_set_chn_attr(dev_num, chn_num, chn_attr);
            if (ret) {
                printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
                goto vb_exit;
            }

            //bind vicap to vo, only support bind two vo chn(K_VO_DISPLAY_CHN_ID1 & K_VO_DISPLAY_CHN_ID2)
            if (device_obj[dev_num].preview[chn_num]) {
                k_s32 vo_chn;
                k_vo_layer layer;
                k_u16 rotation;
                if (vo_count == 0) {
                    vo_chn = K_VO_DISPLAY_CHN_ID1;
                    layer = K_VO_LAYER1;
                    rotation = device_obj[dev_num].rotation[chn_num];
                } else if (vo_count == 1) {
                    vo_chn = K_VO_DISPLAY_CHN_ID2;
                    layer = K_VO_LAYER2;
                    rotation = 4;//layer2 unsupport roation
                } else if (vo_count >= MAX_VO_LAYER_NUM){
                    printf("only support bind two vo channel.\n");
                    continue;
                }
                printf("sample_vicap, vo_count(%d), dev(%d) chn(%d) bind vo chn(%d) layer(%d) rotation(%d)\n", vo_count, dev_num, chn_num, vo_chn, layer, rotation);
                sample_vicap_bind_vo(dev_num, chn_num, vo_chn);

                layer_conf.enable[vo_count] = K_TRUE;
                layer_conf.width[vo_count] = chn_attr.out_win.width;
                layer_conf.height[vo_count] = chn_attr.out_win.height;
                layer_conf.rotation[vo_count] = rotation;
                layer_conf.layer[vo_count] = layer;
                vo_count++;
            }
        }
    }

    ret = sample_vicap_vo_layer_init(&layer_conf);
    if (ret) {
        printf("sample_vicap, vo layer init failed.\n");
        goto vb_exit;
    }

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        printf("sample_vicap, vicap dev(%d) init\n", dev_num);
        ret = kd_mpi_vicap_init(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) init failed.\n", dev_num);
            goto app_exit;
        }

        printf("sample_vicap, vicap dev(%d) start stream\n", dev_num);
        ret = kd_mpi_vicap_start_stream(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) start stream failed.\n", dev_num);
            goto app_exit;
        }
    }

    sample_vicap_vo_enable();


    k_isp_ae_roi ae_roi;
    memset(&ae_roi, 0, sizeof(k_isp_ae_roi));

    k_char select;
    while(K_TRUE)
    {
        if(select != '\n')
        {
            printf("---------------------------------------\n");
            printf(" Input character to select test option\n");
            printf("---------------------------------------\n");
            printf(" d: dump data addr test\n");
            printf(" s: set isp ae roi test\n");
            printf(" g: get isp ae roi test\n");
            printf(" q: to exit\n");
            printf("---------------------------------------\n");
            printf("please Input:\n\n");
        }
        select = (k_char)getchar();
        switch (select)
        {
        case 'd':
            printf("sample_vicap... dump frame.\n");
            for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
                if (!device_obj[dev_num].dev_enable)
                    continue;

                for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
                    if (!device_obj[dev_num].chn_enable[chn_num])
                        continue;

                    printf("sample_vicap, dev(%d) chn(%d) dump frame.\n", dev_num, chn_num);

                    memset(&dump_info, 0 , sizeof(k_video_frame_info));
                    ret = kd_mpi_vicap_dump_frame(dev_num, chn_num, VICAP_DUMP_YUV, &dump_info, 1000);
                    if (ret) {
                        printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                    }

                    static k_u32 dump_count = 0;
                    k_char *suffix;
                    k_u32 data_size = 0;
                    void *virt_addr = NULL;
                    k_char filename[256];

                    if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_YUV_SEMIPLANAR_420) {
                        suffix = "yuv420sp";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3 /2;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_888) {
                        suffix = "rgb888";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3;
                    }  else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_888_PLANAR) {
                        suffix = "rgb888p";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_BAYER_10BPP) {
                        suffix = "raw10";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 2;
                    } else {
                        suffix = "unkown";
                    }

                    virt_addr = kd_mpi_sys_mmap(dump_info.v_frame.phys_addr[0], data_size);
                    if (virt_addr) {
                        memset(filename, 0 , sizeof(filename));

                        snprintf(filename, sizeof(filename), "dev_%02d_chn_%02d_%dx%d_%04d.%s", \
                            dev_num, chn_num, dump_info.v_frame.width, dump_info.v_frame.height, dump_count, suffix);

                        printf("save dump data to file(%s)\n", filename);
                        FILE *file = fopen(filename, "wb+");
                        if (file) {
                            fwrite(virt_addr, 1, data_size, file);
                            fclose(file);
                        } else {
                            printf("sample_vicap, open dump file failed(%s)\n", strerror(errno));
                        }
                    } else {
                        printf("sample_vicap, map dump addr failed.\n");
                    }

                    printf("sample_vicap, release dev(%d) chn(%d) dump frame.\n", dev_num, chn_num);

                    ret = kd_mpi_vicap_dump_release(dev_num, chn_num, &dump_info);
                    if (ret) {
                        printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                    }
                    dump_count++;
                }
            }
            break;
        case 's':
            printf("sample_vicap... set roi.\n");
            printf("roi will set top left corner and lower right corner on the Image\n");

            for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
                if (!device_obj[dev_num].dev_enable)
                    continue;
                ae_roi.roiNum = 2;
                ae_roi.roiWindow[0].weight = 100.0f;
                ae_roi.roiWindow[0].window.hOffset = 0;
                ae_roi.roiWindow[0].window.vOffset = 0;
                ae_roi.roiWindow[0].window.width = 200;
                ae_roi.roiWindow[0].window.height = 200;

                ae_roi.roiWindow[1].weight = 90.0f;
                ae_roi.roiWindow[1].window.hOffset = 1000;
                ae_roi.roiWindow[1].window.vOffset = 500;
                ae_roi.roiWindow[1].window.width = 200;
                ae_roi.roiWindow[1].window.height = 200;
                ret = kd_mpi_isp_ae_set_roi(dev_num, ae_roi);
                if (ret == K_SUCCESS) {
                    printf("sample_vicap...kd_mpi_vicap_dump_frame success.\n");
                }
            }
            break;
        case 'g':
            printf("sample_vicap... get roi.\n");

            for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
                if (!device_obj[dev_num].dev_enable)
                    continue;
                memset(&ae_roi, 0 , sizeof(k_isp_ae_roi));
                ret = kd_mpi_isp_ae_get_roi(dev_num, &ae_roi);
                for(k_u32 i = 0; i < ae_roi.roiNum; i++)
                {
                    printf("dev_num:%d, i: %d, h: %d, v: %d, height: %d, width: %d, weight: %f\n",
                            dev_num, i,
                            ae_roi.roiWindow[i].window.hOffset,
                            ae_roi.roiWindow[i].window.vOffset,
                            ae_roi.roiWindow[i].window.height,
                            ae_roi.roiWindow[i].window.width,
                            ae_roi.roiWindow[i].weight
                        );
                }
            }
            break;
        case 'q':
            goto app_exit;
        default:
            break;
        }
        sleep(1);
    }

app_exit:
    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        printf("sample_vicap, vicap dev(%d) stop stream\n", dev_num);
        ret = kd_mpi_vicap_stop_stream(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) stop stream failed.\n", dev_num);
        }

        printf("sample_vicap, vicap dev(%d) deinit\n", dev_num);
        ret = kd_mpi_vicap_deinit(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) deinit failed.\n", dev_num);
        }
    }

    vo_count = 0;
    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            if (device_obj[dev_num].preview[chn_num]) {
                k_s32 vo_chn;
                k_vo_layer layer;
                if (vo_count == 0) {
                    vo_chn = K_VO_DISPLAY_CHN_ID1;
                    layer = K_VO_LAYER1;
                } else if (vo_count == 1) {
                    vo_chn = K_VO_DISPLAY_CHN_ID2;
                    layer = K_VO_LAYER2;
                } else {
                    printf("only support unbind two vo chn.\n");
                    continue;
                }
                sample_vicap_disable_vo_layer(layer);
                printf("sample_vicap, vo_count(%d), dev(%d) chn(%d) unbind vo chn(%d) layer(%d)\n", vo_count, dev_num, chn_num, vo_chn, layer);
                sample_vicap_unbind_vo(dev_num, chn_num, vo_chn);
                vo_count++;
            }
        }
    }


    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

vb_exit:

    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("sample_vicap, kd_mpi_vb_exit failed.\n");
        return ret;
    }
    kd_mapi_sys_deinit();
    return ret;
}
