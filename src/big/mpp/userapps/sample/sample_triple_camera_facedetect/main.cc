// #include <rtthread.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <nncase/runtime/runtime_op_utility.h>
#include "mobile_retinaface.h"
#include "mpi_sys_api.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;

/* vicap */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <atomic>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
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

#include "mpi_vo_api.h"
#include "sys/ioctl.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#include "k_autoconf_comm.h"

#define CHANNEL 3

#define ISP_CHN1_HEIGHT (720)
#define ISP_CHN1_WIDTH  (1280)
#define ISP_CHN0_WIDTH  (640)
#define ISP_CHN0_HEIGHT (480)

// #define ISP_CHN0_WIDTH1  (640)
// #define ISP_CHN0_HEIGHT1 (480)

#define ISP_INPUT_WIDTH (1920)
#define ISP_INPUT_HEIGHT (1080)

#define LCD_WIDTH       (1080)
#define LCD_HEIGHT      (1920)

// #define TEST_BOOT_TIME

#ifdef TEST_BOOT_TIME
#define TIME_RATE (1600*1000)
uint64_t perf_get_smodecycles(void);
typedef struct kd_pin_mode
{
    unsigned short pin;     /* pin number, from 0 to 63 */
    unsigned short mode;    /* pin level status, 0 low level, 1 high level */
} pin_mode_t;

#define KD_GPIO_HIGH     1
#define KD_GPIO_LOW      0
#define LED_PIN_NUM1    33
#define LED_PIN_NUM2    32
#define	GPIO_DM_OUTPUT           _IOW('G', 0, int)
#define	GPIO_DM_INPUT            _IOW('G', 1, int)
#define	GPIO_DM_INPUT_PULL_UP    _IOW('G', 2, int)
#define	GPIO_DM_INPUT_PULL_DOWN  _IOW('G', 3, int)
#define	GPIO_WRITE_LOW           _IOW('G', 4, int)
#define	GPIO_WRITE_HIGH          _IOW('G', 5, int)

#define	GPIO_PE_RISING           _IOW('G', 7, int)
#define	GPIO_PE_FALLING          _IOW('G', 8, int)
#define	GPIO_PE_BOTH             _IOW('G', 9, int)
#define	GPIO_PE_HIGH             _IOW('G', 10, int)
#define	GPIO_PE_LOW              _IOW('G', 11, int)

#define GPIO_READ_VALUE       	_IOW('G', 12, int)
int unitest_gpio_read_write(int fd)
{
    int ret;
    pin_mode_t mode27;
    mode27.pin = LED_PIN_NUM2;
    ret = ioctl(fd, GPIO_DM_OUTPUT, &mode27);  //pin33 input
    if (ret)
    {
        perror("ioctl /dev/pin err\n");
        return -1;
    }
    ioctl(fd, GPIO_WRITE_LOW, &mode27);
    ioctl(fd, GPIO_WRITE_HIGH, &mode27);
    ioctl(fd, GPIO_READ_VALUE, &mode27);
    printf("mode27.mode %d\n",mode27.mode);
    return ret;
}

int gpio_write_high(int fd)
{
    int ret;
    pin_mode_t mode27;
    mode27.pin = LED_PIN_NUM2;
    // ret = ioctl(fd, GPIO_DM_OUTPUT, &mode27);
    // if (ret)
    // {
    //     perror("ioctl /dev/pin err\n");
    //     return -1;
    // }
    // printf("---mode27.mode %d\n",mode27.mode);
    ioctl(fd, GPIO_WRITE_HIGH, &mode27);
    return ret;
}

int gpio_write_low(int fd)
{
    int ret;
    pin_mode_t mode27;
    mode27.pin = LED_PIN_NUM2;
    // ret = ioctl(fd, GPIO_DM_OUTPUT, &mode27);
    // if (ret)
    // {
    //     perror("ioctl /dev/pin err\n");
    //     return -1;
    // }
    ioctl(fd, GPIO_WRITE_LOW, &mode27);
    return ret;
}

int sample_gpio_op(void)
{
    int ret;
    int gpio_fd = -1;
    pin_mode_t mode27;
    mode27.pin = LED_PIN_NUM2;
    gpio_fd = open("/dev/gpio", O_RDWR);
    if (gpio_fd < 0)
    {
        perror("open /dev/pin err\n");
        return -1;
    }
    ret = ioctl(gpio_fd, GPIO_DM_OUTPUT, &mode27);
    return gpio_fd;
}

static int test_fd;
static k_u64 start_time,stop_time;
#define TEST_TIME(func, func_name)     start_time = perf_get_smodecycles(); \
                                       func; \
                                       stop_time = perf_get_smodecycles(); \
                                       printf("%s use time:%d ms\n", func_name,(stop_time - start_time) / TIME_RATE);

static inline void TEST_BOOT_TIME_INIT(void)
{
    // int test_fd = -1;
    test_fd = sample_gpio_op();
    gpio_write_high(test_fd);
    gpio_write_low(test_fd);
}


static inline void TEST_BOOT_TIME_TRIGER(void)
{
    gpio_write_high(test_fd);
    gpio_write_low(test_fd);
}

static inline void PRINT_TIME_NOW(void)
{
    printf("current time:%ld ms\n", perf_get_smodecycles()/TIME_RATE);
}
#else
#define TEST_TIME(func, func_name)  func
#define TEST_BOOT_TIME_INIT()
#define TEST_BOOT_TIME_TRIGER()
#define PRINT_TIME_NOW()
#endif

typedef struct 
{
    pthread_mutex_t ai_mutex;

}vicap_attr;


int sample_sys_bind_init(k_vicap_dev vicap_dev, k_vicap_chn vicap_chn, k_s32 vo_chn);

std::atomic<bool> quit(true);

bool app_run = true;

void fun_sig(int sig)
{
    if(sig == SIGINT)
    {
        printf("recive ctrl+c\n");
        quit.store(false);
    }
}

uint64_t perf_get_smodecycles(void)
{
    uint64_t cnt;
    __asm__ __volatile__(
        "rdcycle %0" : "=r"(cnt)
    );
    return cnt;
}

k_vo_draw_frame vo_frame = (k_vo_draw_frame) {
    1,
    16,
    16,
    128,
    128,
    1
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

k_s32 sample_connector_init(void)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type = LT9611_MIPI_4LAN_1920X1080_30FPS;// HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
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

static k_u32 sample_vo_creat_osd_test(k_vo_osd osd, osd_info *info)
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

static k_s32 vo_layer_vdss_bind_vo_config(void)
{
    layer_info info;
    osd_info osd;
    k_vo_layer chn_id = K_VO_LAYER1;
    k_vo_osd osd_id = K_VO_OSD0;

    memset(&info, 0, sizeof(info));
    memset(&osd, 0, sizeof(osd));

    sample_connector_init();

    info.act_size.width = ISP_CHN0_WIDTH;//1080;//640;//1080;
    info.act_size.height = ISP_CHN0_HEIGHT;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = 0;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0;//(1080-w)/2,
    info.offset.y = 0;//(1920-h)/2;
    vo_creat_layer_test(chn_id, &info);

    chn_id = K_VO_LAYER2;
    memset(&info, 0, sizeof(info));
    info.act_size.width = ISP_CHN0_WIDTH;//1080;//640;//1080;
    info.act_size.height = ISP_CHN0_HEIGHT;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = 0;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0 + 640;//(1080-w)/2,
    info.offset.y = 0 + 480;//(1920-h)/2;
    vo_creat_layer_test(chn_id, &info);


    osd.act_size.width = ISP_CHN0_WIDTH ;
    osd.act_size.height = ISP_CHN0_HEIGHT;
    osd.offset.x = 0 + 1280;
    osd.offset.y = 0;
    osd.global_alptha = 0xff;// 0x7f;
    osd.format = PIXEL_FORMAT_RGB_888;
    sample_vo_creat_osd_test(osd_id, &osd);

    return 0;
}

static void sample_vo_fn(void *arg)
{
    usleep(10000);
    vo_layer_vdss_bind_vo_config();
    sample_sys_bind_init(VICAP_DEV_ID_0, VICAP_CHN_ID_0, K_VO_DISPLAY_CHN_ID1);
    sample_sys_bind_init(VICAP_DEV_ID_1, VICAP_CHN_ID_0, K_VO_DISPLAY_CHN_ID2);
    sample_sys_bind_init(VICAP_DEV_ID_2, VICAP_CHN_ID_0, K_VO_DISPLAY_CHN_ID3);
    return;
}

static void *sample_vo_thread(void *arg)
{
    TEST_TIME(sample_vo_fn(arg), "sample_vo_fn");
    return NULL;
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

int sample_sys_bind_init(k_vicap_dev vicap_dev, k_vicap_chn vicap_chn, k_s32 vo_chn)
{
    k_s32 ret = 0;
    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
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
    return ret;
}

#define VICAP_INPUT_BUF_NUM    4
#define VICAP_OUTPUT_BUF_NUM   5
int sample_vb_init(void)
{
    k_s32 ret;
    k_vb_config config;
    
    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    //VB for YUV420SP output for dev0 
    config.comm_pool[0].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[0].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);

    //VB for RGB888 output for dev0
    config.comm_pool[1].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);

    //VB for YUV420SP output for dev1
    config.comm_pool[2].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[2].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);

    //VB for RGB888 output for dev1
    config.comm_pool[3].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[3].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);

    //VB for rgb888 output for dev2
    config.comm_pool[4].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[4].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);

    //VB for RGB888 output for dev2
    config.comm_pool[5].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[5].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[5].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);

    //VB for VICAP_INPUT_BUF_NUM output for dev0
    config.comm_pool[6].blk_cnt = VICAP_INPUT_BUF_NUM;
    config.comm_pool[6].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[6].blk_size = VICAP_ALIGN_UP((ISP_INPUT_WIDTH * ISP_INPUT_HEIGHT * 3 ), VICAP_ALIGN_1K);

    //VB for VICAP_INPUT_BUF_NUM output for dev1
    config.comm_pool[7].blk_cnt = VICAP_INPUT_BUF_NUM;
    config.comm_pool[7].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[7].blk_size = VICAP_ALIGN_UP((ISP_INPUT_WIDTH * ISP_INPUT_HEIGHT * 3 ), VICAP_ALIGN_1K);

    //VB for VICAP_INPUT_BUF_NUM output for dev2
    config.comm_pool[8].blk_cnt = VICAP_INPUT_BUF_NUM;
    config.comm_pool[8].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[8].blk_size = VICAP_ALIGN_UP((ISP_INPUT_WIDTH * ISP_INPUT_HEIGHT * 3 ), VICAP_ALIGN_1K);


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
    }
    return ret;
}

int sample_vivcap_init(k_vicap_dev dev_chn, k_vicap_sensor_type type)
{
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;
    k_vicap_sensor_type sensor_type;
    k_s32 ret = 0;

    memset(&dev_attr, 0 ,sizeof(dev_attr));
    memset(&chn_attr, 0 ,sizeof(chn_attr));
    memset(&sensor_info, 0 ,sizeof(sensor_info));

    // sensor_type =  IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    sensor_type = type ;//OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR;
    // kd_mpi_vicap_set_mclk(VICAP_MCLK0, VICAP_PLL0_CLK_DIV4, 16, 1);
    vicap_dev = dev_chn;//VICAP_DEV_ID_0;

    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_get_sensor_info(sensor_type, &sensor_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = ISP_INPUT_WIDTH;
    dev_attr.acq_win.height = ISP_INPUT_HEIGHT;
    dev_attr.mode = VICAP_WORK_OFFLINE_MODE;  

    if(dev_attr.mode == VICAP_WORK_OFFLINE_MODE)
    {
        dev_attr.mode = VICAP_WORK_OFFLINE_MODE;
        dev_attr.buffer_num = VICAP_INPUT_BUF_NUM;
        dev_attr.buffer_size = VICAP_ALIGN_UP((ISP_INPUT_WIDTH * ISP_INPUT_HEIGHT * 2), VICAP_ALIGN_1K);
    }

    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    dev_attr.dw_enable = K_FALSE;

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

    chn_attr.crop_win = dev_attr.acq_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    if(vicap_dev == VICAP_DEV_ID_2)
    {
        chn_attr.pix_format = PIXEL_FORMAT_RGB_888;
        chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);

    }
    else
    {
        chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
        chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);
    }

    chn_attr.buffer_num = VICAP_OUTPUT_BUF_NUM - 1;//at least 3 buffers for isp
    vicap_chn = VICAP_CHN_ID_0;

    // printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, vicap_chn, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }

    //set chn1 output rgb888p
    chn_attr.out_win.h_start = 0;
    chn_attr.out_win.v_start = 0;
    chn_attr.out_win.width = ISP_CHN1_WIDTH ;
    chn_attr.out_win.height = ISP_CHN1_HEIGHT;

    chn_attr.crop_win = dev_attr.acq_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_BGR_888_PLANAR;
    chn_attr.buffer_num = VICAP_OUTPUT_BUF_NUM;//at least 3 buffers for isp
    chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);

    // printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, VICAP_CHN_ID_1, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }
    // set to header file database parse mode
    ret = kd_mpi_vicap_set_database_parse_mode(vicap_dev, VICAP_DATABASE_PARSE_XML_JSON);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_database_parse_mode failed.\n");
        return ret;
    }

    // printf("sample_vicap ...kd_mpi_vicap_init\n");
    ret = kd_mpi_vicap_init(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        return ret;
    }
    // ret = kd_mpi_vicap_start_stream(vicap_dev);
    // if (ret) {
    //     printf("sample_vicap, kd_mpi_vicap_start_stream failed.\n");
    //     return ret;
    // }
    return ret;
}

static k_s32 sample_vicap_stream(k_vicap_dev vicap_dev, k_bool en)
{
    k_s32 ret = 0;
    if(en)
    {
        ret = kd_mpi_vicap_start_stream(vicap_dev);
        if (ret) {
            printf("sample_vicap, kd_mpi_vicap_start_stream failed.\n");
            return ret;
        }
    }
    else
    {
        ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
        if (ret) {
            printf("sample_vicap, stop stream failed.\n");
            return ret;
        }
    }
    return ret;
}


k_u8 exit_flag = 0;

static void *exit_app(void *arg)
{
    printf("press 'q' to exit application!!\n");
    while(getchar() != 'q')
    {
        usleep(10000);
    }
    app_run = false;

    exit_flag = exit_flag  | (1 << 0);

    printf("exit_app exit_flag is %x  \n", exit_flag);
    return NULL;
}

MobileRetinaface *pmodel;

static void *sample_vicap_dev0_ai_thread(void *arg)
{
    k_s32 ret = 0;
    int face_count = 1;
    k_video_frame_info dump_info;
    vicap_attr *ai_attr = (vicap_attr *)arg;
    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    size_t size = CHANNEL * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;
    
    DetectResult box_result;
    std::vector<face_coordinate> boxes;

    k_vicap_dev vicap_dev = VICAP_DEV_ID_0;
    
    while(app_run)
    {
        memset(&dump_info, 0 , sizeof(k_video_frame_info));
        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
        if (ret) {
            // quit.store(false);
            printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
            continue;
        }

        auto vbvaddr = kd_mpi_sys_mmap(dump_info.v_frame.phys_addr[0], size);
        // memcpy(vaddr, (void *)vbvaddr, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3);
        boxes.clear();
        // run kpu
        TEST_BOOT_TIME_TRIGER();

        pthread_mutex_lock(&ai_attr->ai_mutex);

        // model.run(reinterpret_cast<uintptr_t>(vbvaddr), reinterpret_cast<uintptr_t>(dump_info.v_frame.phys_addr[0]));
        // get face boxes
        // box_result = model.get_result();
        pmodel->run(reinterpret_cast<uintptr_t>(vbvaddr), reinterpret_cast<uintptr_t>(dump_info.v_frame.phys_addr[0]));
        box_result = pmodel->get_result();
        boxes = box_result.boxes;
        // printf("0000000000000 \n");
        pthread_mutex_unlock(&ai_attr->ai_mutex); // 解锁

        kd_mpi_sys_munmap(vbvaddr, size);
#ifdef TEST_BOOT_TIME
        TEST_BOOT_TIME_TRIGER();
        if(boxes.size() > 0)
        {
            num++;
            if(num == 1)
            {
                TEST_BOOT_TIME_TRIGER();
            }
            printf("boxes %llu \n",(perf_get_smodecycles()));
        }
#endif
        if(boxes.size() < face_count)
        {
            for (size_t i = boxes.size(); i < face_count; i++)
            {
                if(i > 4)
                    continue;

                vo_frame.draw_en = 0;
                vo_frame.frame_num = i + 1;
                kd_mpi_vo_draw_frame(&vo_frame);
            }
        }

        
        for (size_t i = 0, j = 0; i < boxes.size(); i += 1)
        {
            // std::cout << "[" << boxes[i] << ", " << boxes[i + 1] << ", " << boxes[i + 2] <<", " << boxes[i + 3] << "]" << std::endl;

            // draw frame  < 5
            if(i > 4)
                continue;

            vo_frame.draw_en = 1;
            vo_frame.line_x_start = ((uint32_t)boxes[i].x1) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH;
            vo_frame.line_y_start = ((uint32_t)boxes[i].y1) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT;
            vo_frame.line_x_end = ((uint32_t)boxes[i].x2) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH;
            vo_frame.line_y_end = ((uint32_t)boxes[i].y2) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT;
            vo_frame.frame_num = ++j;
            kd_mpi_vo_draw_frame(&vo_frame);
        }

        face_count = boxes.size();
        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
        if (ret) {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }
        usleep(1000);
    }

    boxes.clear();
    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, stop stream failed.\n");
    }
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
    }

    usleep(1000 * display_ms);
    printf("disable layer1  \n");

    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);

    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_0;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    sample_vicap_unbind_vo(vicap_mpp_chn, vo_mpp_chn);
    
    exit_flag = exit_flag  | (1 << 1);

    printf("sample_vicap_dev0_ai_thread exit_flag is %x  \n", exit_flag);
    return NULL;
}

static void *sample_vicap_dev1_ai_thread(void *arg)
{
    k_s32 ret = 0;
    int face_count = 1;
    k_video_frame_info dump_info;
    vicap_attr *ai_attr = (vicap_attr *)arg;
    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    size_t size = CHANNEL * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;
    // MobileRetinaface model("test.kmodel", CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    DetectResult box_result;
    std::vector<face_coordinate> boxes;

    k_vicap_dev vicap_dev = VICAP_DEV_ID_1;
    
    while(app_run)
    {
        memset(&dump_info, 0 , sizeof(k_video_frame_info));
        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
        if (ret) {
            // quit.store(false);
            printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
            continue;
        }

        auto vbvaddr = kd_mpi_sys_mmap(dump_info.v_frame.phys_addr[0], size);
        // memcpy(vaddr, (void *)vbvaddr, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3);
        boxes.clear();
        // run kpu
        TEST_BOOT_TIME_TRIGER();

        pthread_mutex_lock(&ai_attr->ai_mutex);

        // model.run(reinterpret_cast<uintptr_t>(vbvaddr), reinterpret_cast<uintptr_t>(dump_info.v_frame.phys_addr[0]));
        // get face boxes
        // box_result = model.get_result();
        pmodel->run(reinterpret_cast<uintptr_t>(vbvaddr), reinterpret_cast<uintptr_t>(dump_info.v_frame.phys_addr[0]));
        box_result = pmodel->get_result();
        // printf("1111111111111 \n");
        boxes = box_result.boxes;

        pthread_mutex_unlock(&ai_attr->ai_mutex); // 解锁

        kd_mpi_sys_munmap(vbvaddr, size);
#ifdef TEST_BOOT_TIME
        TEST_BOOT_TIME_TRIGER();
        if(boxes.size() > 0)
        {
            num++;
            if(num == 1)
            {
                TEST_BOOT_TIME_TRIGER();
            }
            printf("boxes %llu \n",(perf_get_smodecycles()));
        }
#endif
        if(boxes.size() < face_count)
        {
            for (size_t i = boxes.size(); i < face_count; i++)
            {
                if(i > 4)
                    continue;

                vo_frame.draw_en = 0;
                vo_frame.frame_num = i + 1 + 5;
                kd_mpi_vo_draw_frame(&vo_frame);
            }
        }

        for (size_t i = 0, j = 0; i < boxes.size(); i += 1)
        {
            // std::cout << "[" << boxes[i] << ", " << boxes[i + 1] << ", " << boxes[i + 2] <<", " << boxes[i + 3] << "]" << std::endl;

            // only 5 fraw 
            if(i > 4)
                continue;

            vo_frame.draw_en = 1;
            vo_frame.line_x_start = ( ((uint32_t)boxes[i].x1) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH ) + 640;
            vo_frame.line_y_start = ( ((uint32_t)boxes[i].y1) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT ) + 480;
            vo_frame.line_x_end = ( ((uint32_t)boxes[i].x2) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH ) + 640;
            vo_frame.line_y_end = ( ((uint32_t)boxes[i].y2) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT ) + 480;
            vo_frame.frame_num = (++j) + 5;
            kd_mpi_vo_draw_frame(&vo_frame);
        }

        face_count = boxes.size();
        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
        if (ret) {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }
        usleep(1000);
    }

    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, stop stream failed.\n");
    }
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
    }

// vicap_init_error:

    usleep(1000 * display_ms);
    printf("disable layer1  \n");

    kd_mpi_vo_disable_video_layer(K_VO_LAYER2);

    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_0;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID2;

    sample_vicap_unbind_vo(vicap_mpp_chn, vo_mpp_chn);
    
    exit_flag = exit_flag  | (1 << 2);

    printf("sample_vicap_dev1_ai_thread exit_flag is %x  \n", exit_flag);
}


static void *sample_vicap_dev2_ai_thread(void *arg)
{
    k_s32 ret = 0;
    int face_count = 1;
    k_video_frame_info dump_info;
    vicap_attr *ai_attr = (vicap_attr *)arg;
    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    size_t size = CHANNEL * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;
    // MobileRetinaface model("test.kmodel", CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    DetectResult box_result;
    std::vector<face_coordinate> boxes;

    k_vicap_dev vicap_dev = VICAP_DEV_ID_2;
    
    while(app_run)
    {
        memset(&dump_info, 0 , sizeof(k_video_frame_info));
        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
        if (ret) {
            quit.store(false);
            printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
            break;
        }

        auto vbvaddr = kd_mpi_sys_mmap(dump_info.v_frame.phys_addr[0], size);
        // memcpy(vaddr, (void *)vbvaddr, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3);
        boxes.clear();
        // run kpu
        TEST_BOOT_TIME_TRIGER();

        pthread_mutex_lock(&ai_attr->ai_mutex);

        // model.run(reinterpret_cast<uintptr_t>(vbvaddr), reinterpret_cast<uintptr_t>(dump_info.v_frame.phys_addr[0]));
        // get face boxes
        // box_result = model.get_result();
        pmodel->run(reinterpret_cast<uintptr_t>(vbvaddr), reinterpret_cast<uintptr_t>(dump_info.v_frame.phys_addr[0]));
        box_result = pmodel->get_result();
        // printf("1111111111111 \n");
        boxes = box_result.boxes;

        pthread_mutex_unlock(&ai_attr->ai_mutex); // 解锁

        kd_mpi_sys_munmap(vbvaddr, size);
#ifdef TEST_BOOT_TIME
        TEST_BOOT_TIME_TRIGER();
        if(boxes.size() > 0)
        {
            num++;
            if(num == 1)
            {
                TEST_BOOT_TIME_TRIGER();
            }
            printf("boxes %llu \n",(perf_get_smodecycles()));
        }
#endif
        if(boxes.size() < face_count)
        {
            for (size_t i = boxes.size(); i < face_count; i++)
            {
                if(i > 4)
                    continue;

                vo_frame.draw_en = 0;
                vo_frame.frame_num = i + 1 + 10;
                kd_mpi_vo_draw_frame(&vo_frame);
            }
        }

        for (size_t i = 0, j = 0; i < boxes.size(); i += 1)
        {
            // std::cout << "[" << boxes[i] << ", " << boxes[i + 1] << ", " << boxes[i + 2] <<", " << boxes[i + 3] << "]" << std::endl;

            // only 5 fraw 
            if(i > 4)
                continue;

            vo_frame.draw_en = 1;
            vo_frame.line_x_start = ( ((uint32_t)boxes[i].x1) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH ) + 1280;
            vo_frame.line_y_start = ( ((uint32_t)boxes[i].y1) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT );
            vo_frame.line_x_end = ( ((uint32_t)boxes[i].x2) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH ) + 1280;
            vo_frame.line_y_end = ( ((uint32_t)boxes[i].y2) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT );
            vo_frame.frame_num = (++j) + 10;
            kd_mpi_vo_draw_frame(&vo_frame);
        }

        face_count = boxes.size();
        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
        if (ret) {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }
        usleep(1000);
    }

    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, stop stream failed.\n");
    }
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
    }

// vicap_init_error:

    usleep(1000 * display_ms);
    printf("disable layer1  \n");

    kd_mpi_vo_osd_disable(K_VO_OSD0);

    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_0;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID3;

    sample_vicap_unbind_vo(vicap_mpp_chn, vo_mpp_chn);
    
    exit_flag = exit_flag  | (1 << 3);

    printf("sample_vicap_dev1_ai_thread exit_flag is %x  \n", exit_flag);
}

int main(int argc, char *argv[])
{
    int num = 0;
    int ret;
    PRINT_TIME_NOW();

    vicap_attr vicap_attr;

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <kmodel>" << std::endl;
        return -1;
    }
    /****fixed operation for ctrl+c****/
    struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = fun_sig;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    pthread_t vo_thread_handle;
    pthread_t exit_thread_handle;
    pthread_t ai0_thread_handle;
    pthread_t ai1_thread_handle;
    pthread_t ai2_thread_handle;
    pthread_create(&exit_thread_handle, NULL, exit_app, NULL);
    // size_t size = CHANNEL * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;;

    MobileRetinaface model(argv[1], CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);

    pmodel = &model;
    // DetectResult box_result;
    // std::vector<face_coordinate> boxes;

    TEST_BOOT_TIME_INIT();

    TEST_TIME(ret = sample_vb_init(), "sample_vb_init");
    if(ret) {
        goto vb_init_error;
    }

    pthread_create(&vo_thread_handle, NULL, sample_vo_thread, NULL);

    ret = pthread_mutex_init(&vicap_attr.ai_mutex, NULL);
    if(ret < 0)
        printf("mutex init failed \n");

    TEST_TIME(ret = sample_vivcap_init(VICAP_DEV_ID_0, OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR),"sample_vicap_init");
    TEST_TIME(ret = sample_vivcap_init(VICAP_DEV_ID_1, OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR),"sample_vicap_init");
    TEST_TIME(ret = sample_vivcap_init(VICAP_DEV_ID_2, OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR),"sample_vicap_init");

    sample_vicap_stream(VICAP_DEV_ID_0, K_TRUE);
    sample_vicap_stream(VICAP_DEV_ID_1, K_TRUE);
    sample_vicap_stream(VICAP_DEV_ID_2, K_TRUE);

    usleep(200000);

    pthread_create(&ai0_thread_handle, NULL, sample_vicap_dev0_ai_thread, &vicap_attr);
    pthread_create(&ai1_thread_handle, NULL,  sample_vicap_dev1_ai_thread, &vicap_attr);
    pthread_create(&ai2_thread_handle, NULL,  sample_vicap_dev2_ai_thread, &vicap_attr);

    while(exit_flag != 0x0f)
    {
        usleep(100000);
    }

    printf("exit_app exit_flag is %x  \n", exit_flag);

    for(size_t i = 0; i < 16 ;i++)
    {
        vo_frame.draw_en = 0;
        vo_frame.frame_num = i + 1;
        kd_mpi_vo_draw_frame(&vo_frame);
    }

    pthread_join(exit_thread_handle, NULL);
    pthread_join(vo_thread_handle, NULL);
    pthread_join(ai0_thread_handle, NULL);
    pthread_join(ai1_thread_handle, NULL);
    pthread_join(ai2_thread_handle, NULL);

    ret = pthread_mutex_destroy(&vicap_attr.ai_mutex);
    if(ret)
        printf("ai_mutex destroy failed \n");

    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("fastboot_app, kd_mpi_vb_exit failed.\n");
        return ret;
    }

vb_init_error:
    return 0;
}