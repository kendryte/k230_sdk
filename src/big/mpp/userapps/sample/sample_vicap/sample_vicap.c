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
#include <fcntl.h>
#include <stdint.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_vicap_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "mpi_sys_api.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"
#include "vo_test_case.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#include "mpi_sensor_api.h"
#include "k_dma_comm.h"
#include "mpi_dma_api.h"

extern k_vo_display_resolution hx8399[20];

#define VICAP_OUTPUT_BUF_NUM 5//3
#define VICAP_INPUT_BUF_NUM 3
#define GDMA_BUF_NUM 3

#define DISPLAY_WITDH  1088
#define DISPLAY_HEIGHT 1920

typedef struct {
    k_vicap_dev dev_num;
    k_bool dev_enable;
    k_vicap_sensor_type sensor_type;
    k_vicap_sensor_info sensor_info;

    k_u16 in_width;
    k_u16 in_height;

    //for mcm
    k_vicap_work_mode mode;
    k_u32 in_size;
    k_pixel_format in_format;

    k_vicap_input_type input_type;
    k_vicap_image_pattern pattern;
    const char *file_path;//input raw image file
    const char *calib_file;
    void *image_data;
    k_u32 dalign;

    k_bool ae_enable;
    k_bool awb_enable;
    k_bool dnr3_enable;
    k_bool hdr_enable;

    k_vicap_chn chn_num[VICAP_CHN_ID_MAX];

    k_bool chn_enable[VICAP_CHN_ID_MAX];
    k_pixel_format out_format[VICAP_CHN_ID_MAX];
    
    k_bool crop_enable[VICAP_CHN_ID_MAX];

    k_vicap_window out_win[VICAP_CHN_ID_MAX];

    k_vicap_window crop_win[VICAP_CHN_ID_MAX];

    k_u32 buf_size[VICAP_CHN_ID_MAX];

    k_video_frame_info dump_info[VICAP_CHN_ID_MAX];

    k_bool preview[VICAP_CHN_ID_MAX];
    k_u16 rotation[VICAP_CHN_ID_MAX];
    k_u8 fps[VICAP_CHN_ID_MAX];
    k_bool dw_enable;
    k_vicap_mirror sensor_mirror;
} vicap_device_obj;

#define MAX_VO_LAYER_NUM 3

typedef struct {
    k_u16 width[MAX_VO_LAYER_NUM];
    k_u16 height[MAX_VO_LAYER_NUM];
    k_u16 rotation[MAX_VO_LAYER_NUM];
    k_vo_layer layer[MAX_VO_LAYER_NUM];
    k_bool enable[MAX_VO_LAYER_NUM];
} k_vicap_vo_layer_conf;

static k_s32 sample_vicap_vo_init(k_connector_type connector_type)
{

    k_u32 ret = 0;
    k_s32 connector_fd;
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
    kd_mpi_connector_power_set(connector_fd, 1);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;

}

static k_s32 sample_vicap_vo_layer_init(k_vicap_vo_layer_conf *layer_conf, k_u32 display_width, k_u32 display_height)
{
    k_s32 ret = 0;
    layer_info info[MAX_VO_LAYER_NUM];
    k_u16 margin = 0;
    k_u16 rotation = 0;
    k_u16 relative_height = 0;
    k_u16 total_height = 0;
    osd_info osd_info;

    memset(&info, 0, sizeof(info));
    memset(&osd_info, 0, sizeof(osd_info));

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
            margin = ((display_height - total_height) / (i+2));
            if ((total_height > display_height) || (info[i].act_size.width > display_width)) {
                printf("%s, the preview window size[%dx%d] exceeds the display window size[%dx%d].\n", \
                    __func__, info[i].act_size.width, total_height, display_width, display_height);
                return -1;
            }
            printf("%s, width(%d), height(%d), margin(%d), total_height(%d)\n", \
                __func__, info[i].act_size.width, info[i].act_size.height, margin, total_height);
        }
    }

    for (int i = 0; i < MAX_VO_LAYER_NUM - 1; i++) {
        if (layer_conf->enable[i]) {
            info[i].offset.x = (display_width - info[i].act_size.width)/2;
            info[i].offset.y = margin + relative_height;
            printf("%s, layer(%d), offset.x(%d), offset.y(%d), relative_height(%d)\n", __func__, layer_conf->layer[i], info[i].offset.x, info[i].offset.y, relative_height);
            relative_height += info[i].act_size.height + margin;

            info[i].format = PIXEL_FORMAT_YVU_PLANAR_420;
            info[i].global_alptha = 0xff;

            vo_creat_layer_test(layer_conf->layer[i], &info[i]);
        }
    }

    // osd enable 
    if(layer_conf->enable[2])
    {
        osd_info.act_size.width = layer_conf->width[2]; ;
        osd_info.act_size.height = layer_conf->height[2];;
        osd_info.offset.x = (display_width - layer_conf->width[2])/2;;
        osd_info.offset.y = margin + relative_height;
        osd_info.global_alptha = 0xff;
        osd_info.format = PIXEL_FORMAT_RGB_888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

        vo_creat_osd_test(layer_conf->layer[2], &osd_info);
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

static void sample_vicap_disable_vo_osd(k_vo_osd layer)
{
    kd_mpi_vo_osd_disable(layer);
}

static k_s32 sample_vicap_vb_init(vicap_device_obj *dev_obj)
{
    k_s32 ret = 0;
    k_vb_config config;
    k_vb_supplement_config supplement_config;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    int k = 0;
    for (int i = 0; i < VICAP_DEV_ID_MAX; i++) {
        if (!dev_obj[i].dev_enable)
            continue;
        printf("%s, enable dev(%d)\n", __func__, i);

        if ((dev_obj[i].mode == VICAP_WORK_OFFLINE_MODE) || (dev_obj[i].mode == VICAP_WORK_SW_TILE_MODE)) {
            config.comm_pool[k].blk_cnt = VICAP_INPUT_BUF_NUM;
            config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            config.comm_pool[k].blk_size = dev_obj[i].in_size;
            printf("%s, dev(%d) pool(%d) in_size(%d) blk_cnt(%d)\n", __func__, i , k ,dev_obj[i].in_size, config.comm_pool[k].blk_cnt);
            k++;
        }

        for (int j = 0; j < VICAP_CHN_ID_MAX; j++) {
            if (!dev_obj[i].chn_enable[j])
                continue;
            printf("%s, enable chn(%d), k(%d)\n", __func__, j, k);
            config.comm_pool[k].blk_cnt = VICAP_OUTPUT_BUF_NUM;
            config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            if (dev_obj[i].preview[j] && dev_obj[i].rotation[j] > 16)
                config.comm_pool[k].blk_cnt += GDMA_BUF_NUM;

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
            printf("%s, dev(%d) chn(%d) pool(%d) buf_size(%d) blk_cnt(%d)\n", __func__, i, j, k ,dev_obj[i].buf_size[j], config.comm_pool[k].blk_cnt);
            k++;
        }
        if (dev_obj[i].dw_enable) {
            // another buffer for isp->dw
            config.comm_pool[k].blk_size = VICAP_ALIGN_UP((dev_obj[i].in_width * dev_obj[i].in_height * 3 / 2), VICAP_ALIGN_1K);
            config.comm_pool[k].blk_cnt = VICAP_MIN_FRAME_COUNT * 2;
            config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            dev_obj[i].buf_size[0] = config.comm_pool[k].blk_size;
            printf("%s, dev(%d) pool(%d) buf_size(%d)\n", __func__, i, k ,dev_obj[i].buf_size[0]);
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

static k_s32 dma_dev_attr_init(void)
{
    k_dma_dev_attr_t dev_attr;

    dev_attr.burst_len = 0;
    dev_attr.ckg_bypass = 0xff;
    dev_attr.outstanding = 7;

    int ret = kd_mpi_dma_set_dev_attr(&dev_attr);
    if (ret != K_SUCCESS)
    {
        printf("set dma dev attr error\r\n");
        return ret;
    }

    ret = kd_mpi_dma_start_dev();
    if (ret != K_SUCCESS)
    {
        printf("start dev error\r\n");
        return ret;
    }

    return ret;
}

#define VICAP_MIN_PARAMETERS (7)

static void usage(void)
{
    printf("usage: ./sample_vicap -mode 0 -dev 0 -sensor 0 -chn 0 -chn 1 -ow 640 -oh 480 -preview 1 -rotation 1\n");
    printf("Options:\n");
    printf(" -mode:         vicap work mode[0: online mode, 1: offline mode. only offline mode support multiple sensor input]\tdefault 0\n");
    printf(" -conn:         vo connector device [0: hx8399, 1: lt9611-1920x1080p60, 2: lt9611-1920x1080p30]\tdefault 0\n");

    printf(" -itype:        vicap input type[0,1]\t0: sensor input, 1: user raw image input, default 0\n");
    printf(" -ifile:        the input raw image file, only used for user raw image input\n");
    printf(" -iw:           the input raw image width, only used for user raw image input\n");
    printf(" -ih:           the input image height, only used for user raw image input\n");
    printf(" -ipat:         the input raw image bayerpattern, only used for user raw image input\n");
    printf(" -icalf:        the input raw image calibration file, only used for user raw image input\n");

    printf(" -dev:          vicap device id[0,1,2]\tdefault 0\n");
    printf(" -dw:           enable dewarp[0,1]\tdefault 0\n");
    printf(" -sensor:       sensor type[0: ov9732@1280x720, 1: ov9286_ir@1280x720], 2: ov9286_speckle@1280x720]\n");
    printf(" -ae:           ae status[0: disable AE, 1: enable AE]\tdefault enable\n");
    printf(" -awb:          awb status[0: disable AWB, 1: enable AWb]\tdefault enable\n");
    printf(" -hdr:          hdr status[0: disable HDR, 1: enable HDR]\tdefault disable\n");

    printf(" -chn:          vicap output channel id[0,1,2]\tdefault 0\n");
    printf(" -ow:           the output image width, default same with input width\n");
    printf(" -oh:           the output image height, default same with input height\n");
    printf(" -ox:           the output crop image start position of x\n");
    printf(" -oy:           the output crop image start position of y\n");
    printf(" -crop_x:       the output crop image width\n");
    printf(" -crop_y:       the output crop image height\n");
    printf(" -fps:          frame-per-second, 0 if unused\n");
    printf(" -crop:         crop enable[0: disable, 1: enable]\n");
    printf(" -ofmt:         the output pixel format[0: yuv, 1: rgb888, 2: rgb888p, 3: raw], only channel 0 support raw data, default yuv\n");
    printf(" -preview:      the output preview enable[0: disable, 1: enable], only support 2 output channel preview\n");
    printf(" -rotation:     display rotaion[0: degree 0, 1: degree 90, 2: degree 180, 3: degree 270, 4: unsupport rotaion, 17: gdma-degree 90, 18: gdma-degree 180, 19: gdma-degree 270]\n");
    printf(" -dalign:       dump data align mode[0: lsb align, 1: msb align]\tdefault 0\n");

    printf(" -help:         print this help\n");

    exit(1);
}

extern k_u32 display_cnt;
extern k_u32 drop_cnt;
extern uint32_t hdr_buf_base_phy_addr;
extern void *hdr_buf_base_vir_addr;

// static void _set_mod_log(k_mod_id mod_id,k_s32  level)
// {
//     k_log_level_conf level_conf;
//     level_conf.level = level;
//     level_conf.mod_id = mod_id;
//     kd_mpi_log_set_level_conf(&level_conf);
// }

static void vb_exit() {
    kd_mpi_vb_exit();
}

static uint64_t get_ticks()
{
    static volatile uint64_t time_elapsed = 0;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

int main(int argc, char *argv[])
{
    // _set_mod_log(K_ID_VI, 6);
    k_s32 ret = 0;
    k_u8 salve_en = 0;
    k_u8 gdma_enable = 0;

    k_u32 work_mode = VICAP_WORK_ONLINE_MODE;
    k_connector_type connector_type = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;//HX8377_V2_MIPI_4LAN_1080X1920_30FPS;//ST7701_V1_MIPI_2LAN_480X854_30FPS;//ST7701_V1_MIPI_2LAN_480X800_30FPS;//HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
    k_connector_info connector_info;
    k_u32 display_width;
    k_u32 display_height;

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
        else if (strcmp(argv[i], "-mode") == 0)
        {
            if ((i + 1) >= argc) {
                printf("mode parameters missing.\n");
                return -1;
            }
            k_s32 mode = atoi(argv[i + 1]);
            if (mode == 0) {
                work_mode = VICAP_WORK_ONLINE_MODE;
            } else if (mode == 1) {
                work_mode = VICAP_WORK_OFFLINE_MODE;
            } else if (mode == 2) {
                work_mode = VICAP_WORK_SW_TILE_MODE;
            } else {
                printf("unsupport mode.\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-conn") == 0)
        {
            if ((i + 1) >= argc) {
                printf("conn parameters missing.\n");
                return -1;
            }
            k_s32 conn = atoi(argv[i + 1]);
            if (conn == 0) {
                connector_type = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
            } else if (conn == 1) {
                connector_type = LT9611_MIPI_4LAN_1920X1080_60FPS;
            } else if (conn == 2) {
                connector_type = LT9611_MIPI_4LAN_1920X1080_30FPS;
            }else if (conn == 3) {
                connector_type = ST7701_V1_MIPI_2LAN_480X800_30FPS;
            } else if(conn == 4) {
                connector_type = ILI9806_MIPI_2LAN_480X800_30FPS;
            } else {
                printf("unsupport connector deivce.\n");
                return -1;
            }
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
            if (cur_dev > VICAP_DEV_ID_MAX)
            {
                printf("unsupported vicap device, the valid device num is:0, 1, 2!\n");
                return -1;
            }
            dev_count++;
            printf("cur_dev(%d), dev_count(%d)\n", cur_dev, dev_count);

            if (dev_count > VICAP_DEV_ID_MAX) {
                printf("only support three vicap device!!!\n");
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
                        else if (strcmp(argv[i], "-crop_x") == 0)
                        {
                            k_u16 crop_w = atoi(argv[i + 1]);
                            device_obj[cur_dev].crop_win[cur_chn].width = crop_w;
                        }
                        else if (strcmp(argv[i], "-crop_y") == 0)
                        {
                            k_u16 crop_y = atoi(argv[i + 1]);
                            device_obj[cur_dev].crop_win[cur_chn].height = crop_y;
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
                            // out_height = VICAP_ALIGN_UP(out_height, 16);
                            device_obj[cur_dev].out_win[cur_chn].height = out_height;
                        }
                        else if (strcmp(argv[i], "-rotation") == 0)
                        {
                            k_u16 rotation = atoi(argv[i + 1]);
                            device_obj[cur_dev].rotation[cur_chn] = rotation;
                        }
                        else if (strcmp(argv[i], "-mirror") == 0)
                        {
                            k_vicap_mirror mirror;
                            mirror  = atoi(argv[i + 1]);
                            device_obj[cur_dev].sensor_mirror = mirror;

                        }
                        else if (strcmp(argv[i], "-fps") == 0)
                        {
                            k_u16 fps = atoi(argv[i + 1]);
                            device_obj[cur_dev].fps[cur_chn] = fps;
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
                        else if (strcmp(argv[i], "-dalign") == 0)
                        {
                            k_u16 dalign = atoi(argv[i + 1]);
                            if (dalign > 1) {
                                printf("invalid dalign paramters.\n");
                                usage();
                            }
                            device_obj[cur_dev].dalign = dalign;
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

                    device_obj[cur_dev].sensor_type = (k_vicap_sensor_type)sensor;                   
                    if(sensor >=SENSOR_TYPE_MAX)
                    {
                        printf("unsupport sensor type.\n");
                        return -1;
                    }
                }
                else if (strcmp(argv[i], "-itype") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("mode parameters missing.\n");
                        return -1;
                    }
                    k_s32 type = atoi(argv[i + 1]);
                    if (type == 0) {
                        device_obj[cur_dev].input_type = VICAP_INPUT_TYPE_SENSOR;
                    } else if (type == 1) {
                        device_obj[cur_dev].input_type = VICAP_INPUT_TYPE_IMAGE;
                    } else {
                        printf("unsupport type.\n");
                        return -1;
                    }

                    //parse itype parameters
                    for (i = i + 2; i < argc; i += 2)
                    {
                        if ((i + 1) >= argc) {
                            printf("chn parameters(%s) error.\n", argv[i]);
                            usage();
                        }
                        if (strcmp(argv[i], "-ifile") == 0)
                        {
                            device_obj[cur_dev].file_path = argv[i + 1];
                        }
                        else if (strcmp(argv[i], "-iw") == 0)
                        {
                            k_u16 in_width = atoi(argv[i + 1]);
                            device_obj[cur_dev].in_width = in_width;
                        }
                        else if (strcmp(argv[i], "-ih") == 0)
                        {
                            k_u16 in_height = atoi(argv[i + 1]);
                            device_obj[cur_dev].in_height = in_height;
                        }
                        else if (strcmp(argv[i], "-ipat") == 0)
                        {
                            if (strcmp(argv[i + 1], "RGGB") == 0) {
                                device_obj[cur_dev].pattern = BAYER_PAT_RGGB;
                            } else if (strcmp(argv[i + 1], "GRBG") == 0) {
                                device_obj[cur_dev].pattern = BAYER_PAT_GRBG;
                            } else if (strcmp(argv[i + 1], "GBRG") == 0) {
                                device_obj[cur_dev].pattern = BAYER_PAT_GBRG;
                            } else if (strcmp(argv[i + 1], "BGGR") == 0) {
                                device_obj[cur_dev].pattern = BAYER_PAT_BGGR;
                            } else {
                                printf("unspported pattern.\n");
                                return -1;
                            }
                        }
                        else if (strcmp(argv[i], "-icalf") == 0)
                        {
                            device_obj[cur_dev].calib_file = argv[i + 1];
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
                            printf("invalid type paramters.\n");
                            usage();
                        }
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
                    device_obj[cur_dev].dw_enable = atoi(argv[i + 1]);
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
    if ((dev_count > 1) && (work_mode == VICAP_WORK_ONLINE_MODE)) {
        printf("only offline mode support multiple sensor input!!!\n");
        return 0;
    }

    ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }
    display_width = connector_info.resolution.hdisplay;
    display_height = connector_info.resolution.vdisplay;
    display_width = VICAP_ALIGN_UP(display_width, 16);

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        if (device_obj[dev_num].input_type == VICAP_INPUT_TYPE_SENSOR) {
            dev_attr.input_type = VICAP_INPUT_TYPE_SENSOR;
            //vicap get sensor info
            ret = kd_mpi_vicap_get_sensor_info(device_obj[dev_num].sensor_type, &device_obj[dev_num].sensor_info);
            if (ret) {
                printf("sample_vicap, the sensor type not supported!\n");
                return ret;
            }
            memcpy(&dev_attr.sensor_info, &device_obj[dev_num].sensor_info, sizeof(k_vicap_sensor_info));

            device_obj[dev_num].in_width = device_obj[dev_num].sensor_info.width;
            device_obj[dev_num].in_height = device_obj[dev_num].sensor_info.height;
        } else {
            dev_attr.input_type = VICAP_INPUT_TYPE_IMAGE;
            work_mode = VICAP_WORK_LOAD_IMAGE_MODE;
            device_obj[dev_num].ae_enable = 0;
            device_obj[dev_num].awb_enable = 0;
        }

        printf("sample_vicap, dev[%d] in size[%dx%d]\n", \
            dev_num, device_obj[dev_num].in_width, device_obj[dev_num].in_height);

        //vicap device attr set
        dev_attr.acq_win.h_start = 0;
        dev_attr.acq_win.v_start = 0;
        dev_attr.acq_win.width = device_obj[dev_num].in_width;
        dev_attr.acq_win.height = device_obj[dev_num].in_height;
        if ((work_mode == VICAP_WORK_OFFLINE_MODE) || (work_mode == VICAP_WORK_LOAD_IMAGE_MODE) ||(work_mode == VICAP_WORK_SW_TILE_MODE)) {
            dev_attr.mode = work_mode;
            dev_attr.buffer_num = VICAP_INPUT_BUF_NUM;
            if(work_mode == VICAP_WORK_SW_TILE_MODE)
                dev_attr.buffer_size = VICAP_ALIGN_UP((device_obj[dev_num].in_width * device_obj[dev_num].in_height * 12 / 8), VICAP_ALIGN_1K);
            else
                dev_attr.buffer_size = VICAP_ALIGN_UP((device_obj[dev_num].in_width * device_obj[dev_num].in_height * 2), VICAP_ALIGN_1K);
            device_obj[dev_num].in_size = dev_attr.buffer_size;
            if(work_mode == VICAP_WORK_SW_TILE_MODE)
                device_obj[dev_num].mode = VICAP_WORK_SW_TILE_MODE;
            else
                device_obj[dev_num].mode = VICAP_WORK_OFFLINE_MODE;
                
            if (work_mode == VICAP_WORK_LOAD_IMAGE_MODE) {
                dev_attr.image_pat = device_obj[dev_num].pattern;
                dev_attr.sensor_info.sensor_name = device_obj[dev_num].calib_file;
                device_obj[dev_num].image_data = NULL;
            }
        } else {
            dev_attr.mode = VICAP_WORK_ONLINE_MODE;
        }

        dev_attr.pipe_ctrl.data = pipe_ctrl;
        dev_attr.pipe_ctrl.bits.af_enable = 0;
        dev_attr.pipe_ctrl.bits.ae_enable = device_obj[dev_num].ae_enable;
        dev_attr.pipe_ctrl.bits.awb_enable = device_obj[dev_num].awb_enable;

        if(work_mode == VICAP_WORK_SW_TILE_MODE)
            dev_attr.pipe_ctrl.bits.dnr3_enable = 1;
        else
            dev_attr.pipe_ctrl.bits.dnr3_enable = device_obj[dev_num].dnr3_enable;

        dev_attr.pipe_ctrl.bits.ahdr_enable = device_obj[dev_num].hdr_enable;

        dev_attr.cpature_frame = 0;
        dev_attr.dw_enable = device_obj[dev_num].dw_enable;

        dev_attr.mirror = device_obj[cur_dev].sensor_mirror;

        ret = kd_mpi_vicap_set_dev_attr(dev_num, dev_attr);
        if (ret) {
            printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
            return ret;
        }

        if (work_mode == VICAP_WORK_LOAD_IMAGE_MODE) {
            //open raw image file
            if (device_obj[dev_num].file_path) {
                printf("open raw image file:%s\n", device_obj[dev_num].file_path);
                k_u32 file_len = 0;
                FILE *raw_file = fopen(device_obj[dev_num].file_path, "rb");
                if (raw_file) {
                    fseek(raw_file, 0, SEEK_END);
                    file_len = ftell(raw_file);
                    device_obj[dev_num].image_data = malloc(file_len);
                    if (device_obj[dev_num].image_data != NULL) {
                        fseek(raw_file, 0, SEEK_SET);
                        if (file_len != fread(device_obj[dev_num].image_data, 1, file_len, raw_file)) {
                            printf("%s, read file failed(%s).\n", __func__, strerror(errno));
                        }
                        ret = kd_mpi_vicap_load_image(dev_num, device_obj[dev_num].image_data, file_len);
                        if (ret) {
                            printf("sample_vicap, kd_mpi_vicap_load_image failed.\n");
                            return ret;
                        }
                    } else {
                        printf("%s, malloc image data mem failed.\n", __func__);
                    }
                    fclose(raw_file);
                } else {
                    printf("%s, open raw image file(%s) failed(%s).\n", __func__, device_obj[dev_num].file_path, strerror(errno));
                    return -1;
                }
            } else {
                printf("please input raw image file!!!\n");
                return 0;
            }
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

            if ((device_obj[dev_num].out_win[chn_num].width > display_width)
                && (device_obj[dev_num].out_win[chn_num].height > display_height)) {
                device_obj[dev_num].preview[chn_num] = K_FALSE;
            }

            if (!device_obj[dev_num].rotation[chn_num]
                && ((device_obj[dev_num].out_win[chn_num].width > display_width)
                && (device_obj[dev_num].out_win[chn_num].width < display_height))) {
                device_obj[dev_num].rotation[chn_num] = 1;
            }

            printf("sample_vicap, dev_num(%d), chn_num(%d), in_size[%dx%d], out_offset[%d:%d], out_size[%dx%d]\n", \
                dev_num, chn_num, device_obj[dev_num].in_width, device_obj[dev_num].in_height, \
                device_obj[dev_num].out_win[chn_num].h_start, device_obj[dev_num].out_win[chn_num].v_start, \
                device_obj[dev_num].out_win[chn_num].width, device_obj[dev_num].out_win[chn_num].height);
        }
    }

    ret = sample_vicap_vo_init(connector_type);
    if (ret) {
        printf("sample_vicap_vo_init failed\n");
        return -1;
    }

    ret = sample_vicap_vb_init(device_obj);
    if (ret) {
        printf("sample_vicap_vb_init failed\n");
        return -1;
    }
    atexit(vb_exit);

    //vicap channel attr set
    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            kd_mpi_vicap_set_dump_reserved(dev_num, chn_num, K_TRUE);

            memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
            if (device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_RGB_BAYER_10BPP) {
                chn_attr.out_win.width = device_obj[dev_num].in_width;
                chn_attr.out_win.height = device_obj[dev_num].in_height;
            } else {
                chn_attr.out_win.width = device_obj[dev_num].out_win[chn_num].width;
                chn_attr.out_win.height = device_obj[dev_num].out_win[chn_num].height;
            }

            if (device_obj[dev_num].crop_enable[chn_num]) {
                chn_attr.crop_win.width = device_obj[dev_num].crop_win[chn_num].width;  //chn_attr.out_win;1166;// 
                chn_attr.crop_win.height = device_obj[dev_num].crop_win[chn_num].height; //1944;//
                chn_attr.crop_win.h_start =device_obj[dev_num].out_win[chn_num].h_start;  //713;
                chn_attr.crop_win.v_start =device_obj[dev_num].out_win[chn_num].v_start;  //0;//
            } else {
                chn_attr.crop_win.width = device_obj[dev_num].in_width;
                chn_attr.crop_win.height = device_obj[dev_num].in_height;
            }

            chn_attr.scale_win = chn_attr.out_win;
            chn_attr.crop_enable = device_obj[dev_num].crop_enable[chn_num];
            chn_attr.scale_enable = K_FALSE;
            chn_attr.chn_enable = K_TRUE;

            chn_attr.pix_format = device_obj[dev_num].out_format[chn_num];
            chn_attr.buffer_num = VICAP_OUTPUT_BUF_NUM;
            chn_attr.buffer_size = device_obj[dev_num].buf_size[chn_num];
            chn_attr.fps = device_obj[dev_num].fps[chn_num];

            printf("sample_vicap, set dev(%d) chn(%d) attr, buffer_size(%d), out size[%dx%d]\n", \
                dev_num, chn_num, chn_attr.buffer_size, chn_attr.out_win.width, chn_attr.out_win.height);

            printf("sample_vicap out_win h_start is %d ,v_start is %d \n", chn_attr.out_win.h_start, chn_attr.out_win.v_start);

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
                k_u16 gdma_rotation = 0;
                k_s32 gdma_chn;
                rotation = device_obj[dev_num].rotation[chn_num];
                if (rotation > 16) {
                    gdma_rotation = rotation - 16;
                    rotation = 4;
                    if (gdma_enable == 0) {
                        gdma_enable = 1;
                        dma_dev_attr_init();
                    }
                }
                if (vo_count == 0) {
                    vo_chn = K_VO_DISPLAY_CHN_ID1;
                    layer = K_VO_LAYER1;
                    gdma_chn = 0;
                } else if (vo_count == 1) {
                    vo_chn = K_VO_DISPLAY_CHN_ID2;
                    layer = K_VO_LAYER2;
                    rotation = 4;//layer2 unsupport roation
                    gdma_chn = 1;
                } else if (vo_count == 2) {
                    vo_chn = K_VO_DISPLAY_CHN_ID3;
                    layer = K_VO_OSD0;
                    rotation = 4;//layer2 unsupport roation
                    gdma_chn = 2;
                } else if (vo_count >= MAX_VO_LAYER_NUM){
                    printf("only support bind two vo channel.\n");
                    continue;
                }
                layer_conf.enable[vo_count] = K_TRUE;
                layer_conf.width[vo_count] = chn_attr.out_win.width;
                layer_conf.height[vo_count] = chn_attr.out_win.height;
                layer_conf.rotation[vo_count] = rotation;
                layer_conf.layer[vo_count] = layer;
                printf("sample_vicap, vo_count(%d), dev(%d) chn(%d) bind vo chn(%d) layer(%d) rotation(%d)\n", vo_count, dev_num, chn_num, vo_chn, layer, rotation);
                if (gdma_rotation) {
                    k_dma_chn_attr_u gdma_attr;
                    k_mpp_chn src_chn;
                    k_mpp_chn dst_chn;
                    memset(&gdma_attr, 0, sizeof(gdma_attr));
                    gdma_attr.gdma_attr.buffer_num = GDMA_BUF_NUM;
                    gdma_attr.gdma_attr.rotation = gdma_rotation;
                    gdma_attr.gdma_attr.x_mirror = K_FALSE;
                    gdma_attr.gdma_attr.y_mirror = K_FALSE;
                    gdma_attr.gdma_attr.width = chn_attr.out_win.width;
                    gdma_attr.gdma_attr.height = chn_attr.out_win.height;
                    gdma_attr.gdma_attr.work_mode = DMA_BIND;
                    gdma_attr.gdma_attr.src_stride[0] = chn_attr.out_win.width;
                    if (gdma_rotation == DEGREE_180) {
                        gdma_attr.gdma_attr.dst_stride[0] = chn_attr.out_win.width;
                    } else {
                        gdma_attr.gdma_attr.dst_stride[0] = chn_attr.out_win.height;
                        layer_conf.width[vo_count] = chn_attr.out_win.height;
                        layer_conf.height[vo_count] = chn_attr.out_win.width;
                    }
                    if (chn_attr.pix_format == PIXEL_FORMAT_RGB_888) {
                        gdma_attr.gdma_attr.pixel_format = DMA_PIXEL_FORMAT_RGB_888;
                        gdma_attr.gdma_attr.src_stride[0] *= 3;
                        gdma_attr.gdma_attr.dst_stride[0] *= 3;
                    } else {
                        gdma_attr.gdma_attr.pixel_format = DMA_PIXEL_FORMAT_YUV_SEMIPLANAR_420_8BIT;
                        gdma_attr.gdma_attr.src_stride[1] = gdma_attr.gdma_attr.src_stride[0];
                        gdma_attr.gdma_attr.dst_stride[1] = gdma_attr.gdma_attr.dst_stride[0];
                    }
                    src_chn.mod_id = K_ID_VI;
                    src_chn.dev_id = dev_num;
                    src_chn.chn_id = chn_num;
                    dst_chn.mod_id = K_ID_DMA;
                    dst_chn.dev_id = 0;
                    dst_chn.chn_id = gdma_chn;
                    ret = kd_mpi_sys_bind(&src_chn, &dst_chn);
                    if (ret) {
                        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
                    }

                    src_chn.mod_id = K_ID_DMA;
                    src_chn.dev_id = 0;
                    src_chn.chn_id = gdma_chn;
                    dst_chn.mod_id = K_ID_VO;
                    dst_chn.dev_id = K_VO_DISPLAY_DEV_ID;
                    dst_chn.chn_id = vo_chn;
                    ret = kd_mpi_sys_bind(&src_chn, &dst_chn);
                    if (ret) {
                        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
                    }

                    ret = kd_mpi_dma_set_chn_attr(gdma_chn, &gdma_attr);
                    if (ret != K_SUCCESS) {
                        printf("set chn attr error\r\n");
                    }
                    ret = kd_mpi_dma_start_chn(gdma_chn);
                    if (ret != K_SUCCESS) {
                        printf("start chn error\r\n");
                    }
                } else {
                    sample_vicap_bind_vo(dev_num, chn_num, vo_chn);
                }
                vo_count++;
            }
        }
    }

    ret = sample_vicap_vo_layer_init(&layer_conf, display_width, display_height);
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
    }

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        printf("sample_vicap, vicap dev(%d) start stream\n", dev_num);
        ret = kd_mpi_vicap_start_stream(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) start stream failed.\n", dev_num);
            goto app_exit;
        }
    }

    sample_vicap_vo_enable();

    if(salve_en == 1)
    {
        k_vicap_slave_info slave_info;

        memset(&slave_info, 0 , sizeof(k_vicap_slave_info));
        slave_info.vs_high = 10;            // 1 / 27 = 37ns
        slave_info.vs_cycle = 900902;       // 33ms / 37ns = 900902
        kd_mpi_vicap_set_slave_attr(VICAP_SLAVE_ID1, &slave_info);

        k_vicap_slave_enable slave_en;

        memset(&slave_en, 0 , sizeof(k_vicap_slave_enable));
        slave_en.vs_enable = 1;
        slave_en.hs_enable = 0;
        kd_mpi_vicap_set_slave_enable(VICAP_SLAVE_ID1, &slave_en);
    }

    k_isp_ae_roi ae_roi;
    memset(&ae_roi, 0, sizeof(k_isp_ae_roi));
    static k_u32 dump_count = 0;

    k_char select = 0;
    while(K_TRUE)
    {
        if(select != '\n')
        {
            printf("---------------------------------------\n");
            printf(" Input character to select test option\n");
            printf("---------------------------------------\n");
            printf(" d: dump data addr test\n");
            printf(" h: dump hdr ddr buffer.\n");
            printf(" s: set isp ae roi test\n");
            printf(" g: get isp ae roi test\n");
            printf(" t: toggle TPG\n");
            printf(" r: dump register config to file.\n");
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
                    uint64_t start = get_ticks();
                    ret = kd_mpi_vicap_dump_frame(dev_num, chn_num, VICAP_DUMP_YUV, &dump_info, 1000);
                    if (ret) {
                        printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                        continue;
                    }
                    uint64_t end = get_ticks();
                    printf("dump cost %lu us\n", (end - start) / 27);

                    k_char *suffix;
                    k_u32 data_size = 0;
                    k_u8 lbit = 0;
                    k_u8 *virt_addr = NULL;
                    k_char filename[256];

                    if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_YUV_SEMIPLANAR_420) {
                        suffix = "yuv420sp";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3 /2;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_YUV_SEMIPLANAR_422) {
                        suffix = "yuv422sp";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3 /2;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_888) {
                        suffix = "rgb888";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3;
                    }  else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_888_PLANAR) {
                        suffix = "rgb888p";
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_BAYER_10BPP) {
                        suffix = "raw10";
                        lbit = 6;
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 2;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_BAYER_12BPP) {
                        suffix = "raw12";
                        lbit = 4;
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 2;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_BAYER_14BPP) {
                        suffix = "raw14";
                        lbit = 2;
                        data_size = dump_info.v_frame.width * dump_info.v_frame.height * 2;
                    } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_BAYER_16BPP) {
                        suffix = "raw16";
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
                            if (device_obj[dev_num].dalign && lbit) {
                                for (k_u32 index = 0; index < data_size; index += 2) {
                                    k_u16 raw_data = (virt_addr[index + 1] << 8 ) | virt_addr[index];
                                    raw_data = raw_data << lbit;
                                    fwrite(&raw_data, sizeof(raw_data), 1, file);
                                }
                            } else {
                                fwrite(virt_addr, 1, data_size, file);
                            }
                            fclose(file);
                        } else {
                            printf("sample_vicap, open dump file failed(%s)\n", strerror(errno));
                        }

                        kd_mpi_sys_munmap(virt_addr, data_size);
                    } else {
                        printf("sample_vicap, map dump addr failed.\n");
                    }

                    printf("sample_vicap, release dev(%d) chn(%d) dump frame.\n", dev_num, chn_num);

                    ret = kd_mpi_vicap_dump_release(dev_num, chn_num, &dump_info);
                    if (ret) {
                        printf("sample_vicap, dev(%d) chn(%d) release dump frame failed.\n", dev_num, chn_num);
                    }
                    dump_count++;
                }
            }
            break;
        case 'h':
            for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
                if (!device_obj[dev_num].dev_enable)
                    continue;

                k_u32 hdr_frame = 0;
                k_u32 data_size = 0;
                k_char filename[256];

                data_size = device_obj[dev_num].in_width * device_obj[dev_num].in_height * 3/2;
                if ((device_obj[dev_num].sensor_info.hdr_mode == VICAP_VCID_HDR_2FRAME)
                  || (device_obj[dev_num].sensor_info.hdr_mode == VICAP_VCID_HDR_2FRAME)) {
                    hdr_frame = 2;
                } else if ((device_obj[dev_num].sensor_info.hdr_mode == VICAP_VCID_HDR_3FRAME)
                  || (device_obj[dev_num].sensor_info.hdr_mode == VICAP_VCID_HDR_3FRAME)){
                    hdr_frame = 3;
                } else {
                    hdr_frame = 0;
                }

                for (int index = 0; index < hdr_frame; index++) {
                    void *hdr_buf_addr = hdr_buf_base_vir_addr + data_size * index;
                    printf("sample_vicap, dump hdr buf(%p) data size:%u\n", hdr_buf_addr, data_size);
                    if (hdr_buf_addr) {
                        memset(filename, 0 , sizeof(filename));
                        snprintf(filename, sizeof(filename), "dev_%02d_%dx%d_%dhdr_%02d_%04d.raw", \
                            dev_num, device_obj[dev_num].in_width, device_obj[dev_num].in_height, hdr_frame, index, dump_count);
                        FILE *file = fopen(filename, "wb");
                        if (file) {
                            printf("save dump hdr buffer to file(%s)\n", filename);
                            fwrite(hdr_buf_addr, data_size, 1, file);
                            fclose(file);
                        } else {
                            printf("sample_vicap, open dump hdr file failed(%s)\n", strerror(errno));
                        }
                    } else {
                        printf("sample_vicap, map dump hdr addr failed.\n");
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
        case 't': {
            static k_bool tpg_enable = 0;
            // toggle
            tpg_enable ^= 1;
            kd_mpi_vicap_tpg_enable(tpg_enable);
            break;
        }
        case 'r':
            printf("sample_vicap... save isp register config.\n");
            for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
                if (!device_obj[dev_num].dev_enable)
                    continue;

                k_char filename[256];
                memset(filename, 0 , sizeof(filename));
                snprintf(filename, sizeof(filename), "dev_%02d_reg_dump.txt", dev_num);
                printf("save dump register to file(%s)\n", filename);
                kd_mpi_vicap_dump_register(dev_num, filename);
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

    if(salve_en == 1)
    {
        k_vicap_slave_enable slave_en;

        memset(&slave_en, 0 , sizeof(k_vicap_slave_enable));
        slave_en.vs_enable = 0;
        slave_en.hs_enable = 0;
        kd_mpi_vicap_set_slave_enable(VICAP_SLAVE_ID1, &slave_en);
    }

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        printf("sample_vicap, vicap dev(%d) stop stream\n", dev_num);
        ret = kd_mpi_vicap_stop_stream(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) stop stream failed.\n", dev_num);
        }
        printf("display_cnt[%d], drop_cnt[%d]\n", display_cnt, drop_cnt);

        printf("sample_vicap, vicap dev(%d) deinit\n", dev_num);
        ret = kd_mpi_vicap_deinit(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) deinit failed.\n", dev_num);
        }

        if (work_mode == VICAP_WORK_LOAD_IMAGE_MODE) {
            if (device_obj[dev_num].image_data != NULL) {
                free(device_obj[dev_num].image_data);
                device_obj[dev_num].image_data = NULL;
            }
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
                k_u16 gdma_rotation = 0;
                k_s32 gdma_chn;
                gdma_rotation = device_obj[dev_num].rotation[chn_num] > 16 ? 1 : 0;
                if (vo_count == 0) {
                    vo_chn = K_VO_DISPLAY_CHN_ID1;
                    layer = K_VO_LAYER1;
                    gdma_chn = 0;
                } else if (vo_count == 1) {
                    vo_chn = K_VO_DISPLAY_CHN_ID2;
                    layer = K_VO_LAYER2;
                    gdma_chn = 1;
                } 
                else if (vo_count == 2) {
                    vo_chn = K_VO_DISPLAY_CHN_ID3;
                    layer = K_VO_OSD0;
                    gdma_chn = 2;
                }else {
                    printf("only support unbind two vo chn.\n");
                    continue;
                }
                if(vo_count < 2)
                {
                    sample_vicap_disable_vo_layer(layer);
                    printf("sample_vicap, vo_count(%d), dev(%d) chn(%d) unbind vo chn(%d) layer(%d)\n", vo_count, dev_num, chn_num, vo_chn, layer);
                }
                else
                {
                    sample_vicap_disable_vo_osd(layer);
                }
                if (gdma_rotation) {
                    k_mpp_chn src_chn;
                    k_mpp_chn dst_chn;
                    ret = kd_mpi_dma_stop_chn(gdma_chn);
                    if (ret != K_SUCCESS) {
                        printf("stop chn error\r\n");
                    }
                    src_chn.mod_id = K_ID_VI;
                    src_chn.dev_id = dev_num;
                    src_chn.chn_id = chn_num;
                    dst_chn.mod_id = K_ID_DMA;
                    dst_chn.dev_id = 0;
                    dst_chn.chn_id = gdma_chn;
                    ret = kd_mpi_sys_unbind(&src_chn, &dst_chn);
                    if (ret) {
                        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
                    }

                    src_chn.mod_id = K_ID_DMA;
                    src_chn.dev_id = 0;
                    src_chn.chn_id = gdma_chn;
                    dst_chn.mod_id = K_ID_VO;
                    dst_chn.dev_id = K_VO_DISPLAY_DEV_ID;
                    dst_chn.chn_id = vo_chn;
                    ret = kd_mpi_sys_unbind(&src_chn, &dst_chn);
                    if (ret) {
                        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
                    }
                } else {
                    sample_vicap_unbind_vo(dev_num, chn_num, vo_chn);
                }
                vo_count++;
            }
        }
    }
    if (gdma_enable) {
        ret = kd_mpi_dma_stop_dev();
        if (ret != K_SUCCESS) {
            printf("stop dev error\r\n");
        }
    }

    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

vb_exit:

    return ret;
}
