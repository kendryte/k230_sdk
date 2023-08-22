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
#include "vo_cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_vo_api.h"
#include "mapi_sys_api.h"
#include "k_video_comm.h"

#include "k_vo_comm.h"

#define ENABLE_VO_LAYER   1

#define TXPHY_445_5_M                                           (295)                       // ok
#define TXPHY_445_5_N                                           (15)
#define TXPHY_445_5_VOC                                         (0x17)
#define TXPHY_445_5_HS_FREQ                                     (0x96)

#define VO_WIDTH                                                1080
#define VO_HEIGHT                                               1920
#define VO_MAX_FRAME_COUNT                                      5

#define PRIVATE_POLL_SZE                                        (1920 * 1080 * 3 / 2)
#define PRIVATE_POLL_NUM                                        (4)

typedef struct
{
    k_u64 osd_phy_addr;
    void *osd_virt_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
} osd_info;

typedef struct
{
    k_u64 layer_phy_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
    //only layer0、layer1
    k_u32 func;
    // only layer0
    k_vo_scaler_attr attr;
} layer_info;


k_vo_display_resolution hx8399[20] =
{
    // {74250, 445500, 1240, 1080, 20, 20, 120, 1988, 1920, 5, 8, 55},
    {37125, 222750, 1240, 1080, 20, 20, 120, 1988, 1920, 5, 8, 55},
};


static void hx8399_v2_init(k_u8 test_mode_en)
{
    k_u8 param1[] = {0xB9, 0xFF, 0x83, 0x99};
    k_u8 param21[] = {0xD2, 0xAA};
    k_u8 param2[] = {0xB1, 0x02, 0x04, 0x71, 0x91, 0x01, 0x32, 0x33, 0x11, 0x11, 0xab, 0x4d, 0x56, 0x73, 0x02, 0x02};
    k_u8 param3[] = {0xB2, 0x00, 0x80, 0x80, 0xae, 0x05, 0x07, 0x5a, 0x11, 0x00, 0x00, 0x10, 0x1e, 0x70, 0x03, 0xd4};
    k_u8 param4[] = {0xB4, 0x00, 0xFF, 0x02, 0xC0, 0x02, 0xc0, 0x00, 0x00, 0x08, 0x00, 0x04, 0x06, 0x00, 0x32, 0x04, 0x0a, 0x08, 0x21, 0x03, 0x01, 0x00, 0x0f, 0xb8, 0x8b, 0x02, 0xc0, 0x02, 0xc0, 0x00, 0x00, 0x08, 0x00, 0x04, 0x06, 0x00, 0x32, 0x04, 0x0a, 0x08, 0x01, 0x00, 0x0f, 0xb8, 0x01};
    k_u8 param5[] = {0xD3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x10, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x05, 0x05, 0x07, 0x00, 0x00, 0x00, 0x05, 0x40};
    k_u8 param6[] = {0xD5, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x21, 0x20, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x2f, 0x2f, 0x30, 0x30, 0x31, 0x31, 0x18, 0x18, 0x18, 0x18};
    k_u8 param7[] = {0xD6, 0x18, 0x18, 0x19, 0x19, 0x40, 0x40, 0x20, 0x21, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x2f, 0x2f, 0x30, 0x30, 0x31, 0x31, 0x40, 0x40, 0x40, 0x40};
    k_u8 param8[] = {0xD8, 0xa2, 0xaa, 0x02, 0xa0, 0xa2, 0xa8, 0x02, 0xa0, 0xb0, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00};
    k_u8 param9[] = {0xBD, 0x01};
    k_u8 param10[] = {0xD8, 0xB0, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x00, 0x00, 0xE2, 0xAA, 0x03, 0xF0, 0xE2, 0xAA, 0x03, 0xF0};
    k_u8 param11[] = {0xBD, 0x02};
    k_u8 param12[] = {0xD8, 0xE2, 0xAA, 0x03, 0xF0, 0xE2, 0xAA, 0x03, 0xF0};
    k_u8 param13[] = {0xBD, 0x00};
    k_u8 param14[] = {0xB6, 0x8D, 0x8D};
    k_u8 param15[] = {0xCC, 0x09};
    k_u8 param16[] = {0xC6, 0xFF, 0xF9};
    k_u8 param22[] = {0xE0, 0x00, 0x12, 0x1f, 0x1a, 0x40, 0x4a, 0x59, 0x55, 0x5e, 0x67, 0x6f, 0x75, 0x7a, 0x82, 0x8b, 0x90, 0x95, 0x9f, 0xa3, 0xad, 0xa2, 0xb2, 0xB6, 0x5e, 0x5a, 0x65, 0x77, 0x00, 0x12, 0x1f, 0x1a, 0x40, 0x4a, 0x59, 0x55, 0x5e, 0x67, 0x6f, 0x75, 0x7a, 0x82, 0x8b, 0x90, 0x95, 0x9f, 0xa3, 0xad, 0xa2, 0xb2, 0xB6, 0x5e, 0x5a, 0x65, 0x77};
    k_u8 param23[] = {0x11};
    k_u8 param24[] = {0x29};
    k_u8 pag20[50] = {0xB2, 0x0b, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};               // 蓝色

    kd_mapi_dsi_send_cmd(param1, sizeof(param1));
    kd_mapi_dsi_send_cmd(param21, sizeof(param21));
    kd_mapi_dsi_send_cmd(param2, sizeof(param2));
    kd_mapi_dsi_send_cmd(param3, sizeof(param3));
    kd_mapi_dsi_send_cmd(param4, sizeof(param4));
    kd_mapi_dsi_send_cmd(param5, sizeof(param5));
    kd_mapi_dsi_send_cmd(param6, sizeof(param6));
    kd_mapi_dsi_send_cmd(param7, sizeof(param7));
    kd_mapi_dsi_send_cmd(param8, sizeof(param8));
    kd_mapi_dsi_send_cmd(param9, sizeof(param9));

    if (test_mode_en == 1)
    {
        kd_mapi_dsi_send_cmd(pag20, 10);                   // test  mode
    }

    kd_mapi_dsi_send_cmd(param10, sizeof(param10));
    kd_mapi_dsi_send_cmd(param11, sizeof(param11));
    kd_mapi_dsi_send_cmd(param12, sizeof(param12));
    kd_mapi_dsi_send_cmd(param13, sizeof(param13));
    kd_mapi_dsi_send_cmd(param14, sizeof(param14));
    kd_mapi_dsi_send_cmd(param15, sizeof(param15));
    kd_mapi_dsi_send_cmd(param16, sizeof(param16));
    kd_mapi_dsi_send_cmd(param22, sizeof(param22));
    kd_mapi_dsi_send_cmd(param23, 1);
    usleep(300000);
    kd_mapi_dsi_send_cmd(param24, 1);
    usleep(100000);
}

static void sample_dwc_dsi_init(int flag)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;

    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));
    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = TXPHY_445_5_M;
    phy_attr.n = TXPHY_445_5_N;
    phy_attr.voc = TXPHY_445_5_VOC;
    phy_attr.hs_freq = TXPHY_445_5_HS_FREQ;
    kd_mapi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mapi_dsi_set_attr(&attr);

    // config scann
    if(flag == 1)
        hx8399_v2_init(1);
    else
        hx8399_v2_init(0);

    // enable dsi
    kd_mapi_dsi_enable(enable);

    if(flag == 2)
        kd_mapi_dsi_set_test_pattern();
}

int sample_vo_creat_layer(k_vo_layer chn_id, layer_info *info)
{
    k_vo_video_layer_attr attr;

    // check layer
    if ((chn_id >= K_MAX_VO_LAYER_NUM) || ((info->func & K_VO_SCALER_ENABLE) && (chn_id != K_VO_LAYER0))
            || ((info->func != 0) && (chn_id == K_VO_LAYER2)))
    {
        printf("input layer num failed \n");
        return -1 ;
    }

    // check scaler

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
    kd_mapi_vo_set_video_layer_attr(chn_id, &attr);
    // enable layer
    kd_mapi_vo_enable_video_layer(chn_id);

    return 0;
}

k_u32 vo_creat_osd_test(k_vo_osd osd, osd_info *info)
{
    k_vo_video_osd_attr attr;

    // set attr
    attr.global_alptha = info->global_alptha;

    if (info->format == PIXEL_FORMAT_ABGR_8888 || info->format == PIXEL_FORMAT_ARGB_8888)
    {
        info->size = info->act_size.width * info->act_size.height * 4;
        info->stride = info->act_size.width * 4 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_565 || info->format == PIXEL_FORMAT_BGR_565)
    {
        info->size = info->act_size.width * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_888 || info->format == PIXEL_FORMAT_BGR_888)
    {
        info->size = info->act_size.width * info->act_size.height * 3;
        info->stride = info->act_size.width * 3 / 8;
    }
    else if (info->format == PIXEL_FORMAT_ARGB_4444 || info->format == PIXEL_FORMAT_ABGR_4444)
    {
        info->size = info->act_size.width * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else if (info->format == PIXEL_FORMAT_ARGB_1555 || info->format == PIXEL_FORMAT_ABGR_1555)
    {
        info->size = info->act_size.width * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else
    {
        printf("set osd pixel format failed  \n");
    }

    attr.stride = info->stride;
    attr.pixel_format = info->format;
    attr.display_rect = info->offset;
    attr.img_size = info->act_size;
    kd_mapi_vo_set_video_osd_attr(osd, &attr);

    // 打开osd层
    kd_mapi_vo_osd_enable(osd);

    return 0;
}

static k_s32 display_hardware_init()
{
    kd_mapi_vo_reset();
    kd_mapi_set_backlight();
    return 0;
}

void vo_layer_init(k_u32 width,k_u32 height)
{
    display_hardware_init();
    sample_dwc_dsi_init(0);
    usleep(100*1000);

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;
    layer_info info;

    void *pic_vaddr = NULL;
    k_vo_layer chn_id = ENABLE_VO_LAYER;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0x808000;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mapi_vo_init();

    // set vo timing
    kd_mapi_vo_set_dev_param(&attr);



#if 0
    // config lyaer
    if (width > 1080)
    {
        info.act_size.width = height;
        info.act_size.height = width;
        info.func = K_ROTATION_90;
    }
    else
    {
        info.act_size.width = width;
        info.act_size.height = height;
        info.func = 0;
    }
#else
    //info.act_size.width = 1080;
    //info.act_size.height = 1920;
    info.act_size.width = width;
    info.act_size.height = height;
    info.func = 0;

#endif

    info.format = PIXEL_FORMAT_YVU_PLANAR_420;

    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    sample_vo_creat_layer(chn_id, &info);

    // enable vo
    kd_mapi_vo_enable();
}



void vo_layer_deinit()
{
    kd_mapi_vo_disable_video_layer(ENABLE_VO_LAYER);
}