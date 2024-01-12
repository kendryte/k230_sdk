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

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vvi_api.h"
#include "mpi_sys_api.h"
#include "mpi_vo_api.h"
#include "k_vo_comm.h"

#include "vo_test_case.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#if 0
k_vo_display_resolution hx8399[20] = {
	{49500, 297000, 1200, 1080, 20, 20, 80, 2000, 1920, 5, 8, 67},				// layer0 、layer1 需要提前30 行预取
};
#else
k_vo_display_resolution hx8399[20] =
{
    // {74250, 445500, 1340, 1080, 20, 20, 220, 1938, 1920, 5, 8, 10},           // display  evblp3
    {74250, 445500, 1240, 1080, 20, 20, 120, 1988, 1920, 5, 8, 55},
    {49500, 297000, 1200, 1080, 20, 20, 80, 2000, 1920, 5, 8, 67},				// layer0 、layer1 需要提前30 行预取
};

k_vo_display_resolution cma0400[20] = {
    {9000, 216000, 500, 454, 5, 10, 31, 600, 454, 5, 10, 131},	 // ok

};

#endif

k_u32 g_pool_id;

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
    k_u8 param15[] = {0xCC, 0x04};  // 0x9
    k_u8 param16[] = {0xC6, 0xFF, 0xF9};
    k_u8 param22[] = {0xE0, 0x00, 0x12, 0x1f, 0x1a, 0x40, 0x4a, 0x59, 0x55, 0x5e, 0x67, 0x6f, 0x75, 0x7a, 0x82, 0x8b, 0x90, 0x95, 0x9f, 0xa3, 0xad, 0xa2, 0xb2, 0xB6, 0x5e, 0x5a, 0x65, 0x77, 0x00, 0x12, 0x1f, 0x1a, 0x40, 0x4a, 0x59, 0x55, 0x5e, 0x67, 0x6f, 0x75, 0x7a, 0x82, 0x8b, 0x90, 0x95, 0x9f, 0xa3, 0xad, 0xa2, 0xb2, 0xB6, 0x5e, 0x5a, 0x65, 0x77};
    k_u8 param23[] = {0x11};
    k_u8 param24[] = {0x29};
    k_u8 pag20[50] = {0xB2, 0x0b, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};               // 蓝色

    kd_mpi_dsi_send_cmd(param1, sizeof(param1));
    kd_mpi_dsi_send_cmd(param21, sizeof(param21));
    kd_mpi_dsi_send_cmd(param2, sizeof(param2));
    kd_mpi_dsi_send_cmd(param3, sizeof(param3));
    kd_mpi_dsi_send_cmd(param4, sizeof(param4));
    kd_mpi_dsi_send_cmd(param5, sizeof(param5));
    kd_mpi_dsi_send_cmd(param6, sizeof(param6));
    kd_mpi_dsi_send_cmd(param7, sizeof(param7));
    kd_mpi_dsi_send_cmd(param8, sizeof(param8));
    kd_mpi_dsi_send_cmd(param9, sizeof(param9));

    if (test_mode_en == 1)
    {
        kd_mpi_dsi_send_cmd(pag20, 10);                   // test  mode
    }

    kd_mpi_dsi_send_cmd(param10, sizeof(param10));
    kd_mpi_dsi_send_cmd(param11, sizeof(param11));
    kd_mpi_dsi_send_cmd(param12, sizeof(param12));
    kd_mpi_dsi_send_cmd(param13, sizeof(param13));
    kd_mpi_dsi_send_cmd(param14, sizeof(param14));
    kd_mpi_dsi_send_cmd(param15, sizeof(param15));
    kd_mpi_dsi_send_cmd(param16, sizeof(param16));
    kd_mpi_dsi_send_cmd(param22, sizeof(param22));
    kd_mpi_dsi_send_cmd(param23, 1);
    usleep(300000);
    kd_mpi_dsi_send_cmd(param24, 1);
    usleep(100000);
}


void cma0400_495x495_config(void)
{
    k_u8 param1[] = {0x11};
    k_u8 param2[] = {0x29};

	kd_mpi_dsi_send_cmd(param1, sizeof(param1));
	usleep(170000);
	kd_mpi_dsi_send_cmd(param2, sizeof(param2));
    usleep(200000);
}

void dwc_dsi_lpmode_test(void)
{

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;
    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;
    int screen_test_mode = 1;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));

    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = 295;
    phy_attr.n = 15;
    phy_attr.voc = 0x17;
    phy_attr.hs_freq = 0x96;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);

    // config screen
    hx8399_v2_init(screen_test_mode);

    // enable dsi
    kd_mpi_dsi_enable(enable);

    printf("Press Enter to exit \n");
    getchar();
}

void dwc_dsi_read_hx8399_id(void)
{

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;
    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;
    int screen_test_mode = 1;

    k_u32 rv_data = 0;
    k_u8 addr = 0x4;
    k_u16 cmd_len = 0x3;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));

    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = 295;
    phy_attr.n = 15;
    phy_attr.voc = 0x17;
    phy_attr.hs_freq = 0x96;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);

    // config screen
    hx8399_v2_init(screen_test_mode);

    kd_mpi_dsi_read_pkg(addr, cmd_len, &rv_data);

    printf("rv_data is %x \n", rv_data);

    // enable dsi
    kd_mpi_dsi_enable(enable);
}


void dwc_dsi_hsmode_test(void)
{

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;

    k_vo_mipi_phy_attr phy_attr;

    int enable = 1;
    int screen_test_mode = 1;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));


    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = 295;
    phy_attr.n = 15;
    phy_attr.voc = 0x17;
    phy_attr.hs_freq = 0x96;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_HS_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);

    // config screen
    hx8399_v2_init(screen_test_mode);

    // enable dsi
    kd_mpi_dsi_enable(enable);

    printf("Press Enter to exit \n");
    getchar();
}


void dwc_dsi_init_with_test_pattern(void)
{

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;

    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;
    int screen_test_mode = 0;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));

    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = 295;
    phy_attr.n = 15;
    phy_attr.voc = 0x17;
    phy_attr.hs_freq = 0x96;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);

    // config screen
    hx8399_v2_init(screen_test_mode);

    // enable dsi
    kd_mpi_dsi_enable(enable);

    // enable test pattern
    kd_mpi_dsi_set_test_pattern();

    printf("Press Enter to exit \n");
    getchar();
}

void dwc_dsi_init(void)
{

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;

    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;
    int screen_test_mode = 0;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));


    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = 295;
    phy_attr.n = 15;
    phy_attr.voc = 0x17;
    phy_attr.hs_freq = 0x96;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);

    // config scann
    hx8399_v2_init(screen_test_mode);

    // enable dsi
    kd_mpi_dsi_enable(enable);

}


void dwc_dsi_layer0_init(void)
{

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 1;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;

    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;
    int screen_test_mode = 0;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));

    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = TXPHY_297_M;
    phy_attr.n = TXPHY_297_N;
    phy_attr.voc = TXPHY_297_VOC;
    phy_attr.hs_freq = TXPHY_297_HS_FREQ;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);
    // config scann
    hx8399_v2_init(screen_test_mode);
    // enable dsi
    kd_mpi_dsi_enable(enable);
}


void dwc_dsi_1lan_init(void)
{

    k_vo_display_resolution *resolution = NULL;
    resolution = &cma0400[0];
    k_vo_dsi_attr attr;

    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));

    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = TXPHY_216_M;
    phy_attr.n = TXPHY_216_N;
    phy_attr.voc = TXPHY_216_VOC;
    phy_attr.hs_freq = TXPHY_216_HS_FREQ;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_1LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);
    // config scann
    cma0400_495x495_config();
    // enable dsi
    kd_mpi_dsi_enable(enable);
}


void vo_background_init(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;
    // vo init
    kd_mpi_vo_init();
    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);
    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to exit \n");
    getchar();
}


void vo_1lan_background_init(void)
{
    k_vo_display_resolution *resolution = NULL;
    resolution = &cma0400[0];

    k_vo_pub_attr attr;

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;
    // vo init
    kd_mpi_vo_init();
    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);
    // enable vo
    kd_mpi_vo_enable();
}

int vo_creat_private_poll(void)
{
    k_s32 ret = 0;
    k_vb_config config;
    k_vb_pool_config pool_config;
    k_u32 pool_id;
    memset(&config, 0, sizeof(config));

    config.max_pool_cnt = 10;
    config.comm_pool[0].blk_cnt = 20;
    config.comm_pool[0].blk_size = PRIVATE_POLL_SZE;          // osd0 - 3 argb 320 x 240
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;//VB_REMAP_MODE_NOCACHE;

    ret = kd_mpi_vb_set_config(&config);
    ret = kd_mpi_vb_init();

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = PRIVATE_POLL_NUM;
    pool_config.blk_size = PRIVATE_POLL_SZE;
    pool_config.mode = VB_REMAP_MODE_NONE;
    pool_id = kd_mpi_vb_create_pool(&pool_config);      // osd0 - 3 argb 320 x 240

    g_pool_id = pool_id;

    return ret;
}

k_s32 vo_release_private_poll(void)
{
    kd_mpi_vb_destory_pool(g_pool_id);
    kd_mpi_vb_exit();
    return 0;
}


k_u32 vo_creat_osd_test(k_vo_osd osd, osd_info *info)
{
    k_vo_video_osd_attr attr;

    // set attr
    attr.global_alptha = info->global_alptha;

    if (info->format == PIXEL_FORMAT_ABGR_8888 || info->format == PIXEL_FORMAT_ARGB_8888)
    {
        info->size = info->act_size.width  * info->act_size.height * 4;
        info->stride  = info->act_size.width * 4 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_565 || info->format == PIXEL_FORMAT_BGR_565)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride  = info->act_size.width * 2 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_888 || info->format == PIXEL_FORMAT_BGR_888)
    {
        info->size = info->act_size.width  * info->act_size.height * 3;
        info->stride  = info->act_size.width * 3 / 8;
    }
    else if(info->format == PIXEL_FORMAT_ARGB_4444 || info->format == PIXEL_FORMAT_ABGR_4444)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride  = info->act_size.width * 2 / 8;
    }
    else if(info->format == PIXEL_FORMAT_ARGB_1555 || info->format == PIXEL_FORMAT_ABGR_1555)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride  = info->act_size.width * 2 / 8;
    }
    else
    {
        printf("set osd pixel format failed  \n");
    }

    attr.stride = info->stride;
    attr.pixel_format = info->format;
    attr.display_rect = info->offset;
    attr.img_size = info->act_size;
    kd_mpi_vo_set_video_osd_attr(osd, &attr);

    kd_mpi_vo_osd_enable(osd);

    return 0;
}


k_vb_blk_handle vo_insert_frame(k_video_frame_info *vf_info, void **pic_vaddr)
{
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_vb_blk_handle handle;
    k_s32 size = 0;

    if (vf_info == NULL)
        return K_FALSE;

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

    size = size + 4096;         // 强制4K ，后边得删了

    printf("vb block size is %x \n", size);

    handle = kd_mpi_vb_get_block(g_pool_id, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }

    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
    // virt_addr = (k_u32 *)kd_mpi_sys_mmap_cached(phys_addr, size);

    if (virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    vf_info->mod_id = K_ID_VO;
    vf_info->pool_id = g_pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
    *pic_vaddr = virt_addr;

    printf("phys_addr is %lx \n", phys_addr);

    return handle;
}

k_s32 vo_release_frame(k_vb_blk_handle handle)
{
    return kd_mpi_vb_release_block(handle);
}

void vo_osd_filling_color(osd_info *osd, void *pic_vaddr)
{
    int i = 0;
    k_u32 *temp_addr = (k_u32 *)pic_vaddr;


    if (osd->format == PIXEL_FORMAT_ABGR_8888)
    {
        for (i = 0; i < (osd->size / sizeof(k_u32)) ; i++)
        {
            temp_addr[i] = COLOR_ABGR_RED;
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

// #define OSD_TEST_PICTURE        "disney_320x240_abgr8888.yuv"
#define OSD_TEST_PICTURE        "disney_320x240_argb8888.yuv"
// #define OSD_TEST_PICTURE       "disney_320x240_rgb565be.yuv"
// #define OSD_TEST_PICTURE         "disney_320x240_bgr565be.yuv"
// #define OSD_TEST_PICTURE2        "person_320x240_bgr565be.yuv"
// #define OSD_TEST_PICTURE        "disney_320x240_bgr888.yuv"
// #define OSD_TEST_PICTURE        "disney_320x240_rgb888.yuv"

// #define OSD_TEST_PICTURE        "disney_640x480_argb1555.yuv"
//#define OSD_TEST_PICTURE        "disney_640x480_argb4444.yuv"
//#define OSD_TEST_PICTURE        "640Wx480H_ARGB4444.argb4444"
// #define OSD_TEST_PICTURE        "640Wx480H_ARGB1555.argb1555"

k_s32 vo_osd_insert_frame_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;
    osd_info osd;
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
    kd_mpi_vo_init();

    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    vo_creat_private_poll();

    // config osd
    vo_creat_osd_test(osd_id, &osd);

    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to start install frame\n");
    getchar();
    // set frame
    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd.act_size.width;
    vf_info.v_frame.height = osd.act_size.height;
    vf_info.v_frame.stride[0] = osd.act_size.width;
    vf_info.v_frame.pixel_format = osd.format;
    vf_info.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

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

    vo_osd_filling_color(&osd, pic_vaddr);

#endif
    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);  //K_VO_OSD0

    printf("Press Enter to exit \n");
    getchar();

    // close plane
    kd_mpi_vo_osd_disable(osd_id);
    vo_release_frame(block);
    vo_release_private_poll();
    // fclose(fd);
    //exit ;
    return 0;
}

k_s32 vo_osd_insert_multi_frame_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block, block2, block3;
    k_video_frame_info vf_info, vf_info2, vf_info3;
    osd_info osd;
    void *pic_vaddr = NULL;
    void *pic_vaddr2 = NULL;
    void *pic_vaddr3 = NULL;
    k_vo_osd osd_id = K_VO_OSD3;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&vf_info2, 0, sizeof(vf_info2));
    memset(&vf_info3, 0, sizeof(vf_info3));

    memset(&attr, 0, sizeof(attr));
    memset(&osd, 0, sizeof(osd));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    osd.act_size.width = 320 ;
    osd.act_size.height = 240;
    osd.offset.x = 10;
    osd.offset.y = 10;
    osd.global_alptha = 0xff;// 0x7f;
    osd.format = PIXEL_FORMAT_ARGB_8888;

    // vo init
    kd_mpi_vo_init();

    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    vo_creat_private_poll();

    // config osd
    vo_creat_osd_test(osd_id, &osd);

    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to start install frame\n");
    getchar();
    // set frame
    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd.act_size.width;
    vf_info.v_frame.height = osd.act_size.height;
    vf_info.v_frame.stride[0] = osd.act_size.width;
    vf_info.v_frame.pixel_format = osd.format;
    vf_info.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;

    vf_info3.v_frame.width = osd.act_size.width;
    vf_info3.v_frame.height = osd.act_size.height;
    vf_info3.v_frame.stride[0] = osd.act_size.width;
    vf_info3.v_frame.pixel_format = osd.format;
    vf_info3.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;

    block = vo_insert_frame(&vf_info, &pic_vaddr);
    block2 = vo_insert_frame(&vf_info2, &pic_vaddr2);
    block3 = vo_insert_frame(&vf_info3, &pic_vaddr3);

#if USE_PICTURE_TEST
    void *read_addr = NULL;
    FILE *fd, fd2;
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

    printf("pic_vaddr is %p  \n", pic_vaddr);
    printf("pic_vaddr2 is %p \n", pic_vaddr2);

    int i = 0;
    k_u32 *temp_addr = (k_u32 *)pic_vaddr;

    for (i = 0; i < osd.size / 4; i++)
    {
        temp_addr[i] = 0xff0000ff;//0x0000ffff;
    }

    temp_addr = (k_u32 *)pic_vaddr3;

    for (i = 0; i < osd.size / 4; i++)
    {
        temp_addr[i] = 0x0000ffff;//COLOR_ARGB_BLUE; 0x00FF00ff
    }

    while (1)
    {

        kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);  //K_VO_OSD0

        sleep(1);

        kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info3);  //K_VO_OSD0

        sleep(1);
    }

#endif
    getchar();

    // close plane
    kd_mpi_vo_osd_disable(osd_id);

    vo_release_frame(block);
    vo_release_frame(block2);
    vo_release_frame(block3);
    vo_release_private_poll();

    // free(read_addr);
    // fclose(fd);
    //exit ;
    return 0;
}



int vo_creat_layer_test(k_vo_layer chn_id, layer_info *info)
{
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


// #define YUV_TEST_PICTURE       "disney_1080x1920_nv12.yuv"
// #define YUV_TEST_PICTURE       "1080p_nv12.yuv"
#define YUV_TEST_PICTURE       "disney_640x480_nv12.yuv"


k_s32 vo_layer_insert_frame_test(void)
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
    kd_mpi_vo_init();

    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    vo_creat_private_poll();

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
    vo_creat_layer_test(chn_id, &info);

    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to start install frame\n");
    getchar();
    // set frame
    vf_info.v_frame.width = info.act_size.width;
    vf_info.v_frame.height = info.act_size.height;
    vf_info.v_frame.stride[0] = info.act_size.width;
    vf_info.v_frame.pixel_format = info.format;
    vf_info.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;

    block = vo_insert_frame(&vf_info, &pic_vaddr);

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

    kd_mpi_vo_chn_insert_frame(chn_id, &vf_info);  //K_VO_OSD0

    printf("Press Enter to exit\n");
    getchar();

    // close plane
    kd_mpi_vo_disable_video_layer(chn_id);
    vo_release_frame(block);
    vo_release_private_poll();
    // fclose(fd);
    //exit ;
    return 0;
}

k_vb_blk_handle vo_set_writeback_attr(k_vo_wbc_attr *attr, void **pic_vaddr)
{
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_vb_blk_handle handle;
    k_s32 size = 0;

    if (attr == NULL)
        return K_FALSE;

    size = attr->target_size.width * attr->target_size.height * 3 / 2;

    size = size + 4096;         // 强制4K ，后边得删了

    printf("vb block size is %x \n", size);

    handle = kd_mpi_vb_get_block(g_pool_id, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }

    attr->y_phy_addr = phys_addr;

    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
    // virt_addr = (k_u32 *)kd_mpi_sys_mmap_cached(phys_addr, size);
    if (virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    *pic_vaddr = virt_addr;

    printf("phys_addr is %lx \n", phys_addr);

    return handle;
}

#define YUV_WB_PICTURE       "wbc_1080x1920_nv12.yuv"

k_s32 vo_writeback_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block, block2;
    k_video_frame_info vf_info;
    layer_info info;
    k_vo_wbc_attr wb_attr;

    void *pic_vaddr = NULL;
    void *wb_vaddr = NULL;
    k_vo_layer chn_id = K_VO_LAYER2;//K_VO_LAYER1;//K_VO_LAYER2;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mpi_vo_init();

    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    vo_creat_private_poll();

    // config lyaer
    info.act_size.width = 1080;//1080;//640;//1080;
    info.act_size.height = 1920;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = 0;
    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;

    wb_attr.pixel_format = PIXEL_FORMAT_YVU_PLANAR_420;
    wb_attr.target_size.width = 1080;
    wb_attr.target_size.height = 1920;
    wb_attr.stride = wb_attr.target_size.width;

    vo_creat_layer_test(chn_id, &info);

    // enable vo
    kd_mpi_vo_enable();

    getchar();
    // set frame
    vf_info.v_frame.width = info.act_size.width;
    vf_info.v_frame.height = info.act_size.height;
    vf_info.v_frame.stride[0] = info.act_size.width;
    vf_info.v_frame.pixel_format = info.format;
    vf_info.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;

    block = vo_insert_frame(&vf_info, &pic_vaddr);
    block2 = vo_set_writeback_attr(&wb_attr, &wb_vaddr);
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


    kd_mpi_vo_chn_insert_frame(chn_id, &vf_info);  //K_VO_OSD0

    getchar();

    kd_mpi_vo_set_wbc_attr(&wb_attr);
    kd_mpi_vo_enable_wbc();

    getchar();


    free(yuv);

    // disable wbc
    kd_mpi_vo_disable_wbc();
    // close plane
    kd_mpi_vo_disable_video_layer(chn_id);
    vo_release_frame(block);
    vo_release_frame(block2);
    vo_release_private_poll();
    // fclose(fd);
    //exit ;
    return 0;

}



k_s32 vo_layer0_scaler_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 1;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;
    layer_info info;

    void *pic_vaddr = NULL;
    k_vo_layer chn_id = K_VO_LAYER0;//K_VO_LAYER1;//K_VO_LAYER2;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mpi_vo_init();

    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    vo_creat_private_poll();

    // config lyaer
    info.act_size.width = 800;
    info.act_size.height = 640;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = K_VO_SCALER_ENABLE ;
    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    info.attr.out_size.width = 400;//640;
    info.attr.out_size.height = 320;//480;
    vo_creat_layer_test(chn_id, &info);

    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to start install frame\n");
    getchar();

    if((info.func & K_VO_SCALER_ENABLE) == K_VO_SCALER_ENABLE)
    {
        vf_info.v_frame.width = info.attr.out_size.width;
        vf_info.v_frame.height = info.attr.out_size.height;
    }
    else
    {
        vf_info.v_frame.width = info.act_size.width;
        vf_info.v_frame.height = info.act_size.height;
    }

    vf_info.v_frame.stride[0] = info.act_size.width;
    vf_info.v_frame.pixel_format = info.format;
    vf_info.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;

    block = vo_insert_frame(&vf_info, &pic_vaddr);

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

    kd_mpi_vo_chn_insert_frame(chn_id, &vf_info);  //K_VO_OSD0

    printf("Press Enter to exit\n");
    getchar();

    // close plane
    kd_mpi_vo_disable_video_layer(chn_id);
    vo_release_frame(block);
    vo_release_private_poll();
    // fclose(fd);
    //exit ;
    return 0;
}

k_s32 sample_connector_init(k_connector_type type)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_u32 chip_id = 0x00;
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
    kd_mpi_connector_power_set(connector_fd, 1);
    // set connect get id
    kd_mpi_connector_id_get(connector_fd, &chip_id);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

k_s32 sample_connector_wbc_dump_frame(k_connector_type type)
{
    
    void *pic_vaddr = NULL;
    
    k_vb_blk_handle block;
    k_video_frame_info vf_info;
    k_vo_wbc_attr wb_attr;
    k_video_frame_info dump_frame;

    layer_info info;
    k_vo_layer chn_id = K_VO_LAYER1;
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
    vo_creat_layer_test(chn_id, &info);

    sample_connector_init(type);

    vo_creat_private_poll();
    // config osd
    // vo_creat_osd_test(osd_id, &osd);

    // set frame 
    vo_creat_layer_test(chn_id, &info);
    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = info.act_size.width;
    vf_info.v_frame.height = info.act_size.height;
    vf_info.v_frame.stride[0] = info.act_size.width;
    vf_info.v_frame.pixel_format = info.format;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    wb_attr.pixel_format = PIXEL_FORMAT_YVU_PLANAR_420;
    wb_attr.target_size.width = 1080;
    wb_attr.target_size.height = 1920;
    wb_attr.stride = wb_attr.target_size.width;
    kd_mpi_vo_set_wbc_attr(&wb_attr);

    printf("kd_mpi_vo_set_wbc_attr ------------------- \n");

    void *read_addr = NULL;
    k_u32 ret = 0;
    FILE *fd;
    k_u32 read_size = info.size;

    read_addr = malloc(read_size);
    if (!read_addr)
    {
        printf("alloc read addr failed\n");
    }
    // add picture
    fd = fopen(YUV_TEST_PICTURE, "rb"); //  YUV_TEST_PICTURE  OSD_TEST_PICTURE
    // get output image
    ret = fread(read_addr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fread  picture_addr is failed ret is %d \n", ret);
    }
    memcpy(pic_vaddr, read_addr, read_size);

    fclose(fd);

    getchar();
    // kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);  //K_VO_OSD0
    kd_mpi_vo_chn_insert_frame(chn_id, &vf_info);

    kd_mpi_vo_enable_wbc();

    k_char select;
    
    k_u8 *virt_addr = NULL;
    static k_u32 dump_count = 0;
    k_char *suffix = "yuv420sp";
    k_char filename[256];
    k_u32 data_size = 1920 * 1080 * 3 / 2;
    while(1)
    {
        select = (k_char)getchar();
        if(select == 'd')
        {
            ret = kd_mpi_wbc_dump_frame(&dump_frame, 1000);
            if(ret < 0)
            {
                printf("kd_mpi_wbc_dump_frame failed \n");
                continue;
            }

            virt_addr = kd_mpi_sys_mmap(dump_frame.v_frame.phys_addr[0], data_size);
            if (virt_addr) 
            {
                memset(filename, 0 , sizeof(filename));

                snprintf(filename, sizeof(filename), "dev_%02d_chn_%02d_%dx%d_%04d.%s", \
                    0, 0, dump_frame.v_frame.width, dump_frame.v_frame.height, dump_count, suffix);

                printf("save dump data to file(%s) dump_frame.v_frame.phys_addr[0] is %lx \n", filename, dump_frame.v_frame.phys_addr[0]);
                FILE *file = fopen(filename, "wb+");
                if (file) 
                {
                    fwrite(virt_addr, 1, data_size, file);
                } 
                else {
                    printf("sample_vicap, open dump file failed()\n");
                }

                fclose(file);
                kd_mpi_sys_munmap(virt_addr, data_size);
            } else 
                printf("sample_vicap, map dump addr failed.\n");

            ret = kd_mpi_wbc_dump_release(&dump_frame);

            dump_count++;
        }
    }
    printf("Press Enter to exit \n");

    getchar();

    // close plane
    // kd_mpi_vo_osd_disable(osd_id);
    kd_mpi_vo_disable_video_layer(chn_id);
    vo_release_frame(block);
    vo_release_private_poll();
    // fclose(fd);
    //exit ;
    return 0;
}

#define CONNECTOR_OSD_TEST_PICTURE        "disney_320x240_argb8888.yuv"
#define LOAD_PICTURE                       0

k_s32 sample_connector_osd_install_frame(k_connector_type type)
{
    osd_info osd;
    void *pic_vaddr = NULL;
    k_vo_osd osd_id = K_VO_OSD3;
    k_vb_blk_handle block;
    k_video_frame_info vf_info;

    osd.act_size.width = 320 ;
    osd.act_size.height = 240;
    osd.offset.x = 10;
    osd.offset.y = 10;
    osd.global_alptha = 0xff;
    osd.format = PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

    sample_connector_init(type);

    vo_creat_private_poll();
    // config osd
    vo_creat_osd_test(osd_id, &osd);
    // set frame
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = osd.act_size.width;
    vf_info.v_frame.height = osd.act_size.height;
    vf_info.v_frame.stride[0] = osd.act_size.width;
    vf_info.v_frame.pixel_format = osd.format;
    block = vo_insert_frame(&vf_info, &pic_vaddr);
#if LOAD_PICTURE
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
    fd = fopen(CONNECTOR_OSD_TEST_PICTURE, "rb");
    // get output image
    ret = fread(read_addr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fread  picture_addr is failed ret is %d \n", ret);
    }
    memcpy(pic_vaddr, read_addr, read_size);

#else
    vo_osd_filling_color(&osd, pic_vaddr);
#endif
    getchar();
    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);  //K_VO_OSD0

    printf("Press Enter to exit \n");

    getchar();

    // close plane
    kd_mpi_vo_osd_disable(osd_id);
    vo_release_frame(block);
    vo_release_private_poll();
#if LOAD_PICTURE
    fclose(fd);
#endif
    //exit ;
    return 0;
}
