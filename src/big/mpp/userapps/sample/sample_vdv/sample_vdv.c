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
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "mpi_sys_api.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"

#include "vo_test_case.h"
#include "sample_vdv.h"

#define USE_GDMA    1

extern k_s32 kd_display_set_backlight(void);
extern k_s32 kd_display_reset(void);
extern k_vo_display_resolution hx8399[20];

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
    info.act_size.width = 720;//1080;//640;//1080;
    info.act_size.height = 1280;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = K_ROTATION_0;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0;//(1080-w)/2,
    info.offset.y = 0;//(1920-h)/2;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;
    vo_creat_layer_test(chn_id, &info);

    // enable vo
    kd_mpi_vo_enable();

    //exit ;
    return 0;
}

int main(int argc, char *argv[])
{
    k_s32 ret = 0;

    k_vb_config config;
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;
    k_vicap_sensor_type sensor_type;
    k_mpp_chn vo_mpp_chn;

    printf("sample_vicap ...\n");

    sensor_type = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
    vicap_dev = VICAP_DEV_ID_0;

    //vo init
    // set hardware reset;
    kd_display_set_backlight();
	// rst display subsystem
    kd_display_reset();

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    /* vi vb init*/
    config.comm_pool[0].blk_cnt = 5;
    config.comm_pool[0].blk_size = VICAP_ALIGN_UP((1280 * 720 * 3 / 2), VICAP_ALIGN_1K);
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;

    /* dma vb init */
    config.comm_pool[1].blk_cnt = 5;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP((1280 * 720 * 3 / 2), VICAP_ALIGN_1K);
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

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

    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_get_sensor_info(sensor_type, &sensor_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = 1280;
    dev_attr.acq_win.height = 720;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    dev_attr.pipe_ctrl.bits.ae_enable = K_TRUE;
    dev_attr.pipe_ctrl.bits.awb_enable = K_TRUE;

    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(vicap_dev, dev_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
        return ret;
    }

    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));

    chn_attr.out_win = dev_attr.acq_win;
    chn_attr.crop_win = chn_attr.out_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
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

    /* bind init */
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn dma_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = vicap_dev;
    vi_mpp_chn.chn_id = vicap_chn;
    dma_mpp_chn.mod_id = K_ID_DMA;
    dma_mpp_chn.dev_id = 0;
    dma_mpp_chn.chn_id = 0;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    ret = kd_mpi_sys_bind(&vi_mpp_chn, &dma_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_bind(&dma_mpp_chn, &vo_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);


    dwc_dsi_init();
    vo_layer_vdss_bind_vo_config();

    /* dma_init */
    ret = sample_vdd_dma_init();
    if (ret) {
        printf("sample_dma_init failed\n");
        // goto err_dpu_delete;
    }

    printf("sample_vicap ...kd_mpi_vicap_init\n");
    ret = kd_mpi_vicap_init(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        goto err_exit;
    }

    printf("sample_vicap ...kd_mpi_vicap_start_stream\n");

    ret = kd_mpi_vicap_start_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        goto err_exit;
    }

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
        printf(" q: to exit\n");
        printf("---------------------------------------\n");
        printf("please Input:\n\n");
        }
        select = (k_char)getchar();
        switch (select)
        {
            case 'q':
                goto sel_exit;
            default:
                break;
        }
        sleep(1);
    }

sel_exit:
    printf("sample_vicap ...kd_mpi_vicap_stop_stream\n");
    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        goto err_exit;
    }

err_exit:
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        return ret;
    }

    kd_mpi_vo_disable_video_layer(1);

    ret = sample_vdd_dma_delete();
    if (ret) {
        printf("sample_dma_delete failed\n");
        return 0;
    }

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = vicap_dev;
    vi_mpp_chn.chn_id = vicap_chn;
    dma_mpp_chn.mod_id = K_ID_DMA;
    dma_mpp_chn.dev_id = 0;
    dma_mpp_chn.chn_id = 0;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &dma_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_unbind(&dma_mpp_chn, &vo_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("sample_vicap, kd_mpi_vb_exit failed.\n");
        return ret;
    }

    return ret;
}
