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


#define SAMPE_VVI_PIPE_NUMS                     1


extern k_vo_display_resolution hx8399[20];


static void sample_vvi_bind_vo(sample_vvi_pipe_conf_t *pipe_conf,  k_u32 ch_id)
{
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn vvo_mpp_chn;

    if (!pipe_conf)
        return;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vvi_mpp_chn.chn_id = pipe_conf[0].chn_num;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vvo_mpp_chn.chn_id = ch_id;
    kd_mpi_sys_bind(&vvi_mpp_chn, &vvo_mpp_chn);

    return;
}


static void sample_vvi_unbind_vo(sample_vvi_pipe_conf_t *pipe_conf, k_u32 ch_id)
{
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn vvo_mpp_chn;

    if (!pipe_conf)
        return;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vvi_mpp_chn.chn_id = pipe_conf[0].chn_num;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vvo_mpp_chn.chn_id = ch_id;
    kd_mpi_sys_unbind(&vvi_mpp_chn, &vvo_mpp_chn);

    return;
}
static void sample_vvi_attr_set(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 i;
    k_vvi_dev_attr dev_attr;
    k_vvi_chn_attr chn_attr;

    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
    {
        memset(&chn_attr, 0, sizeof(chn_attr));
        memset(&dev_attr, 0, sizeof(dev_attr));
        dev_attr.format = pipe_conf[i].dev_format;
        dev_attr.height = pipe_conf[i].dev_height;
        dev_attr.width = pipe_conf[i].dev_width;
        chn_attr.frame_rate = 1;
        chn_attr.format = pipe_conf[i].chn_format;
        chn_attr.height = pipe_conf[i].chn_height;
        chn_attr.width = pipe_conf[i].chn_width;
//        printf("pipe[%d] dev[%d] h:%d w:%d chn[%d] h:%d w:%d \n", i,
//               pipe_conf[i].dev_num, pipe_conf[i].dev_height, pipe_conf[i].dev_width,
//               pipe_conf[i].chn_num, pipe_conf[i].chn_height, pipe_conf[i].chn_width);
        kd_mpi_vvi_set_dev_attr(pipe_conf[i].dev_num, &dev_attr);
        kd_mpi_vvi_set_chn_attr(pipe_conf[i].chn_num, &chn_attr);
    }
    return;
}

static void sample_vvi_start(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 i;

    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
        kd_mpi_vvi_start_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    return;
}



static void sample_vvi_stop(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 i;

    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
        kd_mpi_vvi_stop_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    return;
}


static k_s32 sample_vb_init(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 ret;
    k_vb_config config;
    k_s32 i;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
    {
        config.comm_pool[i].blk_cnt = 5;
        if (pipe_conf->dev_format == PIXEL_FORMAT_YVU_SEMIPLANAR_420)
            config.comm_pool[i].blk_size = pipe_conf[i].chn_height * pipe_conf[i].chn_width * 3 / 2;

        if (pipe_conf->dev_format == PIXEL_FORMAT_ARGB_8888)
            config.comm_pool[i].blk_size = pipe_conf[i].chn_height * pipe_conf[i].chn_width * 4;

        config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
    }
    ret = kd_mpi_vb_set_config(&config);
    printf("\n");
    printf("-----------virtual vi vo sample test------------------------\n");
    if (ret)
        printf("vb_set_config failed ret:%d\n", ret);

    k_vb_supplement_config supplement_config;
    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    if (ret)
        printf("vb_set_supplement_config failed ret:%d\n", ret);
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

sample_vvi_pipe_conf_t g_pipe_conf[2] =
{
    {
        0,
        0,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        720,
        480,
        PIXEL_FORMAT_ARGB_8888,
    },
};

sample_vvi_pipe_conf_t g_yuv_pipe_conf[2] =
{
    {
        0,
        0,
        1920,
        1080,
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        720,
        480,
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    },
};


k_s32 vo_layer_bind_config(layer_bind_config *config)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    layer_info info;

    k_vo_layer chn_id = config->ch;

    // set hardware reset;
    kd_display_set_backlight();
    // rst display subsystem
    kd_display_reset();

    dwc_dsi_init();

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
    printf("%s>w %d, h %d\n", __func__, config->w, config->h);
    // config lyaer
    info.act_size.width = config->w;//1080;//640;//1080;
    info.act_size.height = config->h;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = config->ro;
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

k_s32 vdss_bind_vo_config()
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    layer_info info;

    k_vo_layer chn_id = K_VO_LAYER1;

    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0xffffff;//0x808000;
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
    info.func = K_ROTATION_90;////K_ROTATION_90;
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

k_s32 vo_layer_bind_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    layer_info info;

    k_vo_layer chn_id = K_VO_LAYER2;//K_VO_LAYER1;//K_VO_LAYER2;
    k_u32 vo_chn_id = K_VO_DISPLAY_CHN_ID2;

    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    //  vvi bind vo
    if (sample_vb_init(g_pipe_conf))
    {
        return -1;
    }
    sample_vvi_bind_vo(g_pipe_conf, vo_chn_id);

    // set vvi
    sample_vvi_attr_set(g_pipe_conf);

    // vo init
    kd_mpi_vo_init();
    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    // config lyaer
    info.act_size.width = 720;//1080;//640;//1080;
    info.act_size.height = 480;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = 0;//K_VO_SCALER_ENABLE ;//K_ROTATION_270 ;//| K_VO_GRAY_ENABLE | K_VO_MIRROR_BOTH;//K_ROTATION_90 | K_VO_GRAY_ENABLE | K_VO_MIRROR_BOTH ;//K_VO_MIRROR_VER;//K_VO_MIRROR_HOR;//K_ROTATION_90;//K_ROTATION_180;
    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;
    vo_creat_layer_test(chn_id, &info);

    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to start vvi\n");
    getchar();
    // config vvi
    sample_vvi_start(g_pipe_conf);

    printf("Press Enter to exit\n");
    getchar();

    sample_vvi_stop(g_pipe_conf);
    // close plane
    kd_mpi_vo_disable_video_layer(chn_id);
    sample_vvi_unbind_vo(g_pipe_conf, vo_chn_id);
    sample_vb_exit();

    //exit ;
    return 0;
}


k_s32 vo_osd_bind_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    osd_info osd;


    k_vo_osd osd_id = K_VO_OSD0;//K_VO_OSD2;//K_VO_OSD1; K_VO_OSD0
    k_u32 vo_chn_id = K_VO_DISPLAY_CHN_ID3;

    memset(&attr, 0, sizeof(attr));
    memset(&osd, 0, sizeof(osd));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    osd.act_size.width = 720 ;
    osd.act_size.height = 480;
    osd.offset.x = 0;
    osd.offset.y = 0;
    osd.global_alptha = 0xff;// 0x7f;
    osd.format = PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_BGR_888;// PIXEL_FORMAT_RGB_888;//PIXEL_FORMAT_BGR_565;//PIXEL_FORMAT_RGB_565;//PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_RGB_565;//PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_ABGR_8888;

    //  vvi bind vo
    if (sample_vb_init(g_pipe_conf))
    {
        return -1;
    }

    sample_vvi_bind_vo(g_pipe_conf, vo_chn_id);

    // set vvi
    sample_vvi_attr_set(g_pipe_conf);

    // vo init
    kd_mpi_vo_init();

    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    // config osd
    vo_creat_osd_test(osd_id, &osd);

    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to start vvi\n");
    getchar();
    // config vvi
    sample_vvi_start(g_pipe_conf);

    printf("Press Enter to exit\n");
    getchar();

    sample_vvi_stop(g_pipe_conf);
    // close plane
    kd_mpi_vo_osd_disable(osd_id);

    sample_vvi_unbind_vo(g_pipe_conf, vo_chn_id);
    sample_vb_exit();

    //exit ;
    return 0;
}


#define RGB_DUMP_PICTURE            "dump_rgb8888.bin"

k_s32 vo_osd_bind_dump_frame_test(void)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    osd_info osd;

    k_video_frame_info vf_info;
    k_u32 timeout = 0;
    k_u32 ret = 0;
    // k_u32 *virt_addr = NULL;

    k_vo_osd osd_id = K_VO_OSD0;//K_VO_OSD2;//K_VO_OSD1; K_VO_OSD0
    k_u32 vo_chn_id = K_VO_DISPLAY_CHN_ID3;

    memset(&attr, 0, sizeof(attr));
    memset(&osd, 0, sizeof(osd));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    osd.act_size.width = 720 ;
    osd.act_size.height = 480;
    osd.offset.x = 0;
    osd.offset.y = 0;
    osd.global_alptha = 0xff;// 0x7f;
    osd.format = PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_BGR_888;// PIXEL_FORMAT_RGB_888;//PIXEL_FORMAT_BGR_565;//PIXEL_FORMAT_RGB_565;//PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_RGB_565;//PIXEL_FORMAT_ARGB_8888;//PIXEL_FORMAT_ABGR_8888;

    //  vvi bind vo
    if (sample_vb_init(g_pipe_conf))
    {
        return -1;
    }

    sample_vvi_bind_vo(g_pipe_conf, vo_chn_id);

    // set vvi
    sample_vvi_attr_set(g_pipe_conf);

    // vo init
    kd_mpi_vo_init();

    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);

    // config osd
    vo_creat_osd_test(osd_id, &osd);

    // enable vo
    kd_mpi_vo_enable();

    printf("Press Enter to start vvi\n");
    getchar();
    // config vvi
    sample_vvi_start(g_pipe_conf);

    printf("Press Enter to dump frame\n");
    getchar();
    //dump frame
    timeout = 1000;
    ret = kd_mpi_vo_chn_dump_frame(K_VO_DISPLAY_CHN_ID3, &vf_info, timeout);
    if (ret < 0)
    {
        printf("dump frame failed reg is %d \n", ret);
    }

    // virt_addr = (k_u32 *)kd_mpi_sys_mmap(vf_info.v_frame.phys_addr[0], osd.size);

    getchar();

    printf("vf_info phyy addr is %lx \n", vf_info.v_frame.phys_addr[0]);

#if 0
    //get wbc
    FILE *fd;
    k_u32 read_size = osd.size;

    // add picture
    fd = fopen(RGB_DUMP_PICTURE, "wb");
    // get output image
    ret = fwrite(virt_addr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fwrite  picture_addr is failed ret is %d \n", ret);
    }
#endif
    printf("Press Enter to exit\n");
    getchar();

    kd_mpi_vo_chn_dump_release(K_VO_DISPLAY_CHN_ID3,  &vf_info);

    sample_vvi_stop(g_pipe_conf);
    // close plane
    kd_mpi_vo_osd_disable(osd_id);

    sample_vvi_unbind_vo(g_pipe_conf, vo_chn_id);
    sample_vb_exit();

    //exit ;
    return 0;
}
