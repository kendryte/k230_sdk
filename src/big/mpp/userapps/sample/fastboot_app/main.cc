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

#define CHANNEL 3
#define ISP_CHN1_HEIGHT (1280)
#define ISP_CHN1_WIDTH  (720)
#define ISP_CHN0_WIDTH  (1088)
#define ISP_CHN0_HEIGHT (1920)

#define ISP_INPUT_WIDTH (2592)
#define ISP_INPUT_HEIGHT (1944)
#define ISP_CROP_W_OFFSET (768)
#define ISP_CROP_H_OFFSET (16)
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


extern k_s32 kd_display_set_backlight(void);
extern k_s32 kd_display_reset(void);
int sample_sys_bind_init(void);

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
    usleep(300);
    kd_mpi_dsi_send_cmd(param24, 1);
    usleep(100);
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
    info.act_size.width = ISP_CHN0_WIDTH;//1080;//640;//1080;
    info.act_size.height = ISP_CHN0_HEIGHT;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = K_ROTATION_180;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0;//(1080-w)/2,
    info.offset.y = 0;//(1920-h)/2;
    vo_creat_layer_test(chn_id, &info);
    kd_mpi_vo_enable();
    return 0;
}

static void sample_vo_fn(void *arg)
{
    usleep(10000);
    // set hardware reset;
    kd_display_set_backlight();
	// rst display subsystem
    kd_display_reset();
    dwc_dsi_init();
    vo_layer_vdss_bind_vo_config();
    sample_sys_bind_init();
    return;
}

static void *sample_vo_thread(void *arg)
{
    TEST_TIME(sample_vo_fn(arg), "sample_vo_fn");
    return NULL;
}

k_vicap_dev vicap_dev;
k_vicap_chn vicap_chn;
k_vicap_dev_attr dev_attr;
k_vicap_chn_attr chn_attr;
k_vicap_sensor_info sensor_info;
k_vicap_sensor_type sensor_type;
k_video_frame_info dump_info;

static void sample_vicap_unbind_vo(k_mpp_chn vicap_mpp_chn, k_mpp_chn vo_mpp_chn)
{
    k_s32 ret;

    ret = kd_mpi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }
    return;
}

int sample_sys_bind_init(void)
{
    k_s32 ret = 0;
    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = vicap_chn;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    ret = kd_mpi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }
    return ret;
}

int sample_vb_init(void)
{
    k_s32 ret;
    k_vb_config config;
    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    //VB for YUV420SP output
    config.comm_pool[0].blk_cnt = 5;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[0].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);

    //VB for RGB888 output
    config.comm_pool[1].blk_cnt = 5;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);

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

int sample_vivcap_init( void )
{
    k_s32 ret = 0;
    sensor_type =  IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    vicap_dev = VICAP_DEV_ID_0;

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
    chn_attr.crop_win.h_start = ISP_CROP_W_OFFSET;
    chn_attr.crop_win.v_start = ISP_CROP_H_OFFSET;
    chn_attr.crop_win.width = ISP_CHN0_WIDTH;
    chn_attr.crop_win.height = ISP_CHN0_HEIGHT;

    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
    chn_attr.buffer_num = VICAP_MAX_FRAME_COUNT;//at least 3 buffers for isp
    chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);;
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

    chn_attr.crop_win.h_start = ISP_CROP_W_OFFSET;
    chn_attr.crop_win.v_start = ISP_CROP_H_OFFSET;
    chn_attr.crop_win.width = ISP_CHN0_WIDTH;
    chn_attr.crop_win.height = ISP_CHN0_HEIGHT;

    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_BGR_888_PLANAR;
    chn_attr.buffer_num = VICAP_MAX_FRAME_COUNT;//at least 3 buffers for isp
    chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);

    // printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, VICAP_CHN_ID_1, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }
    // set to header file database parse mode
    ret = kd_mpi_vicap_set_database_parse_mode(vicap_dev, VICAP_DATABASE_PARSE_HEADER);
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
    ret = kd_mpi_vicap_start_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_start_stream failed.\n");
        return ret;
    }
    return ret;
}

static void *exit_app(void *arg)
{
    printf("press 'q' to exit application!!\n");
    while(getchar() != 'q')
    {
        usleep(10000);
    }
    app_run = false;
    return NULL;
}

int main(int argc, char *argv[])
{
    /*Allow one frame time for the VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    int face_count = 1;

    int num = 0;
    int ret;
    PRINT_TIME_NOW();
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
    pthread_create(&exit_thread_handle, NULL, exit_app, NULL);
    size_t size = CHANNEL * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;;

    MobileRetinaface model(argv[1], CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    std::vector<int> boxes;

    TEST_BOOT_TIME_INIT();

    TEST_TIME(ret = sample_vb_init(), "sample_vb_init");
    if(ret) {
        goto vb_init_error;
    }

    pthread_create(&vo_thread_handle, NULL, sample_vo_thread, NULL);
    TEST_TIME(ret = sample_vivcap_init(),"sample_vicap_init");
    pthread_join(vo_thread_handle, NULL);
    if(ret) {
        goto vicap_init_error;
    }

    TEST_BOOT_TIME_TRIGER();
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

        model.run(reinterpret_cast<uintptr_t>(vbvaddr), reinterpret_cast<uintptr_t>(dump_info.v_frame.phys_addr[0]));
        kd_mpi_sys_munmap(vbvaddr, size);
        // model.run();
        // get face boxes
        boxes = model.get_result();
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
        if(boxes.size() / 4 < face_count)
        {
            for (size_t i = boxes.size() / 4; i < face_count; i++)
            {
                vo_frame.draw_en = 0;
                vo_frame.frame_num = i + 1;
                kd_mpi_vo_draw_frame(&vo_frame);
            }
        }
        for (size_t i = 0, j = 0; i < boxes.size(); i += 4)
        {
            // std::cout << "[" << boxes[i] << ", " << boxes[i + 1] << ", " << boxes[i + 2] <<", " << boxes[i + 3] << "]" << std::endl;
            vo_frame.draw_en = 1;
            vo_frame.line_x_start = (ISP_CHN1_WIDTH - (uint32_t)boxes[2 + i]) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH;
            vo_frame.line_y_start = (ISP_CHN1_HEIGHT - (uint32_t)boxes[3 + i]) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT;
            vo_frame.line_x_end = (ISP_CHN1_WIDTH - (uint32_t)boxes[0 + i]) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH;
            vo_frame.line_y_end = (ISP_CHN1_HEIGHT - (uint32_t)boxes[1 + i]) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT;
            vo_frame.frame_num = ++j;
            kd_mpi_vo_draw_frame(&vo_frame);
        }
        face_count = boxes.size() / 4;
        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
        if (ret) {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }
    }

app_exit:
    pthread_join(exit_thread_handle, NULL);
    for(size_t i = 0;i < boxes.size()/4;i++)
    {
        vo_frame.draw_en = 0;
        vo_frame.frame_num = i + 1;
        kd_mpi_vo_draw_frame(&vo_frame);
    }
    boxes.clear();
    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, stop stream failed.\n");
    }
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        return ret;
    }

vicap_init_error:
    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);

    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = vicap_dev;
    vicap_mpp_chn.chn_id = vicap_chn;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    sample_vicap_unbind_vo(vicap_mpp_chn, vo_mpp_chn);
    usleep(1000 * display_ms);

    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("fastboot_app, kd_mpi_vb_exit failed.\n");
        return ret;
    }

vb_init_error:
    return 0;
}