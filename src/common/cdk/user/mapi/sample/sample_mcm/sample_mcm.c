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


#define ISP_CHN0_WIDTH  (1280)
#define ISP_CHN0_HEIGHT (720)
#define VICAP_OUTPUT_BUF_NUM   4

static k_u8 exit_flag = 0;


static int sample_vb_init(void)
{
    k_s32 ret;
    k_u16 sride = 0;

    k_mapi_media_attr_t media_attr;

    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));

    media_attr.media_config.vb_config.max_pool_cnt = 64;

    sride = ISP_CHN0_WIDTH;
    //VB for YUV444 output for dev0 
    media_attr.media_config.vb_config.comm_pool[0].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    media_attr.media_config.vb_config.comm_pool[0].blk_size = VICAP_ALIGN_UP((sride * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);
    media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;

    media_attr.media_config.vb_config.comm_pool[1].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    media_attr.media_config.vb_config.comm_pool[1].blk_size = VICAP_ALIGN_UP((sride * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);
    media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    media_attr.media_config.vb_config.comm_pool[2].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    media_attr.media_config.vb_config.comm_pool[2].blk_size = VICAP_ALIGN_UP((sride * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);
    media_attr.media_config.vb_config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;

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

    return ret;
}

static int sample_vicap_init(k_vicap_dev dev_chn, k_vicap_sensor_type type)
{
    k_vicap_dev_set_info dev_attr;
    k_vicap_chn_set_info chn_attr;
    k_s32 ret = 0;

    memset(&dev_attr, 0 ,sizeof(dev_attr));
    memset(&chn_attr, 0 ,sizeof(chn_attr));

    dev_attr.vicap_dev = dev_chn;
    dev_attr.mode = VICAP_WORK_ONLY_MCM_MODE;
    dev_attr.sensor_type = type;
    dev_attr.buffer_num = VICAP_OUTPUT_BUF_NUM;
    dev_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.dw_en = 0;

    ret = kd_mapi_vicap_set_dev_attr(dev_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
        return ret;
    }

    chn_attr.crop_en = K_FALSE;
    chn_attr.scale_en = K_FALSE;
    chn_attr.chn_en = K_TRUE;
    chn_attr.crop_h_start = 0;
    chn_attr.crop_v_start = 0;
    chn_attr.out_height = ISP_CHN0_HEIGHT;
    chn_attr.out_width = ISP_CHN0_WIDTH;
    chn_attr.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_444;//PIXEL_FORMAT_YUV_SEMIPLANAR_444;
    chn_attr.vicap_dev = dev_chn;
    chn_attr.buffer_num = VICAP_OUTPUT_BUF_NUM;
    // chn_attr_info.vicap_dev = arg_dev;
    chn_attr.vicap_chn = VICAP_CHN_ID_0;
    chn_attr.fps = 0;
    chn_attr.buf_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);
    ret = kd_mapi_vicap_set_chn_attr(chn_attr);

    return 0;
}

static k_s32 sample_vicap_stream(k_vicap_dev vicap_dev, k_bool en)
{
    k_s32 ret = 0;
    if(en)
    {
        ret = kd_mapi_vicap_start(vicap_dev);
        if (ret) {
            printf("sample_vicap, kd_mpi_vicap_start_stream failed.\n");
            return ret;
        }
    }
    else
    {
        ret = kd_mapi_vicap_stop(vicap_dev);
        if (ret) {
            printf("sample_vicap, stop stream failed.\n");
            return ret;
        }
    }
    return ret;
}

static k_s32 sample_vo_connector_init(k_connector_type type)
{
    k_s32 ret = 0;
    char dev_name[64] = {0};
    k_s32 connector_fd;
    k_connector_type connector_type = type;
    k_connector_info connector_info;

    memset(&connector_info, 0, sizeof(k_connector_info));
    connector_info.connector_name = (char *)dev_name;

    //connector get sensor info
    ret = kd_mapi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    connector_fd = kd_mapi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return K_ERR_VO_NOTREADY;
    }

    // set connect power
    kd_mapi_connector_power_set(connector_fd, 1);
    // // connector init
    kd_mapi_connector_init(connector_fd, &connector_info);

    return 0;
}


static void sample_vo_init(k_connector_type type)
{
    layer_info info;
    k_vo_layer chn_id = K_VO_LAYER1;

    memset(&info, 0, sizeof(info));

    sample_vo_connector_init(type);

    if(type == HX8377_V2_MIPI_4LAN_1080X1920_30FPS)
    {
        info.act_size.width = ISP_CHN0_HEIGHT ;//1080;//640;//1080;
        info.act_size.height = ISP_CHN0_WIDTH;//1920;//480;//1920;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = K_ROTATION_90;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = 0;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        sample_vo_creat_layer(chn_id, &info);
    }
    else
    {
        info.act_size.width = ISP_CHN0_WIDTH ;//1080;//640;//1080;
        info.act_size.height = ISP_CHN0_HEIGHT;//1920;//480;//1920;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = 0;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = 0;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        sample_vo_creat_layer(chn_id, &info);
    }
}

static void sample_mcm_bind_vo_init(k_vicap_dev vicap_dev, k_s32 vo_chn)
{
    k_s32 ret;

    k_mpp_chn vicap_mpp_chn, vo_mpp_chn;

    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = 0;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = vo_chn;

    ret = kd_mapi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mapi_sys_bind failed:0x%x\n", ret);
    }

    return;
}


static void sample_mcm_ubind_vo_init(k_vicap_dev vicap_dev, k_s32 vo_chn)
{
    k_s32 ret;

    k_mpp_chn vicap_mpp_chn, vo_mpp_chn;

    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = 0;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = vo_chn;

    ret = kd_mapi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }

    return;
}


int main(int argc, char *argv[])
{
    int ret;

    // ipcmsg service init
    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS)
    {
        printf("kd_mapi_sys_init error: %x\n", ret);
        return 0;
    }

    ret = sample_vb_init();
    if(ret) {
        goto vb_init_error;
    }

    sample_vo_init(LT9611_MIPI_4LAN_1920X1080_60FPS);   //HX8377_V2_MIPI_4LAN_1080X1920_30FPS

    // three mcm init 
    ret = sample_vicap_init(VICAP_DEV_ID_0, XS9950_MIPI_CSI0_1280X720_30FPS_YUV422);
    if(ret < 0)
    {
        printf("vicap VICAP_DEV_ID_0 init failed \n");
        sleep(1);
        goto vicap_init_error;
    }

    // ret = sample_vicap_init(VICAP_DEV_ID_1, XS9950_MIPI_CSI1_1280X720_30FPS_YUV422);
    // if(ret < 0)
    // {
    //     printf("vicap VICAP_DEV_ID_1 init failed \n");
    //     goto vicap_init_error;
    // }

    // ret = sample_vicap_init(VICAP_DEV_ID_2, XS9950_MIPI_CSI2_1280X720_30FPS_YUV422);
    // if(ret < 0)
    // {
    //     printf("vicap VICAP_DEV_ID_2 init failed \n");
    //     goto vicap_init_error;
    // }

    sample_mcm_bind_vo_init(VICAP_DEV_ID_0, K_VO_DISPLAY_CHN_ID1);

    usleep(200000);

    // start three vicap
    sample_vicap_stream(VICAP_DEV_ID_0, K_TRUE);
    // sample_vicap_stream(VICAP_DEV_ID_1, K_TRUE);
    // sample_vicap_stream(VICAP_DEV_ID_2, K_TRUE);

    k_char select = 0;
    while(exit_flag != 1)
    {
        select = (k_char)getchar();
        switch (select)
        {
            case 'q':
                exit_flag = 1;
            default :
                break;
        }
    }

    sample_vicap_stream(VICAP_DEV_ID_0, K_FALSE);
    // sample_vicap_stream(VICAP_DEV_ID_1, K_FALSE);
    // sample_vicap_stream(VICAP_DEV_ID_2, K_FALSE);

    usleep(1000 * 34);
    kd_mapi_vo_disable_video_layer(K_VO_LAYER1);

    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();

    return 0;

vicap_init_error:
    sample_vicap_stream(VICAP_DEV_ID_0, K_FALSE);
    // sample_vicap_stream(VICAP_DEV_ID_1, K_FALSE);
    // sample_vicap_stream(VICAP_DEV_ID_2, K_FALSE);


vb_init_error:
    return 0;

}