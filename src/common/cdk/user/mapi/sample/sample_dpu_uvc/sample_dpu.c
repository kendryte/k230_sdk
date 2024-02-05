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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "sample_dpu.h"
#include "kstream.h"
#include "log.h"

#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"
#include "k_dpu_comm.h"
#include "k_connector_comm.h"

#include "mapi_vvi_api.h"
#include "mapi_dpu_api.h"
#include "frame_cache.h"
#include "vicap_vo_cfg.h"
#include "uvc_recv_file.h"

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

#if !RUN_UVC
pthread_t dpu_startup_tid;
#endif

#define   ENCWIDTH    1280
#define   ENCHEIGHT   720

#define   D1_WIDTH    640
#define   D1_HEIGHT   480

// #define MAX_CHN_NUM 3
#define MAX_DEV_CNT 2

#define VICAP_OUTPUT_BUF_NUM 15
#define VICAP_INPUT_BUF_NUM 6
#define DPU_BUF_NUM    6
#define DMA_BUF_NUM    3

#define VICAP_WIDTH     1280
#define VICAP_HEIGHT    720

#define DISPLAY_WITDH  1088
#define DISPLAY_HEIGHT 1920

#define MAX_VO_LAYER_NUM 2

#define MAX_CHN_NUM 3
#define VI_ALIGN_UP(addr, size) (((addr)+((size)-1U))&(~((size)-1U)))

#define   VB_YUV_PIC(w,h)   (((w)*(h)*2 + 0xfff) & ~0xfff)//(w*h*3/2+4095)/4096
#define   VB_PIC_STREAM(w,h)   (((w)*(h)/2 + 0xfff) & ~0xfff)//(w*h/2+4095)/4096
#define   PIX_FMT   PIXEL_FORMAT_YUV_SEMIPLANAR_420

unsigned int g_sensor0 = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
unsigned int g_sensor1 = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;

static FILE * fp = NULL;
static k_mapi_media_attr_t media_attr = {0};
int fd = -1;
extern uvc_grab_init_parameters_ex g_grab_init_param;

static k_u8 __started = 0;
encoder_property __encoder_property;

static char OUT_PATH[50] = {"\0"};
static k_u8 g_exit = 0;
static k_s32 g_mmap_fd_tmp = 0;
static k_vb_blk_handle ir_handle;
static k_u64 ir_phys_addr;
static k_u8 *ir_virt_addr;
static k_u32 vicap_width;
static k_u32 vicap_height;
static k_u32 ir_buf_size;

extern char g_out_path[];
extern unsigned int g_gz1;

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

    k_dpu_result_type dpu_result_type;
} vicap_device_obj;

typedef struct {
    k_u16 width[MAX_VO_LAYER_NUM];
    k_u16 height[MAX_VO_LAYER_NUM];
    k_u16 rotation[MAX_VO_LAYER_NUM];
    k_vo_layer layer[MAX_VO_LAYER_NUM];
    k_bool enable[MAX_VO_LAYER_NUM];
} k_vicap_vo_layer_conf;


static vicap_device_obj g_dev_obj[VICAP_DEV_ID_MAX];
static k_dpu_info_t g_dpu_info;

static unsigned int g_rgb_frame_number = 0;
static unsigned int g_depth_frame_number = 0;
static unsigned int g_ir_frame_number = 0;
static unsigned int g_speckle_frame_number = 0;

static int          g_frame_rate = -1;
static unsigned int g_cur_frame_keep_time = 0;
static unsigned long long g_origin_frame_rgb_time = 0;
static unsigned long long g_origin_frame_depth_time = 0;
static unsigned long long g_origin_frame_ir_time = 0;
static unsigned long long g_origin_frame_speckle_time = 0;
static unsigned int g_send_frame_rgb_cnt = 0;
static unsigned int g_send_frame_depth_cnt = 0;
static unsigned int g_send_frame_ir_cnt = 0;
static unsigned int g_send_frame_speckle_cnt = 0;
static unsigned int g_discard_rgb_frame = 0;

static const int g_uvc_rgb_data_start_code = 0xaabbccdd;
static const int g_uvc_depth_data_start_code = 0xddccbbaa;
static const int g_uvc_ir_data_start_code = 0xddbbaacc;
static const int g_uvc_speckle_data_start_code = 0xddaaccbb;

typedef struct
{
    int                sync_timestamp;         //是否同步了时间戳
    unsigned long long start_server_timestamp; //首帧pc时间戳
    unsigned long long start_k230_timestamp;  //首帧k230时间戳
    unsigned long long valid_frame_start_number;  //有效帧起始number
}FRAME_SYNC_CLOCK_INFO;
typedef enum
{
    em_sync_clock_rgb = 0,
    em_sync_clock_depth,
    em_sync_clock_ir,
    em_sync_clock_speckle,
    em_sync_clock_max,
}SYNC_CLOCK_FRAME_TYPE;

static unsigned long long g_sync_clock_server_timestamp = 0;
static unsigned long long g_sync_clock_client_timestamp = 0;
static FRAME_SYNC_CLOCK_INFO g_frame_sync_clock_info[em_sync_clock_max];

typedef struct {
	int data_start_code;
	int data_size;
	int data_type;//1:start 2:middle  3:last
	int frame_size;
	unsigned int frame_number;
	unsigned int packet_number_of_frame;
    float temperature;
	int width;
	int height;
	int reserve[3];
#ifdef __linux__
	unsigned long pts;
#elif defined(_WIN32)
	unsigned long long pts;
#endif

}UVC_PRIVATE_DATA_HEAD_INFO;

typedef struct
{
    k_u32 dev_cnt :2;
    k_vicap_chn chn_num[VICAP_CHN_ID_MAX];
    k_vicap_sensor_type sensor_type[VICAP_CHN_ID_MAX];
    k_pixel_format out_format[VICAP_CHN_ID_MAX];
    k_bool preview[VICAP_CHN_ID_MAX];
    k_u16 rotation[VICAP_CHN_ID_MAX];
} vicap_init_info_t;

vicap_init_info_t g_vicap_init =
{
    .dev_cnt = 2,

    .chn_num[0] = 1,
    .sensor_type[0] = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR,
    .out_format[0] = PIXEL_FORMAT_YUV_SEMIPLANAR_420,//PIXEL_FORMAT_BGR_888_PLANAR,
    .preview[0] = K_FALSE,
    .rotation[0] = 1,//0,

    .chn_num[1] = 0,
    .sensor_type[1] = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE,
    .out_format[1] = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    .preview[1] = K_FALSE,
    .rotation[1] = 1
};

typedef struct {
    k_u32 buffsize_with_align;
    k_u32 buffsize;
    k_char *suffix;
} buf_size_calc;

extern k_s32 start_get_yuv(k_u32 dev_num, k_u32 chn_num);
extern k_s32 stop_get_yuv(void);
static frame_node_t *fnode = NULL;
static int count = 0;
static k_s32 sample_dpu_startup(void);
static k_s32 sample_dpu_shutdown(void);

static unsigned long long get_system_time_microsecond()
{
    struct timeval timestamp;
    if (0 == gettimeofday(&timestamp, NULL))
        return (unsigned long long)timestamp.tv_sec * 1000000 + timestamp.tv_usec;

    return 0;
}

static unsigned long long  _do_sync_frame_timestamp(SYNC_CLOCK_FRAME_TYPE frame_type,unsigned long long capture_timestatmp)
{
    //return capture_timestatmp;
    if (0 == g_frame_sync_clock_info[frame_type].sync_timestamp)
    {
        //if (g_frame_sync_clock_info[frame_type].valid_frame_start_number++ > (g_frame_rate > 0)?g_frame_rate*5:50)
        if(1)
        {
            //以k230时钟计算，从同步时间戳时刻算起，到当前共运行时间微妙
            unsigned long long take_time_from_sync = get_system_time_microsecond()-g_sync_clock_client_timestamp;
            //printf("=========take sync time:%llu\n",take_time_from_sync);

            g_frame_sync_clock_info[frame_type].start_server_timestamp = g_sync_clock_server_timestamp +  take_time_from_sync;
            g_frame_sync_clock_info[frame_type].start_k230_timestamp = capture_timestatmp;
            g_frame_sync_clock_info[frame_type].sync_timestamp = 1;
        }
        else
        {
            return 0;
        }
    }

    return g_frame_sync_clock_info[frame_type].start_server_timestamp + (capture_timestatmp - g_frame_sync_clock_info[frame_type].start_k230_timestamp);
}

int do_dpu_ctrol_cmd(UVC_TRANSFER_CONTROL_CMD cmd_ctrl)
{
    if(cmd_ctrl.type == em_uvc_transfer_control_sync_clock)
    {
        g_sync_clock_client_timestamp = get_system_time_microsecond();
        g_sync_clock_server_timestamp = cmd_ctrl.ctrl_info.sync_clock;
        printf("recv pc sync clock %llu,cur clock:%llu,differ(%lld)\n",g_sync_clock_server_timestamp,g_sync_clock_client_timestamp,g_sync_clock_server_timestamp-g_sync_clock_client_timestamp);
        for (int i =0; i < em_sync_clock_max;i ++)
        {
            g_frame_sync_clock_info[i].sync_timestamp = 0;
            g_frame_sync_clock_info[i].valid_frame_start_number = 0;
        }
    }
    else if (cmd_ctrl.type == em_uvc_transfer_control_grab_data)
    {
        printf("recv uvc ctl cmd ctrl grab flag %d\n",cmd_ctrl.ctrl_info.start_grab);
    }
    else if (cmd_ctrl.type == em_uvc_transfer_control_set_framerate)
    {
        printf("recv uvc ctl cmd ctrl set framerate:%d\n",cmd_ctrl.ctrl_info.frame_rate);

        g_frame_rate = cmd_ctrl.ctrl_info.frame_rate;
        if (g_frame_rate > 0 && g_frame_rate <= 60)
        {
            g_cur_frame_keep_time = 1000000/g_frame_rate;
            g_origin_frame_rgb_time = get_system_time_microsecond();
            g_origin_frame_depth_time = g_origin_frame_rgb_time;
            g_origin_frame_ir_time = g_origin_frame_rgb_time;
            g_origin_frame_speckle_time = g_origin_frame_rgb_time;

            g_send_frame_rgb_cnt = 0;
            g_send_frame_depth_cnt = 0;
            g_send_frame_ir_cnt = 0;
            g_send_frame_speckle_cnt = 0;
        }
        else
        {
            g_cur_frame_keep_time = -1;
        }

    }

    return 0;
}

static void *_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);

    if (g_mmap_fd_tmp == 0) {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);
    if (mmap_addr != (void*)(-1))
        virt_addr = (void*)((char*)mmap_addr + (phys_addr & page_mask));
    else
        printf("**** sys_mmap failed\n");

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phys_addr, void *virt_addr, k_u32 size)
{
    if (g_mmap_fd_tmp == 0) {
        return -1;
    }
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);
    void* mmap_addr = (void*)((char*)virt_addr - (phys_addr & page_mask));
    if( munmap(mmap_addr, mmap_size) < 0) {
        printf("**** munmap failed\n");
    }
    return 0;
}

static k_s32 sample_dpu_vicap_vb_init(vicap_device_obj *dev_obj)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret = 0;
    k_vb_config *config;
    // k_vb_supplement_config supplement_config;

    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));

    config = &media_attr.media_config.vb_config;
    memset(config, 0, sizeof(config));
    config->max_pool_cnt = 64;

    int k = 0;
    for (int i = 0; i < VICAP_DEV_ID_MAX; i++) {
        if (!dev_obj[i].dev_enable)
            continue;

        if (dev_obj[i].mode == VICAP_WORK_OFFLINE_MODE) {
            config->comm_pool[k].blk_cnt = VICAP_INPUT_BUF_NUM;
            config->comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            config->comm_pool[k].blk_size = dev_obj[i].in_size;
            printf("%s, dev(%d) pool(%d) in_size(%d) blk_cnt(%d)\n", __func__, i , k ,dev_obj[i].in_size, config->comm_pool[k].blk_cnt);
            k++;
        }

        for (int j = 0; j < VICAP_CHN_ID_MAX; j++) {
            if (!dev_obj[i].chn_enable[j])
                continue;

            config->comm_pool[k].blk_cnt = VICAP_OUTPUT_BUF_NUM;
            config->comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;

            k_pixel_format pix_format =  g_vicap_init.out_format[i];
            k_u16 out_width = dev_obj[i].out_win[j].width;
            k_u16 out_height = dev_obj[i].out_win[j].height;
            k_u16 in_width = dev_obj[i].in_width;
            k_u16 in_height = dev_obj[i].in_height;
            printf("%s>in_width %d, in_height %d, out_width %d, out_height %d\n", \
                __FUNCTION__, in_width, in_height, out_width, out_height);

            switch (pix_format) {
            case PIXEL_FORMAT_YUV_SEMIPLANAR_420:
                config->comm_pool[k].blk_size = VICAP_ALIGN_UP((out_width * out_height * 3 / 2), 0x1000);
                break;
            case PIXEL_FORMAT_RGB_888:
            case PIXEL_FORMAT_BGR_888_PLANAR:
                config->comm_pool[k].blk_size = VICAP_ALIGN_UP((out_width * out_height * 3), 0x1000);
                break;
            case PIXEL_FORMAT_RGB_BAYER_10BPP:
                config->comm_pool[k].blk_size = VICAP_ALIGN_UP((in_width * in_height * 2), 0x1000);
                break;
            default:
                g_vicap_init.out_format[i] = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
                config->comm_pool[k].blk_size = VICAP_ALIGN_UP((out_width * out_height * 3 / 2), 0x1000);
                break;
            }
            dev_obj[i].buf_size[j] = config->comm_pool[k].blk_size;
            printf("%s, dev(%d) chn(%d) pool(%d) buf_size(%d) blk_cnt(%d)\n", __func__, i, j, k ,dev_obj[i].buf_size[j], config->comm_pool[k].blk_cnt);
            k++;
        }

        if (dev_obj[i].dw_enable) {
            // another buffer for isp->dw
            config->comm_pool[k].blk_size = VICAP_ALIGN_UP((dev_obj[i].in_width * dev_obj[i].in_height * 3 / 2), 0x1000);
            config->comm_pool[k].blk_cnt = VICAP_MIN_FRAME_COUNT * 2;
            config->comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            dev_obj[i].buf_size[0] = config->comm_pool[k].blk_size;
            printf("%s, dev(%d) pool(%d) buf_size(%d) dw_enable\n", __func__, i, k ,dev_obj[i].buf_size[0]);
            k++;
        }
    }

    /* dma vb init */
    config->comm_pool[k].blk_cnt = DMA_BUF_NUM;
    config->comm_pool[k].blk_size = VICAP_WIDTH * VICAP_HEIGHT;
    config->comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;

    config->comm_pool[k+1].blk_cnt = DMA_BUF_NUM;
    config->comm_pool[k+1].blk_size = VICAP_WIDTH * VICAP_HEIGHT * 3;
    config->comm_pool[k+1].mode = VB_REMAP_MODE_NOCACHE;

    /* dpu vb init */
    config->comm_pool[k+2].blk_cnt = DPU_BUF_NUM;
    config->comm_pool[k+2].blk_size = 5 * 1024 * 1024;
    config->comm_pool[k+2].mode = VB_REMAP_MODE_NOCACHE;

    //ir
    ir_buf_size = ((vicap_width * vicap_height *3/2 + 0x3ff) & ~0x3ff);
    config->comm_pool[k+3].blk_cnt = 1;
    config->comm_pool[k+3].blk_size = ir_buf_size;
    config->comm_pool[k+3].mode = VB_REMAP_MODE_NOCACHE;

    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK | VB_SUPPLEMENT_ISP_INFO_MASK;

    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }

    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;

    ret = kd_mapi_sys_get_vb_block(&pool_id, &ir_phys_addr, config->comm_pool[k+3].blk_size, NULL);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_get_vb_block error: %x\n", ret);
        goto media_deinit;
    }
    printf("sample_vicap_vb_init end\n");

    return 0;
media_deinit:
    kd_mapi_media_deinit();
    return K_FAILED;
}
static k_s32 sample_dpu_get_sensor_info(vicap_device_obj *dev_obj)
{
    int ret = 0;

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!dev_obj[dev_num].dev_enable)
            continue;

        // sensor info
        k_vicap_sensor_info sensor_info;
        memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
        sensor_info.sensor_type = dev_obj[dev_num].sensor_type;
        printf("%s>sensor_type %d\n", __FUNCTION__, sensor_info.sensor_type);
        ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
        if(ret)
        {
            printf("kd_mapi_vicap_get_sensor_info failed %d\n", ret);
        }
        memcpy(&dev_obj[dev_num].sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));
        dev_obj[dev_num].in_width = dev_obj[dev_num].sensor_info.width;
        dev_obj[dev_num].in_height = dev_obj[dev_num].sensor_info.height;
        dev_obj[dev_num].in_size = VICAP_ALIGN_UP((dev_obj[dev_num].in_width * dev_obj[dev_num].in_height * 2), VICAP_ALIGN_1K);
        printf("sample_vicap, dev[%d] in size[%dx%d]\n", \
            dev_num, dev_obj[dev_num].in_width, dev_obj[dev_num].in_height);

        k_vicap_dev_set_info dev_attr_info;
        memset(&dev_attr_info, 0, sizeof(k_vicap_dev_set_info));
        dev_attr_info.dw_en = dev_obj[dev_num].dw_enable;
        dev_attr_info.pipe_ctrl.data = 0xFFFFFFFF;
        dev_attr_info.sensor_type = dev_obj[dev_num].sensor_type;
        dev_attr_info.vicap_dev = (k_vicap_dev)dev_num;
        dev_attr_info.mode = dev_obj[dev_num].mode;
        dev_attr_info.buffer_num = VICAP_INPUT_BUF_NUM;
        dev_attr_info.buffer_size = VICAP_ALIGN_UP((dev_obj[dev_num].in_width * dev_obj[dev_num].in_height * 2), VICAP_ALIGN_1K);
        ret = kd_mapi_vicap_set_dev_attr(dev_attr_info);

        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            printf("%s>chn %d\n", __FUNCTION__, chn_num);
            if (!dev_obj[dev_num].chn_enable[chn_num])
                continue;
            //set default value
            if (!dev_obj[dev_num].out_format[chn_num]) {
                dev_obj[dev_num].out_format[chn_num] = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
            }

            if (!dev_obj[dev_num].out_win[chn_num].width) {
                dev_obj[dev_num].out_win[chn_num].width = VICAP_WIDTH;
            }

            if (!dev_obj[dev_num].out_win[chn_num].height) {
                dev_obj[dev_num].out_win[chn_num].height = VICAP_HEIGHT;
            }

            if ( dev_obj[dev_num].out_win[chn_num].h_start || dev_obj[dev_num].out_win[chn_num].v_start) {
                dev_obj[dev_num].crop_enable[chn_num] = K_TRUE;
            }

            if ((dev_obj[dev_num].out_win[chn_num].width > DISPLAY_WITDH)
                && (dev_obj[dev_num].out_win[chn_num].height > DISPLAY_HEIGHT)) {
                dev_obj[dev_num].preview[chn_num] = K_FALSE;
            }

            if (!dev_obj[dev_num].rotation[chn_num]
                && ((dev_obj[dev_num].out_win[chn_num].width > DISPLAY_WITDH)
                && (dev_obj[dev_num].out_win[chn_num].width < DISPLAY_HEIGHT))) {
                dev_obj[dev_num].rotation[chn_num] = 1;
            }

            printf("sample_vicap, dev_num(%d), chn_num(%d), in_size[%dx%d], out_offset[%d:%d], out_size[%dx%d]\n", \
                dev_num, chn_num, dev_obj[dev_num].in_width, dev_obj[dev_num].in_height, \
                dev_obj[dev_num].out_win[chn_num].h_start, dev_obj[dev_num].out_win[chn_num].v_start, \
                dev_obj[dev_num].out_win[chn_num].width, dev_obj[dev_num].out_win[chn_num].height);
        }
    }

    return K_SUCCESS;
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

    ret = kd_mapi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mapi_sys_bind failed:0x%x\n", ret);
    }

    return;
}

static k_s32 sample_vicap_vo_layer_init(k_vicap_vo_layer_conf *layer_conf, k_vicap_dev arg_dev)
{
    k_s32 ret = 0;
    layer_info info[MAX_VO_LAYER_NUM];
    k_u16 margin = 0;
    k_u16 rotation = 0;
    k_u16 relative_height = 0;
    k_u16 total_height = 0;

    memset(&info, 0, sizeof(info));

    for (int i = 0; i <= arg_dev; i++) {
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
        margin = ((DISPLAY_HEIGHT - total_height) / (i+2));
        if ((total_height > DISPLAY_HEIGHT) || (info[i].act_size.width > DISPLAY_WITDH)) {
            printf("%s, the preview window size[%dx%d] exceeds the display window size[%dx%d].\n", \
                __func__, info[i].act_size.width, total_height, DISPLAY_WITDH, DISPLAY_HEIGHT);
            return -1;
        }
        printf("%s, width(%d), height(%d), margin(%d), total_height(%d)\n", \
            __func__, info[i].act_size.width, info[i].act_size.height, margin, total_height);
    }

    for (int i = 0; i <= arg_dev; i++) {
        info[i].offset.x = (DISPLAY_WITDH - info[i].act_size.width) / 2;
        info[i].offset.y = margin + relative_height;
        printf("%s, layer(%d), offset.x(%d), offset.y(%d), relative_height(%d)\n", __func__, layer_conf->layer[i], info[i].offset.x, info[i].offset.y, relative_height);
        relative_height += info[i].act_size.height + margin;

        info[i].format = PIXEL_FORMAT_YVU_PLANAR_420;
        info[i].global_alptha = 0xff;

        sample_vo_creat_layer(layer_conf->layer[i], &info[i]);
    }

    return ret;
}

static k_s32 sample_dpu_vicap_init(vicap_device_obj *dev_obj)
{
    k_u8 dev_cnt = 0;
    k_u8 vo_count = 0;
    k_bool preview = K_FALSE;
    int ret = 0;

    k_vicap_vo_layer_conf layer_conf;
    memset(&layer_conf, 0 , sizeof(k_vicap_vo_layer_conf));

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!dev_obj[dev_num].dev_enable)
            continue;
        for (int chn_num = 0; chn_num < VICAP_CHN_ID_MAX; chn_num++) {
            if (!dev_obj[dev_num].chn_enable[chn_num])
                continue;
            k_vicap_chn_set_info chn_attr_info;
            memset(&chn_attr_info, 0, sizeof(k_vicap_chn_set_info));
            chn_attr_info.crop_en = K_FALSE;
            chn_attr_info.scale_en = K_FALSE;
            chn_attr_info.chn_en = K_TRUE;
            chn_attr_info.crop_h_start = 0;
            chn_attr_info.crop_v_start = 0;
            chn_attr_info.out_height = VICAP_HEIGHT;
            chn_attr_info.out_width = VICAP_WIDTH;
            chn_attr_info.buffer_num = 6;
            chn_attr_info.pixel_format = dev_obj[dev_num].out_format[chn_num];
            chn_attr_info.vicap_dev = dev_obj[dev_num].dev_num;
            chn_attr_info.vicap_chn = (k_vicap_chn)dev_obj[dev_num].chn_num[chn_num];
            if(chn_attr_info.pixel_format != PIXEL_FORMAT_RGB_BAYER_10BPP && chn_attr_info.pixel_format != PIXEL_FORMAT_RGB_BAYER_12BPP)
            {
                // dewarp can enable in bayer pixel format
                if(dev_obj[dev_num].dw_enable && chn_num == 0)
                {
                    printf("dw buf\n");
                    chn_attr_info.buf_size = dev_obj[dev_num].buf_size[chn_num];
                }
                else
                {
                    printf("dw disable buf\n");
                    chn_attr_info.buf_size = dev_obj[dev_num].buf_size[chn_num];
                }
            }
            else
            {
                printf("dw is force disabled, becase of pixformat BAYER\n");
                dev_obj[dev_num].dw_enable = 0;
                chn_attr_info.buf_size = dev_obj[dev_num].buf_size[chn_num];
            }
            ret = kd_mapi_vicap_set_chn_attr(chn_attr_info);
            if(ret != K_SUCCESS)
            {
                printf("kd_mapi_vicap_set_chn_attr failed %d\n", ret);
            }
            else
                printf("kd_mapi_vicap_set_chn_attr ok\n");

            //bind vicap to vo, only support bind two vo chn(K_VO_DISPLAY_CHN_ID1 & K_VO_DISPLAY_CHN_ID2)
            if (dev_obj[dev_num].preview[chn_num]) {
                preview = K_TRUE;
                k_s32 vo_chn;
                k_vo_layer layer;
                k_u16 rotation;
                if (vo_count == 0) {
                    vo_chn = K_VO_DISPLAY_CHN_ID1;
                    layer = K_VO_LAYER1;
                    rotation = dev_obj[dev_num].rotation[chn_num];
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
                layer_conf.width[vo_count] = chn_attr_info.out_width;
                layer_conf.height[vo_count] = chn_attr_info.out_height;
                layer_conf.rotation[vo_count] = rotation;
                layer_conf.layer[vo_count] = layer;
                vo_count++;
            }
        }
        dev_cnt++;
    }

    if(preview){
        ret = sample_vicap_vo_layer_init(&layer_conf, dev_cnt);
        if (ret)
        {
            printf("sample_vicap_vo_layer_init failed %d\n", ret);
        }
    }

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!dev_obj[dev_num].dev_enable)
            continue;
        printf("%s>start vicap dev %d\n", __FUNCTION__, dev_obj[dev_num].dev_num);
        ret = kd_mapi_vicap_start(dev_obj[dev_num].dev_num);
        if(ret != K_SUCCESS)
        {
            printf("kd_mapi_vicap_start failed %d\n", ret);
        }
        else
            printf("kd_mapi_vicap_start ok\n");
    }
    usleep(5000);

    return K_SUCCESS;
}

static void do_transfer_frame_data(unsigned char* pdata,int len,unsigned int frame_number, unsigned long pts,float temperature,int frame_start_code,int frame_total_cnt,int frame_total_size,int cur_cnt)
{
    uvc_cache_t *uvc_cache = uvc_cache_get();
    UVC_PRIVATE_DATA_HEAD_INFO  private_head_info;
    int head_len = sizeof(UVC_PRIVATE_DATA_HEAD_INFO);
    int package_total_size = __encoder_property.width*__encoder_property.height*3/2;
    int package_data_size = package_total_size - head_len;
    static unsigned int packet_number_of_frame = 0;
    int ncount = len/package_data_size + (len%package_data_size != 0);
    frame_node_t *fnode = NULL;
    //printf("=======do_transfer_frame_data package count:%d\n",ncount);
    int try_cnt = 0;
    for (int i =0;i < ncount;i ++)
    {
        //get free queue node
        if (uvc_cache)
        {
            while(1)
            {
                get_node_from_queue(uvc_cache->free_queue, &fnode);
                if (!fnode)
                {
                    if (try_cnt ++ < 5)
                    {
                        usleep(1000);
                        continue;
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    break;
                }
            }

            fnode->used = 0;
        }

        //fill package
        if (i == 0 )  //first
        {
            private_head_info.data_size = package_data_size;
            private_head_info.data_type = (cur_cnt == 1)?1:2;
            if (1 == private_head_info.data_type)
            {
                packet_number_of_frame = 0;
            }
        }
        else if (i == ncount -1) //last
        {

            private_head_info.data_size = len - package_data_size*(ncount-1);
            private_head_info.data_type = (frame_total_cnt == cur_cnt)?3:2;
        }
        else  //middle
        {
            private_head_info.data_size = package_data_size;
            private_head_info.data_type = 2;
        }
        packet_number_of_frame ++;
        private_head_info.data_start_code = frame_start_code;
        private_head_info.pts = pts;
        private_head_info.frame_size = frame_total_size;
        private_head_info.frame_number = frame_number;
        private_head_info.packet_number_of_frame = packet_number_of_frame;
        private_head_info.width = g_grab_init_param.camera_width;
        private_head_info.height = g_grab_init_param.camera_height;
        private_head_info.temperature = temperature;
        //fill package head
        memcpy(fnode->mem + fnode->used, &private_head_info, sizeof(private_head_info));
        fnode->used += sizeof(private_head_info);
        //fill package data
        memcpy(fnode->mem + fnode->used,pdata+i*package_data_size,private_head_info.data_size);
        fnode->used = __encoder_property.width*__encoder_property.height*3/2;

        put_node_to_queue(uvc_cache->ok_queue, fnode);
        fnode = NULL;
    }

}

static void get_img(k_u32 frame_number, unsigned  long long pts, kd_dpu_data_s* p_dpu_data,int frame_start_code, k_u8 *p_private_data)
{
    void *virt_addr = NULL;
    k_u32 width, height, data_size;

    width = p_dpu_data->dpu_result.img_result.width;
    height = p_dpu_data->dpu_result.img_result.height;
    //printf("pixel format: %d\n", p_dpu_data->dpu_result.img_result.pixel_format);
    if(p_dpu_data->dpu_result.img_result.pixel_format == PIXEL_FORMAT_BGR_888_PLANAR)
    {
        ;
    }
    else if(p_dpu_data->dpu_result.img_result.pixel_format == PIXEL_FORMAT_YUV_SEMIPLANAR_420)
    {
        unsigned int packet_number_of_frame = 0;
        for (int i =0;i < 2;i ++)
        {
            if (0 == i)
            {
                data_size = width * height;
                virt_addr = (void*)p_dpu_data->dpu_result.img_result.virt_addr[i];
                if(virt_addr)
                {
                    do_transfer_frame_data((unsigned char*)virt_addr, data_size,frame_number,pts,p_dpu_data->temperature, frame_start_code,2,width * height*3/2,1);
                }
            }
            else if (1 == i)
            {
                data_size = width * height / 2;
                virt_addr = (void*)p_dpu_data->dpu_result.img_result.virt_addr[i];
                if(virt_addr)
                {
                    do_transfer_frame_data((unsigned char*)virt_addr, data_size,frame_number,pts,p_dpu_data->temperature,frame_start_code,2,width * height*3/2,2);
                }
            }
        }
    }
}

static int _discard_frame(unsigned long long origin_frame_time,unsigned int* frame_send_cnt)
{
    unsigned int elapsed_time = get_system_time_microsecond()- origin_frame_time;
    //printf("elapsed_time:%d,send frame time:%d,frame_keep_time:%d\n",elapsed_time,(*frame_send_cnt) * g_cur_frame_keep_time,g_cur_frame_keep_time);
    if (elapsed_time - (*frame_send_cnt) * g_cur_frame_keep_time >= g_cur_frame_keep_time)
    {
        *frame_send_cnt  += 1;
        return -1;
    }

    return 0;
}

k_s32 dpu_get_img(k_u32 dev_num, kd_dpu_data_s* p_dpu_data, k_u8 *p_private_data)
{
    if (g_cur_frame_keep_time > 0)
    {
        if (1 == g_discard_rgb_frame)
        {
            g_discard_rgb_frame = 0;
            return 0;
        }
    }

    //p_dpu_data->dpu_result.img_result.pts = get_system_time_microsecond();
    g_rgb_frame_number = p_dpu_data->dpu_result.img_result.time_ref;
    get_img(g_rgb_frame_number,_do_sync_frame_timestamp(em_sync_clock_rgb,p_dpu_data->dpu_result.img_result.pts),p_dpu_data,g_uvc_rgb_data_start_code, p_private_data);
    return K_SUCCESS;
}

k_s32 dpu_get_depth(k_u32 dev_num, kd_dpu_data_s* p_dpu_data, k_u8 *p_private_data)
{
    unsigned int elapsed_time = 0;
    if(g_dpu_info.mode == IMAGE_MODE_RGB_DEPTH ||
       g_dpu_info.mode == IMAGE_MODE_IR_DEPTH ||
       g_dpu_info.mode == IMAGE_MODE_NONE_DEPTH)
    {
        k_dpu_chn_lcn_result_t lcn_result;
        void *virt_addr = NULL;

        memcpy(&lcn_result, &p_dpu_data->dpu_result.lcn_result, sizeof(k_dpu_chn_lcn_result_t));

        virt_addr = p_dpu_data->dpu_result.lcn_virt_addr;
        if (virt_addr)
        {
            if (g_cur_frame_keep_time > 0)
            {
                if (0 == _discard_frame(g_origin_frame_depth_time,&g_send_frame_depth_cnt))
                {

                    //discard pair rgb frame
                    if (g_dpu_info.mode == IMAGE_MODE_RGB_DEPTH)
                    {
                        g_discard_rgb_frame = 1;
                    }
                    return 0;
                }
            }

            /*
            //p_dpu_data->dpu_result.lcn_result.pts = get_system_time_microsecond();
            {
                static int ncount = 0;
                printf("[%d_%llu]========depth timestamp:%llu,diff(%llu)\n",ncount++,get_system_time_microsecond(),p_dpu_data->dpu_result.lcn_result.pts,get_system_time_microsecond() - p_dpu_data->dpu_result.lcn_result.pts);
            }
            */

            g_depth_frame_number = p_dpu_data->dpu_result.lcn_result.time_ref;
            do_transfer_frame_data((unsigned char*)virt_addr, lcn_result.depth_out.length,g_depth_frame_number,_do_sync_frame_timestamp(em_sync_clock_depth,p_dpu_data->dpu_result.lcn_result.pts),p_dpu_data->temperature,g_uvc_depth_data_start_code,1,lcn_result.depth_out.length,1);
        }
    }
    else
    {
        if (g_dpu_info.mode == IMAGE_MODE_NONE_IR)
        {
            if (g_cur_frame_keep_time > 0)
            {
                if (0 == _discard_frame(g_origin_frame_ir_time,&g_send_frame_ir_cnt))
                {
                    return 0;
                }
            }

            g_ir_frame_number = p_dpu_data->dpu_result.img_result.time_ref;
            get_img(g_ir_frame_number,_do_sync_frame_timestamp(em_sync_clock_ir,p_dpu_data->dpu_result.img_result.pts), p_dpu_data,g_uvc_ir_data_start_code, p_private_data);
        }
        else if (g_dpu_info.mode == IMAGE_MODE_NONE_SPECKLE)
        {
            if (g_cur_frame_keep_time >= 0)
            {
                if (0 == _discard_frame(g_origin_frame_speckle_time,&g_send_frame_speckle_cnt))
                {
                    return 0;
                }
            }

            g_speckle_frame_number = p_dpu_data->dpu_result.img_result.time_ref;
            get_img(g_speckle_frame_number,_do_sync_frame_timestamp(em_sync_clock_speckle,p_dpu_data->dpu_result.img_result.pts), p_dpu_data, g_uvc_speckle_data_start_code,p_private_data);

        }
        else if (g_dpu_info.mode == IMAGE_MODE_RGB_IR)
        {
            if (GRAB_IMAGE_MODE_RGB_SPECKLE == g_grab_init_param.grab_mode)
            {
                if (g_cur_frame_keep_time > 0)
                {
                    if (0 == _discard_frame(g_origin_frame_speckle_time,&g_send_frame_speckle_cnt))
                    {
                        //discard pair rgb frame
                        g_discard_rgb_frame = 1;

                        return 0;
                    }
                }

                g_speckle_frame_number = p_dpu_data->dpu_result.img_result.time_ref;
                get_img(g_speckle_frame_number,_do_sync_frame_timestamp(em_sync_clock_speckle,p_dpu_data->dpu_result.img_result.pts), p_dpu_data, g_uvc_speckle_data_start_code,p_private_data);

            }
            else
            {
                if (g_cur_frame_keep_time > 0)
                {
                    if (0 == _discard_frame(g_origin_frame_ir_time,&g_send_frame_ir_cnt))
                    {
                        //discard pair rgb frame
                        g_discard_rgb_frame = 1;
                        return 0;
                    }
                }

                g_ir_frame_number = p_dpu_data->dpu_result.img_result.time_ref;
                get_img(g_ir_frame_number,_do_sync_frame_timestamp(em_sync_clock_ir,p_dpu_data->dpu_result.img_result.pts), p_dpu_data,g_uvc_ir_data_start_code, p_private_data);

            }
        }

    }

    return K_SUCCESS;
}

static k_s32 parse_pramas(vicap_init_info_t *init, vicap_device_obj *device_obj)
{
    for(int i = 0; i < init->dev_cnt; i++)
    {
        device_obj[i].mode = VICAP_WORK_OFFLINE_MODE;
        device_obj[i].dev_num = i;
        device_obj[i].dev_enable = K_TRUE;
        device_obj[i].ae_enable = K_TRUE;//default enable ae
        device_obj[i].awb_enable = K_TRUE;//default enable awb
        device_obj[i].dnr3_enable = K_FALSE;//default disable 3ndr

        device_obj[i].sensor_type = init->sensor_type[i];//OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;

        k_u8 chn_num = init->chn_num[i];
        device_obj[i].chn_num[chn_num] = chn_num;
        device_obj[i].chn_enable[chn_num] = K_TRUE;
        device_obj[i].preview[chn_num] = init->preview[i];
        device_obj[i].out_format[chn_num] = init->out_format[i];//PIXEL_FORMAT_BGR_888_PLANAR;//rgb888p
    }
    return K_SUCCESS;
}

static k_s32 sample_dpu_init(void)
{
    k_s32 ret = 0;
    printf("client %s\n", __func__);

    ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_init failed, %x.\n", ret);
        return -1;
    }
    printf("kd_mapi_sys_init ok\n");


    return K_SUCCESS;
}

static k_s32 sample_dpu_deinit(void)
{
    printf("%s\n", __func__);
    k_s32 ret = 0;

    ret = kd_mapi_sys_deinit();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_init failed, %x.\n", ret);
        return -1;
    }

    return 0;
}

static k_s32 sample_dpu_startup(void)
{
    printf("%s\n", __func__);
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_s32 ret = 0;

    strcpy(OUT_PATH, g_out_path);

    memset(g_dev_obj, 0, sizeof(g_dev_obj));

    g_sensor0 = g_grab_init_param.sensor_type[0];
    g_sensor1 = g_grab_init_param.sensor_type[1];

    g_vicap_init.sensor_type[0] = g_sensor0;
    g_vicap_init.sensor_type[1] = g_sensor1;

    parse_pramas(&g_vicap_init, g_dev_obj);

    ret = sample_dpu_get_sensor_info(g_dev_obj);
    if(ret != K_SUCCESS)
    {
        printf("sample_dpu_get_sensor_info failed %d\n", ret);
    }
    else
        printf("sample_dpu_get_sensor_info ok, s0 %d, s1 %d\n", g_sensor0, g_sensor1);

    ret = sample_vicap_vo_init();
    if (ret != K_SUCCESS) {
        printf("sample_vicap_vo_init failed, %x.\n", ret);
        return -1;
    }

    ret = sample_dpu_vicap_vb_init(g_dev_obj);
    if(ret != K_SUCCESS)
    {
        printf("sample_dpu_vicap_vb_init failed\n");
        return K_FAILED;
    }
    printf("sample_dpu_vicap_vb_init ok\n");

    memset(&g_dpu_info, 0, sizeof(k_dpu_info_t));
    g_dpu_info.ir_phys_addr = ir_phys_addr;
    g_dpu_info.dev_cnt = 2;
    g_dpu_info.rgb_dev = 0;
    g_dpu_info.rgb_chn = 1;
    g_dpu_info.speckle_dev = 1;
    g_dpu_info.speckle_chn = 0;
    g_dpu_info.dpu_bind = DPU_UNBIND;
    //g_dpu_info.width = VICAP_WIDTH;
    //g_dpu_info.height = VICAP_HEIGHT;
    g_dpu_info.width = g_grab_init_param.camera_width;
    g_dpu_info.height = g_grab_init_param.camera_height;
    g_dpu_info.dpu_buf_cnt = DPU_BUF_NUM;
    g_dpu_info.dma_buf_cnt = DPU_BUF_NUM;
    /*g_dpu_info.adc_en = K_TRUE;
    g_dpu_info.temperature.ref = 36.289;
    g_dpu_info.temperature.cx = 640;
    g_dpu_info.temperature.cy = 360;
    g_dpu_info.temperature.kx = 0.00015;
    g_dpu_info.temperature.ky = 0.00015;
    g_dpu_info.mode = IMAGE_MODE_RGB_DEPTH;*/

    g_dpu_info.adc_en = g_grab_init_param.adc_enable;
    g_dpu_info.temperature.ref = g_grab_init_param.temperature.temperature_ref;
    g_dpu_info.temperature.cx = g_grab_init_param.temperature.temperature_cx;
    g_dpu_info.temperature.cy = g_grab_init_param.temperature.temperature_cy;
    g_dpu_info.temperature.kx = g_grab_init_param.temperature.kxppt;
    g_dpu_info.temperature.ky = g_grab_init_param.temperature.kyppt;

    //rgb_speckle and rgb_ir is the same
    if (GRAB_IMAGE_MODE_RGB_SPECKLE == g_grab_init_param.grab_mode)
    {
        g_dpu_info.mode = IMAGE_MODE_RGB_IR;
    }
    else
    {
        g_dpu_info.mode = (k_dpu_image_mode)g_grab_init_param.grab_mode;
    }

    g_dpu_info.delay_ms = 5;

    ret = kd_mapi_dpu_init(&g_dpu_info);
    if(ret != K_SUCCESS)
    {
        printf("dpu init failed %d\n", ret);
        goto media_dedinit;
    }

    ret = sample_dpu_vicap_init(g_dev_obj);
    if(ret != K_SUCCESS)
    {
        printf("sample_dpu_vicap_init failed\n");
        return K_FAILED;
    }

    kd_dpu_callback_s dpu_cb_img;
    dpu_cb_img.pfn_data_cb = dpu_get_img;
    dpu_cb_img.p_private_data = NULL;
    kd_mapi_dpu_registercallback(0, &dpu_cb_img);

    kd_dpu_callback_s dpu_cb_depth;
    dpu_cb_depth.pfn_data_cb = dpu_get_depth;
    dpu_cb_depth.p_private_data = NULL;
    kd_mapi_dpu_registercallback(1, &dpu_cb_depth);

    k_s32 i = 0;
    __started = 1;

    ret = kd_mapi_dpu_start_grab();
    if(ret != K_SUCCESS)
    {
        printf("start grab failed\n");
        kd_mapi_dpu_close(g_vicap_init.dev_cnt);
    }

    printf("%s>done\n", __func__);

    return K_SUCCESS;

media_dedinit:
    kd_mapi_media_deinit();
    return K_FAILED;
}

static k_s32 sample_dpu_shutdown(void)
{
    int ret = 0;

    printf("%s\n", __func__);
    if ((!__started))
    {
        return 0;
    }

    ret = kd_mapi_dpu_stop_grab();
    if(ret != K_SUCCESS)
    {
        printf("stop grab failed\n");
    }

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        if (!g_dev_obj[dev_num].dev_enable)
            continue;
        printf("%s>stop vicap dev %d\n", __FUNCTION__, g_dev_obj[dev_num].dev_num);
        ret = kd_mapi_vicap_stop(g_dev_obj[dev_num].dev_num);
        if(ret != K_SUCCESS)
        {
            printf("kd_mapi_vicap_stop failed %d\n", ret);
        }
        else
            printf("kd_mapi_vicap_stop ok\n");
    }

    if(kd_mapi_dpu_close(g_vicap_init.dev_cnt))
    {
        printf("dpu close failed\n");
        return K_FAILED;
    }

    kd_mapi_sys_release_vb_block(ir_phys_addr, ir_buf_size);
    kd_mapi_media_deinit();

    __started = 0;

    return K_SUCCESS;
}

static k_s32 sample_dpu_set_property(encoder_property *p)
{
    __encoder_property = *p;
    return 0;
}

static struct stream_control_ops dpu_sc_ops = {
    .init = sample_dpu_init,
    .startup = sample_dpu_startup,
    .shutdown = sample_dpu_shutdown,
    .set_property = sample_dpu_set_property,
    .deinit = sample_dpu_deinit,
};

void sample_dpu_config(void)
{
    kstream_register_mpi_ops(&dpu_sc_ops);
}
