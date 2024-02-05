#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <sys/mman.h>
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
#include "k_nonai_2d_comm.h"

#define ISP_CHN0_WIDTH  (1280)
#define ISP_CHN0_HEIGHT (720)
#define VICAP_OUTPUT_BUF_NUM        10
#define NONAI_2D_BUF_NUM            6

#define OSD_ID                      K_VO_OSD3
#define OSD_WIDTH                   (1280)
#define OSD_HEIGHT                  (720)

#define AUDIO_PERSEC_DIV_NUM 25
#define VENC_STREAM_VB_CNT 10
#define VI_ALIGN_UP(addr, size) (((addr)+((size)-1U))&(~((size)-1U)))

#define TOTAL_ENABLE_2D_CH_NUMS     6
#define NONAI_2D_RGB_CH             4
#define NONAI_2D_BIND_CH_0          0
#define NONAI_2D_BIND_CH_1          1
#define NONAI_2D_BIND_CH_2          2

typedef struct
{
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


typedef struct
{
    k_u64 osd_phy_addr;
    void *osd_virt_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
} osd_info;


static k_s32 g_mmap_fd_tmp = 0;
static std::mutex mmap_mutex_;
static void *_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);

    std::unique_lock<std::mutex> lck(mmap_mutex_);
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
    std::unique_lock<std::mutex> lck(mmap_mutex_);
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

static k_s32 nonai_2d_init()
{
    int i;
    k_nonai_2d_chn_attr attr_2d;

    for(i = 0; i < TOTAL_ENABLE_2D_CH_NUMS; i++)
    {
        attr_2d.mode = K_NONAI_2D_CALC_MODE_CSC;
        if(i == NONAI_2D_RGB_CH)
        {
            attr_2d.dst_fmt = PIXEL_FORMAT_RGB_888_PLANAR;
        }
        else
        {
            attr_2d.dst_fmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        }
        kd_mapi_nonai_2d_init(i, &attr_2d);

        kd_mapi_nonai_2d_start(i);
    }

    return K_SUCCESS;
}

static k_s32 nonai_2d_deinit()
{
    int i;

    for(i = 0; i < TOTAL_ENABLE_2D_CH_NUMS; i++)
    {
        kd_mapi_nonai_2d_stop(i);
        kd_mapi_nonai_2d_deinit(i);
    }

    return K_SUCCESS;
}

static void sample_bind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vo_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_0;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 0;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = 1;

    ret = kd_mapi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    ret = kd_mapi_sys_bind(&nonai_2d_mpp_chn, &venc_mpp_chn);
    ret = kd_mapi_sys_bind(&nonai_2d_mpp_chn, &vo_mpp_chn);

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 1;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_1;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 1;

    ret = kd_mapi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    ret = kd_mapi_sys_bind(&nonai_2d_mpp_chn, &venc_mpp_chn);

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 2;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_2;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 2;

    ret = kd_mapi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    ret = kd_mapi_sys_bind(&nonai_2d_mpp_chn, &venc_mpp_chn);

    printf("%s>ret %d\n", __func__, ret);

    return;
}

static void sample_unbind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vo_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_0;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 0;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = 1;

    ret = kd_mapi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    ret = kd_mapi_sys_unbind(&nonai_2d_mpp_chn, &venc_mpp_chn);
    ret = kd_mapi_sys_unbind(&nonai_2d_mpp_chn, &vo_mpp_chn);

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 1;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_1;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 1;

    ret = kd_mapi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    ret = kd_mapi_sys_unbind(&nonai_2d_mpp_chn, &venc_mpp_chn);

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 2;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_2;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 2;

    ret = kd_mapi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    ret = kd_mapi_sys_unbind(&nonai_2d_mpp_chn, &venc_mpp_chn);

    printf("%s>ret %d\n", __func__, ret);

    return;
}

int KdMedia::Init(const KdMediaInputConfig &config) {
    int i =0;
    config_ = config;
    int session_num = config_.session_num;
    int ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_sys_init error." << std::endl;
        return ret;
    }

    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = 64;

    if (config.video_valid) {
        k_u64 pic_size =ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3;
        k_u64 stream_size = config.venc_width * config.venc_height / 2;

        printf("KdMedia::Init session_num  is %d \n", session_num);

        //VB for YUV444 output for dev0
        media_attr.media_config.vb_config.comm_pool[0].blk_cnt = VICAP_OUTPUT_BUF_NUM;
        media_attr.media_config.vb_config.comm_pool[0].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), 0x1000);
        media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;

        media_attr.media_config.vb_config.comm_pool[1].blk_cnt = VICAP_OUTPUT_BUF_NUM;
        media_attr.media_config.vb_config.comm_pool[1].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), 0x1000);
        media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

        media_attr.media_config.vb_config.comm_pool[2].blk_cnt = VICAP_OUTPUT_BUF_NUM;
        media_attr.media_config.vb_config.comm_pool[2].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), 0x1000);
        media_attr.media_config.vb_config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;

        media_attr.media_config.vb_config.comm_pool[3].blk_cnt = NONAI_2D_BUF_NUM;
        media_attr.media_config.vb_config.comm_pool[3].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), 0x1000);
        media_attr.media_config.vb_config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;

        media_attr.media_config.vb_config.comm_pool[4].blk_cnt = 2;
        media_attr.media_config.vb_config.comm_pool[4].blk_size = VICAP_ALIGN_UP((1920 * 1080 * 4), 0x1000);
        media_attr.media_config.vb_config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;

        for (i =0;i < session_num;i ++)
        {
            media_attr.media_config.vb_config.comm_pool[5+i*2].blk_cnt = VENC_STREAM_VB_CNT;
            media_attr.media_config.vb_config.comm_pool[5+i*2].blk_size = ((stream_size + 0xfff) & ~0xfff);
            media_attr.media_config.vb_config.comm_pool[5+i*2].mode = VB_REMAP_MODE_NOCACHE;
        }
    }

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_media_init error." << std::endl;
        kd_mapi_sys_deinit();
        return ret;
    }

    nonai_2d_init();

    sample_bind();

    return ret;
}

int KdMedia::Deinit() {
    nonai_2d_deinit();
    sample_unbind();

    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
    return 0;
}

k_s32 KdMedia::VideoEncCallback(k_u32 chn_num, kd_venc_data_s* stream_data, k_u8* p_private_data) {
    KdMedia *thiz = (KdMedia*)p_private_data;

    if (thiz && thiz->on_venc_data_) {
        int cut = stream_data->status.cur_packs;
        for (int i = 0; i < cut; i++) {
            thiz->on_venc_data_->OnVEncData(chn_num, stream_data->astPack[i].vir_addr, stream_data->astPack[i].len, stream_data->astPack[i].pts);
        }
    }
    return 0;
}


int KdMedia::InitVcap(uint32_t chn_id, k_vicap_sensor_type type)
{
    int ret;
    k_vicap_chn_set_info vi_chn_attr_info;
    k_vicap_dev_set_info dev_attr;

    memset(&vi_chn_attr_info, 0, sizeof(vi_chn_attr_info));
    memset(&dev_attr, 0 ,sizeof(k_vicap_dev_set_info));

    dev_attr.vicap_dev = (k_vicap_dev)chn_id;
    dev_attr.mode = VICAP_WORK_ONLY_MCM_MODE;
    dev_attr.sensor_type = type;
    dev_attr.buffer_num = VICAP_OUTPUT_BUF_NUM;
    dev_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.dw_en = K_FALSE;

    printf("--------------------- vicap init chn is %d ===============type is %d   \n", chn_id, type);

    ret = kd_mapi_vicap_set_dev_attr(dev_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
        return ret;
    }

    vi_chn_attr_info.crop_en = K_FALSE;
    vi_chn_attr_info.scale_en = K_FALSE;
    vi_chn_attr_info.chn_en = K_TRUE;
    vi_chn_attr_info.crop_h_start = 0;
    vi_chn_attr_info.crop_v_start = 0;
    vi_chn_attr_info.out_width = ISP_CHN0_WIDTH;
    vi_chn_attr_info.out_height = ISP_CHN0_HEIGHT;
    vi_chn_attr_info.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_444;
    vi_chn_attr_info.vicap_dev = (k_vicap_dev)chn_id;
    vi_chn_attr_info.vicap_chn = VICAP_CHN_ID_0;
    vi_chn_attr_info.buffer_num = VICAP_OUTPUT_BUF_NUM;
    vi_chn_attr_info.alignment = 0;
    vi_chn_attr_info.buf_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);

    ret = kd_mapi_vicap_set_chn_attr(vi_chn_attr_info);
    if (ret != K_SUCCESS) {
        printf("vicap chn %d set attr failed, %x.\n", chn_id, ret);
        return -1;
    }

    kd_mapi_vicap_init((k_vicap_dev)chn_id);
    return 0;
}

int KdMedia::InitVenc(uint32_t chn_id,IOnVEncData *on_venc_data)
{
    if (!config_.video_valid) {
        return 0;
    }
    on_venc_data_ = on_venc_data;

    k_venc_chn_attr chn_attr;
    memset(&chn_attr, 0, sizeof(chn_attr));
    k_u64 stream_size = config_.venc_width * config_.venc_height / 2;
    chn_attr.venc_attr.pic_width = config_.venc_width;
    chn_attr.venc_attr.pic_height = config_.venc_height;
    chn_attr.venc_attr.stream_buf_size = ((stream_size + 0xfff) & ~0xfff);
    chn_attr.venc_attr.stream_buf_cnt = VENC_STREAM_VB_CNT;
    chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
    chn_attr.rc_attr.cbr.src_frame_rate = 25;
    chn_attr.rc_attr.cbr.dst_frame_rate = 25;
    chn_attr.rc_attr.cbr.bit_rate = config_.bitrate_kbps;
    if (config_.video_type ==  KdMediaVideoType::kVideoTypeH264) {
        chn_attr.venc_attr.type = K_PT_H264;
        chn_attr.venc_attr.profile = VENC_PROFILE_H264_HIGH;
    } else if (config_.video_type ==  KdMediaVideoType::kVideoTypeH265) {
        chn_attr.venc_attr.type = K_PT_H265;
        chn_attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
    } else if (config_.video_type ==  KdMediaVideoType::kVideoTypeMjpeg) {
        chn_attr.venc_attr.type = K_PT_JPEG;
        chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_MJPEG_FIXQP;
        chn_attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
        chn_attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
        chn_attr.rc_attr.mjpeg_fixqp.q_factor = 45;
    }

    uint32_t venc_chn_id = chn_id;
    int ret = kd_mapi_venc_init(venc_chn_id, &chn_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_venc_init error." << std::endl;
        return -1;
    }

    if (config_.video_type != KdMediaVideoType::kVideoTypeMjpeg) {
        ret = kd_mapi_venc_enable_idr(venc_chn_id, K_TRUE);
        if (ret != K_SUCCESS) {
            std::cout << "kd_mapi_venc_enable_idr error." << std::endl;
            return -1;
        }
    }

    kd_venc_callback_s venc_callback;
    memset(&venc_callback, 0, sizeof(venc_callback));
    venc_callback.p_private_data = (k_u8*)this;
    venc_callback.pfn_data_cb = KdMedia::VideoEncCallback;
    kd_mapi_venc_registercallback(venc_chn_id, &venc_callback);

    return 0;
}

int KdMedia::DeinitVenc(uint32_t chn_id)
{
    if (!config_.video_valid) {
        return 0;
    }
    kd_mapi_venc_deinit(chn_id);
    return 0;
}

int KdMedia::StartVcap(k_vicap_dev vicap_dev)
{
    kd_mapi_vicap_start_stream(vicap_dev);
    return 0;
}

int KdMedia::StopVcap(k_vicap_dev vicap_dev)
{
    kd_mapi_vicap_stop(vicap_dev);
    return 0;
}

int KdMedia::StartVenc(uint32_t chn_id)
{
    kd_mapi_venc_start(chn_id, -1);
    return 0;
}

int KdMedia::StopVenc(uint32_t chn_id)
{
    kd_mapi_venc_stop(chn_id);
    return 0;
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
    kd_mapi_connector_power_set(connector_fd, K_TRUE);
    // // connector init
    kd_mapi_connector_init(connector_fd, &connector_info);

    return 0;
}


static int sample_vo_creat_layer(k_vo_layer chn_id, layer_info *info)
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
    kd_mapi_vo_set_video_layer_attr(chn_id, &attr);
    // enable layer
    kd_mapi_vo_enable_video_layer(chn_id);

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
    kd_mapi_vo_set_video_osd_attr(osd, &attr);
    kd_mapi_vo_osd_enable(osd);

    return 0;
}


int KdMedia::InitVO(k_connector_type type)
{
    layer_info info;
    osd_info osd;
    k_vo_layer chn_id = K_VO_LAYER1;

    memset(&info, 0, sizeof(layer_info));
    memset(&osd, 0, sizeof(osd_info));

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

        osd.act_size.width = OSD_WIDTH ;
        osd.act_size.height = OSD_HEIGHT;
        osd.offset.x = 0;
        osd.offset.y = 0;
        osd.global_alptha = 0xff;// 0x7f;
        osd.format = PIXEL_FORMAT_ARGB_8888;
        sample_vo_creat_osd_test(OSD_ID, &osd);

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

        osd.act_size.width = OSD_WIDTH ;
        osd.act_size.height = OSD_HEIGHT;
        osd.offset.x = 0;
        osd.offset.y = 0;
        osd.global_alptha = 0xff;// 0x7f;
        osd.format = PIXEL_FORMAT_ARGB_8888;
        sample_vo_creat_osd_test(OSD_ID, &osd);
    }
}

int KdMedia::DeinitVO()
{
    kd_mapi_vo_disable_video_layer(K_VO_LAYER1);
}
