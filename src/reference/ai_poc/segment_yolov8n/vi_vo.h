#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>

#include "mpi_sys_api.h"

/* vicap */
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
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "mpi_sys_api.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"

#include "vo_test_case.h"

extern k_s32 kd_display_set_backlight(void);
extern k_s32 kd_display_reset(void);
// extern k_vo_display_resolution hx8399[20];

#define SENSOR_CHANNEL (3)     // isp通道数
#define SENSOR_HEIGHT (1280)  // isp高度，ai输入，竖屏
#define SENSOR_WIDTH (720)    // isp宽度，ai输入，竖屏

#define ISP_CHN0_WIDTH  (1088)//(1920)
#define ISP_CHN0_HEIGHT (1920)//(1080)

#define vicap_install_osd                   (1)
#define osd_id                              K_VO_OSD3
#define osd_width                           (1080)
#define osd_height                          (1920)


k_vb_config config;
k_vicap_dev vicap_dev;
k_vicap_chn vicap_chn;
k_vicap_dev_attr dev_attr;
k_vicap_chn_attr chn_attr;
k_vicap_sensor_info sensor_info;
k_vicap_sensor_type sensor_type;
k_mpp_chn vicap_mpp_chn;
k_mpp_chn vo_mpp_chn;

k_video_frame_info dump_info;

k_vo_draw_frame vo_frame = (k_vo_draw_frame) {
    1,
    16,
    16,
    128,
    128,
    1
};

static k_vb_blk_handle block;
k_u32 g_pool_id;
// osd  我加的-------------------------------------


k_vo_display_resolution hx8399[20] =
{
    //{74250, 445500, 1340, 1080, 20, 20, 220, 1938, 1920, 5, 8, 10},           // display  evblp3
    {37125, 222750, 1240, 1080, 20, 20, 120, 1988, 1920, 5, 8, 55},
};

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
    kd_mpi_vo_set_video_layer_attr(chn_id, &attr);

    // enable layer
    kd_mpi_vo_enable_video_layer(chn_id);

    return 0;
}

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

    k_u8 pag20[50] = {0xB2, 0x0b, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};               // ��ɫ


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

// 我加的-----------------------------------
k_vb_blk_handle vo_insert_frame(k_video_frame_info *vf_info, void **pic_vaddr)
{
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_vb_blk_handle handle;
    k_s32 size;

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

    printf("phys_addr is %lx g_pool_id is %d \n", phys_addr, g_pool_id);

    return handle;
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

void sample_vicap_install_osd(void)
{
    osd_info osd;

    osd.act_size.width = osd_width ;
    osd.act_size.height = osd_height;
    osd.offset.x = 0;
    osd.offset.y = 0;
    osd.global_alptha = 0xff;
    // osd.global_alptha = 0x32;
    osd.format = PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

    vo_creat_osd_test(osd_id, &osd);
}

void vo_osd_release_block(void)
{
    if(vicap_install_osd == 1)
    {
        kd_mpi_vo_osd_disable(osd_id);
        kd_mpi_vb_release_block(block);
    }
    
}
// --------------------------------------------------------------------------------


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

static k_s32 vo_layer_vdss_bind_vo_config(void)
{
    k_vo_display_resolution *resolution = NULL;
    k_s32 resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    layer_info info;

    k_vo_layer chn_id = K_VO_LAYER1;

    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0x808000;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mpi_vo_init();
    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);
    // printf("%s>w %d, h %d\n", __func__, w, h);
    // config lyaer
    info.act_size.width = ISP_CHN0_WIDTH;//ISP_CHN0_HEIGHT;//1080;//640;//1080;
    info.act_size.height = ISP_CHN0_HEIGHT;//ISP_CHN0_WIDTH;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = K_ROTATION_180;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0;//(1080-w)/2,
    info.offset.y = 0;//(1920-h)/2;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;
    vo_creat_layer_test(chn_id, &info);

    if(vicap_install_osd == 1)
        sample_vicap_install_osd();

    // enable vo
    kd_mpi_vo_enable();

    //exit ;
    return 0;
}

static void sample_vicap_bind_vo(k_mpp_chn vicap_mpp_chn, k_mpp_chn vo_mpp_chn)
{
    k_s32 ret;

    ret = kd_mpi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }

    return;
}

static void sample_vicap_unbind_vo(k_mpp_chn vicap_mpp_chn, k_mpp_chn vo_mpp_chn)
{
    k_s32 ret;

    ret = kd_mpi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }

    return;
}

int vivcap_start()
{
    k_s32 ret = 0;


    // ------------------------------------------
    k_u32 pool_id;
    k_vb_pool_config pool_config;
    // ------------------------------------------

    printf("sample_vicap ...\n");

    // sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR;
    // vicap_dev = VICAP_DEV_ID_1;

    sensor_type = IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    vicap_dev = VICAP_DEV_ID_0;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    //VB for YUV420SP output
    config.comm_pool[0].blk_cnt = 5;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[0].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);
   
    //VB for RGB888 output
    config.comm_pool[1].blk_cnt = 5;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP((SENSOR_HEIGHT * SENSOR_WIDTH * 3 ), VICAP_ALIGN_1K);

    ret = kd_mpi_vb_set_config(&config);
    if (ret) {
        printf("vb_set_config failed ret:%d\n", ret);
        return ret;
    }

    k_vb_supplement_config supplement_config;
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
    printf("sample_vicap ...kd_mpi_vicap_get_sensor_info\n");

    //vo init
    // set hardware reset;
    kd_display_set_backlight();
	// rst display subsystem
    kd_display_reset();

    dwc_dsi_init();
    vo_layer_vdss_bind_vo_config();

    // ---------------------------------------------------------------------------------------
    if(vicap_install_osd == 1)
    {
        memset(&pool_config, 0, sizeof(pool_config));
        pool_config.blk_size = VICAP_ALIGN_UP((osd_width * osd_height * 4 * 2), VICAP_ALIGN_1K);
        pool_config.blk_cnt = 4;
        pool_config.mode = VB_REMAP_MODE_NOCACHE;
        pool_id = kd_mpi_vb_create_pool(&pool_config);      // osd0 - 3 argb 320 x 240
        g_pool_id = pool_id;

        printf("--------aa--------------g_pool_id is %d pool_id is %d \n",g_pool_id, pool_id);
    }
    // ----------------------------------------------------------------------------------------


    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_get_sensor_info(sensor_type, &sensor_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = 2592;//SENSOR_HEIGHT;
    dev_attr.acq_win.height = 1944;//SENSOR_WIDTH;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;


    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(vicap_dev, dev_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
        return ret;
    }

    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));

    //set chn0 output yuv420sp
    // chn_attr.out_win = dev_attr.acq_win;
    // chn_attr.crop_win = chn_attr.out_win;
    chn_attr.out_win.h_start = 0;
    chn_attr.out_win.v_start = 0;
    chn_attr.out_win.width = ISP_CHN0_WIDTH;
    chn_attr.out_win.height = ISP_CHN0_HEIGHT;

    // chn_attr.crop_win = dev_attr.acq_win;
    chn_attr.crop_win.h_start = 768;
    chn_attr.crop_win.v_start = 16;
    chn_attr.crop_win.width = ISP_CHN0_WIDTH;
    chn_attr.crop_win.height = ISP_CHN0_HEIGHT;

    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
    chn_attr.buffer_num = VICAP_MAX_FRAME_COUNT;//at least 3 buffers for isp
    chn_attr.buffer_size = config.comm_pool[0].blk_size;
    vicap_chn = VICAP_CHN_ID_0;

    printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, vicap_chn, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }

    //bind vicap chn 0 to vo
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = vicap_chn;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    sample_vicap_bind_vo(vicap_mpp_chn, vo_mpp_chn);
    printf("sample_vicap ...dwc_dsi_init\n");

    //set chn1 output rgb888p
    // chn_attr.out_win = dev_attr.acq_win;
    // chn_attr.crop_win = chn_attr.out_win;
    chn_attr.out_win.h_start = 0;
    chn_attr.out_win.v_start = 0;
    chn_attr.out_win.width = SENSOR_WIDTH ;
    chn_attr.out_win.height = SENSOR_HEIGHT;
    // chn_attr.crop_win = dev_attr.acq_win;

    chn_attr.crop_win.h_start = 768;
    chn_attr.crop_win.v_start = 16;
    chn_attr.crop_win.width = ISP_CHN0_WIDTH;
    chn_attr.crop_win.height = ISP_CHN0_HEIGHT;

    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_BGR_888_PLANAR;
    chn_attr.buffer_num = VICAP_MAX_FRAME_COUNT;//at least 3 buffers for isp
    chn_attr.buffer_size = config.comm_pool[1].blk_size;

    printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, VICAP_CHN_ID_1, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }

    // dwc_dsi_init();
    // vo_layer_vdss_bind_vo_config();

    printf("sample_vicap ...kd_mpi_vicap_init\n");
    ret = kd_mpi_vicap_init(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        // goto err_exit;
    }

    printf("sample_vicap ...kd_mpi_vicap_start_stream\n");
    ret = kd_mpi_vicap_start_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        // goto err_exit;
    }

    return ret;
}

int vivcap_stop()
{
    printf("sample_vicap ...kd_mpi_vicap_stop_stream\n");
    int ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        return ret;
    }

    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        return ret;
    }

    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);

    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = vicap_chn;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    sample_vicap_unbind_vo(vicap_mpp_chn, vo_mpp_chn);

    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("sample_vicap, kd_mpi_vb_exit failed.\n");
        return ret;
    }

    return 0;
}

void yuv_rotate_90(char *des, char *src,int width,int height)
{
    int n = 0;
    int hw = width>>1;
    int hh = height>>1;
    int size = width * height;
    int hsize = size>>2;

    int pos = 0;

    for(int i = width-1;i >= 0;i--)
    {
        pos = 0;
        for(int j= 0;j < height;j++)
        {
            des[n++]= src[pos+i];
            pos += width;
        }
    }

}

