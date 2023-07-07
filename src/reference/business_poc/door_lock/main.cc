#include <chrono>
#include <fstream>
#include <iostream>
#include <sys/prctl.h>
#include <nncase/runtime/runtime_op_utility.h>
#include "mobile_retinaface.h"
#include "mobile_face.h"
#include "util.h"
#include "mpi_sys_api.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;


/* vicap */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <signal.h>
#include <atomic>
#include <fcntl.h>
#include "k_module.h"
#include <dirent.h>
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
#include "sys/ioctl.h"
#include "vo_test_case.h"
#include "mpi_dma_api.h"
#include "k_ipcmsg.h"
#include "sample_define.h"
#include "vi_vo.h"

#define CONFIG_MEM_FACE_DATA_BASE 0x14000000 //feature_data;
#define CONFIG_MEM_FACE_DATA_SIZE 0x00040000 //feature mem size
#define FEATURE_SIZE 768

#define CHANNEL 3
#define ISP_CHN1_HEIGHT (1280)
#define ISP_CHN1_WIDTH  (720)
#define ISP_CHN0_WIDTH  (1088)
#define ISP_CHN0_HEIGHT (1920)

#define ISP_INPUT_WIDTH (2592)
#define ISP_INPUT_HEIGHT (1944)
#define ISP_CROP_W_OFFSET (768)
#define ISP_CROP_H_OFFSET (16)
#define LED_PIN_NUM1    33
#define LED_PIN_NUM2    32
#define IPC_MSG "door_lock"
#define TEST_BOOT_TIME

#ifdef TEST_BOOT_TIME
#define TIME_RATE (1600*1000)
#define SEND_TIME_INTERVAL_US   (100 * 1000)
uint64_t perf_get_smodecycles(void);
typedef struct kd_pin_mode
{
    unsigned short pin;     /* pin number, from 0 to 63 */
    unsigned short mode;    /* pin level status, 0 low level, 1 high level */
} pin_mode_t;

#define KD_GPIO_HIGH     1
#define KD_GPIO_LOW      0

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

feature_db_t *mem_feature_data;
static bool key_press = false;
bool sensor_process = true;
bool is_clear_feature = false;
std::string name;
k_s32 s32Id1;
pthread_t threadid1;
pthread_t ipc_run_handle;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char dir_name[40];
int mem_fd = -1;
int gpio_write_high(int fd)
{
    int ret;
    pin_mode_t mode27;
    mode27.pin = LED_PIN_NUM2;
    ioctl(fd, GPIO_WRITE_HIGH, &mode27);
    return ret;
}

int gpio_write_low(int fd)
{
    int ret;
    pin_mode_t mode27;
    mode27.pin = LED_PIN_NUM2;
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
    ret = ioctl(gpio_fd, GPIO_DM_INPUT, &mode27);
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
    test_fd = sample_gpio_op();
    // gpio_write_high(test_fd);
    // gpio_write_low(test_fd);
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

int get_keyvalue(int fd)
{
    int ret;
    pin_mode_t mode32;
    mode32.pin = LED_PIN_NUM2;
    ioctl(fd, GPIO_READ_VALUE, &mode32);
    return mode32.mode;
}
extern k_s32 kd_display_set_backlight(void);
extern k_s32 kd_display_reset(void);
int sample_sys_bind_init(void);

std::atomic<bool> quit(true);

#define BLOCK_TIME              100

int key_values = 0;
int pressed_key = 0;
k_dma_dev_attr_t dma_dev_attr;
k_dma_chn_attr_u dma_chn_attr[DMA_MAX_CHN_NUMS];
k_video_frame_info df_info_dst;
bool app_run = true;


void read_mem_feature()
{
    int feature_num = 0;
    mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (mem_fd < 0) {
        printf("open /dev/mem error\n");
        return;
    }
    mem_feature_data = (feature_db_t *)mmap(NULL, CONFIG_MEM_FACE_DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, CONFIG_MEM_FACE_DATA_BASE);
    if(mem_feature_data->count > 100 )
    {      
        mem_feature_data->count = 0;
    }
    return;
}

int clear_feature(void)
{
    pthread_mutex_lock(&mutex);
    mem_feature_data->count = 0;    
    pthread_mutex_unlock(&mutex);
}



int ipc_send_thread(ipc_msg_cmd_t cmd)
{
    k_u32 u32Cmd;
    void *content;
    uint8_t success = 0;
    uint8_t fail = 1;
    uint32_t size = mem_feature_data->count*sizeof(feature) + sizeof(uint32_t);
    uint32_t cmd_feature[2];
    switch(cmd)
    {
        case MSG_CMD_SIGNUP_RESULT:
        case MSG_CMD_IMPORT_RESULT:
        case MSG_CMD_DELETE_RESULT:
            content = &success;
            break;
        case MSG_CMD_FEATURE_SAVE:
            // sprintf((char*)content,"%x%d",CONFIG_MEM_FACE_DATA_BASE,size);
            cmd_feature[0] = CONFIG_MEM_FACE_DATA_BASE;
            cmd_feature[1] = size;
            content = cmd_feature;
            break;
        case MSG_CMD_ERROR:
            content = &fail;
            break;            
        default:
            printf("can not recongnise ipc_msg cmd\n");
            break;
    }
    k_ipcmsg_message_t* pReq = kd_ipcmsg_create_message(SEND_ONLY_MODULE_ID, cmd, content,sizeof(content));
    kd_ipcmsg_send_only(s32Id1, pReq);
    kd_ipcmsg_destroy_message(pReq);
    usleep(SEND_TIME_INTERVAL_US * 3);
    return 0;
}

void handle_feature(k_s32 s32Id, k_ipcmsg_message_t* msg)
{
    k_s32 s32Ret = 0;
    char content[64];

    memset(content, 0, 64);
    switch(msg->u32Module)
    {
        case SEND_SYNC_MODULE_ID:
            snprintf(content, 64, "modle:%d, cmd:%08x, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SNED_ASYNC_MODULE_ID:
            snprintf(content, 64, "modle:%d, cmd:%08x, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SEND_ONLY_MODULE_ID:
            /*
            If a reply message is created for kd_ipcmsg_send_only,
            it will trigger the "Sync msg is too late" alert on the other side..
            */
            printf("receive msg from %d: %s, len: %d\n", s32Id, (char*)msg->pBody, msg->u32BodyLen);
            return;
        default:
            snprintf(content, 64, "modle:%d, cmd:%08x, is not found.", msg->u32Module, msg->u32CMD);
            s32Ret = -1;
    }
    switch(msg->u32CMD)
    {
        case MSG_CMD_IMPORT:
            sensor_process = false;
            sprintf(dir_name,"%s",msg->pBody);
            ipc_send_thread(MSG_CMD_IMPORT_RESULT);
            break;
        case MSG_CMD_SIGNUP:
            key_press = true;
            name = (char*)msg->pBody;
            ipc_send_thread(MSG_CMD_SIGNUP_RESULT);
            break;
        case MSG_CMD_DELETE:
            clear_feature();
            ipc_send_thread(MSG_CMD_DELETE_RESULT);
            break;
        default:
            printf("can not recongnise ipc_msg cmd\n");
            break;
    }
}


static void* thread_ipcmsg(void* arg)
{
    kd_ipcmsg_run(s32Id1);
    return NULL;
}

void *ipc_msg_server(void *arg)
{
    
    int ret = 0;
    k_ipcmsg_connect_t stConnectAttr;

    stConnectAttr.u32RemoteId = 0;
    stConnectAttr.u32Port = 101;
    stConnectAttr.u32Priority = 0;
    kd_ipcmsg_add_service(IPC_MSG,&stConnectAttr);

    if(ret != 0)
    {
        printf("kd_ipcmsg_add_service return err:%x\n", ret);
    }
    ret = kd_ipcmsg_connect(&s32Id1, IPC_MSG, handle_feature);
    if(ret != 0)
    {
        printf("Connect fail\n");
    }
    kd_ipcmsg_run(s32Id1);
    return NULL;
}

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
    usleep(30000);
    kd_mpi_dsi_send_cmd(param24, 1);
    usleep(10000);
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

    info.act_size.width = ISP_CHN0_WIDTH;//1080;//640;//1080;
    info.act_size.height = ISP_CHN0_HEIGHT;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = K_ROTATION_0;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0;//(1080-w)/2,
    info.offset.y = 0;//(1920-h)/2;
    vo_creat_layer_test(chn_id, &info);
    if (vicap_install_osd == 1)
    {
        osd_info osd;
        osd.act_size.width = ISP_CHN0_WIDTH;
        osd.act_size.height = ISP_CHN0_HEIGHT;
        osd.offset.x = 0;
        osd.offset.y = 0;
        osd.global_alptha = 0xff;
        // osd.global_alptha = 0x32;
        osd.format = PIXEL_FORMAT_ARGB_8888; // PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

        vo_creat_osd_test(osd_id, &osd);
    }
    kd_mpi_vo_enable();
    return 0;
}

static void sample_vo_fn(void *arg)
{
    // set hardware reset;
    kd_display_set_backlight();
	// rst display subsystem
    kd_display_reset();
    dwc_dsi_init();
    vo_layer_vdss_bind_vo_config();
    sample_sys_bind_init();
    return;
}

static int sample_vo_init(void)
{
    kd_display_set_backlight();
	// rst display subsystem
    kd_display_reset();
    dwc_dsi_init();
    vo_layer_vdss_bind_vo_config();
    return 0;
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
k_vb_config config;

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
    vicap_mpp_chn.dev_id = VICAP_DEV_ID_0;
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_0;

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
    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    config.comm_pool[0].blk_cnt = 5;
    config.comm_pool[0].blk_size = VICAP_ALIGN_UP((ISP_CHN0_HEIGHT * ISP_CHN0_WIDTH * 3 / 2), VICAP_ALIGN_1K);
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    //gdma
    // config.comm_pool[1].blk_cnt = 4;
    // config.comm_pool[1].blk_size = ISP_OUT_HEIGHT*ISP_OUT_WIDTH*3;
    // config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
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
    sensor_type = IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
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
    //set chn0 output yuv
    chn_attr.out_win.h_start = 0;
    chn_attr.out_win.v_start = 0;
    chn_attr.out_win.width = ISP_CHN0_WIDTH;
    chn_attr.out_win.height = ISP_CHN0_HEIGHT;

    chn_attr.crop_win.h_start = ISP_CROP_W_OFFSET;
    chn_attr.scale_win.v_start = ISP_CROP_H_OFFSET;
    chn_attr.crop_win.width = ISP_CHN0_WIDTH;
    chn_attr.crop_win.height = ISP_CHN0_HEIGHT;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
    chn_attr.buffer_num = VICAP_MAX_FRAME_COUNT;//at least 3 buffers for isp
    chn_attr.buffer_size = config.comm_pool[0].blk_size;
    vicap_chn = VICAP_CHN_ID_0;

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
    chn_attr.buffer_size =  config.comm_pool[1].blk_size;

    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, VICAP_CHN_ID_1, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }

    ret = kd_mpi_vicap_init(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        return ret;
        // goto err_exit;
    }
    ret = kd_mpi_vicap_start_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        return ret;
        // goto err_exit;
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
}

void draw_result(cv::Mat& src_img,box_t& bbox,FaceMaskInfo& result, bool pic_mode,bool is_label,std::vector<cv::Point2f> points1)
{
    int src_w = src_img.cols;
    int src_h = src_img.rows;
    int max_src_size = std::max(src_w,src_h);

    char text[30];
    // sprintf(text, "%.2f",result.score);
	sprintf(text, "%s",result.label.c_str());

    if(pic_mode)
    {
        cv::rectangle(src_img, cv::Rect(bbox.x, bbox.y , bbox.w, bbox.h), cv::Scalar(255, 255, 255), 2, 2, 0);
        if(is_label == true)
        {
            if(result.score<mask_thresh_)
                cv::putText(src_img, text , {bbox.x,std::max(int(bbox.y-10),0)}, cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 0, 0), 1, 8, 0);
            else
                cv::putText(src_img, text , {bbox.x,std::max(int(bbox.y-10),0)}, cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(0, 0, 255), 1, 8, 0);
            is_label = false;
        }

    }
    else
    {
        int x = bbox.x;
        int y = bbox.y;
        int w = bbox.w;
        int h = bbox.h;
        cv::rectangle(src_img, cv::Rect(x, y , w, h), cv::Scalar(255,255, 255, 255), 2, 2, 0);
        for(int i = 0; i < points1.size();i++)
        {
            // printf(" circle %f %f \n",points1[i].x,points1[i].y);
            cv::circle(src_img,points1[i],3,cv::Scalar(255,255, 0, 0), 3);
        }
            
        if(is_label == true)
        {
            if(result.score<mask_thresh_)
                cv::putText(src_img, text, {std::max(int(x),0), std::max(int(y - 10),0)}, cv::FONT_HERSHEY_COMPLEX, 2.5, cv::Scalar(255,255, 0, 0), 4, 8, 0);
            else
                cv::putText(src_img, text, {std::max(int(x-10),0), std::max(int(y - 10),0)}, cv::FONT_HERSHEY_COMPLEX, 2.5, cv::Scalar(255,0, 0, 255), 4, 8, 0);
            is_label = false;
        }
        
        
    }
    return;  
}

void getFileNames(char *path, std::vector<std::string>& files)
{
    DIR *dir;
    struct dirent *ptr; 
    dir = opendir("/sharefs/pic/");
    char pic_name[128];
    if(dir == NULL)
    {
        return;
    }
    while((ptr = readdir(dir)) != NULL)
    {
        if(strstr(ptr->d_name,".jpg"))
        {
            printf("ptr->d_name %s\n",ptr->d_name);
            files.push_back(ptr->d_name);
        }

    }
    closedir(dir);
    return;

}



int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <kmodel>" << std::endl;
        return -1;
    }
    k_u32 display_ms = 1000 / 33;
    int face_count = 32;   
    size_t paddr = 0;
    void *vaddr = nullptr;
    DetectResult detect_result;
    std::vector<float> feature_result;
    std::vector<box_t> _draw_result;
    bool depth_pred = true;
    bool ir_pred = false;
    float score_max = 0;
    int score_index = 0;
    float score_threshold = 0.82f;
    bool is_label = false;

    pthread_t vo_thread_handle;
    pthread_t exit_thread_handle;
    // pthread_t key_opreation_handle;
    pthread_t ipc_message_handle;

    size_t size = CHANNEL * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;

    k_u32 pool_id;
    k_vb_pool_config pool_config;
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL; 
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
    read_mem_feature();
    // MobileRetinaface retinaface(argv[1], ai2d_input_h, ai2d_input_w);
    MobileRetinaface retinaface((const char*)argv[1], CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    MobileFace mf((const char*)argv[2], CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    // MobileRetinaface retinaface((const char*)argv[1], CHANNEL, ISP_CHN1_WIDTH, ISP_CHN1_HEIGHT);
    // MobileFace mf((const char*)argv[2], CHANNEL, ISP_CHN1_WIDTH, ISP_CHN1_HEIGHT);
    pthread_create(&ipc_message_handle, NULL, ipc_msg_server, NULL);
    pthread_create(&exit_thread_handle, NULL, exit_app, NULL);
    // pthread_create(&key_opreation_handle, NULL, Get_KeyValue, NULL);
    sample_vo_init();
    sample_sys_bind_init();
    // pthread_create(&vo_thread_handle, NULL, sample_vo_thread, NULL);

    TEST_TIME(ret = sample_vb_init(), "sample_vb_init");
    if(ret) {
        goto vb_init_error;
    }
    
    TEST_TIME(ret = sample_vivcap_init(),"sample_vicap_init");
    if (vicap_install_osd == 1)
    {
        memset(&pool_config, 0, sizeof(pool_config));
        pool_config.blk_size = VICAP_ALIGN_UP((ISP_CHN0_HEIGHT * ISP_CHN0_WIDTH * 4), VICAP_ALIGN_1K);
        pool_config.blk_cnt = 2;
        pool_config.mode = VB_REMAP_MODE_NOCACHE;
        pool_id = kd_mpi_vb_create_pool(&pool_config); // osd0 - 3 argb 320 x 240
        g_pool_id = pool_id;
    }

    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = ISP_CHN0_WIDTH;
    vf_info.v_frame.height = ISP_CHN0_HEIGHT;
    vf_info.v_frame.stride[0] = ISP_CHN0_WIDTH;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);
    while(app_run)
    {
        if(sensor_process == true)
        {
            uint64_t start_time,end_time;
            
            int num = 0;
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                quit.store(false);
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
            box_t face_box;
            
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);

            memcpy(vaddr, (void *)vbvaddr, ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT * 3); 
            

            kd_mpi_sys_munmap(vbvaddr, size);
            detect_result.boxes.clear();
            detect_result.landmarks.clear();
            _draw_result.clear();
            // run kpu
            // TEST_BOOT_TIME_TRIGER();
            start_time = perf_get_smodecycles();
            retinaface.run(reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr));
            end_time = perf_get_smodecycles();
            // get face boxes
            detect_result = retinaface.get_result();
            cv::Mat osd_frame(ISP_CHN0_HEIGHT, ISP_CHN0_WIDTH, CV_8UC4, cv::Scalar(0, 0, 0, 0));
               
            for (size_t i = 0; i < detect_result.boxes.size(); i++)
            {
                auto box = detect_result.boxes[i];
                FaceMaskInfo fm_result;
                cv::Point2f point;
                std::vector<cv::Point2f> points1;
                // std::cout << "face_location: " << box.x1 << ", " << box.y1 << ", " << box.x2 << ", " << box.y2 << std::endl;       
                vo_frame.line_x_start = ((uint32_t)box.x1) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH;
                vo_frame.line_y_start = ((uint32_t)box.y1) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT;
                vo_frame.line_x_end = ((uint32_t)box.x2) * ISP_CHN0_WIDTH / ISP_CHN1_WIDTH;
                vo_frame.line_y_end = ((uint32_t)box.y2) * ISP_CHN0_HEIGHT / ISP_CHN1_HEIGHT;
                face_box.x = (float)vo_frame.line_x_start;
                face_box.y = (float)vo_frame.line_y_start;
                face_box.w = (float)(vo_frame.line_x_end - vo_frame.line_x_start);
                face_box.h = (float)(vo_frame.line_y_end - vo_frame.line_y_start);
                for(int t = 0;t<5;t++)
                {
                    // printf("%f %f\n",detect_result.landmarks[i].points[2*t + 0],detect_result.landmarks[i].points[2*t + 1]);
                    point.x = detect_result.landmarks[i].points[2*t + 0] * 1920 - 420;
                    point.y = detect_result.landmarks[i].points[2*t + 1] * 1920;
                    // printf("%f %f\n",point.x,point.y);
                    points1.push_back(point);
                }
                
                mf.update_ai2d_config(detect_result.landmarks[i]);
                mf.run(reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr));
                feature_result = mf.get_result();

                if(key_press == true)
                {                
                    pthread_mutex_lock(&mutex);   
                    printf("mem_feature_data->count = %d\n",mem_feature_data->count);               
                    memcpy(mem_feature_data->feature_db_data[mem_feature_data->count].feature, feature_result.data(), feature_result.size() * sizeof(feature_result[0]));
                    sprintf(mem_feature_data->feature_db_data[mem_feature_data->count].name,"%s",name.c_str());
                    mem_feature_data->count++;
                    l2normalize_feature_db(mem_feature_data,mem_feature_data->count);
                    pthread_mutex_unlock(&mutex);
                    ipc_send_thread(MSG_CMD_FEATURE_SAVE);
                    key_press = false;
                }
                pthread_mutex_lock(&mutex); 
                score_index = calulate_score(mem_feature_data->count, feature_result, &score_max);
                pthread_mutex_unlock(&mutex);
                if (score_max >= score_threshold)
                {
                    // std::cout << "face compare result: score = " << score_max << ", index = " << score_index << std::endl;

                    fm_result.label = mem_feature_data->feature_db_data[score_index].name;
                    is_label = true;                   
                }
                draw_result(osd_frame,face_box,fm_result,false,is_label,points1);
            }
                memcpy(pic_vaddr, osd_frame.data, ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 4);
                kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);
                ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                if (ret) {
                    printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                }
        }
        else
        {
            cv::Mat ori_img_R = cv::Mat(ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH, CV_8UC1, vaddr);
            cv::Mat ori_img_G = cv::Mat(ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH, CV_8UC1, vaddr + ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH);
            cv::Mat ori_img_B = cv::Mat(ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH, CV_8UC1, vaddr + ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 2);
            std::vector<std::string> files;
            std::vector<cv::Mat> input_channels;
            
            getFileNames(dir_name,files);
            for(int i = 0;i < files.size();i++)
            {
                char pic_path[100];
                sprintf(pic_path,"/sharefs/pic/%s",files[i].c_str());
                printf("pic_path %s\n",pic_path);
                cv::Mat img = cv::imread(pic_path);
                cv::resize(img, img, cv::Size(720, 1280));
                cv::split(img, input_channels);
                memcpy(ori_img_R.data, input_channels[2].data, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * sizeof(char));
                memcpy(ori_img_G.data, input_channels[1].data, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * sizeof(char));
                memcpy(ori_img_B.data, input_channels[0].data, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * sizeof(char));
                retinaface.run(reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr));
                detect_result = retinaface.get_result();
                if(detect_result.boxes.size() > 0)
                {
                    mf.update_ai2d_config(detect_result.landmarks[0]);
                    mf.run(reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr));
                    feature_result = mf.get_result();
                    pthread_mutex_lock(&mutex);
                    memcpy(mem_feature_data->feature_db_data[mem_feature_data->count].feature, feature_result.data(), feature_result.size() * sizeof(feature_result[0]));
                    sprintf(mem_feature_data->feature_db_data[mem_feature_data->count].name,"%s",files[i].erase(files[i].find(".jpg"),4).c_str());
                    mem_feature_data->count++;
                    l2normalize_feature_db(mem_feature_data,mem_feature_data->count);
                    pthread_mutex_unlock(&mutex);
                    ipc_send_thread(MSG_CMD_FEATURE_SAVE);
                }
                else
                {
                    ipc_send_thread(MSG_CMD_ERROR);
                }
                
            }
            sensor_process = true;
        }
    } 
app_exit:
    close(mem_fd);
    munmap(mem_feature_data, CONFIG_MEM_FACE_DATA_SIZE);
    // pthread_join(vo_thread_handle, NULL);
    pthread_join(exit_thread_handle, NULL);
    // pthread_join(key_opreation_handle, NULL);
    kd_ipcmsg_disconnect(s32Id1);
    
    pthread_join(ipc_message_handle, NULL);

    kd_ipcmsg_del_service(IPC_MSG);      
    vo_osd_release_block();
    kd_mpi_sys_munmap(pic_vaddr, osd_width * osd_height * 4);
    for(size_t i = 0;i < detect_result.boxes.size();i++)
    {
        vo_frame.draw_en = 0;
        vo_frame.frame_num = i + 1;
        kd_mpi_vo_draw_frame(&vo_frame);
    }
    detect_result.boxes.clear();
    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
    }
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        return ret;
    }

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
dma_init_error:
    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("fastboot_app, kd_mpi_vb_exit failed.\n");
        return ret;
    }

vb_init_error:
    return 0;
}