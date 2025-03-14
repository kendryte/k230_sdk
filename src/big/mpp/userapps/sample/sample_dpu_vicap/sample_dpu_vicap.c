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
#include <sys/select.h>
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
#include "sample_dpu_vicap.h"

#include "vo_test_case.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#define VICAP_OUTPUT_BUF_NUM 30
#define VICAP_INPUT_BUF_NUM 4

// #define g_display_witdh  1920  //1088
// #define g_display_height 1080 //1920

#define	GPIO_DM_INPUT           _IOW('G', 1, int)
#define GPIO_READ_VALUE       	_IOW('G', 12, int)

#define IR_BUF_SIZE ((DMA_CHN0_WIDTH * DMA_CHN0_WIDTH *3/2 + 0x3ff) & ~0x3ff)//(5 * 1024 * 1024)

extern k_dpu_dev_attr_t dpu_dev_attr;

k_u32 g_display_witdh = 0;
k_u32 g_display_height = 0;

typedef struct kd_pin_mode
{
    unsigned short pin;     /* pin number, from 0 to 63 */
    unsigned short mode;    /* pin level status, 0 low level, 1 high level */
} pin_mode_t;

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

    k_vicap_chn chn_num[VICAP_CHN_ID_MAX];

    k_bool chn_enable[VICAP_CHN_ID_MAX];
    k_pixel_format out_format[VICAP_CHN_ID_MAX];
    k_bool crop_enable[VICAP_CHN_ID_MAX];

    k_vicap_window out_win[VICAP_CHN_ID_MAX];

    k_u32 buf_size[VICAP_CHN_ID_MAX];

    k_video_frame_info dump_info[VICAP_CHN_ID_MAX];

    k_bool preview[VICAP_CHN_ID_MAX];
    k_u16 rotation[VICAP_CHN_ID_MAX];
    k_u8 fps[VICAP_CHN_ID_MAX];
    k_bool dw_enable;
} vicap_device_obj;

#define MAX_VO_LAYER_NUM 2

typedef struct {
    k_u16 width[MAX_VO_LAYER_NUM];
    k_u16 height[MAX_VO_LAYER_NUM];
    k_u16 rotation[MAX_VO_LAYER_NUM];
    k_vo_layer layer[MAX_VO_LAYER_NUM];
    k_bool enable[MAX_VO_LAYER_NUM];
} k_vicap_vo_layer_conf;

static vicap_device_obj device_obj[VICAP_DEV_ID_MAX];
static k_u32 dump_count = 0;
static pthread_t output_tid = 0;
static pthread_t gpio_tid = 0;
static pthread_t adc_tid = 0;
static k_bool exiting = K_FALSE;
static k_bool dump_exited = K_FALSE;
static k_bool adc_exited = K_FALSE;
static k_bool gpio_exited = K_FALSE;
static k_video_frame_info rgb_buf[VICAP_OUTPUT_BUF_NUM];
// static k_video_frame_info ir_buf[VICAP_OUTPUT_BUF_NUM];
static k_u32 rgb_wp=0;
static k_u32 rgb_rp=0;
static k_u32 rgb_total_cnt=0;
static char OUT_PATH[50] = {"\0"};
static int rgb_dev;
static int rgb_chn;
static k_vb_blk_handle ir_handle;
static k_u64 ir_phys_addr;
static k_u8 *ir_virt_addr;
static k_bool gen_calibration = K_FALSE;
static k_bool adc_en = K_FALSE;
static k_u32 delay_ms=3000;
int gpio_fd = -1;
pin_mode_t gpio;
#if ENABLE_CDC   //USB in big core
static int fd_usb = -1;
#endif

static k_u32 sample_vicap_vo_init(k_connector_type type)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type = type;//HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
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
    kd_mpi_connector_power_set(connector_fd, K_TRUE);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

static k_s32 sample_vicap_vo_layer_init(k_vicap_vo_layer_conf *layer_conf)
{
    k_s32 ret = 0;
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
            margin = ((g_display_height - total_height) / (i+2));
            if ((total_height > g_display_height) || (info[i].act_size.width > g_display_witdh)) {
                printf("%s, the preview window size[%dx%d] exceeds the display window size[%dx%d].\n", \
                    __func__, info[i].act_size.width, total_height, g_display_witdh, g_display_height);
                return -1;
            }
            printf("%s, width(%d), height(%d), margin(%d), total_height(%d)\n", \
                __func__, info[i].act_size.width, info[i].act_size.height, margin, total_height);
        }
    }

    for (int i = 0; i < MAX_VO_LAYER_NUM; i++) {
        if (layer_conf->enable[i]) {
            info[i].offset.x = (g_display_witdh - info[i].act_size.width)/2;
            info[i].offset.y = margin + relative_height;
            printf("%s, layer(%d), offset.x(%d), offset.y(%d), relative_height(%d)\n", __func__, layer_conf->layer[i], info[i].offset.x, info[i].offset.y, relative_height);
            relative_height += info[i].act_size.height + margin;

            info[i].format = PIXEL_FORMAT_YVU_PLANAR_420;
            info[i].global_alptha = 0xff;

            vo_creat_layer_test(layer_conf->layer[i], &info[i]);
        }
    }

    return ret;
}


static void sample_vicap_disable_vo_layer(k_vo_layer layer)
{
    kd_mpi_vo_disable_video_layer(layer);
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

        if (dev_obj[i].mode == VICAP_WORK_OFFLINE_MODE) {
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
            printf("%s, dev(%d) pool(%d) buf_size(%d) dw_enable\n", __func__, i, k ,dev_obj[i].buf_size[0]);
            k++;
        }
    }

    /* dma vb init */
    config.comm_pool[k].blk_cnt = 3;
    config.comm_pool[k].blk_size = 1280 * 720;
    config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;

    config.comm_pool[k+1].blk_cnt = 3;
    config.comm_pool[k+1].blk_size = 1280 * 720 * 3;
    config.comm_pool[k+1].mode = VB_REMAP_MODE_NOCACHE;

    /* dpu vb init */
    config.comm_pool[k+2].blk_cnt = (3);
    config.comm_pool[k+2].blk_size = 5 * 1024 * 1024;
    config.comm_pool[k+2].mode = VB_REMAP_MODE_NOCACHE;

    //ir
    config.comm_pool[k+3].blk_cnt = (3);
    config.comm_pool[k+3].blk_size = IR_BUF_SIZE;//5 * 1024 * 1024;
    config.comm_pool[k+3].mode = VB_REMAP_MODE_NOCACHE;

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

    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;


    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, config.comm_pool[k+3].blk_size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return -1;
    }
    ir_handle  = handle;

    pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if (pool_id == VB_INVALID_POOLID)
    {
        printf("%s get pool id error\n", __func__);
        return -1;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s get phys addr error\n", __func__);
        return -1;
    }

    ir_phys_addr = phys_addr;

    printf("%s>phys_addr 0x%lx, blk_size %ld\n", __func__, phys_addr, config.comm_pool[k+3].blk_size);
    ir_virt_addr = (k_u8 *)kd_mpi_sys_mmap_cached(phys_addr, config.comm_pool[k+3].blk_size);
    if (ir_virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return -1;
    }
    printf("%s>ir_virt_addr_Y %p\n", __func__, ir_virt_addr);

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

static void vb_exit() {
    kd_mpi_vb_exit();
}

static void dpu_dump()
{
    k_video_frame_info dump_info;
    k_video_frame_info dma_get_info;
    k_dpu_chn_result_u lcn_result;
    k_dpu_chn_result_u ir_result;
    k_s32 ret;
    void *dpu_virt_addr = NULL;
    k_bool get_ir = K_FALSE;
#if ENABLE_CDC
    k_u32 remain_size = 0;
    void* wp = NULL;
#endif

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            if(!gen_calibration)
                if(dpu_dev_attr.dev_param.spp.flag_align)
                    get_ir = K_TRUE;

            //printf("sample_vicap, dev(%d) chn(%d) dump frame.\n", dev_num, chn_num);
            if(device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_YUV_SEMIPLANAR_420)
            {
                memset(&dump_info, 0 , sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(dev_num, chn_num, VICAP_DUMP_YUV, &dump_info, 1000);
                if (ret) {
                    printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                }
                printf("dump_info pts: %ld\n", dump_info.v_frame.pts);
                //printf("addr: [%lx, %lx, %lx]\n", dump_info.v_frame.phys_addr[0], dump_info.v_frame.phys_addr[1], dump_info.v_frame.phys_addr[2]);

                if(gen_calibration)
                {
                    ret = kd_mpi_dma_send_frame(dev_num, &dump_info, -1);
                    if (ret != K_SUCCESS)
                    {
                        printf("send frame error\r\n");
                    }

                    ret = kd_mpi_dma_get_frame(dev_num, &dma_get_info, -1);
                    if (ret != K_SUCCESS)
                    {
                        printf("get frame error\r\n");
                    }

                    /* save vicap ir file */
                    // void *dpu_virt_addr = NULL;
                    k_char vicap_ir_filename[256];

                    memset(vicap_ir_filename, 0 , sizeof(vicap_ir_filename));
                    sprintf(vicap_ir_filename, "%svicap_ir_%04d.yuv", OUT_PATH, dump_count);

                    dpu_virt_addr = kd_mpi_sys_mmap(dma_get_info.v_frame.phys_addr[0], dma_get_info.v_frame.width* dma_get_info.v_frame.height*3/2);
                    if (dpu_virt_addr) {
                        FILE *file = fopen(vicap_ir_filename, "wb+");
                        if (file) {
                            fwrite(dpu_virt_addr, 1, dma_get_info.v_frame.width* dma_get_info.v_frame.height*3/2, file);
                            fclose(file);
                            printf("save %s success\n", vicap_ir_filename);
                        } else {
                            printf("can't create file\n");
                        }
                    } else {
                        printf("save ir out failed\n");
                    }

                    kd_mpi_sys_munmap(dpu_virt_addr, dma_get_info.v_frame.width* dma_get_info.v_frame.height*3/2);

                    kd_mpi_dma_release_frame(dev_num, &dma_get_info);

                }
                else
                {
                    printf("lcn dma\n");
                    ret = kd_mpi_dma_send_frame(dev_num, &dump_info, -1);
                    if (ret != K_SUCCESS)
                    {
                        printf("send frame error\r\n");
                    }

                    ret = kd_mpi_dma_get_frame(dev_num, &dma_get_info, -1);
                    if (ret != K_SUCCESS)
                    {
                        printf("get frame error\r\n");
                    }
                    //printf("dma addr:%lx\n", dma_get_info.v_frame.phys_addr[0]);

                    printf("lcn dpu\n");
                    ret = kd_mpi_dpu_send_frame(0, dma_get_info.v_frame.phys_addr[0], 200);
                    if (ret) {
                        printf("kd_mpi_dpu_send_frame lcn failed: %d\n", ret);
                    }

                    if(get_ir)
                    {
                        printf("ir dpu\n");

                        ret = kd_mpi_dpu_send_frame(1, ir_phys_addr, 200);

                        if (ret) {
                            printf("kd_mpi_dpu_send_frame ir failed: %d\n", ret);
                        }
                    }

                    ret = kd_mpi_dpu_get_frame(0, &lcn_result, 200);
                    if (ret) {
                        printf("kd_mpi_dpu_get_frame failed: %d\n", ret);
                    }

#if ENABLE_CDC
                    remain_size = lcn_result.lcn_result.depth_out.length;
                    dpu_virt_addr = kd_mpi_sys_mmap(lcn_result.lcn_result.depth_out.depth_phys_addr, lcn_result.lcn_result.depth_out.length);
                    wp = dpu_virt_addr;
                    while(remain_size)
                    {
                        if(remain_size >= 1024)
                        {
                            write(fd_usb, wp, 1024);
                            wp += 1024;
                            remain_size -= 1024;
                        }
                        else
                        {
                            write(fd_usb, wp, remain_size);
                            wp = NULL;
                            remain_size = 0;
                        }
                    }
                    kd_mpi_sys_munmap(dpu_virt_addr, lcn_result.lcn_result.depth_out.length);
#else
                    /* save file */
                    // void *dpu_virt_addr = NULL;
                    k_char dpu_filename[256];

                    memset(dpu_filename, 0 , sizeof(dpu_filename));
                    snprintf(dpu_filename, sizeof(dpu_filename), "%sdepth_%04d.bin", OUT_PATH, dump_count);

                    dpu_virt_addr = kd_mpi_sys_mmap(lcn_result.lcn_result.depth_out.depth_phys_addr, lcn_result.lcn_result.depth_out.length);
                    if (dpu_virt_addr) {
                        FILE *file = fopen(dpu_filename, "wb+");
                        if (file) {
                            fwrite(dpu_virt_addr, 1, lcn_result.lcn_result.depth_out.length, file);
                            fclose(file);
                            printf("save %s success, pts %ld\n", dpu_filename, dump_info.v_frame.pts);
                        } else {
                            printf("can't create file\n");
                        }
                    } else {
                        printf("save depthout failed\n");
                    }

                    kd_mpi_sys_munmap(dpu_virt_addr, lcn_result.lcn_result.depth_out.length);
#endif

                    kd_mpi_dma_release_frame(dev_num, &dma_get_info);
                    //printf("depth addr:%lx\n", lcn_result.lcn_result.depth_out.depth_phys_addr);

                    kd_mpi_dpu_release_frame();

                    if(get_ir)
                    {
                        ret = kd_mpi_dpu_get_frame(1, &ir_result, 200);
                        if (ret) {
                            printf("kd_mpi_dpu_get_frame ir failed: %d\n", ret);
                        }

                        kd_mpi_dpu_release_frame();


                        get_ir = K_FALSE;
                    }

                    // kd_mpi_dpu_release_frame();
                }
            }

            if(device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_RGB_888 ||
                device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_RGB_888_PLANAR)
            {
                k_u32 data_size = 0;
                void *virt_addr = NULL;

                kd_mpi_vicap_3d_mode_crtl(K_FALSE);
                usleep(delay_ms);
                memset(&dump_info, 0 , sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(dev_num, chn_num, VICAP_DUMP_RGB, &dump_info, 1000);
                if (ret) {
                    printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                }
                usleep(delay_ms);
                kd_mpi_vicap_3d_mode_crtl(K_TRUE);

                //printf("addr: [%lx, %lx, %lx]\n", dump_info.v_frame.phys_addr[0], dump_info.v_frame.phys_addr[1], dump_info.v_frame.phys_addr[2]);

                if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_YUV_SEMIPLANAR_420) {
                    data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3 /2;
                } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_888) {
                    data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3;
                }  else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_888_PLANAR) {
                    data_size = dump_info.v_frame.width * dump_info.v_frame.height * 3;
                } else if (dump_info.v_frame.pixel_format == PIXEL_FORMAT_RGB_BAYER_10BPP) {
                    data_size = dump_info.v_frame.width * dump_info.v_frame.height * 2;
                } else {
                }

                ret = kd_mpi_dma_send_frame(dev_num, &dump_info, -1);
                if (ret != K_SUCCESS)
                {
                    printf("send frame error\r\n");
                }

                ret = kd_mpi_dma_get_frame(dev_num, &dma_get_info, -1);
                if (ret != K_SUCCESS)
                {
                    printf("get frame error\r\n");
                }

#if ENABLE_CDC
                remain_size = data_size;
                printf("data size %d\n", data_size);
                virt_addr = kd_mpi_sys_mmap(dma_get_info.v_frame.phys_addr[0], data_size);
                wp = virt_addr;
                int block = 1024;
                while(remain_size)
                {
                    if(remain_size >= block)
                    {
                        write(fd_usb, wp, block);
                        wp += block;
                        remain_size -= block;
                    }
                    else
                    {
                        write(fd_usb, wp, remain_size);
                        wp = NULL;
                        remain_size = 0;
                    }
                    usleep(5000);
                }
                kd_mpi_sys_munmap(virt_addr, data_size);
#else
                k_char filename[256];
                virt_addr = kd_mpi_sys_mmap(dma_get_info.v_frame.phys_addr[0], data_size);
                if (virt_addr) {
                    memset(filename, 0 , sizeof(filename));

                    snprintf(filename, sizeof(filename), "%spic_%04d.rgb", OUT_PATH, dump_count);

                    printf("save %s success, pts %ld\n", filename, dump_info.v_frame.pts);
                    FILE *file = fopen(filename, "wb+");
                    if (file) {
                        fwrite(virt_addr, 1, data_size, file);
                        fclose(file);
                    } else {
                        printf("sample_vicap, open dump file failed(%s)\n", strerror(errno));
                    }
                } else {
                    printf("sample_vicap, map dump addr failed.\n");
                }

                kd_mpi_sys_munmap(virt_addr, data_size);
#endif
                kd_mpi_dma_release_frame(dev_num, &dma_get_info);
            }

            //printf("sample_vicap, release dev(%d) chn(%d) dump frame.\n", dev_num, chn_num);

            ret = kd_mpi_vicap_dump_release(dev_num, chn_num, &dump_info);
            if (ret) {
                printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
            }
        }
    }
    dump_count++;
}


static void dpu_continue_dump()
{
    k_video_frame_info dump_info;
    k_video_frame_info dma_get_info;
    k_dpu_chn_result_u lcn_result;
    k_dpu_chn_result_u ir_result;
    k_bool get_ir = K_FALSE;
    k_s32 ret;

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            if(dpu_dev_attr.dev_param.spp.flag_align)
                get_ir = K_TRUE;

            if(device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_RGB_888 ||
                device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_RGB_888_PLANAR)
            {
                kd_mpi_vicap_3d_mode_crtl(K_FALSE);
                usleep(delay_ms);
                memset(&rgb_buf[rgb_wp], 0 , sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(dev_num, chn_num, VICAP_DUMP_RGB, &rgb_buf[rgb_wp], 1000);
                usleep(delay_ms);
                kd_mpi_vicap_3d_mode_crtl(K_TRUE);
                if (ret) {
                    printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                }
                rgb_wp++;
                rgb_wp %= VICAP_OUTPUT_BUF_NUM;
                if (rgb_wp == rgb_rp)
                {
                    printf("rgb buffer overflow\n");
                }

                rgb_dev = dev_num;
                rgb_chn = chn_num;
                rgb_total_cnt++;
            }
        }
    }

    if(rgb_total_cnt % 3 != 0)
        return;

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            //printf("sample_vicap, dev(%d) chn(%d) dump frame.\n", dev_num, chn_num);

            if(device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_YUV_SEMIPLANAR_420)
            {
                memset(&dump_info, 0 , sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(dev_num, chn_num, VICAP_DUMP_YUV, &dump_info, 1000);
                if (ret) {
                    printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                }

                //printf("addr: [%lx, %lx, %lx]\n", dump_info.v_frame.phys_addr[0], dump_info.v_frame.phys_addr[1], dump_info.v_frame.phys_addr[2]);

                ret = kd_mpi_dma_send_frame(dev_num, &dump_info, -1);
                if (ret != K_SUCCESS)
                {
                    printf("send frame error\r\n");
                }

                ret = kd_mpi_dma_get_frame(dev_num, &dma_get_info, -1);
                if (ret != K_SUCCESS)
                {
                    printf("get frame error\r\n");
                }
                //printf("dma addr:%lx\n", dma_get_info.v_frame.phys_addr[0]);

                ret = kd_mpi_dpu_send_frame(0, dma_get_info.v_frame.phys_addr[0], 200);
                if (ret) {
                    printf("kd_mpi_dpu_send_frame lcn failed\n");
                }

                if(get_ir)
                {
                    ret = kd_mpi_dpu_send_frame(1, ir_phys_addr, 200);
                    if (ret) {
                        printf("kd_mpi_dpu_send_frame lcn failed\n");
                    }
                }

                ret = kd_mpi_dpu_get_frame(0, &lcn_result, 200);
                if (ret) {
                    printf("kd_mpi_dpu_get_frame failed\n");
                }

                if(get_ir)
                {
                    ret = kd_mpi_dpu_get_frame(1, &ir_result, 200);
                    if (ret) {
                        printf("kd_mpi_dpu_get_frame failed\n");
                    }
                }

                /* save depth file */
                void *dpu_virt_addr = NULL;
                k_char dpu_filename[256];

                memset(dpu_filename, 0 , sizeof(dpu_filename));
                sprintf(dpu_filename, "%sdepth_%04d.bin", OUT_PATH, dump_count);

                dpu_virt_addr = kd_mpi_sys_mmap(lcn_result.lcn_result.depth_out.depth_phys_addr, lcn_result.lcn_result.depth_out.length);
                if (dpu_virt_addr) {
                    FILE *file = fopen(dpu_filename, "wb+");
                    if (file) {
                        fwrite(dpu_virt_addr, 1, lcn_result.lcn_result.depth_out.length, file);
                        fclose(file);
                        printf("save %s success\n", dpu_filename);
                    } else {
                        printf("can't create file\n");
                    }
                } else {
                    printf("save depthout failed\n");
                }

                kd_mpi_dma_release_frame(dev_num, &dma_get_info);
                //printf("depth addr:%lx\n", lcn_result.lcn_result.depth_out.depth_phys_addr);

                kd_mpi_dpu_release_frame();

                kd_mpi_sys_munmap(dpu_virt_addr, lcn_result.lcn_result.depth_out.length);

                //printf("sample_vicap, release dev(%d) chn(%d) dump frame.\n", dev_num, chn_num);

                ret = kd_mpi_vicap_dump_release(dev_num, chn_num, &dump_info);
                if (ret) {
                    printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", dev_num, chn_num);
                }

                while(rgb_wp != rgb_rp)
                {
                    k_u64 delta;
                    k_u32 temp;

                    delta = dump_info.v_frame.pts - rgb_buf[rgb_rp].v_frame.pts;
                    temp = (rgb_rp + 1) % VICAP_OUTPUT_BUF_NUM;
                    if((rgb_buf[rgb_rp].v_frame.pts >= dump_info.v_frame.pts) ||
                        (delta < 35000) ||
                        (temp == rgb_wp))
                    {
                        k_u32 data_size = 0;
                        void *virt_addr = NULL;
                        k_char filename[256];

                        data_size = rgb_buf[rgb_rp].v_frame.width * rgb_buf[rgb_rp].v_frame.height * 3;

                        ret = kd_mpi_dma_send_frame(rgb_dev, &rgb_buf[rgb_rp], -1);
                        if (ret != K_SUCCESS)
                        {
                            printf("send frame error\r\n");
                        }

                        ret = kd_mpi_dma_get_frame(rgb_dev, &dma_get_info, -1);
                        if (ret != K_SUCCESS)
                        {
                            printf("get frame error\r\n");
                        }

                        virt_addr = kd_mpi_sys_mmap(dma_get_info.v_frame.phys_addr[0], data_size);
                        if (virt_addr) {
                            memset(filename, 0 , sizeof(filename));

                            sprintf(filename, "%spic_%04d.rgb", OUT_PATH, dump_count);

                            printf("save %s success, pts delta %ldms\n", filename, delta/1000);
                            FILE *file = fopen(filename, "wb+");
                            if (file) {
                                fwrite(virt_addr, 1, data_size, file);
                                fclose(file);
                            } else {
                                printf("sample_vicap, open dump file failed(%s)\n", strerror(errno));
                            }
                        } else {
                            printf("sample_vicap, map dump addr failed.\n");
                        }

                        kd_mpi_sys_munmap(virt_addr, data_size);

                        kd_mpi_dma_release_frame(DMA_CHN1, &dma_get_info);
                    }

                    ret = kd_mpi_vicap_dump_release(rgb_dev, rgb_chn, &rgb_buf[rgb_rp]);
                    if (ret) {
                        printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", rgb_dev, rgb_chn);
                    }
                    rgb_rp++;
                    rgb_rp %= VICAP_OUTPUT_BUF_NUM;
                }
            }
        }
    }
    dump_count++;
}

static void *dump_thread(void *arg)
{
    k_s32 ret;

    while(!exiting)
    {
        dpu_continue_dump();
        usleep(10000);
    }

    while(rgb_wp != rgb_rp)
    {
        ret = kd_mpi_vicap_dump_release(rgb_dev, rgb_chn, &rgb_buf[rgb_rp]);
        if (ret) {
            printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", rgb_dev, rgb_chn);
        }
        rgb_rp++;
        rgb_rp %= VICAP_OUTPUT_BUF_NUM;
    }
    dump_exited = K_TRUE;
    return arg;
}

static void *adc_thread(void *arg)
{
    printf("%s\n", __FUNCTION__);
    float temp = -300.0;//初始变量建议定义到正常范围之外，如果没有正常赋值，不会影响计算。
	float old_temp = -300.0;
	// char input = 'a';
	while (!exiting)
	{
		if (!sample_adc(&temp))
		{
			//printf("%f\n", temp - old_temp);
			if (temp - old_temp > 5 || temp - old_temp < -5)
			{
				sample_dv_dpu_update_temp(temp);
				old_temp = temp;
			}
		}

		fd_set rfds;
		struct timeval tv;
		int retval;

		FD_ZERO(&rfds);
		FD_SET(0, &rfds); // 监视标准输入流

		tv.tv_sec = 5; // 设置等待时间为5秒
		tv.tv_usec = 0;

		retval = select(1, &rfds, NULL, NULL, &tv);
		if (retval == -1) {
			perror("select()");
		}
		else if (retval > 0) {
			FD_ISSET(0, &rfds);
		}
		else {
			printf("Timeout reached\n");
		}

		//k_u32 display_ms = 1000;//get temperature per 1s
		//usleep(1000 * display_ms);
	}
    adc_exited = K_TRUE;
    printf("adc exit\n");

    return arg;
}

static void *gpio_thread(void *arg)
{
    while(!exiting)
    {
        ioctl(gpio_fd, GPIO_READ_VALUE, &gpio);
        if(gpio.mode == 0)
        {
            dpu_dump();
            usleep(1000000);
        }
        usleep(30000);
    }
    gpio_exited = K_TRUE;
    return arg;
}

#define VICAP_MIN_PARAMETERS (7)

static void usage(void)
{
    printf("usage: ./sample_dpu_vicap.elf -mode 1 -dev 0 -sensor 2 -chn 0 -preview 1 -rotation 1 -dev 1 -sensor 1 -chn 1 -preview 0 -ofmt 2\n");
    printf("Options:\n");
    printf(" -o             output path, default is /sharefs/\n");
    printf(" -mode:         vicap work mode[0: online mode, 1: offline mode. only offline mode support multiple sensor input]\tdefault 0\n");
    printf(" -vo:           video output, 0: HX8377, 101: LT9611(hdmi)");
    printf(" -cal:          1: for calibration, set it before -dev\n");
    printf(" -gpio:         gpio number, set it before -dev\n");
    printf(" -delay:        delay time ms, set it before -dev\n");
    printf(" -dev:          vicap device id[0,1,2]\tdefault 0\n");
    printf(" -sensor:       sensor type[see K230_Camera_Sensor_Adaptation_Guide.md]\n");
    printf(" -ae:           ae status[0: disable AE, 1: enable AE]\tdefault enable\n");
    printf(" -awb:          awb status[0: disable AWB, 1: enable AWb]\tdefault enable\n");
    printf(" -chn:          vicap output channel id[0,1,2]\tdefault 0\n");
    printf(" -ow:           the output image width, default same with input width\n");
    printf(" -oh:           the output image height, default same with input height\n");
    printf(" -ox:           the output image start position of x\n");
    printf(" -oy:           the output image start position of y\n");
    printf(" -crop:         crop enable[0: disable, 1: enable]\n");
    printf(" -ofmt:         the output pixel format[0: yuv, 1: rgb888, 2: rgb888p, 3: raw], only channel 0 support raw data, default yuv\n");
    printf(" -preview:      the output preview enable[0: disable, 1: enable], only support 2 output channel preview\n");
    printf(" -rotation:     display rotaion[0: degree 0, 1: degree 90, 2: degree 180, 3: degree 270, 4: unsupport rotaion]\n");
    printf(" -gdma:         gdma rotaion[0: degree 0, 1: degree 90, 2: degree 180, 3: degree 270, 4: unsupport rotaion]\n");
    printf(" -help:         print this help\n");

    exit(1);
}

int main(int argc, char *argv[])
{
    k_s32 ret = 0;

    k_u32 work_mode = VICAP_WORK_ONLINE_MODE;
    k_gdma_rotation_e dma_rotation[2] = {DEGREE_0, DEGREE_0};

    strcpy(OUT_PATH, "/sharefs/");

    memset(&device_obj, 0 , sizeof(device_obj));

    k_vicap_vo_layer_conf layer_conf;
    memset(&layer_conf, 0 , sizeof(k_vicap_vo_layer_conf));

    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;

    k_u8 dev_count = 0, cur_dev = 0;
    k_u8 chn_count = 0, cur_chn = 0;
    k_u8 vo_count = 0, preview_count = 0;
    k_connector_type connector_type = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;

    k_u32 pipe_ctrl = 0xFFFFFFFF;
    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));

    gpio.pin = 30;

    if (argc < VICAP_MIN_PARAMETERS) {
        printf("sample_vicap requires some necessary parameters:\n");
        usage();
    }

#if ENABLE_CDC
    fd_usb = open("/dev/ttyUSB1", O_RDWR);
#endif
    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            usage();
            return 0;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if ((i + 1) >= argc) {
                printf("output path parameters missing.\n");
                return -1;
            }
            memset(OUT_PATH, 0 , sizeof(OUT_PATH));
            strcpy(OUT_PATH, argv[i + 1]);
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
            } else {
                printf("unsupport mode.\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-vo") == 0)
        {
            connector_type = atoi(argv[i + 1]);
            if(connector_type == LT9611_MIPI_4LAN_1920X1080_60FPS)
            {
                g_display_witdh = 1920;
                g_display_height  = 1080;
            }
            else
            {
                g_display_witdh = 1088;
                g_display_height  = 1920;
            }
            printf("set connector_type = %d\n", connector_type);
        }
        else if (strcmp(argv[i], "-adc") == 0)
        {
            adc_en = atoi(argv[i + 1]);
            printf("set adc_en = %d\n", adc_en);
        }
        else if (strcmp(argv[i], "-cal") == 0)
        {
            gen_calibration = atoi(argv[i + 1]);
            printf("set gen_calibration = %d\n", gen_calibration);
        }
        else if (strcmp(argv[i], "-gpio") == 0)
        {
            gpio.pin = atoi(argv[i + 1]);
            printf("set gpio = %d\n", gpio.pin);
        }
        else if (strcmp(argv[i], "-delay") == 0)
        {
            delay_ms = atoi(argv[i + 1]);
            delay_ms *= 1000;
            printf("set delay_ms = %d\n", delay_ms);
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
                        else if (strcmp(argv[i], "-gdma") == 0)
                        {
                            k_u16 rotation = atoi(argv[i + 1]);
                            dma_rotation[cur_dev] = (k_gdma_rotation_e)rotation;
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
                    device_obj[cur_dev].sensor_type = atoi(argv[i + 1]);
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

    sample_vicap_vo_init(connector_type);

    printf("sample_vicap ...kd_mpi_vicap_get_sensor_info\n");

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

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
        printf("sample_vicap, dev[%d] in size[%dx%d]\n", \
            dev_num, device_obj[dev_num].in_width, device_obj[dev_num].in_height);

        //vicap device attr set
        dev_attr.acq_win.h_start = 0;
        dev_attr.acq_win.v_start = 0;
        dev_attr.acq_win.width = device_obj[dev_num].in_width;
        dev_attr.acq_win.height = device_obj[dev_num].in_height;
        if ((work_mode == VICAP_WORK_OFFLINE_MODE) || (work_mode == VICAP_WORK_LOAD_IMAGE_MODE)) {
            dev_attr.mode = work_mode;
            dev_attr.buffer_num = VICAP_INPUT_BUF_NUM;
            dev_attr.buffer_size = VICAP_ALIGN_UP((device_obj[dev_num].in_width * device_obj[dev_num].in_height * 2), VICAP_ALIGN_1K);
            device_obj[dev_num].in_size = dev_attr.buffer_size;
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
        dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
        dev_attr.pipe_ctrl.bits.ae_enable = device_obj[dev_num].ae_enable;
        dev_attr.pipe_ctrl.bits.awb_enable = device_obj[dev_num].awb_enable;
        dev_attr.pipe_ctrl.bits.dnr3_enable = device_obj[dev_num].dnr3_enable;

        dev_attr.cpature_frame = 0;
        dev_attr.dw_enable = device_obj[dev_num].dw_enable;

        ret = kd_mpi_vicap_set_dev_attr(dev_num, dev_attr);
        if (ret) {
            printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
            return ret;
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

            if ((device_obj[dev_num].out_win[chn_num].width > g_display_witdh)
                && (device_obj[dev_num].out_win[chn_num].height > g_display_height)) {
                device_obj[dev_num].preview[chn_num] = K_FALSE;
            }

            if (!device_obj[dev_num].rotation[chn_num]
                && ((device_obj[dev_num].out_win[chn_num].width > g_display_witdh)
                && (device_obj[dev_num].out_win[chn_num].width < g_display_height))) {
                device_obj[dev_num].rotation[chn_num] = 1;
            }

            printf("sample_vicap, dev_num(%d), chn_num(%d), in_size[%dx%d], out_offset[%d:%d], out_size[%dx%d]\n", \
                dev_num, chn_num, device_obj[dev_num].in_width, device_obj[dev_num].in_height, \
                device_obj[dev_num].out_win[chn_num].h_start, device_obj[dev_num].out_win[chn_num].v_start, \
                device_obj[dev_num].out_win[chn_num].width, device_obj[dev_num].out_win[chn_num].height);
        }
    }

    ret = sample_vicap_vb_init(device_obj);
    if (ret) {
        printf("sample_vicap_vb_init failed\n");
        return -1;
    }
    atexit(vb_exit);

    ret = sample_dv_dpu_init();
    if (ret) {
        printf("sample_dpu_init failed\n");
        return -1;
    }

    ret = sample_dv_dma_init(dma_rotation, gen_calibration);
    if (ret) {
        printf("sample_dma_init failed\n");
        return -1;
    }

    //vicap channel attr set
    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!device_obj[dev_num].chn_enable[chn_num])
                continue;

            memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
            if (device_obj[dev_num].out_format[chn_num] == PIXEL_FORMAT_RGB_BAYER_10BPP) {
                chn_attr.out_win.width = device_obj[dev_num].in_width;
                chn_attr.out_win.height = device_obj[dev_num].in_height;
            } else {
                chn_attr.out_win.width = device_obj[dev_num].out_win[chn_num].width;
                chn_attr.out_win.height = device_obj[dev_num].out_win[chn_num].height;
            }

            if (device_obj[dev_num].crop_enable[chn_num]) {
                chn_attr.crop_win = chn_attr.out_win;
                chn_attr.crop_win.h_start = device_obj[dev_num].out_win[chn_num].h_start;
                chn_attr.crop_win.v_start = device_obj[dev_num].out_win[chn_num].v_start;
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

            printf("sample_vicap, set dev(%d) chn(%d) attr, buffer_size(%d), out size[%dx%d]\n", \
                dev_num, chn_num, chn_attr.buffer_size, chn_attr.out_win.width, chn_attr.out_win.height);
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
                if (vo_count == 0) {
                    vo_chn = K_VO_DISPLAY_CHN_ID1;
                    layer = K_VO_LAYER1;
                    rotation = device_obj[dev_num].rotation[chn_num];
                } else if (vo_count == 1) {
                    vo_chn = K_VO_DISPLAY_CHN_ID2;
                    layer = K_VO_LAYER2;
                    rotation = 4;//layer2 unsupport roation
                } else if (vo_count >= MAX_VO_LAYER_NUM){
                    printf("only support bind two vo channel.\n");
                    continue;
                }
                printf("sample_vicap, vo_count(%d), dev(%d) chn(%d) bind vo chn(%d) layer(%d) rotation(%d)\n", vo_count, dev_num, chn_num, vo_chn, layer, rotation);
                sample_vicap_bind_vo(dev_num, chn_num, vo_chn);

                layer_conf.enable[vo_count] = K_TRUE;
                layer_conf.width[vo_count] = chn_attr.out_win.width;
                layer_conf.height[vo_count] = chn_attr.out_win.height;
                layer_conf.rotation[vo_count] = rotation;
                layer_conf.layer[vo_count] = layer;
                vo_count++;
            }
        }
    }

    ret = sample_vicap_vo_layer_init(&layer_conf);
    if (ret) {
        printf("sample_vicap, vo layer init failed.\n");
        goto vb_exit;
    }
    printf("sample_vicap, vo layer init ok.\n");

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

    k_isp_ae_roi ae_roi;
    memset(&ae_roi, 0, sizeof(k_isp_ae_roi));

    k_char select = 0;

    if(adc_en){
        pthread_create(&adc_tid, NULL, adc_thread, NULL);
    }

    gpio_fd = open("/dev/gpio", O_RDWR);
    if (gpio_fd < 0)
    {
        perror("open /dev/pin err\n");
    }
    ret = ioctl(gpio_fd, GPIO_DM_INPUT, &gpio);  //pin33 input
    if (ret)
    {
        perror("ioctl /dev/pin err\n");
    }

    pthread_create(&gpio_tid, NULL, gpio_thread, NULL);

    while(K_TRUE)
    {
        if(select != '\n')
        {
            printf("---------------------------------------\n");
            printf(" Input character to select test option\n");
            printf("---------------------------------------\n");
            printf(" d: dump single depth\n");
            printf(" c: dump multi depth\n");
            printf(" q: to exit\n");
            printf("---------------------------------------\n");
            printf("please Input:\n\n");
        }
        select = (k_char)getchar();
        switch (select)
        {
        case 'd':
            printf("sample_dpu_vicap... dump frame.\n");
            dpu_dump();
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
        case 'c':
            pthread_create(&output_tid, NULL, dump_thread, NULL);
            break;
        case 'q':
            if(output_tid || adc_tid || gpio_tid)
            {
                exiting = K_TRUE;
                while((output_tid && !dump_exited) ||
                      (adc_tid && !adc_exited) ||
                      (gpio_tid && !gpio_exited) )
                {
                    usleep(30000);
                }
            }
            goto app_exit;
        default:
            break;
        }

        sleep(1);
    }

app_exit:
    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!device_obj[dev_num].dev_enable)
            continue;

        printf("sample_vicap, vicap dev(%d) stop stream\n", dev_num);
        ret = kd_mpi_vicap_stop_stream(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) stop stream failed.\n", dev_num);
        }

        printf("sample_vicap, vicap dev(%d) deinit\n", dev_num);
        ret = kd_mpi_vicap_deinit(dev_num);
        if (ret) {
            printf("sample_vicap, vicap dev(%d) deinit failed.\n", dev_num);
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
                if (vo_count == 0) {
                    vo_chn = K_VO_DISPLAY_CHN_ID1;
                    layer = K_VO_LAYER1;
                } else if (vo_count == 1) {
                    vo_chn = K_VO_DISPLAY_CHN_ID2;
                    layer = K_VO_LAYER2;
                } else {
                    printf("only support unbind two vo chn.\n");
                    continue;
                }
                sample_vicap_disable_vo_layer(layer);
                printf("sample_vicap, vo_count(%d), dev(%d) chn(%d) unbind vo chn(%d) layer(%d)\n", vo_count, dev_num, chn_num, vo_chn, layer);
                sample_vicap_unbind_vo(dev_num, chn_num, vo_chn);
                vo_count++;
            }
        }
    }

    if(adc_tid)
    {
        pthread_cancel(adc_tid);
        pthread_join(adc_tid, NULL);
    }

    if(output_tid)
    {
        pthread_cancel(output_tid);
        pthread_join(output_tid, NULL);
    }

    if(gpio_tid)
    {
        pthread_cancel(gpio_tid);
        pthread_join(gpio_tid, NULL);
    }

    exiting = K_FALSE;
    printf("%d: exiting %d\n", __LINE__, exiting);

    if (gpio_fd >= 0)
    {
        close(gpio_fd);
    }

    ret = sample_dv_dma_delete();
    if (ret) {
        printf("sample_dma_delete failed\n");
        return 0;
    }

    ret = sample_dv_dpu_delete();
    if (ret) {
        printf("sample_dpu_delete failed\n");
        return 0;
    }

    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);

    kd_mpi_sys_munmap(ir_virt_addr, IR_BUF_SIZE);
    kd_mpi_vb_release_block(ir_handle);
#if ENABLE_CDC
    close(fd_usb);
#endif

vb_exit:

    return ret;
}
