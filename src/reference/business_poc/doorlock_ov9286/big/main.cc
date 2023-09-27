#include <chrono>
#include <fstream>
#include <iostream>
#include <sys/prctl.h>
#include <nncase/runtime/runtime_op_utility.h>
#include "mobile_retinaface.h"
#include "mobile_face.h"
#include "mobilenetv2_depth.h"
#include "mobilenetv2_ir.h"
#include "util.h"
#include "mpi_sys_api.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

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
#include "mpi_dpu_api.h"
#include "k_autoconf_comm.h"
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
k_video_frame_info df_info_dst;
bool app_run = true;


enum VideoFrameSourceType {
    VIDEO_FRAME_VICAP,
    VIDEO_FRAME_DMA,
    VIDEO_FRAME_DPU
};

union VideoFrameSourceParam {
    struct {
        k_vicap_dev vicap_dev;
        k_vicap_chn vicap_chn;
    } vicap;
    k_u8 dma_chn;
} source_parm;

class VideoFrame {
public:
    k_video_frame_info *video_frame_info;
    VideoFrameSourceType source_type;
    union VideoFrameSourceParam source_parm;

    VideoFrame(k_video_frame_info *video_frame_info, k_vicap_dev dev, k_vicap_chn chn):
        video_frame_info(video_frame_info)
    {
        source_type = VIDEO_FRAME_VICAP;
        source_parm.vicap.vicap_dev = dev;
        source_parm.vicap.vicap_chn = chn;
    }

    VideoFrame(k_video_frame_info *video_frame_info, k_u8 dma_chn):
        video_frame_info(video_frame_info)
    {
        source_type = VIDEO_FRAME_DMA;
        source_parm.dma_chn = dma_chn;
    }

    // for DPU
    VideoFrame(): video_frame_info(nullptr), source_type(VIDEO_FRAME_DPU) {}

    ~VideoFrame() {
        if (source_type == VIDEO_FRAME_VICAP) {
            kd_mpi_vicap_dump_release(source_parm.vicap.vicap_dev, source_parm.vicap.vicap_chn, video_frame_info);
        } else if (source_type == VIDEO_FRAME_DMA) {
            kd_mpi_dma_release_frame(source_parm.dma_chn, video_frame_info);
        } else if (source_type == VIDEO_FRAME_DPU) {
            kd_mpi_dpu_release_frame();
        }
    }
};

class BufferMap {
public:
    void* vir;
    k_u64 phy;
    size_t size;

    BufferMap(k_u64 phy, size_t size, bool cached=false): phy(phy), size(size) {
        if (cached) {
            vir = kd_mpi_sys_mmap_cached(phy, size);
            if (vir)
                kd_mpi_sys_mmz_flush_cache(phy, vir, size);
        } else {
            vir = kd_mpi_sys_mmap(phy, size);
        }
    }

    ~BufferMap() {
        if (size)
            kd_mpi_sys_munmap(vir, size);
    }
};
#define CKE(x,h) {int e=x;if(e){fprintf(stderr,"%s:%d "#x" error: %d\n",__FILE__,__LINE__,e);h;}}

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
    l2normalize_feature_db(mem_feature_data,mem_feature_data->count);
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
            ipc_send_thread(MSG_CMD_FEATURE_SAVE);
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

k_s32 sample_connector_init(void)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
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

static k_s32 vo_layer_vdss_bind_vo_config(void)
{
    layer_info info;
    k_vo_layer chn_id = K_VO_LAYER1;

    memset(&info, 0, sizeof(info));

    sample_connector_init();

    info.act_size.width = ISP_CHN1_WIDTH;//1080;//640;//1080;
    info.act_size.height = ISP_CHN1_HEIGHT;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = K_ROTATION_270 | K_VO_GRAY_ENABLE;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = (1080-ISP_CHN1_WIDTH)/2;//(1080-w)/2,
    info.offset.y = (1920-ISP_CHN1_HEIGHT)/2;//(1920-h)/2;
    vo_creat_layer_test(chn_id, &info);
    if (vicap_install_osd == 1)
    {
        osd_info osd;
        osd.act_size.width = ISP_CHN1_WIDTH;
        osd.act_size.height = ISP_CHN1_HEIGHT;
        osd.offset.x = (1080-ISP_CHN1_WIDTH)/2;
        osd.offset.y = (1920-ISP_CHN1_HEIGHT)/2;
        osd.global_alptha = 0xff;
        // osd.global_alptha = 0x32;
        osd.format = PIXEL_FORMAT_ARGB_8888; // PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

        vo_creat_osd_test(osd_id, &osd);
    }
    return 0;
}

static void sample_vo_fn(void *arg)
{
    // set hardware reset;
    usleep(10000);
    vo_layer_vdss_bind_vo_config();
    sample_sys_bind_init();
    return;
}


static int sample_vo_init(void)
{
    usleep(10000);
    vo_layer_vdss_bind_vo_config();
    return 0;
}

static void *sample_vo_thread(void *arg)
{
    TEST_TIME(sample_vo_fn(arg), "sample_vo_fn");
    return NULL;
}



static k_vicap_dev_attr dev_attr;
static k_vicap_chn_attr chn_attr;
static k_vicap_sensor_info sensor_info;
static k_vicap_sensor_type sensor_type;
static k_video_frame_info dump_info;
static k_vb_config config;
static k_dma_dev_attr_t dma_dev_attr;
static k_dma_chn_attr_u dma_chn_attr[DMA_MAX_CHN_NUMS];
static k_dpu_init_t dpu_init;
static k_dpu_dev_attr_t dpu_dev_attr;
static k_dpu_chn_lcn_attr_t lcn_attr;
static k_dpu_chn_ir_attr_t ir_attr;
static k_dpu_user_space_t g_temp_space;

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
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_1;

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
{
    // config.comm_pool[0].blk_cnt = 3;
    // config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    // config.comm_pool[0].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 / 2), VICAP_ALIGN_1K);

    // // VB for dev 1 chn 0 YUV420SP dummy output, if chn0 disable, chn1 rgb888p will failed
    config.comm_pool[1].blk_cnt = 3;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 / 2), VICAP_ALIGN_1K);
    // VB for RGB888 output
    config.comm_pool[2].blk_cnt = 3;
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[2].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);
      

    config.comm_pool[3].blk_cnt = 2;
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[3].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 2), VICAP_ALIGN_1K);

    config.comm_pool[4].blk_cnt = 2;
    config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[4].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 2), VICAP_ALIGN_1K);

    /* dpu vb init */
    config.comm_pool[5].blk_cnt = 1;
    config.comm_pool[5].blk_size = 3 * 1024 * 1024;
    config.comm_pool[5].mode = VB_REMAP_MODE_NOCACHE;

    /* dma vb init */
    config.comm_pool[6].blk_cnt = 2;
    config.comm_pool[6].blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3), VICAP_ALIGN_1K);
    config.comm_pool[6].mode = VB_REMAP_MODE_NOCACHE;

}
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
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_get_sensor_info(OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR_SPECKLE, &sensor_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = sensor_info.width;
    dev_attr.acq_win.height = sensor_info.height;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    dev_attr.dw_enable = K_FALSE;
    dev_attr.dev_enable = K_TRUE;

    dev_attr.buffer_num = 2;
    dev_attr.buffer_size = VICAP_ALIGN_UP((dev_attr.acq_win.width * dev_attr.acq_win.height * 2), VICAP_ALIGN_1K);

    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;

    dev_attr.pipe_ctrl.bits.dnr3_enable = 0;
    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(VICAP_DEV_ID_0, dev_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
        return ret;
    }

    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
    //set chn0 output yuv
    chn_attr.out_win = dev_attr.acq_win;
    chn_attr.crop_win = dev_attr.acq_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
    chn_attr.buffer_num = 3;//at least 3 buffers for isp
    chn_attr.buffer_size = config.comm_pool[1].blk_size;


    ret = kd_mpi_vicap_set_chn_attr(VICAP_DEV_ID_0, VICAP_CHN_ID_0, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
    //set chn1 output rgb888p
    chn_attr.out_win = dev_attr.acq_win;
    chn_attr.crop_win = chn_attr.out_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_BGR_888_PLANAR;
    chn_attr.buffer_num = 3;//at least 3 buffers for isp
    chn_attr.buffer_size = config.comm_pool[2].blk_size;

    ret = kd_mpi_vicap_set_chn_attr(VICAP_DEV_ID_0, VICAP_CHN_ID_1, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }
    // -sensor 2 -mode 1 -chn 0 -rotation 1
    // memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    // ret = kd_mpi_vicap_get_sensor_info(OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE, &sensor_info);
    // if (ret) {
    //     printf("sample_vicap, the sensor type not supported!\n");
    //     return ret;
    // }

    // memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    // dev_attr.acq_win.h_start = 0;
    // dev_attr.acq_win.v_start = 0;
    // dev_attr.acq_win.width = sensor_info.width;
    // dev_attr.acq_win.height = sensor_info.height;
    // dev_attr.mode = VICAP_WORK_OFFLINE_MODE;

    // dev_attr.dw_enable = K_FALSE;
    // dev_attr.dev_enable = K_TRUE;

    // dev_attr.buffer_num = 2;
    // dev_attr.buffer_size = VICAP_ALIGN_UP((dev_attr.acq_win.width * dev_attr.acq_win.height * 2), VICAP_ALIGN_1K);

    // dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    // dev_attr.pipe_ctrl.bits.af_enable = 0;
    // dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    // dev_attr.pipe_ctrl.bits.dnr3_enable = 0;
    // dev_attr.cpature_frame = 0;
    // memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    // ret = kd_mpi_vicap_set_dev_attr(VICAP_DEV_ID_1, dev_attr);
    // if (ret) {
    //     printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
    //     return ret;
    // }
    // memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
    // //set chn0 output yuv420sp
    // chn_attr.out_win = dev_attr.acq_win;
    // chn_attr.crop_win = chn_attr.out_win;
    // chn_attr.scale_win = chn_attr.out_win;
    // chn_attr.crop_enable = K_FALSE;
    // chn_attr.scale_enable = K_FALSE;
    // // chn_attr.dw_enable = K_FALSE;
    // chn_attr.chn_enable = K_TRUE;
    // chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
    // chn_attr.buffer_num = 3;//at least 3 buffers for isp
    // chn_attr.buffer_size = config.comm_pool[0].blk_size;

    // ret = kd_mpi_vicap_set_chn_attr(VICAP_DEV_ID_1, VICAP_CHN_ID_0, chn_attr);
    // if (ret) {
    //     printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
    //     return ret;
    // }
    ret = kd_mpi_vicap_set_database_parse_mode(VICAP_DEV_ID_0, VICAP_DATABASE_PARSE_HEADER);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_database_parse_mode failed.\n");
        return ret;
    }

    // ret = kd_mpi_vicap_set_database_parse_mode(VICAP_DEV_ID_1, VICAP_DATABASE_PARSE_HEADER);
    // if (ret) {
    //     printf("sample_vicap, kd_mpi_vicap_set_database_parse_mode failed.\n");
    //     return ret;
    // }
    ret = kd_mpi_vicap_init(VICAP_DEV_ID_0);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init VICAP_DEV_ID_0 failed.\n");
        return ret;
        // goto err_exit;
    }
    // ret = kd_mpi_vicap_init(VICAP_DEV_ID_1);
    // if (ret) {
    //     printf("sample_vicap, kd_mpi_vicap_init VICAP_DEV_ID_1 failed.\n");
    //     return ret;
    //     // goto err_exit;
    // }
    printf("sample_vicap ...kd_mpi_vicap_start_stream\n");
    ret = kd_mpi_vicap_start_stream(VICAP_DEV_ID_0);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        return ret;
        // goto err_exit;
    }
    // ret = kd_mpi_vicap_start_stream(VICAP_DEV_ID_1);
    // if (ret) {
    //     printf("sample_vicap, kd_mpi_vicap_init failed.\n");
    //     return ret;
    //     // goto err_exit;
    // }
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
    pthread_exit(0);
}

void draw_result(cv::Mat& src_img,box_t& bbox,FaceMaskInfo& result, bool pic_mode,bool is_label,std::vector<cv::Point2f> points1,bool ir_pred,bool depth_pred)
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

static int sample_dma_start() {
    dma_dev_attr.burst_len = 0;
    dma_dev_attr.ckg_bypass = K_TRUE;
    dma_dev_attr.outstanding = 7;

    k_gdma_chn_attr_t *gdma_attr;

    gdma_attr = &dma_chn_attr[0].gdma_attr;
    gdma_attr->buffer_num = 1;
    gdma_attr->rotation = DEGREE_90;
    gdma_attr->x_mirror = K_FALSE;
    gdma_attr->y_mirror = K_FALSE;
    gdma_attr->width = 1280;
    gdma_attr->height = 720;
    gdma_attr->src_stride[0] = 1280;
    gdma_attr->dst_stride[0] = 720;
    gdma_attr->work_mode = DMA_UNBIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YUV_400_8BIT;

    gdma_attr = &dma_chn_attr[1].gdma_attr;
    gdma_attr->buffer_num = 1;
    gdma_attr->rotation = DEGREE_270;
    gdma_attr->x_mirror = K_FALSE;
    gdma_attr->y_mirror = K_FALSE;
    gdma_attr->width = 1280;
    gdma_attr->height = 720;
    gdma_attr->src_stride[0] = 1280;
    gdma_attr->dst_stride[0] = 720;
    gdma_attr->work_mode = DMA_UNBIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YUV_400_8BIT;

    int ret = kd_mpi_dma_set_dev_attr(&dma_dev_attr);
    if (ret != K_SUCCESS) {
        printf("set dev attr error\r\n");
        goto err_return;
    }

    ret = kd_mpi_dma_start_dev();
    if (ret != K_SUCCESS) {
        printf("start dev error\r\n");
        goto err_return;
    }

    ret = kd_mpi_dma_set_chn_attr(0, &dma_chn_attr[0]);
    if (ret != K_SUCCESS) {
        printf("set chn(0) attr error\r\n");
        goto err_dma_dev;
    }
    ret = kd_mpi_dma_start_chn(0);
    if (ret != K_SUCCESS) {
        printf("start chn(0) error\r\n");
        goto err_dma_dev;
    }
    ret = kd_mpi_dma_set_chn_attr(1, &dma_chn_attr[1]);
    if (ret != K_SUCCESS) {
        printf("set chn(1) attr error\r\n");
        goto err_dma_dev;
    }
    ret = kd_mpi_dma_start_chn(1);
    if (ret != K_SUCCESS) {
        printf("start chn(1) error\r\n");
        goto err_dma_dev;
    }
    return K_SUCCESS;

    ret = kd_mpi_dma_stop_chn(0);
    if (ret != K_SUCCESS) {
        printf("stop chn error\r\n");
    }

err_dma_dev:
    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS) {
        printf("stop dev error\r\n");
    }

err_return:
    return K_FAILED;
}
static void sample_dma_stop() {
    int ret;

    ret = kd_mpi_dma_stop_chn(0);
    if (ret != K_SUCCESS) {
        printf("stop chn(0) error\r\n");
    }
    ret = kd_mpi_dma_stop_chn(1);
    if (ret != K_SUCCESS) {
        printf("stop chn(0) error\r\n");
    }
    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS) {
        printf("stop dev error\r\n");
    }
}
static int sample_dpu_start() {
    int ret;
    dpu_init.start_num = 0;
    dpu_init.buffer_num = 1;
    ret = kd_mpi_dpu_init(&dpu_init);
    if (ret) {
        printf("kd_mpi_dpu_init failed\n");
    }

    /* parse file */
    ret = kd_mpi_dpu_parse_file(PARAM_PATH,
                                &dpu_dev_attr.dev_param,
                                &lcn_attr.lcn_param,
                                &ir_attr.ir_param,
                                &g_temp_space);
    if (g_temp_space.virt_addr == NULL) {
        printf("g_temp_space.virt_addr is NULL\n");

    }
    if (ret) {
        printf("kd_mpi_dpu_parse_file failed\n");
    }

    /* set device attribute */
    dpu_dev_attr.mode = DPU_UNBIND;
    dpu_dev_attr.tytz_temp_recfg = K_TRUE;
    dpu_dev_attr.align_depth_recfg = K_TRUE;
    dpu_dev_attr.param_valid = 123;
    dpu_dev_attr.dev_param.spp.flag_align = K_FALSE;

    ret = kd_mpi_dpu_set_dev_attr(&dpu_dev_attr);
    if (ret) {
        printf("kd_mpi_dpu_set_dev_attr failed\n");
    }

    /* set reference image */
    ret = kd_mpi_dpu_set_aligned_ref_image(REF_PATH);
    if (ret) {
        printf("kd_mpi_dpu_set_ref_image failed\n");
    }
    /* set template image */
    ret = kd_mpi_dpu_set_template_image(&g_temp_space);
    if (ret) {
        printf("kd_mpi_dpu_set_template_image failed\n");
    }
    /* start dev */
    ret = kd_mpi_dpu_start_dev();
    if (ret) {
        printf("kd_mpi_dpu_start_dev failed\n");
    }
    /* set chn attr */
    lcn_attr.chn_num = 0;
    lcn_attr.param_valid = 0;
    ir_attr.chn_num = 1;
    ir_attr.param_valid = 0;
    ret = kd_mpi_dpu_set_chn_attr(&lcn_attr, &ir_attr);
    if (ret) {
        printf("kd_mpi_dpu_set_chn_attr failed\n");
    }
    printf("kd_mpi_dpu_set_chn_attr success\n");

    /* start channel 0 */
    ret = kd_mpi_dpu_start_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_start_chn 0 failed\n");
    }

    return K_SUCCESS;
}
static int sample_dpu_stop() {
    k_s32 ret;

    ret = kd_mpi_dpu_stop_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }

    ret = kd_mpi_dpu_stop_dev();
    if (ret) {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }

    kd_mpi_dpu_delete();

    return K_SUCCESS;
}

static void sample_dpu_fn(void *arg)
{
    // set hardware reset;
    sample_dpu_start();
    return;
}

static void *sample_dpu_thread(void *arg)
{
    TEST_TIME(sample_dpu_fn(arg), "sample_dpu_fn");
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " <kmodel>" << std::endl;
        return -1;
    }
    bool is_triger = true;
    bool is_dpu = true;
    TEST_BOOT_TIME_INIT();
    if(is_triger == true)
    {
        TEST_BOOT_TIME_TRIGER();
    }
    k_u32 display_ms = 1000 / 33;
    DetectResult detect_result;
    std::vector<float> feature_result;
    float score_max = 0;
    int score_index = 0;
    float score_threshold = 0.82f;
    bool is_label = false; 
    pthread_t vo_thread_handle;
    pthread_t exit_thread_handle;
    pthread_t dpu_thread_handle;
    pthread_t ipc_message_handle;
    size_t size = CHANNEL * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;
    void *vaddr_u8 = nullptr, *vaddr_u16 = nullptr;
    size_t paddr_u8 = 0, paddr_u16 = 0;
    k_u32 pool_id;
    k_vb_pool_config pool_config;
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL; 

    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr_u8, &vaddr_u8, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }   
    read_mem_feature();
    MobileRetinaface retinaface((const char*)argv[1], CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    Mobilenetv2Depth depth((const char*)argv[2], 1, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    Mobilenetv2Ir ir((const char*)argv[3], CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    MobileFace mf((const char*)argv[4], CHANNEL, ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH);
    if(is_triger == true)
    {
        TEST_BOOT_TIME_TRIGER();
    }
    TEST_TIME(ret = sample_vb_init(), "sample_vb_init");  
    TEST_TIME(ret = sample_vivcap_init(),"sample_vicap_init"); 
    pthread_create(&vo_thread_handle, NULL, sample_vo_thread, NULL);
    
    // pthread_join(vo_thread_handle, NULL);
    TEST_TIME(ret = sample_dpu_start(), "sample_dpu_start");
    // pthread_create(&dpu_thread_handle, NULL, sample_dpu_thread, NULL); 
    pthread_create(&ipc_message_handle, NULL, ipc_msg_server, NULL);
    pthread_create(&exit_thread_handle, NULL, exit_app, NULL);
    // pthread_join(dpu_thread_handle, NULL);
    TEST_TIME(ret = sample_dma_start(), "sample_dma_start");
    
    if (vicap_install_osd == 1)
    {
        memset(&pool_config, 0, sizeof(pool_config));
        pool_config.blk_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 4), VICAP_ALIGN_1K);
        pool_config.blk_cnt = 1;
        pool_config.mode = VB_REMAP_MODE_NOCACHE;
        pool_id = kd_mpi_vb_create_pool(&pool_config); // osd0 - 3 argb 320 x 240
        g_pool_id = pool_id;
    
        memset(&vf_info, 0, sizeof(vf_info));
        vf_info.v_frame.width = ISP_CHN1_WIDTH;
        vf_info.v_frame.height = ISP_CHN1_HEIGHT;
        vf_info.v_frame.stride[0] = ISP_CHN1_WIDTH;
        vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
        block = vo_insert_frame(&vf_info, &pic_vaddr);
    }
    if(is_triger == true)
    {
        TEST_BOOT_TIME_TRIGER();
    }
    size_t depth_size = 2 * ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH;

    while(app_run)
    {
        if(sensor_process == true)
        {
            k_video_frame_info ir_info,ir_info_pre,ir_rotated, speckle_info, speckle_info_pre,speckle_rotated_info;
            k_dpu_chn_result_u lcn_result;
            box_t face_box;

            // dump ir, dev1 chn1, should be dev 0 chn 0 but there are some problems, anyway it works
            memset(&ir_info, 0 , sizeof(k_video_frame_info));
            memset(&ir_rotated, 0 , sizeof(k_video_frame_info));
            memset(&ir_info_pre, 0 , sizeof(k_video_frame_info));
            memset(&speckle_info_pre, 0 , sizeof(k_video_frame_info));
            memset(&speckle_info, 0, sizeof(k_video_frame_info));
            memset(&speckle_rotated_info, 0, sizeof(k_video_frame_info));
            memset(&lcn_result, 0, sizeof(k_dpu_chn_result_u));

            CKE(kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &ir_info_pre, 1000),continue)
            if(ir_info_pre.v_frame.priv_data == VICAP_FILL_LIGHT_CTRL_IR)
            {
                memcpy(&ir_info,&ir_info_pre,sizeof(k_video_frame_info));
            }
            else
            {
                ret = kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_1, &ir_info_pre);
                if (ret) {
                    printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                }
                CKE(kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &ir_info_pre, 1000),continue)
                memcpy(&ir_info,&ir_info_pre,sizeof(k_video_frame_info));    
                if(is_triger == true)
                {
                    TEST_BOOT_TIME_TRIGER();
                }            
            }
            
            CKE(kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_0, VICAP_DUMP_YUV, &speckle_info_pre, 1000),continue) 
            if(is_triger == true)
            {
                TEST_BOOT_TIME_TRIGER();
            }    
            if(speckle_info_pre.v_frame.priv_data == VICAP_FILL_LIGHT_CTRL_SPECKLE)
            {
                memcpy(&speckle_info,&speckle_info_pre,sizeof(k_video_frame_info));
            }            
            else
            {
                ret = kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &speckle_info_pre);
                if (ret) {
                    printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                }
                CKE(kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_0, VICAP_DUMP_YUV, &speckle_info_pre, 1000),continue)       
                memcpy(&speckle_info,&speckle_info_pre,sizeof(k_video_frame_info));
            }
            // CKE(kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &ir_info, 1000),continue)
            // VideoFrame ir_frame(&ir_info, VICAP_DEV_ID_0, VICAP_CHN_ID_1);

            // CKE(kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_0, VICAP_DUMP_YUV, &speckle_info, 1000),continue)
            // VideoFrame speckle_frame(&speckle_info, VICAP_DEV_ID_1, VICAP_CHN_ID_0);


            #if ENABLE_DEBUG
                    dump_to_bin("depth_rot.bin", vaddr_u16, depth_size);
            #endif

            #if ENABLE_PROFILING
                ScopedTiming st("yuv_rotate_90");
            #endif
            CKE(kd_mpi_dma_send_frame(1, &ir_info, 300),continue);
            CKE(kd_mpi_dma_get_frame(1, &ir_rotated, 300),continue);
            VideoFrame ir_rotated_frame(&ir_rotated, 1);
            BufferMap ir_map(ir_rotated.v_frame.phys_addr[0], ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT, true);

            CKE(ir_map.vir == NULL,continue)

            memcpy((char*)vaddr_u8, (char*)ir_map.vir, ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT);
            memcpy((char*)vaddr_u8 + ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT, (char*)ir_map.vir, ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT);
            memcpy((char*)vaddr_u8 + ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT * 2, (char*)ir_map.vir, ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT);

            kd_mpi_sys_mmz_flush_cache(paddr_u8, vaddr_u8, size);
            detect_result.boxes.clear();
            detect_result.landmarks.clear();
            // 1. run retinaface
            retinaface.run(reinterpret_cast<uintptr_t>(vaddr_u8), reinterpret_cast<uintptr_t>(paddr_u8));
            // get face boxes
            detect_result = retinaface.get_result();

            cv::Mat osd_frame(ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH, CV_8UC4, cv::Scalar(0, 0, 0, 0));
            #if ENABLE_DEBUG
            {
                auto size = HEIGHT * WIDTH;
                cv::Mat r_img = cv::Mat(HEIGHT, WIDTH, CV_8UC1, vaddr_u8);
                cv::Mat g_img = cv::Mat(HEIGHT, WIDTH, CV_8UC1, reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(vaddr_u8) + size));
                cv::Mat b_img = cv::Mat(HEIGHT, WIDTH, CV_8UC1, reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(vaddr_u8) + size * 2));

                std::vector<cv::Mat> channels;
                channels.push_back(b_img);
                channels.push_back(g_img);
                channels.push_back(r_img);

                cv::Mat img;
                cv::merge(channels, img);

                for (size_t i = 0; i < detect_result.boxes.size(); i++)
                {
                    auto box = detect_result.boxes[i];
                    auto landmark = detect_result.landmarks[i];
                    cv::rectangle(img, cv::Point(box.x1, box.y1), cv::Point(box.x2, box.y2), cv::Scalar(0, 0, 255), 2);

                    for (int j = 0; j < 5; j++)
                    {
                        cv::circle(img, cv::Point(landmark.points[2 * j + 0], landmark.points[2 * j + 1]), 1, cv::Scalar(255, 0, 0), 2);
                    }
                }

                cv::imwrite("detect_result.jpg", img);
            }
            #endif

            for (size_t i = 0; i < detect_result.boxes.size(); i++)
            {
                auto box = detect_result.boxes[i];
                FaceMaskInfo fm_result;
                cv::Point2f point;
                bool depth_pred = false;
                bool ir_pred = false;
                std::vector<cv::Point2f> points1;  
                face_box.x = (float)box.x1;
                face_box.y = (float)box.y1;
                face_box.w = (float)(box.x2 - box.x1);
                face_box.h = (float)(box.y2 - box.y1);
                for(int t = 0;t<5;t++)
                {
                    point.x = detect_result.landmarks[i].points[2*t + 0];
                    point.y = detect_result.landmarks[i].points[2*t + 1];
                    points1.push_back(point);
                }

                mf.update_ai2d_config(detect_result.landmarks[i]);
                mf.run(reinterpret_cast<uintptr_t>(vaddr_u8), reinterpret_cast<uintptr_t>(paddr_u8));
                feature_result = mf.get_result();

                if(key_press == true)
                {              
                    pthread_mutex_lock(&mutex);            
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
                if(is_triger == true)
                {
                    TEST_BOOT_TIME_TRIGER();
                }
                pthread_mutex_unlock(&mutex);
                if (score_max >= score_threshold)
                {
                    {
                        //ir kmodel
                        ir.update_ai2d_config(face_box.x, face_box.y, face_box.w, face_box.h);
                        ir.run(reinterpret_cast<uintptr_t>(vaddr_u8), reinterpret_cast<uintptr_t>(paddr_u8));
                        ir_pred = ir.get_result();
                    }
                    if(ir_pred == 1)
                    {
                        #if ENABLE_PROFILING
                            ScopedTiming st("GMDA+DPU");
                        #endif
                            // if(is_dpu == true)
                            // {
                            //     // TEST_TIME(ret = sample_dpu_start(), "sample_dpu_start");
                            //     is_dpu = false;
                            // }
                            // GDMA rotate speckle
                            CKE(kd_mpi_dma_send_frame(0, &speckle_info, 1000),continue)
                            CKE(kd_mpi_dma_get_frame(0, &speckle_rotated_info, 1000),continue)
                            VideoFrame speckle_rotated_frame(&speckle_rotated_info, 0);
                        #if ENABLE_DEBUG
                            {
                                auto size = HEIGHT * WIDTH;
                                void *v_u8 = kd_mpi_sys_mmap(speckle_rotated_info.v_frame.phys_addr[0], size);
                                dump_to_bin("speckle_rotated_info.bin", v_u8, size);
                                kd_mpi_sys_munmap(v_u8, size);
                            }
                        #endif
                            // to DPU

                            CKE(kd_mpi_dpu_send_frame(0, speckle_rotated_info.v_frame.phys_addr[0], 1000),continue)
                            CKE(kd_mpi_dpu_get_frame(0, &lcn_result, 1000),continue)
                            VideoFrame dpu_frame;
                            BufferMap dpu_map(lcn_result.lcn_result.depth_out.depth_phys_addr, depth_size);
                            CKE(dpu_map.vir == NULL, continue);
                            paddr_u16 = dpu_map.phy;
                            vaddr_u16 = dpu_map.vir;  

                            depth.update_ai2d_config(face_box.x, face_box.y, face_box.w, face_box.h);
                            depth.run(reinterpret_cast<uintptr_t>(vaddr_u16), reinterpret_cast<uintptr_t>(paddr_u16));
                            depth_pred = depth.get_result();        
                    }
                    if(ir_pred && depth_pred)
                    {
                        fm_result.label = mem_feature_data->feature_db_data[score_index].name;
                        is_label = true;  
                        if(is_triger == true)
                        {
                            TEST_BOOT_TIME_TRIGER();
                            is_triger = false;
                        }                          
                    }  
                }
                               
                draw_result(osd_frame,face_box,fm_result,false,is_label,points1,ir_pred,depth_pred);
                
            }

            memcpy(pic_vaddr, osd_frame.data, ISP_CHN1_WIDTH * ISP_CHN1_HEIGHT * 4);
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);
            ret = kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &speckle_info_pre);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
            ret = kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_1, &ir_info_pre);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }

        }
        else
        {
            cv::Mat ori_img_R = cv::Mat(ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH, CV_8UC1, vaddr_u8);
            cv::Mat ori_img_G = cv::Mat(ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH, CV_8UC1, vaddr_u8 + ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH);
            cv::Mat ori_img_B = cv::Mat(ISP_CHN1_HEIGHT, ISP_CHN1_WIDTH, CV_8UC1, vaddr_u8 + ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 2);
            std::vector<std::string> files;
            std::vector<cv::Mat> input_channels;
            getFileNames(dir_name,files);
            for(int i = 0;i < files.size();i++)
            {
                char pic_path[100];
                sprintf(pic_path,"/sharefs/pic/%s",files[i].c_str());
                cv::Mat img = cv::imread(pic_path);
                cv::resize(img, img, cv::Size(720, 1280));
                cv::split(img, input_channels);
                memcpy(ori_img_R.data, input_channels[2].data, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * sizeof(char));
                memcpy(ori_img_G.data, input_channels[1].data, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * sizeof(char));
                memcpy(ori_img_B.data, input_channels[0].data, ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * sizeof(char));
                retinaface.run(reinterpret_cast<uintptr_t>(vaddr_u8), reinterpret_cast<uintptr_t>(paddr_u8));
                detect_result = retinaface.get_result();
                if(detect_result.boxes.size() > 0)
                {
                    mf.update_ai2d_config(detect_result.landmarks[0]);
                    mf.run(reinterpret_cast<uintptr_t>(vaddr_u8), reinterpret_cast<uintptr_t>(paddr_u8));
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
    pthread_join(exit_thread_handle, NULL);
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
    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
    }
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_0);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        return ret;
    }
    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_1);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
    }
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_1);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        return ret;
    }
    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);


    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = VICAP_DEV_ID_0;
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_0;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    sample_vicap_unbind_vo(vicap_mpp_chn, vo_mpp_chn);
    sample_dma_stop();
    sample_dpu_stop();
    usleep(1000 * display_ms);
dma_init_error:
    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("fastboot_app, kd_mpi_vb_exit failed.\n");
        return ret;
    }
    ret = kd_mpi_sys_mmz_free(paddr_u8, vaddr_u8);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
vb_init_error:
    return 0;
}
