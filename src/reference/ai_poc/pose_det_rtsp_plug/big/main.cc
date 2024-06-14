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
#include "mpi_vo_api.h"
#include "mpi_sys_api.h"
#include "k_vvo_comm.h"
#include "mpi_venc_api.h"
#include "k_venc_comm.h"
#include "mpi_vvi_api.h"
#include "vo_test_case.h"
#include "pose_detect.h"

#include "vi_vo_rtsp.h"
#include "k_datafifo.h"

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

// datafifo
#define READER_INDEX    0
#define WRITER_INDEX    1
static k_s32 g_s32Index = 0;
static k_datafifo_handle hDataFifo[2] = {(k_datafifo_handle)K_DATAFIFO_INVALID_HANDLE, (k_datafifo_handle)K_DATAFIFO_INVALID_HANDLE};
static const k_s32 BLOCK_LEN = 1024000;
k_char *buf = (k_char*)malloc(BLOCK_LEN);

//*******************************encoder*************
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
    k_u32 osd_width_;
    k_u32 osd_height_;
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
std::atomic<bool> isp_stop(false);

VENC_SAMPLE_STATUS g_venc_sample_status = VENC_SAMPLE_STATUS_IDLE;
venc_conf_t g_venc_conf;

//****************function***********************************

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

/**
*vb初始化
*/
static k_s32 sample_vb_init(k_u32 ch_cnt, k_bool osd_enable)
{
    k_s32 ret;
    k_vb_config config;

    memset(&config, 0, sizeof(config));
    
    config.max_pool_cnt = 64;
    config.comm_pool[0].blk_cnt = VE_INPUT_BUF_CNT * ch_cnt;
    config.comm_pool[0].blk_size = VE_FRAME_BUF_SIZE;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_cnt = VE_OUTPUT_BUF_CNT * ch_cnt;
    config.comm_pool[1].blk_size =VE_STREAM_BUF_SIZE;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[2].blk_cnt = 4;
    config.comm_pool[2].blk_size =OSD_BUF_SIZE;
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;


    #if defined(CONFIG_BOARD_K230D_CANMV) 
    //VB for YUV420SP output
    config.comm_pool[0].blk_cnt = 4;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[0].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);
    //VB for RGB888 output
    config.comm_pool[1].blk_cnt = 5;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP((SENSOR_HEIGHT * SENSOR_WIDTH * 3 ), VICAP_ALIGN_1K);
    #else
    //VB for YUV420SP output
    config.comm_pool[3].blk_cnt = 5;
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;
    // ！！！！！！！！===============  这里是编码的时候为了 4k对齐设置的 12; 0x1000);=============== ！！！！！！！！！
    config.comm_pool[3].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), 0x1000);
   
    //VB for RGB888 output
    config.comm_pool[4].blk_cnt = 5;
    config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[4].blk_size = VICAP_ALIGN_UP((SENSOR_HEIGHT * SENSOR_WIDTH * 3 ), VICAP_ALIGN_1K);
    #endif

    ret = kd_mpi_vb_set_config(&config);

    k_vb_supplement_config supplement_config;
    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;

    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    if (ret) {
        printf("vb_set_supplement_config failed ret:%d\n", ret);
        return ret;
    }

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


// datafifo

static void release(void* pStream)
{
    printf("release %p\n", pStream);
}

static int datafifo_init(void)
{
    k_s32 s32Ret = K_SUCCESS;

    k_datafifo_params_s writer_params = {10, BLOCK_LEN, K_TRUE, DATAFIFO_WRITER};

    s32Ret = kd_datafifo_open(&hDataFifo[WRITER_INDEX], &writer_params);

    if (K_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    k_u64 phyAddr = 0;
    s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_GET_PHY_ADDR, &phyAddr);

    if (K_SUCCESS != s32Ret)
    {
        printf("get datafifo phy addr error:%x\n", s32Ret);
        return -1;
    }

    printf("PhyAddr: %lx\n", phyAddr);

    s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, (void *)release);
    // s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, &phyAddr);

    if (K_SUCCESS != s32Ret)
    {
        printf("set release func callback error:%x\n", s32Ret);
        return -1;
    }

    printf("datafifo_init finish\n");

    return 0;
}

void datafifo_deinit(void)
{
    k_s32 s32Ret = K_SUCCESS;
    // call write NULL to flush and release stream buffer.
    s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
    }
    printf(" kd_datafifo_close %lx\n", hDataFifo[WRITER_INDEX]);
    // printf(" kd_datafifo_close %lx\n", hDataFifo[READER_INDEX]);
    kd_datafifo_close(hDataFifo[WRITER_INDEX]);
    // kd_datafifo_close(hDataFifo[READER_INDEX]);
    printf(" finish\n");
}


/**
*编码器输出线程逻辑
*/
static void *venc_output_thread(void *arg)
{
    // datafifo
    k_s32 s32Ret = K_SUCCESS;
    s32Ret = datafifo_init();
    if (0 != s32Ret)
    {
        std::cout << "====== datafifo init failed ======";
    }

    memset(buf, 0, BLOCK_LEN);
    k_venc_stream output;
    int out_cnt, out_frames;
    k_s32 ret;
    int i;
    k_u32 total_len = 0;
    output_info *info = (output_info *)arg;
    out_cnt = 0;
    out_frames = 0;
    
    // int index = 0;
    while (1)
    {
        // datafifo
        k_u32 availWriteLen = 0;
        // call write NULL to flush
        s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
        }
        s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
        if (K_SUCCESS != s32Ret)
        {
            printf("get available write len error:%x\n", s32Ret);
            break;
        }
        

        k_venc_chn_status status;
        ret = kd_mpi_venc_query_status(info->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;
        output.pack = static_cast<k_venc_pack*>(malloc(sizeof(k_venc_pack) * output.pack_cnt));

        // // 设置关键帧频率
        // if (index % 4 == 0)
        // {
        //     index = 0;
        //     ret = kd_mpi_venc_request_idr(0);
        // }
        // index ++;
        
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

            if (availWriteLen >= BLOCK_LEN)
            {
                memcpy(buf, (void *)&(output.pack[i].pts), sizeof(k_u64));
                memcpy(buf + sizeof(k_u64), (void *)&(output.pack[i].len), sizeof(k_u32));
                memcpy(buf + sizeof(k_u64) + sizeof(k_u32), (void *)pData, output.pack[i].len);
                
                s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], buf);
                if (K_SUCCESS != s32Ret)
                {
                    printf("write error:%x\n", s32Ret);
                    break;
                }
                s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_WRITE_DONE, NULL);
                if (K_SUCCESS != s32Ret)
                {
                    printf("write done error:%x\n", s32Ret);
                    break;
                }

                g_s32Index++;
                
            }

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
    free(buf);

    return K_SUCCESS;
}

/**
*解码器输出线程逻辑
*/
void output_thread(char *argv[])
{
    vivcap_start();

    // alloc memory,get isp memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;

    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    int debug_mode=atoi(argv[4]);
    std::string fd_kmodel_path=argv[1];
    float facedet_obj_thresh=atof(argv[2]);
    float facedet_nms_thresh=atof(argv[3]);
    poseDetect pd(fd_kmodel_path.c_str(), facedet_obj_thresh,facedet_nms_thresh, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);

    cv::Vec4d params = pd.params;
    
    std::vector<OutputPose> results;

    //*******************ai计算后要将得到的结果组装成k_video_frame_info帧对象的格式*******************
    //这里做了一些初始化设置，选择使用1080P、ARGB8888格式数据
    //这里使用缓冲池2作为ai结果的发送缓存
    k_u32 g_pool_id=2;
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    k_vb_blk_handle block_enc = init_venc_frame(&vf_info, &pic_vaddr,g_pool_id);
    //**********************************************************************************************

    cv::Mat osd_frame0(osd_height, osd_width, CV_8UC1, cv::Scalar(255));
    cv::Mat osd_frame1(osd_height, osd_width, CV_8UC1, cv::Scalar(0));
    cv::Mat osd_frame2(osd_height, osd_width, CV_8UC1, cv::Scalar(0));
    cv::Mat osd_frame3(osd_height, osd_width, CV_8UC1, cv::Scalar(0));
    std::vector<cv::Mat> channels_argb;
    cv::Mat osd_frame;
    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", debug_mode);
            // 从vivcap中读取一帧图像到dump_info
            memset(&dump_info, 0, sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret)
            {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }

        {
            ScopedTiming st("isp copy", debug_mode);
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        channels_argb.clear();
        results.clear();
        pd.pre_process();
        pd.inference();
        bool find_ = pd.post_process(results,params);

        {      
            ScopedTiming st("memcpy", debug_mode);
            memcpy(osd_frame1.data, (void *)vaddr, SENSOR_HEIGHT * SENSOR_WIDTH);
            memcpy(osd_frame2.data, (void *)vaddr + SENSOR_HEIGHT * SENSOR_WIDTH, SENSOR_HEIGHT * SENSOR_WIDTH);
            memcpy(osd_frame3.data, (void *)vaddr + SENSOR_HEIGHT * SENSOR_WIDTH * 2, SENSOR_HEIGHT * SENSOR_WIDTH);
            channels_argb.push_back(osd_frame0);
            channels_argb.push_back(osd_frame1);
            channels_argb.push_back(osd_frame2);
            channels_argb.push_back(osd_frame3);
            cv::merge(channels_argb, osd_frame);
        }
        
        {
            ScopedTiming st("osd draw", debug_mode);
            Utils::DrawPred_video(osd_frame,{SENSOR_WIDTH,SENSOR_HEIGHT}, results, SKELETON, KPS_COLORS, LIMB_COLORS);
        }

        {
            ScopedTiming st("venc_send_frame", debug_mode);
            //将RGB图像转换成ARGB图像，发送给编码器
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            //1通道是解码器，0通道是编码器，发送给0通道，vf_info是帧数据指针，-1表示为阻塞方式
            ret=kd_mpi_venc_send_frame(0, &vf_info, -1);
            CHECK_RET(ret, __func__, __LINE__);

            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret)
            {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
        }
    }

    vivcap_stop();

    //解码结束后，编码随之结束，一定要释放对应的k_vb_blk_handle
    ret = kd_mpi_vb_release_block(block_enc);
    CHECK_RET(ret, __func__, __LINE__);

    // free memory
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
}

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel> <obj_thresh> <nms_thresh> <debug_mode> " << endl
         << "For example: " << endl
         << " [for isp] ./pose_detect.elf yolov8n-pose.kmodel 0.5 0.45 0" << endl
         << "Options:" << endl
         << " 1> kmodel    pose检测kmodel文件路径 \n"
         << " 2> obj_thresh  pose检测阈值\n"
         << " 3> nms_thresh  NMS阈值\n"
         << " 4> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 5)
    {
        print_usage(argv[0]);
        return -1;
    }

    #if defined(CONFIG_BOARD_K230_CANMV)
    {
        k_s32 ret;
        //**********************encoder****************************************
        //编码器配置，编码通道编号为0
        int chnum = 1;
        int ve_ch = 0;
        k_u32 output_frames = 10;
        k_u32 bitrate   = 4000;   //kbps
        int width       = 1280;
        int height      = 720;
        k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
        k_payload_type ve_type     = K_PT_H265;
        k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
        memset(&g_venc_conf, 0, sizeof(venc_conf_t));
        
        //vb初始化，(venc 以及 vicap)
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
        // 关键帧
        kd_mpi_venc_enable_idr(ve_ch, K_TRUE);
        //启动编码通道
        ret = kd_mpi_venc_start_chn(ve_ch);
        CHECK_RET(ret, __func__, __LINE__);
        g_venc_sample_status = VENC_SAMPLE_STATUS_START;
        //编码输出码流设置
        output_info info;
        memset(&info, 0, sizeof(info));
        info.ch_id = ve_ch;
        info.output_frames = output_frames;
        //启动线程将输出的码流写入h265文件
        pthread_create(&g_venc_conf.output_tid, NULL, venc_output_thread, &info);
        g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

        // 启动 视频流 ai 线程 
        std::thread face_det_enc(output_thread, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        face_det_enc.join();
        usleep(10000);
        sample_exit(&g_venc_conf);

        // datafifo 退出
        datafifo_deinit();
        //vb退出
        sample_vb_exit();

        vdec_debug("sample decode done!\n");
    }
    #elif defined(CONFIG_BOARD_K230_CANMV_V2)
    {
        k_s32 ret;
        //**********************encoder****************************************
        //编码器配置，编码通道编号为0
        int chnum = 1;
        int ve_ch = 0;
        k_u32 output_frames = 10;
        k_u32 bitrate   = 4000;   //kbps
        int width       = 1280;
        int height      = 720;
        k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
        k_payload_type ve_type     = K_PT_H265;
        k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
        memset(&g_venc_conf, 0, sizeof(venc_conf_t));
        
        //vb初始化，(venc 以及 vicap)
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
        // 关键帧
        kd_mpi_venc_enable_idr(ve_ch, K_TRUE);
        //启动编码通道
        ret = kd_mpi_venc_start_chn(ve_ch);
        CHECK_RET(ret, __func__, __LINE__);
        g_venc_sample_status = VENC_SAMPLE_STATUS_START;
        //编码输出码流设置
        output_info info;
        memset(&info, 0, sizeof(info));
        info.ch_id = ve_ch;
        info.output_frames = output_frames;
        //启动线程将输出的码流写入h265文件
        pthread_create(&g_venc_conf.output_tid, NULL, venc_output_thread, &info);
        g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

        // 启动 视频流 ai 线程 
        std::thread face_det_enc(output_thread, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        face_det_enc.join();
        usleep(10000);
        sample_exit(&g_venc_conf);

        // datafifo 退出
        datafifo_deinit();
        //vb退出
        sample_vb_exit();

        vdec_debug("sample decode done!\n");
    }
    #elif defined(CONFIG_BOARD_K230D_CANMV)
    {
        k_s32 ret;
        //**********************encoder****************************************
        //编码器配置，编码通道编号为0
        int chnum = 1;
        int ve_ch = 0;
        k_u32 output_frames = 10;
        k_u32 bitrate   = 4000;   //kbps
        int width       = 1280;
        int height      = 720;
        k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
        k_payload_type ve_type     = K_PT_H265;
        k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
        memset(&g_venc_conf, 0, sizeof(venc_conf_t));
        
        //vb初始化，(venc 以及 vicap)
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
        // 关键帧
        kd_mpi_venc_enable_idr(ve_ch, K_TRUE);
        //启动编码通道
        ret = kd_mpi_venc_start_chn(ve_ch);
        CHECK_RET(ret, __func__, __LINE__);
        g_venc_sample_status = VENC_SAMPLE_STATUS_START;
        //编码输出码流设置
        output_info info;
        memset(&info, 0, sizeof(info));
        info.ch_id = ve_ch;
        info.output_frames = output_frames;
        //启动线程将输出的码流写入h265文件
        pthread_create(&g_venc_conf.output_tid, NULL, venc_output_thread, &info);
        g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

        // 启动 视频流 ai 线程 
        std::thread face_det_enc(output_thread, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        face_det_enc.join();
        usleep(10000);
        sample_exit(&g_venc_conf);

        // datafifo 退出
        datafifo_deinit();
        //vb退出
        sample_vb_exit();

        vdec_debug("sample decode done!\n");
    }
    #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
    {
        k_s32 ret;
        //**********************encoder****************************************
        //编码器配置，编码通道编号为0
        int chnum = 1;
        int ve_ch = 0;
        k_u32 output_frames = 10;
        k_u32 bitrate   = 4000;   //kbps
        int width       = 1280;
        int height      = 720;
        k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
        k_payload_type ve_type     = K_PT_H265;
        k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
        memset(&g_venc_conf, 0, sizeof(venc_conf_t));
        
        //vb初始化，(venc 以及 vicap)
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
        // 关键帧
        kd_mpi_venc_enable_idr(ve_ch, K_TRUE);
        //启动编码通道
        ret = kd_mpi_venc_start_chn(ve_ch);
        CHECK_RET(ret, __func__, __LINE__);
        g_venc_sample_status = VENC_SAMPLE_STATUS_START;
        //编码输出码流设置
        output_info info;
        memset(&info, 0, sizeof(info));
        info.ch_id = ve_ch;
        info.output_frames = output_frames;
        //启动线程将输出的码流写入h265文件
        pthread_create(&g_venc_conf.output_tid, NULL, venc_output_thread, &info);
        g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

        // 启动 视频流 ai 线程 
        std::thread face_det_enc(output_thread, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        face_det_enc.join();
        usleep(10000);
        sample_exit(&g_venc_conf);

        // datafifo 退出
        datafifo_deinit();
        //vb退出
        sample_vb_exit();

        vdec_debug("sample decode done!\n");
    }
    #else
    {
        k_s32 ret;
        //**********************encoder****************************************
        //编码器配置，编码通道编号为0
        int chnum = 1;
        int ve_ch = 0;
        k_u32 output_frames = 10;
        k_u32 bitrate   = 4000;   //kbps
        int width       = 1280;
        int height      = 720;
        k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
        k_payload_type ve_type     = K_PT_H265;
        k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
        memset(&g_venc_conf, 0, sizeof(venc_conf_t));
        
        //vb初始化，(venc 以及 vicap)
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
        // 关键帧
        kd_mpi_venc_enable_idr(ve_ch, K_TRUE);
        //启动编码通道
        ret = kd_mpi_venc_start_chn(ve_ch);
        CHECK_RET(ret, __func__, __LINE__);
        g_venc_sample_status = VENC_SAMPLE_STATUS_START;
        //编码输出码流设置
        output_info info;
        memset(&info, 0, sizeof(info));
        info.ch_id = ve_ch;
        info.output_frames = output_frames;
        //启动线程将输出的码流写入h265文件
        pthread_create(&g_venc_conf.output_tid, NULL, venc_output_thread, &info);
        g_venc_sample_status = VENC_SAMPLE_STATUE_RUNING;

        // 启动 视频流 ai 线程 
        std::thread face_det_enc(output_thread, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        face_det_enc.join();
        usleep(10000);
        sample_exit(&g_venc_conf);

        // datafifo 退出
        datafifo_deinit();
        //vb退出
        sample_vb_exit();

        vdec_debug("sample decode done!\n");
    }
    #endif

    return 0;
}
