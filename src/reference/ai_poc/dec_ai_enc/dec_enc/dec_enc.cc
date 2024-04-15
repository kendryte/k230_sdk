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
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "utils.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vdec_api.h"
#include "mpi_vo_api.h"
#include "mpi_sys_api.h"
#include "k_vdec_comm.h"
#include "k_vvo_comm.h"
#include "mpi_venc_api.h"
#include "k_venc_comm.h"
#include "mpi_vvi_api.h"
#include "vo_test_case.h"
#include "face_detection.h"

#define ENABLE_VDEC_DEBUG    1
#define BIND_VO_LAYER   1

#ifdef ENABLE_VDEC_DEBUG
    #define vdec_debug  printf
#else
    #define vdec_debug(ARGS...)
#endif

#define MAX_WIDTH 1088
#define MAX_HEIGHT 1920
#define STREAM_BUF_SIZE MAX_WIDTH*MAX_HEIGHT
#define FRAME_BUF_SIZE MAX_WIDTH*MAX_HEIGHT*2
#define INPUT_BUF_CNT   4
#define OUTPUT_BUF_CNT  6

#define VENC_MAX_IN_FRAMES   30
#define ENABLE_VENC_DEBUG    1

#ifdef ENABLE_VDSS
    #include "k_vdss_comm.h"
    #include "mpi_vdss_api.h"
#else
    #include "mpi_vicap_api.h"
#endif

#ifdef ENABLE_VENC_DEBUG
    #define venc_debug  printf
#else
    #define venc_debug(ARGS...)
#endif

#define VE_MAX_WIDTH 1920
#define VE_MAX_HEIGHT 1080
#define VE_STREAM_BUF_SIZE ((VE_MAX_WIDTH*VE_MAX_HEIGHT/2 + 0xfff) & ~0xfff)
#define VE_FRAME_BUF_SIZE ((VE_MAX_WIDTH*VE_MAX_HEIGHT*2 + 0xfff) & ~0xfff)
#define OSD_MAX_WIDTH 1920
#define OSD_MAX_HEIGHT 1088
#define OSD_BUF_SIZE OSD_MAX_WIDTH*OSD_MAX_HEIGHT*4
#define VE_INPUT_BUF_CNT   6
#define VE_OUTPUT_BUF_CNT  15
#define OSD_BUF_CNT     20

typedef struct
{
    k_pixel_format chn_format;
    k_u32 file_size;
    k_s32 pool_id;
    pthread_t input_tid;
    pthread_t output_tid;
    FILE *input_file;
    k_u32 ch_id;
    char *dump_frame;
    k_u32 dump_frame_size;
    k_bool done;
    k_payload_type type;
    k_vb_blk_handle vb_handle[INPUT_BUF_CNT];
    k_pixel_format pic_format;
    k_u32 act_width;
    k_u32 act_height;
    k_u32 input_pool_id;
    k_u32 output_pool_id;
} sample_vdec_conf_t;

static sample_vdec_conf_t g_vdec_conf[VDEC_MAX_CHN_NUMS];

//*******************************encoder*************

// extern const unsigned int osd_data;
// extern const int osd_data_size;

typedef enum
{
    VENC_SAMPLE_STATUS_IDLE = 0,
    VENC_SAMPLE_STATUS_INIT,
    VENC_SAMPLE_STATUS_START,
    VENC_SAMPLE_STATUS_BINDED,
    VENC_SAMPLE_STATUS_UNBINDED,
    VENC_SAMPLE_STATUE_RUNING,
    VENC_SAMPLE_STATUS_STOPED,
    VENC_SAMPLE_STATUS_BUTT
} VENC_SAMPLE_STATUS;

typedef struct
{
    k_u32 osd_width;
    k_u32 osd_height;
    k_u32 osd_phys_addr[VENC_MAX_IN_FRAMES][3];
    void *osd_virt_addr[VENC_MAX_IN_FRAMES][3];
    k_u32 osd_startx;
    k_u32 osd_starty;
    k_venc_2d_src_dst_fmt video_fmt;
    k_venc_2d_osd_fmt osd_fmt;
    k_u16 bg_alpha;
    k_u16 osd_alpha;
    k_u16 video_alpha;
    k_venc_2d_add_order add_order;
    k_u32 bg_color;
    k_u16 osd_coef[K_VENC_2D_COEF_NUM];
    k_u8 osd_region_num;
    k_bool osd_matrix_en;
} osd_conf_t;

typedef struct
{
    k_u16 width;
    k_u16 height;
    k_u16 line_width;
    k_u16 startx;
    k_u16 starty;
} border_conf_t;

typedef struct
{
    k_u32 ch_id;
    k_u32 output_frames;
} output_info;

typedef struct
{
    k_u32 chnum;
    pthread_t output_tid;
    k_bool osd_enable;
    osd_conf_t *osd_conf;
    k_vb_blk_handle osd_blk_handle;
    k_bool ch_done;
} venc_conf_t;

char filename[50];
FILE *output_file = NULL;
std::atomic<bool> is_stop(false);

VENC_SAMPLE_STATUS g_venc_sample_status = VENC_SAMPLE_STATUS_IDLE;
venc_conf_t g_venc_conf;

//****************function***********************************

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

/**
*单独创建pool给解码器的输入输出使用
*/
static k_s32 vb_create_pool(int ch)
{
    k_vb_pool_config pool_config;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = INPUT_BUF_CNT;
    pool_config.blk_size =STREAM_BUF_SIZE,VICAP_ALIGN_1K;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].input_pool_id = kd_mpi_vb_create_pool(&pool_config);
    vdec_debug("input_pool_id %d\n", g_vdec_conf[ch].input_pool_id);

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = OUTPUT_BUF_CNT;
    pool_config.blk_size = FRAME_BUF_SIZE,VICAP_ALIGN_1K;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].output_pool_id = kd_mpi_vb_create_pool(&pool_config);
    vdec_debug("output_pool_id %d\n", g_vdec_conf[ch].output_pool_id);

    return 0;
}

/**
*销毁创建的pool
*/
static k_s32 vb_destory_pool(int ch)
{
    vdec_debug("destory_pool input %d \n", g_vdec_conf[ch].input_pool_id);
    kd_mpi_vb_destory_pool(g_vdec_conf[ch].input_pool_id);
    vdec_debug("destory_pool output %d \n", g_vdec_conf[ch].output_pool_id);
    kd_mpi_vb_destory_pool(g_vdec_conf[ch].output_pool_id);
    return 0;
}


/**
*vb初始化，pool个数为5，先初始化3个pool给编码器，后面再创建两个pool给解码器
*/
static k_s32 sample_vb_init(k_u32 ch_cnt, k_bool osd_enable)
{
    k_s32 ret;
    k_vb_config config;

    memset(&config, 0, sizeof(config));
    
    config.max_pool_cnt = 5;
    config.comm_pool[0].blk_cnt = VE_INPUT_BUF_CNT * ch_cnt;
    config.comm_pool[0].blk_size = VE_FRAME_BUF_SIZE;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_cnt = VE_OUTPUT_BUF_CNT * ch_cnt;
    config.comm_pool[1].blk_size =VE_STREAM_BUF_SIZE;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[2].blk_cnt = 4;
    config.comm_pool[2].blk_size =OSD_BUF_SIZE;
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;

    ret = kd_mpi_vb_set_config(&config);

    venc_debug("-----------venc sample test------------------------\n");

    if (ret)
        venc_debug("vb_set_config failed ret:%d\n", ret);

    ret = kd_mpi_vb_init();
    if (ret)
        venc_debug("vb_init failed ret:%d\n", ret);

    return ret;
}

/**
*vb退出
*/
static k_s32 sample_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        vdec_debug("vb_exit failed ret:%d\n", ret);
    return ret;
}


/**
*编码器输出线程逻辑
*/
static void *venc_output_thread(void *arg)
{
    k_venc_stream output;
    int out_cnt, out_frames;
    k_s32 ret;
    int i;
    k_u32 total_len = 0;
    output_info *info = (output_info *)arg;
    out_cnt = 0;
    out_frames = 0;
    while (1)
    {
        k_venc_chn_status status;
        ret = kd_mpi_venc_query_status(info->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;
        output.pack = static_cast<k_venc_pack*>(malloc(sizeof(k_venc_pack) * output.pack_cnt));
        //获取编码码流
        ret = kd_mpi_venc_get_stream(info->ch_id, &output, -1);
        CHECK_RET(ret, __func__, __LINE__);
        //将码流写入h265文件
        out_cnt += output.pack_cnt;
        for (i = 0; i < output.pack_cnt; i++)
        {
            if (output.pack[i].type != K_VENC_HEADER)
            {
                out_frames++;
            }

            k_u8 *pData;
            pData = (k_u8 *)kd_mpi_sys_mmap(output.pack[i].phys_addr, output.pack[i].len);
            if (output_file)
                fwrite(pData, 1, output.pack[i].len, output_file);
            kd_mpi_sys_munmap(pData, output.pack[i].len);

            total_len += output.pack[i].len;
        }

        ret = kd_mpi_venc_release_stream(info->ch_id, &output);
        CHECK_RET(ret, __func__, __LINE__);

        free(output.pack);

    }
    //关闭文件
    if (output_file)
        fclose(output_file);
    venc_debug("%s>done, ch %d: out_frames %d, size %d bits\n", __func__, info->ch_id, out_frames, total_len * 8);
    return arg;
}

/**
*绑定输入通道和视频编码通道
*/
static void sample_vi_bind_venc(k_u32 chn_id)
{
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vi_mpp_chn;
    k_s32 ret;

#ifdef ENABLE_VDSS
    vi_mpp_chn.mod_id = K_ID_VICAP;
#else
    vi_mpp_chn.mod_id = K_ID_VI;
#endif

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_id;

    vi_mpp_chn.dev_id = chn_id;
    vi_mpp_chn.chn_id = chn_id;
    ret = kd_mpi_sys_bind(&vi_mpp_chn, &venc_mpp_chn);
    if (ret)
    {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }

    return;
}

/**
*解绑输入通道和视频编码通道
*/
static void sample_vi_unbind_venc(k_u32 chn_id)
{
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vi_mpp_chn;

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_id;

#ifdef ENABLE_VDSS
    vi_mpp_chn.mod_id = K_ID_VICAP;
#else
    vi_mpp_chn.mod_id = K_ID_VI;
#endif
    vi_mpp_chn.dev_id = chn_id;
    vi_mpp_chn.chn_id = chn_id;
    kd_mpi_sys_unbind(&vi_mpp_chn, &venc_mpp_chn);

    return;
}

/**
*解码器输入线程逻辑
*/
static void *input_thread(void *arg)
{
    sample_vdec_conf_t *vdec_conf;
    k_vdec_stream stream;
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u8 *virt_addr;
    k_u32 blk_size, stream_len;
    k_u32 file_size;
    int i = 0;
    k_s32 ret;

    vdec_conf = (sample_vdec_conf_t *)arg;

    file_size = 0;
    blk_size = STREAM_BUF_SIZE;

    int poolid = vdec_conf->input_pool_id;
    vdec_debug("%s>poolid:%d \n", __func__, poolid);
    //循环读取文件
    while (file_size < vdec_conf->file_size)
    {
        memset(&stream, 0, sizeof(k_vdec_stream));
        handle = kd_mpi_vb_get_block(poolid, blk_size, NULL);

        if (handle == VB_INVALID_HANDLE)
        {
            //vdec_debug("%s>no vb\n", __func__);
            usleep(30000);
            continue;
        }

        pool_id = kd_mpi_vb_handle_to_pool_id(handle);
        if (pool_id == VB_INVALID_POOLID)
        {
            vdec_debug("%s get pool id error\n", __func__);
            break;
        }
        if(i >= INPUT_BUF_CNT)
            i = 0;
        vdec_conf->pool_id = pool_id;
        vdec_conf->vb_handle[i] = handle;

        phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
        if (phys_addr == 0)
        {
            vdec_debug("%s get phys addr error\n", __func__);
            break;
        }

        vdec_debug("%s>ch %d, vb_handle 0x%x, pool_id %d, blk_size %d\n", __func__,
                   vdec_conf->ch_id, vdec_conf->vb_handle[i], pool_id, blk_size);

        virt_addr = (k_u8 *)kd_mpi_sys_mmap_cached(phys_addr, blk_size);

        if (virt_addr == NULL)
        {
            vdec_debug("%s mmap error\n", __func__);
            break;
        }

        if (file_size + blk_size > vdec_conf->file_size)
        {
            fread(virt_addr, 1, (vdec_conf->file_size - file_size), vdec_conf->input_file);
            stream_len = vdec_conf->file_size - file_size;
            stream.end_of_stream = K_TRUE;
        }
        else
        {
            fread(virt_addr, 1, blk_size, vdec_conf->input_file);
            stream_len = blk_size;
        }

        ret = kd_mpi_sys_mmz_flush_cache(phys_addr, virt_addr, stream_len);
        CHECK_RET(ret, __func__, __LINE__);

        file_size += stream_len;

        stream.phy_addr = phys_addr;
        stream.len = stream_len;

        vdec_debug("ch %d send stream: phys_addr 0x%lx, len %d\n", vdec_conf->ch_id, stream.phy_addr, stream.len);
        ret = kd_mpi_vdec_send_stream(vdec_conf->ch_id, &stream, -1);
        CHECK_RET(ret, __func__, __LINE__);

        ret = kd_mpi_sys_munmap((void *)virt_addr, blk_size);
        CHECK_RET(ret, __func__, __LINE__);

        ret = kd_mpi_vb_release_block(handle);
        CHECK_RET(ret, __func__, __LINE__);

        i++;

        if (vdec_conf->done)
            break;
    }

    return arg;
}


/**
*初始化AI计算后发给编码器的帧
*/
k_vb_blk_handle init_venc_frame(k_video_frame_info *vf_info, void **pic_vaddr,k_u32 g_pool_id)
{
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_vb_blk_handle handle;
    k_s32 size;

    if (vf_info == NULL)
        return K_FALSE;

    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_8888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_8888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_565 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_565)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_4444 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_4444)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_1555 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_1555)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3 / 2;      

    printf("vb block size is %x \n", size);

    handle = kd_mpi_vb_get_block(g_pool_id, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }

    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);

    if (virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    vf_info->mod_id = K_ID_VO;
    vf_info->pool_id = g_pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
    *pic_vaddr = virt_addr;

    printf("phys_addr is %lx g_pool_id is %d \n", phys_addr, g_pool_id);

    return handle;
}

/**
* 停止并销毁编码器
*/
k_s32 sample_exit(venc_conf_t *venc_conf)
{
    int ch = 0;
    int ret = 0;

    printf("%s>g_venc_sample_status = %d\n", __FUNCTION__, g_venc_sample_status);
    switch (g_venc_sample_status)
    {
    case VENC_SAMPLE_STATUE_RUNING:
    case VENC_SAMPLE_STATUS_BINDED:
        sample_vi_unbind_venc(ch);
    case VENC_SAMPLE_STATUS_START:
        kd_mpi_venc_stop_chn(ch);
    case VENC_SAMPLE_STATUS_INIT:
        kd_mpi_venc_destroy_chn(ch);
        break;
    default:
        break;
    }

    pthread_cancel(venc_conf->output_tid);
    pthread_join(venc_conf->output_tid, NULL);

    venc_debug("kill ch %d thread done! ch_done %d, chnum %d\n", ch, g_venc_conf.ch_done, g_venc_conf.chnum);

    ret = kd_mpi_venc_close_fd();
    CHECK_RET(ret, __func__, __LINE__);

    if (output_file)
        fclose(output_file);

    g_venc_conf.ch_done = K_TRUE;

    return K_SUCCESS;
}

/**
* NV12(YUV420)数据是解码器输出格式，需要将其转换为RGB格式给AI使用
*/
cv::Mat nv12ToRGBHWC(const uint8_t* nv12Data, int width, int height, uint8_t* rgbChwData) {
    cv::Mat nv12Mat(height + height / 2, width, CV_8UC1, const_cast<uint8_t*>(nv12Data));
    cv::Mat rgbMat(height, width, CV_8UC3, rgbChwData);
    cv::cvtColor(nv12Mat, rgbMat, cv::COLOR_YUV2RGB_NV12);
    // cv::imwrite("test.jpg",rgbMat);
    return rgbMat;
}

/**
* AI计算后得到的RGB图像要转换成ARGB格式发送给编码器
*/
cv::Mat convertToARGB(const cv::Mat& src) {
    CV_Assert(src.channels() == 3); // 输入图像应该是 3 通道的 RGB 图像
    cv::Mat dst(src.rows, src.cols, CV_8UC4);

    for (int y = 0; y < src.rows; ++y) {
        const cv::Vec3b* src_row = src.ptr<cv::Vec3b>(y);
        cv::Vec4b* dst_row = dst.ptr<cv::Vec4b>(y);
        for (int x = 0; x < src.cols; ++x) {
            // RGB 到 ARGB 的通道转换
            dst_row[x] = cv::Vec4b(255,src_row[x][0], src_row[x][1], src_row[x][2]);
        }
    }

    return dst;
}

/**
*解码器输出线程逻辑
*/
static void *output_thread(void *arg)
{
    sample_vdec_conf_t *vdec_conf;
    k_s32 ret;
    int out_cnt;
    k_video_frame_info output;
    k_vdec_supplement_info supplement;
    FILE *output_file=NULL;
    void *virt_addr = NULL;
    k_u32 frame_size=0;
    int frame_number=0;
    vdec_conf = (sample_vdec_conf_t *)arg;
    //AI计算相关设置，kmodel路径，阈值等，这里以人脸检测为例
    int debug_mode=0;
    std::string fd_kmodel_path="face_detection_hwc.kmodel";
    float facedet_obj_thresh=0.5;
    float facedet_nms_thresh=0.2;
    FaceDetection fd(fd_kmodel_path.c_str(), facedet_obj_thresh,facedet_nms_thresh, debug_mode);
    std::vector<FaceDetectionInfo> results;
    
    //*******************ai计算后要将得到的结果组装成k_video_frame_info帧对象的格式*******************
    //这里做了一些初始化设置，选择使用1080P、ARGB8888格式数据
    cv::Mat rgb_image;
    //这里使用缓冲池2作为ai结果的发送缓存
    k_u32 g_pool_id=2;
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = 1920;
    vf_info.v_frame.height = 1080;
    vf_info.v_frame.stride[0] = 1920;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    k_vb_blk_handle block = init_venc_frame(&vf_info, &pic_vaddr,g_pool_id);
    //**********************************************************************************************
    
    //循环解码
    while (1)
    {
        //获取解码器通道状态
        k_vdec_chn_status status;
        ret = kd_mpi_vdec_query_status(vdec_conf->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);
        if (status.end_of_stream)
        {
            vdec_debug("%s>ch %d, receive eos\n", __func__, vdec_conf->ch_id);
            break;
        }
        else
        {
            //获取一帧数据
            ret = kd_mpi_vdec_get_frame(vdec_conf->ch_id, &output, &supplement, -1);
            CHECK_RET(ret, __func__, __LINE__);
            if (supplement.is_valid_frame)
            {
                out_cnt++;
            }
            //获取nv12格式数据大小
            frame_size = status.width*status.height*3/2;
            //获取数据的虚拟地址，从帧内的物理地址做映射
            virt_addr = kd_mpi_sys_mmap_cached(output.v_frame.phys_addr[0], frame_size);
            
            //保证分辨率满足1080P
            if(status.width==1920){
                // nv12(YUV420)转换成RGB_HWC数据
                uint8_t *rgb_buffer = (uint8_t *)malloc(status.width * status.height * 3);
                cv::Mat rgb_image=nv12ToRGBHWC((uint8_t *)virt_addr,status.width, status.height,rgb_buffer);
                
                //AI计算部分，预处理、模型推理、后处理、绘制结果
                results.clear();
                fd.pre_process(rgb_image);
                fd.inference();
                fd.post_process({status.width, status.height}, results);
                fd.draw_result(rgb_image,results);

                //将RGB图像转换成ARGB图像，发送给编码器
                cv::Mat rgba_image=convertToARGB(rgb_image);
                memcpy(pic_vaddr, rgba_image.data, vf_info.v_frame.width * vf_info.v_frame.height * 4);
                //1通道是解码器，0通道是编码器，发送给0通道，vf_info是帧数据指针，-1表示为阻塞方式
                ret=kd_mpi_venc_send_frame(0, &vf_info, -1);
                CHECK_RET(ret, __func__, __LINE__);
                free(rgb_buffer);
            }
            
           
            kd_mpi_sys_munmap(virt_addr, frame_size);
            ret = kd_mpi_vdec_release_frame(vdec_conf->ch_id, &output);
            CHECK_RET(ret, __func__, __LINE__);

            if (supplement.end_of_stream)
            {
                vdec_debug("%s>ch %d, type %d, receive eos\n", __func__, supplement.type, vdec_conf->ch_id);
                break;
            }
        }

    }
    //解码结束后，编码随之结束，一定要释放对应的k_vb_blk_handle
    ret = kd_mpi_vb_release_block(block);
    CHECK_RET(ret, __func__, __LINE__);

    if(output_file)
    {
        fclose(output_file);
    }

    vdec_conf->done = K_TRUE;

    return arg;
}

int main(int argc, char *argv[])
{
    k_s32 ret;
    //**********************encoder****************************************
    //编码器配置，编码通道编号为0
    int chnum = 1;
    int ve_ch = 0;
    k_u32 output_frames = 10;
    k_u32 bitrate   = 4000;   //kbps
    int width       = 1920;
    int height      = 1080;
    k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
    k_payload_type ve_type     = K_PT_H265;
    k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
    k_vicap_sensor_type sensor_type = OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR;
    // sprintf(filename, "test_new_data.h265");
    // output_file = fopen(filename, "wb");
    pthread_t exit_thread_handle;
    memset(&g_venc_conf, 0, sizeof(venc_conf_t));
    

    //**********************decoder****************************************
    //解码器配置，解码通道编号为1
    int ch = 1;
    k_vdec_chn_attr attr;
    k_payload_type type = K_PT_BUTT;
    int j;
    int i;
    FILE *input_file = NULL;

    memset(g_vdec_conf, 0, sizeof(sample_vdec_conf_t)*VDEC_MAX_CHN_NUMS);

    //参数解析
    for (i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            printf("Please input:\n");
            printf("-i: input file name\n");
            printf("-o: output file name\n");
            printf("./sample_vdec.elf -i input_file.h265 -o output_file.h265\n");
            return -1;
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            printf("The answer is: %d\n", 1);
            char *ptr = strchr(argv[i + 1], '.');

            vdec_debug("infilename: %s\n", argv[i + 1]);
            if ((input_file = fopen(argv[i + 1], "rb")) == NULL)
            {
                vdec_debug("Cannot open input file!!!\n");
                return -1;
            }
           if (strcmp(ptr, ".h265") == 0 || strcmp(ptr, ".hevc") == 0 || strcmp(ptr, ".265") == 0)
            {
                type = K_PT_H265;
                vdec_debug("file type is H265\n");
            }
            else
            {
                vdec_debug("Error input type\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            strcpy(filename, argv[i + 1]);
            if ((output_file = fopen(filename, "wb")) == NULL)
            {
                venc_debug("Cannot open output file\n");
            }
            printf("filename: %s\n", filename);
        }
        else
        {
            vdec_debug("Error :Invalid arguments %s\n", argv[i]);
            return -1;
        }
    }
    
    //vb初始化，申请两个缓冲池
    sample_vb_init(chnum, K_FALSE);
    //配置编码通道属性
    k_venc_chn_attr ve_attr;
    memset(&ve_attr, 0, sizeof(ve_attr));
    ve_attr.venc_attr.pic_width = width;
    ve_attr.venc_attr.pic_height = height;
    ve_attr.venc_attr.stream_buf_size = VE_STREAM_BUF_SIZE;
    ve_attr.venc_attr.stream_buf_cnt = VE_OUTPUT_BUF_CNT;
    ve_attr.rc_attr.rc_mode = rc_mode;
    ve_attr.rc_attr.cbr.src_frame_rate = 30;
    ve_attr.rc_attr.cbr.dst_frame_rate = 30;
    ve_attr.rc_attr.cbr.bit_rate = bitrate;
    ve_attr.venc_attr.type = ve_type;
    ve_attr.venc_attr.profile = profile;
    venc_debug("payload type is H265\n");
    //创建编码通道
    ret = kd_mpi_venc_create_chn(ve_ch, &ve_attr);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_INIT;
    //启动编码通道
    ret = kd_mpi_venc_start_chn(ve_ch);
    CHECK_RET(ret, __func__, __LINE__);
    g_venc_sample_status = VENC_SAMPLE_STATUS_START;
    sample_vi_bind_venc(ve_ch);
    g_venc_sample_status = VENC_SAMPLE_STATUS_BINDED;
    //编码输出码流设置
    output_info info;
    memset(&info, 0, sizeof(info));
    info.ch_id = ve_ch;
    info.output_frames = output_frames;
    //启动线程将输出的码流写入h265文件
    pthread_create(&g_venc_conf.output_tid, NULL, venc_output_thread, &info);
    g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;
    
    //在编码器中已完成vb初始化，增加缓冲池用于解码器
    vb_create_pool(ch);
    //解码器设置
    g_vdec_conf[ch].ch_id = ch;
    for (j = 0; j < INPUT_BUF_CNT; j++)
    {
        g_vdec_conf[ch].vb_handle[j] = VB_INVALID_HANDLE;
    }
    g_vdec_conf[ch].input_file = input_file;
    if (g_vdec_conf[ch].input_file)
    {
        fseek(g_vdec_conf[ch].input_file, 0L, SEEK_END);
        g_vdec_conf[ch].file_size = ftell(g_vdec_conf[ch].input_file);
        fseek(g_vdec_conf[ch].input_file, 0, SEEK_SET);
    }
    else
        g_vdec_conf[ch].file_size = 0;

    vdec_debug("input file size %d\n", g_vdec_conf[ch].file_size);
    attr.pic_width = MAX_WIDTH;
    attr.pic_height = MAX_HEIGHT;
    attr.frame_buf_cnt = OUTPUT_BUF_CNT;
    attr.frame_buf_size = FRAME_BUF_SIZE;
    attr.stream_buf_size = STREAM_BUF_SIZE;
    attr.type = type;
	attr.frame_buf_pool_id = g_vdec_conf[ch].output_pool_id;
    //创建解码器通道
    ret = kd_mpi_vdec_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);
    //启动解码器通道
    ret = kd_mpi_vdec_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);
    //启动两个线程，一个线程从h265文件中读取数据，另一个线程输出NV12（YUV420）格式的数据
    pthread_create(&g_vdec_conf[ch].input_tid, NULL, input_thread, &g_vdec_conf[ch]);
    pthread_create(&g_vdec_conf[ch].output_tid, NULL, output_thread, &g_vdec_conf[ch]);

    //解码结束后，停止并销毁解码器通道
    while (1)
    {
        if (g_vdec_conf[ch].done == K_TRUE)
        {
            ret = kd_mpi_vdec_stop_chn(ch);
            CHECK_RET(ret, __func__, __LINE__);

            ret = kd_mpi_vdec_destroy_chn(ch);
            CHECK_RET(ret, __func__, __LINE__);

            pthread_kill(g_vdec_conf[ch].input_tid, SIGALRM);
            pthread_join(g_vdec_conf[ch].input_tid, NULL);

            pthread_kill(g_vdec_conf[ch].output_tid, SIGALRM);
            pthread_join(g_vdec_conf[ch].output_tid, NULL);

            fclose(g_vdec_conf[ch].input_file);
            vb_destory_pool(ch);
            vdec_debug("kill ch %d thread done!\n", ch);

            usleep(10000);
            //解码完成后，停止并销毁编码器通道
            sample_exit(&g_venc_conf);
            break;
        }
        else
            usleep(50000);
    }

    ret = kd_mpi_vdec_close_fd();
    CHECK_RET(ret, __func__, __LINE__);
    //vb退出
    sample_vb_exit();

    vdec_debug("sample decode done!\n");

    return 0;
}
