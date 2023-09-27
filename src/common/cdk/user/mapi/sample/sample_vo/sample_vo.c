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
#include "mapi_vo_api.h"
#include "mapi_sys_api.h"

#include "k_vo_comm.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#define TXPHY_445_5_M                                           (295)                       // ok
#define TXPHY_445_5_N                                           (15)
#define TXPHY_445_5_VOC                                         (0x17)
#define TXPHY_445_5_HS_FREQ                                     (0x96)

#define VO_WIDTH                                                1080
#define VO_HEIGHT                                               1920 
#define VO_MAX_FRAME_COUNT                                      5

#define PRIVATE_POLL_SZE                                        (1920 * 1080 * 3 / 2)
#define PRIVATE_POLL_NUM                                        (4)


typedef enum
{
    DISPLAY_DSI_LP_MODE_TEST,
    DISPLAY_DSI_TEST_PATTERN,
    DISPALY_VO_BACKGROUND_TEST,
    DISPALY_VO_WRITEBACK_TEST,
    DISPALY_VO_OSD0_TEST,
    DISPALY_VO_INSERT_MULTI_FRAME_OSD0_TEST,
    DISPALY_VO_LAYER_INSERT_FRAME_TEST,
    DISPALY_VVI_BING_VO_LAYER_TEST,
    DISPALY_VVI_BING_VO_OSD_TEST,
    DISPALY_VVI_BING_VO_OSD_DUMP_FRAME_TEST,
    DISPALY_VO_1LAN_CASE_TEST,
    DISPALY_VO_DSI_READ_ID,
    DISPALY_VO_CONNECTOR_TEST,
    // DISPALY_VO_LAYER_FUNCTION_TEST,
} display_test_case;

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


static k_s32 mmap_fd = -1;
static k_s32 g_pool_id;

k_vo_display_resolution hx8399[20] =
{
    {74250, 445500, 1240, 1080, 20, 20, 120, 1988, 1920, 5, 8, 55},
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


void sample_dwc_dsi_init(int flag)
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

k_u32 sample_vo_creat_osd(k_vo_osd osd, layer_info *info)
{
    k_vo_video_osd_attr attr;

    // set attr
    attr.global_alptha = info->global_alptha;

    if (info->format == PIXEL_FORMAT_ABGR_8888 || info->format == PIXEL_FORMAT_ARGB_8888)
    {
        info->size = info->act_size.width  * info->act_size.height * 4;
        info->stride = info->act_size.width * 4 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_565 || info->format == PIXEL_FORMAT_BGR_565)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_888 || info->format == PIXEL_FORMAT_BGR_888)
    {
        info->size = info->act_size.width  * info->act_size.height * 3;
        info->stride = info->act_size.width * 3 / 8;
    }
    else if(info->format == PIXEL_FORMAT_ARGB_4444 || info->format == PIXEL_FORMAT_ABGR_4444)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else if(info->format == PIXEL_FORMAT_ARGB_1555 || info->format == PIXEL_FORMAT_ABGR_1555)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else if(info->format == PIXEL_FORMAT_ARGB_1555)
    {
        info->size = info->act_size.width * info->act_size.height;
        info->stride = info->act_size.width / 8;
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
    kd_mapi_vo_osd_enable(osd);

    return 0;
}


void display_hardware_init()
{
    kd_mapi_vo_reset();
    kd_mapi_set_backlight();
}


static k_s32 sample_vo_get_chn_size(k_video_frame_info *vf_info)
{
    k_s32 size;

    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_8888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_8888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_565 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_565)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_4444 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_4444)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_1555 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_1555)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3 / 2;
    else if(vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_MONOCHROME_8BPP)
        size = vf_info->v_frame.height * vf_info->v_frame.width;
    
    return size;
}


void sample_vo_filling_color(layer_info *osd, void *pic_vaddr)
{
    int i = 0;
    k_u32 *temp_addr = (k_u32 *)pic_vaddr;

    if (osd->format == PIXEL_FORMAT_ABGR_8888)
    {
        for (i = 0; i < (osd->size / sizeof(k_u32)) ; i++)
        {
            temp_addr[i] = 0x00FF00ff;
        }
    }
    else if (osd->format == PIXEL_FORMAT_ARGB_8888)
    {
        for (i = 0; i < osd->size / sizeof(k_u32) ; i++)
        {
            temp_addr[i] = 0x00FF00ff;//COLOR_ARGB_RED;
        }
    }
    else if (osd->format == PIXEL_FORMAT_RGB_565)
    {
        for (i = 0; i < osd->size / sizeof(k_u32); i++)
        {
            temp_addr[i] = 0xFF0000FF;
        }
    }
    else if (osd->format == PIXEL_FORMAT_BGR_565)
    {
        for (i = 0; i < osd->size / sizeof(k_u32); i++)
        {
            temp_addr[i] = 0xFF0000FF;
        }
    }
    else if (osd->format == PIXEL_FORMAT_RGB_888)
    {
        for (i = 0; i < osd->size / sizeof(k_u32); i++)
        {
            temp_addr[i] = 0xFF0000FF;
        }
    }
    else if (osd->format == PIXEL_FORMAT_BGR_888)
    {
        for (i = 0; i < osd->size / sizeof(k_u32); i++)
        {
            temp_addr[i] = 0xFF0000FF;
        }
    }
}

static k_mapi_media_attr_t media_attr = {0};

static k_s32 sample_vb_init(void)
{
    k_vb_pool_config pool_config;
    k_s32 pool_id;
    k_u32 ret = 0;

    media_attr.media_config.vb_config.max_pool_cnt = 10;
    media_attr.media_config.vb_config.comm_pool[0].blk_cnt = VO_MAX_FRAME_COUNT;//5;
    media_attr.media_config.vb_config.comm_pool[0].blk_size = VO_WIDTH * VO_HEIGHT * 3;
    media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;

    media_attr.media_config.vb_config.comm_pool[1].blk_cnt = VO_MAX_FRAME_COUNT;//5;
    media_attr.media_config.vb_config.comm_pool[1].blk_size = VO_WIDTH * VO_HEIGHT * 3;
    media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }

    // memset(&pool_config, 0, sizeof(pool_config));
    // pool_config.blk_cnt = PRIVATE_POLL_NUM;
    // pool_config.blk_size = PRIVATE_POLL_SZE;
    // pool_config.mode = VB_REMAP_MODE_NOCACHE;
    // pool_id = kd_mapi_vb_create_pool(&pool_config);

    // g_pool_id = pool_id;

    if(mmap_fd == -1)
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
}

k_u64 phys_addr = 0;
k_s32 size = 0;

k_s32 samble_vo_insert_frame(k_video_frame_info *vf_info, void **pic_vaddr)
{
    k_s32 pool_id = 0;
    void *mmap_addr = NULL;
    k_u32 *virt_addr;

    size = sample_vo_get_chn_size(vf_info);

    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u32 mmap_size = (((size) + (page_size) - 1) & ~((page_size) - 1));
    k_u64 page_mask = (page_size - 1);
    kd_mapi_sys_get_vb_block(&pool_id, &phys_addr, size, NULL);
    
    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, phys_addr & ~page_mask);
    if(mmap_addr)
        virt_addr = mmap_addr + (phys_addr & page_mask);

    if(virt_addr == NULL) {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    vf_info->mod_id = K_ID_VO;
    vf_info->pool_id = pool_id;//g_pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
    *pic_vaddr = virt_addr;

    printf("phys_addr is %lx \n", phys_addr);

    return 0;
}


k_s32 samble_vo_release_vb(void)
{
    // kd_mapi_vb_destory_pool(g_pool_id);
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

    kd_mapi_sys_release_vb_block(phys_addr, size);

    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
}


static void sample_vo_layer_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;
    layer_info info;

    void *pic_vaddr = NULL;
    k_vo_layer chn_id = K_VO_LAYER1;//K_VO_LAYER1;//K_VO_LAYER2;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mapi_vo_init();

    // set vo timing
    kd_mapi_vo_set_dev_param(&attr);

    sample_vb_init();

    // config lyaer
    info.act_size.width = 640;
    info.act_size.height = 480;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = 0;
    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;
    sample_vo_creat_layer(chn_id, &info);

    // enable vo
    kd_mapi_vo_enable();

    printf("Press Enter to start install frame\n");
    getchar();
    // set frame
    vf_info.v_frame.width = info.act_size.width;
    vf_info.v_frame.height = info.act_size.height;
    vf_info.v_frame.stride[0] = info.act_size.width;
    vf_info.v_frame.pixel_format = info.format;
    samble_vo_insert_frame(&vf_info, &pic_vaddr);

#if USE_PICTURE_TEST
    void *read_addr = NULL;
    FILE *fd;
    int ret = 0;
    k_u32 read_size = info.size;

    read_addr = malloc(read_size);
    if (!read_addr)
    {
        printf("alloc read addr failed\n");
    }
    // add picture
    fd = fopen(YUV_TEST_PICTURE, "rb");
    // get output image
    ret = fread(read_addr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fread  picture_addr is failed ret is %d \n", ret);
    }
    memcpy(pic_vaddr, read_addr, read_size);

#else
    k_u8 *yuv;
    k_u8 color = 0x10;
    k_u32 step = 1;

    yuv = malloc(info.size);

    for (int i = 0; i < info.act_size.width; i++)
    {
        memset(yuv + info.act_size.height * i, color, info.act_size.height);
        if (color == 0x10)
            step = 1;
        else if (color == 0xeb)
            step = -1;
        color += step;
    }
    memset(yuv + info.act_size.height * info.act_size.width, 0x80, info.act_size.height * info.act_size.width / 2);

    memcpy(pic_vaddr, yuv, info.size);
#endif

    kd_mapi_vo_chn_insert_frame(chn_id, &vf_info);  //K_VO_OSD0

    printf("Press Enter to exit\n");
    getchar();

    // close plane
    kd_mapi_vo_disable_video_layer(chn_id);

    samble_vo_release_vb();
}

static void sample_vo_osd_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;
    layer_info osd;
    void *pic_vaddr = NULL;
    k_vo_osd osd_id = K_VO_OSD3;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&attr, 0, sizeof(attr));
    memset(&osd, 0, sizeof(osd));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    osd.act_size.width = 640 ;
    osd.act_size.height = 480;
    osd.offset.x = 10;
    osd.offset.y = 10;
    osd.global_alptha = 0xff;
    osd.format = PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

    // vo init
    kd_mapi_vo_init();

    // set vo timing
    kd_mapi_vo_set_dev_param(&attr);

    sample_vb_init();

    // config osd
    sample_vo_creat_osd(osd_id, &osd);

    // enable vo
    kd_mapi_vo_enable();

    printf("Press Enter to start install frame\n");
    getchar();
    // set frame
    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd.act_size.width;
    vf_info.v_frame.height = osd.act_size.height;
    vf_info.v_frame.stride[0] = osd.act_size.width;
    vf_info.v_frame.pixel_format = osd.format;
    block = samble_vo_insert_frame(&vf_info, &pic_vaddr);

#if 0
    void *read_addr = NULL;
    FILE *fd;
    int ret = 0;
    k_u32 read_size = osd.size;

    read_addr = malloc(read_size);
    if (!read_addr)
    {
        printf("alloc read addr failed\n");
    }
    // add picture
    fd = fopen(OSD_TEST_PICTURE, "rb");
    // get output image
    ret = fread(read_addr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fread  picture_addr is failed ret is %d \n", ret);
    }
    memcpy(pic_vaddr, read_addr, read_size);
#else
    sample_vo_filling_color(&osd, pic_vaddr);
#endif
    kd_mapi_vo_chn_insert_frame(osd_id + 3, &vf_info);  //K_VO_OSD0

    printf("Press Enter to exit \n");
    getchar();

    // close plane
    kd_mapi_vo_osd_disable(osd_id);

    samble_vo_release_vb();
}


static void sample_vo_background_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    void *pic_vaddr = NULL;

    memset(&attr, 0, sizeof(attr));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mapi_vo_init();

    // set vo timing
    kd_mapi_vo_set_dev_param(&attr);

    // enable vo
    kd_mapi_vo_enable();
}

static k_s32 wbc_size = 0;
static k_s32 wbc_phy_addr = 0;
k_s32 sample_vo_set_writeback_attr(k_vo_wbc_attr *attr, void **pic_vaddr)
{
    k_s32 pool_id = 0;
    void *mmap_addr = NULL;
    k_u32 *virt_addr;

    wbc_size = attr->target_size.width * attr->target_size.height * 3 / 2;

    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u32 mmap_size = (((wbc_size) + (page_size) - 1) & ~((page_size) - 1));
    k_u64 page_mask = (page_size - 1);
    kd_mapi_sys_get_vb_block(&pool_id, &wbc_phy_addr, size, NULL);
    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, wbc_phy_addr & ~page_mask);
    if(mmap_addr)
        virt_addr = mmap_addr + (wbc_phy_addr & page_mask);

    if(virt_addr == NULL) {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    usleep(100 * 1000);
    *pic_vaddr = virt_addr;

    printf("writeback phys_addr is %lx \n", wbc_phy_addr);

    return 0;
}

#define YUV_WB_PICTURE       "wbc_1080x1920_nv12.yuv"

k_s32 sample_vo_writeback(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;
    layer_info info;
    k_vo_wbc_attr wb_attr;

    void *pic_vaddr = NULL;
    void *wb_pic_vaddr = NULL;
    k_vo_layer chn_id = K_VO_LAYER1;//K_VO_LAYER1;//K_VO_LAYER2;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mapi_vo_init();
    // set vo timing
    kd_mapi_vo_set_dev_param(&attr);

    sample_vb_init();

    // config lyaer
    info.act_size.width = 640;
    info.act_size.height = 480;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = 0;
    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;
    sample_vo_creat_layer(chn_id, &info);

    wb_attr.pixel_format = PIXEL_FORMAT_YVU_PLANAR_420;
    wb_attr.target_size.width = 1080;
    wb_attr.target_size.height = 1920;
    wb_attr.stride = wb_attr.target_size.width;
    sample_vo_set_writeback_attr(&wb_attr, &wb_pic_vaddr);
    // enable vo
    kd_mapi_vo_enable();

    printf("Press Enter to start install frame\n");
    getchar();
    // set frame
    vf_info.v_frame.width = info.act_size.width;
    vf_info.v_frame.height = info.act_size.height;
    vf_info.v_frame.stride[0] = info.act_size.width;
    vf_info.v_frame.pixel_format = info.format;
    samble_vo_insert_frame(&vf_info, &pic_vaddr);

#if USE_PICTURE_TEST
    void *read_addr = NULL;
    FILE *fd;
    int ret = 0;
    k_u32 read_size = info.size;

    read_addr = malloc(read_size);
    if (!read_addr)
    {
        printf("alloc read addr failed\n");
    }
    // add picture
    fd = fopen(YUV_TEST_PICTURE, "rb");
    // get output image
    ret = fread(read_addr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fread  picture_addr is failed ret is %d \n", ret);
    }
    memcpy(pic_vaddr, read_addr, read_size);

#else
    k_u8 *yuv;
    k_u8 color = 0x10;
    k_u32 step = 1;

    yuv = malloc(info.size);

    for (int i = 0; i < info.act_size.width; i++)
    {
        memset(yuv + info.act_size.height * i, color, info.act_size.height);
        if (color == 0x10)
            step = 1;
        else if (color == 0xeb)
            step = -1;
        color += step;
    }
    memset(yuv + info.act_size.height * info.act_size.width, 0x80, info.act_size.height * info.act_size.width / 2);

    memcpy(pic_vaddr, yuv, info.size);
#endif

    kd_mapi_vo_chn_insert_frame(chn_id, &vf_info);  //K_VO_OSD0

    printf("Press Enter to get writeback picture\n");
    getchar();

    //set wbc
    kd_mapi_vo_set_wbc_attr(&wb_attr);
    kd_mapi_vo_enable_wbc();

    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);
    kd_mapi_vo_disable_wbc();

    // get picture 
    FILE *fd;
    int ret = 0;
    k_u32 read_size = 1920 * 1080 * 3 / 2;
    // add picture
    fd = fopen(YUV_WB_PICTURE, "wb");
    // get output image
    ret = fwrite(wb_pic_vaddr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fread  picture_addr is failed ret is %d \n", ret);
    }

    printf("Press Enter to exit\n");
    getchar();

    // close plane
    kd_mapi_vo_disable_video_layer(chn_id);

    
    usleep(1000 * display_ms);

    kd_mapi_sys_release_vb_block(phys_addr, size);
    kd_mapi_sys_release_vb_block(wbc_phy_addr, wbc_size);

    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
}


k_s32 sample_connector_init(void)
{
    k_s32 ret = 0;
    char dev_name[64] = {0};
    k_s32 connector_fd;
    k_connector_type connector_type = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
    k_connector_info connector_info;

    memset(&connector_info, 0, sizeof(k_connector_info));
    connector_info.connector_name = (char *)dev_name;

    //connector get sensor info
    ret = kd_mapi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }
    // printf("connector_info name is %s \n", connector_info.connector_name);

    connector_fd = kd_mapi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return K_ERR_VO_NOTREADY;
    }

    printf("connector_fd  is %d ret is %d \n", connector_fd, ret);

    // set connect power
    kd_mapi_connector_power_set(connector_fd, 1);
    // // connector init
    kd_mapi_connector_init(connector_fd, &connector_info);

    return 0;
}

k_s32 sample_connector_osd_install_frame(void)
{
    layer_info osd;
    void *pic_vaddr = NULL;
    k_vo_osd osd_id = K_VO_OSD3;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;

    osd.act_size.width = 640 ;
    osd.act_size.height = 480;
    osd.offset.x = 10;
    osd.offset.y = 10;
    osd.global_alptha = 0xff;
    osd.format = PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

    sample_connector_init();

    sample_vb_init();

    // config osd
    sample_vo_creat_osd(osd_id, &osd);
    // set frame
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = osd.act_size.width;
    vf_info.v_frame.height = osd.act_size.height;
    vf_info.v_frame.stride[0] = osd.act_size.width;
    vf_info.v_frame.pixel_format = osd.format;
    block = samble_vo_insert_frame(&vf_info, &pic_vaddr);

    sample_vo_filling_color(&osd, pic_vaddr);

    getchar();
    kd_mapi_vo_chn_insert_frame(osd_id + 3, &vf_info);  //K_VO_OSD0

    printf("Press Enter to exit \n");

    getchar();

    // close plane
    kd_mapi_vo_osd_disable(osd_id);
    samble_vo_release_vb();
    // fclose(fd);
    //exit ;
    return 0;
}


int main(int argc, char * const argv[])
{
    k_u8 i = 0;
    k_s32 ret = 0;
    int test_case;

    test_case = atoi(argv[1]);

    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_init error: %x\n", ret);
        return -1 ;
    }

    display_hardware_init();

    switch (test_case)
    {
        case DISPALY_VO_CONNECTOR_TEST :
            sample_connector_osd_install_frame();
            break;

        case DISPLAY_DSI_LP_MODE_TEST:
            sample_dwc_dsi_init(1);
            break;

        case DISPLAY_DSI_TEST_PATTERN :
            sample_dwc_dsi_init(2);
            break;

        case DISPALY_VO_BACKGROUND_TEST :
            sample_dwc_dsi_init(0);
            sample_vo_background_test();
            break;

        case DISPALY_VO_OSD0_TEST :
            sample_dwc_dsi_init(0);
            sample_vo_osd_test();
            break;

        case DISPALY_VO_LAYER_INSERT_FRAME_TEST :
            sample_dwc_dsi_init(0);
            sample_vo_layer_test();
            break;

    }
}