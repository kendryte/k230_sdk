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
#include "mpi_vdss_api.h"
#include "mpi_sys_api.h"
#include "k_vdss_comm.h"


#include "mpi_sys_api.h"
#include "mpi_vo_api.h"
#include "k_vo_comm.h"
#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vdec_api.h"
#include "mpi_vo_api.h"
#include "mpi_sys_api.h"
#include "k_vvo_comm.h"

#include "vo_test_case.h"


#define SAMPE_VICAP_PIPE_NUMS   3
#define SAMPE_VICAP_DEV_NUMS   3

typedef struct {
    k_u32 chn_num;
    k_u32 chn_height;
    k_u32 chn_width;
    k_u32 enable;
    k_pixel_format chn_format;
}sample_vicap_chn_conf;

typedef struct {
    k_u32 sensor_type;
    k_u32 dev_num;
    k_u32 dev_height;
    k_u32 dev_width;
    k_vi_attr artr;
    k_u32 enable;
    sample_vicap_chn_conf chn_conf[3];
} sample_vicap_pipe_conf_t;


static sample_vicap_pipe_conf_t g_pipe_conf[SAMPE_VICAP_PIPE_NUMS] ;


void sample_vicap_pipe_config(void)
{
    // config dev1 pip0
    g_pipe_conf[0].dev_num = 0;
    g_pipe_conf[0].sensor_type = 3;
    g_pipe_conf[0].enable = 1;
    g_pipe_conf[0].artr.csi = CSI1;
    g_pipe_conf[0].dev_height = 720;
    g_pipe_conf[0].dev_width = 1280;

    g_pipe_conf[0].artr.type = FOLLOW_STROBE_MODE;
    g_pipe_conf[0].artr.mode = LINERA_MODE;
    g_pipe_conf[0].artr.dev_format[0] = RAW10;
    g_pipe_conf[0].artr.phy_attr.lan_num = MIPI_2LAN;
    g_pipe_conf[0].artr.phy_attr.freq = MIPI_800M;
    g_pipe_conf[0].artr.bind_dvp = DVP_CSI1_FLASE_TRIGGER0;

    g_pipe_conf[0].chn_conf[0].chn_num = 0;
    g_pipe_conf[0].chn_conf[0].enable = 1;
    g_pipe_conf[0].chn_conf[0].chn_width = 1280;
    g_pipe_conf[0].chn_conf[0].chn_height = 720;
    g_pipe_conf[0].chn_conf[0].chn_format= PIXEL_FORMAT_YVU_SEMIPLANAR_420;
}


void sample_vicap_rgb_sensor_pipe_config(void)
{
    // config dev1 pip0
    g_pipe_conf[0].dev_num = 0;
    g_pipe_conf[0].sensor_type = 1;
    g_pipe_conf[0].enable = 1;
    g_pipe_conf[0].artr.csi = CSI0;
    g_pipe_conf[0].dev_height = 720;
    g_pipe_conf[0].dev_width = 1280;

    g_pipe_conf[0].artr.type = CLOSE_3D_MODE;
    g_pipe_conf[0].artr.mode = LINERA_MODE;
    g_pipe_conf[0].artr.dev_format[0] = RAW10;
    g_pipe_conf[0].artr.phy_attr.lan_num = MIPI_1LAN;
    g_pipe_conf[0].artr.phy_attr.freq = MIPI_800M;
    g_pipe_conf[0].artr.bind_dvp = DVP_CSI1_FLASE_TRIGGER0;

    g_pipe_conf[0].chn_conf[0].chn_num = 0;
    g_pipe_conf[0].chn_conf[0].enable = 1;
    g_pipe_conf[0].chn_conf[0].chn_width = 1280;
    g_pipe_conf[0].chn_conf[0].chn_height = 720;
    g_pipe_conf[0].chn_conf[0].chn_format= PIXEL_FORMAT_YVU_SEMIPLANAR_420;
}

static void sample_vicap_attr_set(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 dev_num, chn_num;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;

    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        if(pipe_conf[dev_num].enable == 1)
        {
            memset(&dev_attr, 0, sizeof(dev_attr));
            dev_attr.dev_num = pipe_conf[dev_num].dev_num;
            dev_attr.sensor_type = pipe_conf[dev_num].sensor_type;

            printf("pipe_conf[dev_num].sensor_type is %d dev_attr.sensor_type is %d \n", pipe_conf[dev_num].sensor_type, dev_attr.sensor_type);
            dev_attr.height = pipe_conf[dev_num].dev_height;
            dev_attr.width = pipe_conf[dev_num].dev_width;
            memcpy(&dev_attr.artr, &pipe_conf[dev_num].artr, sizeof(k_vi_attr));
            kd_mpi_vdss_set_dev_attr(&dev_attr);

            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++) {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                {
                    memset(&chn_attr, 0, sizeof(chn_attr));
                    chn_attr.format = pipe_conf[dev_num].chn_conf[chn_num].chn_format;
                    chn_attr.height = pipe_conf[dev_num].chn_conf[chn_num].chn_height;
                    chn_attr.width = pipe_conf[dev_num].chn_conf[chn_num].chn_width;
                    chn_attr.enable = pipe_conf[dev_num].chn_conf[chn_num].enable;

                    printf("pipe[%d] dev[%d] h:%d w:%d chn[%d] h:%d w:%d \n", dev_num,
                            pipe_conf[dev_num].dev_num, pipe_conf[dev_num].dev_height, pipe_conf[dev_num].dev_width,
                            pipe_conf[dev_num].chn_conf[chn_num].chn_num, pipe_conf[dev_num].chn_conf[chn_num].chn_height, pipe_conf[dev_num].chn_conf[chn_num].chn_width);

                    kd_mpi_vdss_set_chn_attr(dev_num, chn_num, &chn_attr);
                }
            }
        }

    }

    return;
}

static k_s32 sample_vb_init(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 ret;
    k_vb_config config;
    k_s32 dev_num, chn_num;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        if(pipe_conf[dev_num].enable == 1)
        {
            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++)
            {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                {
                    config.comm_pool[chn_num].blk_cnt = VDSS_MAX_FRAME_COUNT;//5;
                    config.comm_pool[chn_num].blk_size = pipe_conf[dev_num].chn_conf[chn_num].chn_height * pipe_conf[dev_num].chn_conf[chn_num].chn_width * 3 / 2;
                    config.comm_pool[chn_num].mode = VB_REMAP_MODE_NOCACHE;
                }
            }
        }
    }
    ret = kd_mpi_vb_set_config(&config);
    printf("\n");
    printf("----------- vi vo sample test------------------------\n");
    if(ret)
        printf("vb_set_config failed ret:%d\n", ret);

    k_vb_supplement_config supplement_config;
    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    if(ret)
        printf("vb_set_supplement_config failed ret:%d\n", ret);
    ret = kd_mpi_vb_init();
    if(ret)
        printf("vb_init failed ret:%d\n", ret);
    return ret;
}

static k_s32 sample_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if(ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}


// only bind one chn
static void sample_vicap_bind_vo(sample_vicap_pipe_conf_t* pipe_conf, k_u32 vo_ch_id)
{
    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    k_s32 ret;

    if(!pipe_conf)
        return;

    printf("pipe_conf[0].dev_num is %d pipe_conf[0].chn_conf[0].chn_num is %d \n", pipe_conf[0].dev_num, pipe_conf[0].chn_conf[0].chn_num);

    vicap_mpp_chn.mod_id = K_ID_VICAP;
    vicap_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vicap_mpp_chn.chn_id = pipe_conf[0].chn_conf[0].chn_num;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = vo_ch_id;
    ret = kd_mpi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if(ret) {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }

    // vicap_mpp_chn.mod_id = K_ID_VICAP;
    // vicap_mpp_chn.dev_id = pipe_conf[0].dev_num;
    // vicap_mpp_chn.chn_id = pipe_conf[0].chn_conf[0].chn_num;
    // vo_mpp_chn.mod_id = K_ID_VO;
    // vo_mpp_chn.dev_id = 0;
    // vo_mpp_chn.chn_id = 0;
    // ret = kd_mpi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    // if(ret) {
    //     printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    // }
    return;
}

static void sample_vicap_unbind_vo(sample_vicap_pipe_conf_t* pipe_conf, k_u32 vo_ch_id)
{
    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;

    if(!pipe_conf)
        return;

    vicap_mpp_chn.mod_id = K_ID_VICAP;
    vicap_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vicap_mpp_chn.chn_id = pipe_conf[0].chn_conf[0].chn_num;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = vo_ch_id;
    kd_mpi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);

    // vicap_mpp_chn.mod_id = K_ID_V_VI;
    // vicap_mpp_chn.dev_id = pipe_conf[0].dev_num;
    // vicap_mpp_chn.chn_id = pipe_conf[0].chn_conf[0].chn_num;
    // vo_mpp_chn.mod_id = K_ID_V_VO;
    // vo_mpp_chn.dev_id = 0;
    // vo_mpp_chn.chn_id = 0;
    // kd_mpi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);

    return;
}

static void sample_vicap_start(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 dev_num, chn_num;

    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        if(pipe_conf[dev_num].enable == 1)
        {
            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++)
            {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                    kd_mpi_vdss_start_pipe(dev_num, chn_num);
            }
        }

    }
    return;
}


static void sample_vicap_stop(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 dev_num, chn_num;

    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        if(pipe_conf[dev_num].enable == 1)
        {
            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++)
            {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                    kd_mpi_vdss_stop_pipe(dev_num, chn_num);
            }
        }

    }
    return;
}

#if 0
int main(void)
{
    // k_s32 ret;

    mpi_vdss_rst_all(2);           //rst ov9286

    // set hardware reset;
    kd_display_set_backlight();
	// rst display subsystem
    kd_display_reset();

    // sample_vicap_pipe_config();
    sample_vicap_rgb_sensor_pipe_config();

    if(sample_vb_init(g_pipe_conf)) {
        return -1;
    }

    sample_vicap_bind_vo(g_pipe_conf, 1);
    sample_vicap_attr_set(g_pipe_conf);

    dwc_dsi_init();
    vo_layer_vdss_bind_vo_config();

    sample_vicap_start(g_pipe_conf);

    getchar();

    sample_vicap_stop(g_pipe_conf);
    kd_mpi_vo_disable_video_layer(1);
    sample_vicap_unbind_vo(g_pipe_conf, 1);

    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the virtual VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

    sample_vb_exit();
    return 0;
}
#else
int main(void)
{
    // k_s32 ret;
    int null = 0;
    int ret = 0;
    k_video_frame_info vf_info;
    mpi_vdss_rst_all(1);           //rst ov9286

    // set hardware reset;
    kd_display_set_backlight();
	// rst display subsystem
    kd_display_reset();


    sample_vicap_pipe_config();
    // sample_vicap_rgb_sensor_pipe_config();

    if(sample_vb_init(g_pipe_conf)) {
        return -1;
    }
    if(null != 0)
        sample_vicap_bind_vo(g_pipe_conf, 1);

    sample_vicap_attr_set(g_pipe_conf);


    dwc_dsi_init();
    vdss_bind_vo_config();

    sample_vicap_start(g_pipe_conf);

    usleep(50000);
    while(1)
    {
        // printf("----------------------------------ret is %d \n", ret);
        ret = kd_mpi_vdss_dump_frame(0, 0, &vf_info, 10);
        printf("ret is %d \n", ret);
        if(ret >= 0)
        {
            // kd_mpi_vo_chn_insert_frame(1, &vf_info);  //K_VO_OSD0
            sleep(1);
            kd_mpi_vdss_chn_release_frame(0, 0, &vf_info);
        }
        else
            sleep(1);
    }

    getchar();

    sample_vicap_stop(g_pipe_conf);

    kd_mpi_vo_disable_video_layer(1);
    if(null != 0)
        sample_vicap_unbind_vo(g_pipe_conf, 1);

    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the virtual VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

    sample_vb_exit();
    return 0;
}

#endif
