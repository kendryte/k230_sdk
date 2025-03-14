/**
  @file videocapture_basic.cpp
  @brief A very basic sample for using VideoCapture and VideoWriter
  @author PkLab.net
  @date Aug 24, 2016
*/

#include <iostream>
#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <getopt.h>

#include "k_vicap_comm.h"
#include "k_video_comm.h"
#include "mpi_vicap_api.h"

#include "k_sys_comm.h"
#include "mpi_sys_api.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"

#include "k_vo_comm.h"
#include "mpi_vo_api.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

using namespace cv;
using namespace std;

#define MAX_VO_LAYER_NUM 2

static k_u32 display_width = 0;
static k_u32 display_height = 0;

typedef struct {
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

typedef struct {
    k_u16 width[MAX_VO_LAYER_NUM];
    k_u16 height[MAX_VO_LAYER_NUM];
    k_u16 rotation[MAX_VO_LAYER_NUM];
    k_vo_layer layer[MAX_VO_LAYER_NUM];
    k_bool enable[MAX_VO_LAYER_NUM];
} k230_display_layer_conf;

k_connector_type connector_type = LT9611_MIPI_4LAN_1920X1080_60FPS;

static int k230_vo_creat_layer(k_vo_layer chn_id, layer_info *info)
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

static int k230_display_layer_init(k230_display_layer_conf *layer_conf, k_u32 display_width, k_u32 display_height)
{
    int ret = 0;
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

    for (int i = 0; i < MAX_VO_LAYER_NUM; i++) {
        if (layer_conf->enable[i]) {
            info[i].offset.x = (display_width - info[i].act_size.width)/2;
            info[i].offset.y = margin + relative_height;
            printf("%s, layer(%d), offset.x(%d), offset.y(%d), relative_height(%d)\n", __func__, layer_conf->layer[i], info[i].offset.x, info[i].offset.y, relative_height);
            relative_height += info[i].act_size.height + margin;

            info[i].format = PIXEL_FORMAT_YVU_PLANAR_420;
            info[i].global_alptha = 0xff;

            k230_vo_creat_layer(layer_conf->layer[i], &info[i]);
        }
    }

    return ret;
}

static int k230_display_connector_init()
{
    int ret = 0;
    int connector_fd;

    k_connector_info connector_info;

    memset(&connector_info, 0, sizeof(k_connector_info));

    ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("%s, get connector info failed.\n", __func__);
        return -1;
    }

    display_width = VICAP_ALIGN_UP(connector_info.resolution.hdisplay, 16);
    display_height = connector_info.resolution.vdisplay;

    printf("display_width:%u, display_height:%u\n", display_width, display_height);

    connector_fd = kd_mpi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return -1;
    }

    // set connect power
    kd_mpi_connector_power_set(connector_fd, K_TRUE);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

static void k230_display_enable(void)
{
    kd_mpi_vo_enable();
}

static void k230_display_layer_disable(k_vo_layer layer)
{
    kd_mpi_vo_disable_video_layer(layer);
}

static void k230_display_bind(k_s32 vicap_dev, k_s32 vicap_chn, k_s32 vo_chn)
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
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }

    return;
}

static void k230_display_unbind(k_s32 vicap_dev, k_s32 vicap_chn, k_s32 vo_chn)
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

static const char *get_camera_name(int sensor)
{
    return kd_mpi_vicap_get_sensor_string((k_vicap_sensor_type)sensor);
}

static k_pixel_format get_pixel_format(k_u32 pix_fmt)
{
    switch (pix_fmt)
    {
    case 0:
        return PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    case 1:
        return PIXEL_FORMAT_RGB_888;
    case 2:
        return PIXEL_FORMAT_RGB_888_PLANAR;
    case 3:
        return PIXEL_FORMAT_RGB_BAYER_10BPP;
    default:
        printf("unsupported pixel format %d, use dafault YUV_SEMIPLANAR_420\n", pix_fmt);
        break;
    }
    return PIXEL_FORMAT_YUV_SEMIPLANAR_420;
}

// argparse
static struct option long_options[] = {
    {"sensor", required_argument, NULL, 's'},
    {"chn",    required_argument, NULL, 'c'},
    {"ofmt",    required_argument, NULL, 'f'},
    {"width",     required_argument, NULL, 'W'},
    {"height",     required_argument, NULL, 'H'},
    {"work_mode",     required_argument, NULL, 'm'},
    {"help",   no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

static void usage(char * const argv[])
{
    printf("[sample_vicap]#\n");
    printf("Usage: %s -D 101 -m 0 -d 0 -s 24 -c 0 -f 0 -W 1920 -H 1080\n", argv[0]);
    printf("          -D:  vo(Display) connector device [0: hx8399, 101: HDMI-lt9611-1920x1080p60\tdefault 101\n");
    printf("          -m or --mode work mode, 0: online, 1: offline, multiple sensor will use offline mode\n");
    printf("          -d or --dev device num, 0, 1, 2, set 1 will enter multiple sensor test\n");
    printf("          -s\n");
    printf("               0: ov9732\n");
    printf("               1: ov9286 ir\n");
    printf("               2: ov9286 speckle\n");
    printf("               7: imx335 2LANE 1920Wx1080H\n");
    printf("               8: imx335 2LANE 2592Wx1944H\n");
    printf("               9: imx335 4LANE 2592Wx1944H\n");
    printf("               10: imx335 2LANE MCLK 7425 1920Wx1080H\n");
    printf("               11: imx335 2LANE MCLK 7425 2592Wx1944H\n");
    printf("               12: imx335 4LANE MCLK 7425 2592Wx1944H\n");
    printf("               24: OV5647 CSI0 1920X1080 30FPS\n");
    printf("          -c or --chn channel num, 0, 1, 2\n");
    printf("          -f or --ofmt out pixel format, 0: yuv420sp, 1: rgb888, 2: rgb888p, 3: raw\n");
    printf("          -w or --width output width\n");
    printf("          -h or --height output height\n");
    printf("          -h or --help will print this usage\n");
    return;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    const char *cam_name = NULL;
    char filename[64] = {0};
    uint32_t  frame_number = 0;

    int out_width = VICAP_ALIGN_UP(1080, 16);
    int out_height = 720;
    int out_channel = VICAP_CHN_ID_0;
    int out_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    int work_mode = 0;

    k230_display_layer_conf layer_conf;
    memset(&layer_conf, 0 , sizeof(k230_display_layer_conf));

    printf("opencv camera test, %s-%s enter\n", __DATE__, __TIME__);

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "D:s:c:f:W:H:m:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
        case 'D':
            connector_type =k_connector_type(atoi(optarg));
            printf("[opencv_camera]# display: %d\n", connector_type);
            break;
        case 's':
            cam_name = get_camera_name(atoi(optarg));
            printf("[opencv_camera]# device_name: %s\n", cam_name);
            break;
        case 'c':
            out_channel = (k_vicap_chn)atoi(optarg);
            printf("[opencv_camera]# chn: %d\n", out_channel);
            break;
        case 'f':
            out_format = get_pixel_format(atoi(optarg));
            printf("[opencv_camera]# pixel format: %d\n", out_format);
            break;
        case 'W':
            out_width = atoi(optarg);
            printf("[opencv_camera]# ori out width: %d, ", out_width);
            out_width = VICAP_ALIGN_UP(out_width, 16);
            printf("align out width: %d\n", out_width);
            break;
        case 'H':
            out_height = atoi(optarg);
            printf("[opencv_camera]# out_height: %d\n", out_height);
            break;
        case 'm':
            work_mode = atoi(optarg);
            printf("[opencv_camera]# work_mode: %d\n", work_mode);
            break;
        case 'h':
            usage(argv);
            return 0;
        default:
            fprintf(stderr, "Invalid option\n");
            break;
        }
    }

    ret = k230_display_connector_init();
    if (ret < 0) {
        printf("opencv camera test, display connector init failed\n");
        return ret;
    }

    Mat frame;
    VideoCapture cap;

    // open the camera using camera string
    if (cam_name == NULL) {
        cam_name = "cam-imx335-mode0";
    }
    cap.open(cam_name);
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cap.set(CAP_PROP_FRAME_WIDTH, out_width);
    cap.set(CAP_PROP_FRAME_HEIGHT, out_height);
    cap.set(CAP_PROP_FORMAT, out_format);
    cap.set(CAP_PROP_MODE, work_mode);
    cap.set(CAP_PROP_CHANNEL, out_channel);

    out_width = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    out_height = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    out_channel = (int)cap.get(CAP_PROP_CHANNEL);
    out_format = (int)cap.get(CAP_PROP_FORMAT);
    work_mode = (int)cap.get(CAP_PROP_MODE);

    printf("opencv camera test, out_width(%d)\n", out_width);
    printf("opencv camera test, out_height(%d)\n", out_height);
    printf("opencv camera test, out_channel(%d)\n", out_channel);
    printf("opencv camera test, out_format(%d)\n", out_format);
    printf("opencv camera test, work_mode(%d)\n", work_mode);

    k230_display_bind(VICAP_DEV_ID_0, out_channel, K_VO_DISPLAY_CHN_ID1);

    layer_conf.enable[0] = K_TRUE;
    layer_conf.width[0] = out_width;
    layer_conf.height[0] = out_height;
    layer_conf.rotation[0] = 0;
    layer_conf.layer[0] = K_VO_LAYER1;

    ret = k230_display_layer_init(&layer_conf, display_width, display_height);
    if (ret < 0) {
        printf("opencv camera test, display layer init failed\n");
        goto app_exit;
    }

    k230_display_enable();

    // wait for a new frame from camera and store it into 'frame'
    cap.read(frame);
    // check if we succeeded
    if (frame.empty()) {
        cerr << "ERROR! blank frame grabbed\n";
        goto app_exit;
    }

    char select = 0;
    for (;;)
    {
        if(select != '\n')
        {
            printf("---------------------------------------\n");
            printf(" Input character to select test option\n");
            printf("---------------------------------------\n");
            printf(" d: save frame\n");
            printf(" q: to exit\n");
            printf("---------------------------------------\n");
            printf("please Input:\n\n");
        }

        select = (char)getchar();
        switch (select)
        {
        case 'd':
            printf("opencv read & save frame\n");
            cap.read(frame);

            if (frame.empty()) {
                cerr << "ERROR! blank frame grabbed\n";
                goto app_exit;
            }
            snprintf(filename, sizeof(filename), "k230_opencv_camera_%dx%d_%04d.jpg", out_width, out_height, frame_number++);
            printf("save to camera frame to file:%s\n", filename);
            imwrite(filename, frame);
            break;
        case 'q':
            goto app_exit;
        }
    }

app_exit:
    k230_display_layer_disable(K_VO_LAYER1);

    k230_display_unbind(VICAP_DEV_ID_0, out_channel, K_VO_DISPLAY_CHN_ID1);

    printf("opencv camera test, %s-%s exit\n", __DATE__, __TIME__);
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

