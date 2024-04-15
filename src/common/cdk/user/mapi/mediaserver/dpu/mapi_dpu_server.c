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
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <cconfig.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <math.h>

#include "msg_dpu.h"
#include "mpi_dpu_api.h"
#include "mpi_dma_api.h"
#include "mpi_vicap_api.h"
#include "mpi_vvi_api.h"
#include "mapi_dpu_comm.h"
#include "mapi_sys_api.h"
#include "k_dpu_comm.h"
#include "k_dma_comm.h"
#include "mpi_venc_api.h"
#include "mpi_sys_api.h"
#include "k_venc_comm.h"
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "k_datafifo.h"
#include "k_video_comm.h"
#include "mapi_dpu_api.h"
#include "k_vvi_comm.h"
#include "mpi_vvi_api.h"

#define REF_VOL (1.8)
#define RESOLUTION (4096)

#define ADC_CHN_ENABLE (0)
#define ADC_CHN_DISABLE (1)

/* dpu file path define */
#define PARAM_PATH "/sharefs/H1280W720_conf.bin"
#define REF_PATH "/sharefs/H1280W720_ref.bin"

#define MSG_KEY_TYPE 0XFFF1
#define BLOCKLEN (512)
#define DATAFIFO_CHN 4

#define VICAP_OUTPUT_BUF_NUM 10

#define CHECK_MAPI_DPU_NULL_PTR(paraname, ptr)                      \
    do                                                              \
    {                                                               \
        if ((ptr) == NULL)                                          \
        {                                                           \
            mapi_dpu_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_DPU_NULL_PTR;                         \
        }                                                           \
    } while (0)

static inline void VDD_CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

static k_datafifo_handle hDataFifo[DATAFIFO_CHN] = {K_DATAFIFO_INVALID_HANDLE, K_DATAFIFO_INVALID_HANDLE, K_DATAFIFO_INVALID_HANDLE, K_DATAFIFO_INVALID_HANDLE};
static k_u32 dump_count = 0;
static pthread_t output_tid = 0;
static pthread_t adc_tid = 0;
static pthread_t image_tid = 0;
static pthread_t speckle_tid = 0;
static k_bool exiting = K_FALSE;
static k_bool image_exiting = K_FALSE;
static k_bool speckle_exiting = K_FALSE;
static k_video_frame_info rgb_buf[VICAP_OUTPUT_BUF_NUM];
static k_video_frame_info speckle_buf[VICAP_OUTPUT_BUF_NUM];
static k_dpu_chn_result_u depth_buf[VICAP_OUTPUT_BUF_NUM];
static k_bool image_overflow;
static k_u32 rgb_wp = 0;
static k_u32 rgb_rp = 0;
static k_u32 speckle_wp = 0;
static k_u32 speckle_rp = 0;
static k_u32 depth_wp=0;
static k_u32 depth_rp=0;
static k_u32 rgb_total_cnt = 0;
static k_u32 speckle_total_cnt = 0;
static k_u32 depth_total_cnt=0;
static int rgb_dev;
static int rgb_chn;
static int speckle_dev;
static int speckle_chn;
static k_vb_blk_handle ir_handle;
static k_bool adc_en = K_FALSE;
static k_u64 dump_start_time = 0;
static k_u64 rgb_dump_start_time = 0;
static k_bool depth_send_done;
static k_bool rgb_send_done;
static kd_dpu_callback_s dpu_callback_attr[VICAP_DEV_ID_MAX];
static k_datafifo_params_s dpu_writer_params = {128, BLOCKLEN, K_TRUE, DATAFIFO_WRITER};
static int IsInitok[DATAFIFO_CHN] = {0};
static k_dpu_mode_e dpu_bind = DPU_UNBIND;
static k_u32 width;
static k_u32 height;
static k_video_frame_info ir_frame;
static k_char depth_send_buf[BLOCKLEN * 8];
static k_char image_send_buf[BLOCKLEN * 8];
static k_dma_dev_attr_t dma_dev_attr;
static k_dma_chn_attr_u chn_attr[DMA_MAX_CHN_NUMS];
static k_dpu_init_t dpu_init;
static k_dpu_dev_attr_t dpu_dev_attr;
static k_dpu_chn_lcn_attr_t lcn_attr;
static k_dpu_chn_ir_attr_t ir_attr;
static k_dpu_user_space_t g_temp_space;
static k_u32 ir_buf_size;
static k_u32 dpu_buf_cnt;
static k_u32 dma_buf_cnt;
static rt_device_t adc_dev = NULL;
static k_dpu_temperature_t temperature;
static float g_cur_temperature = 0;
static k_dpu_image_mode image_mode;
static k_u32 delay_us = 3000;
static pthread_mutex_t dump_mutex = PTHREAD_MUTEX_INITIALIZER;

static k_u64 get_ticks()
{
    volatile k_u64 time_elapsed;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

int sample_adc(float *temp)
{
    // adc
    int ret = 0;
    unsigned int channel = 2;
    unsigned int reg_value = 0;
    float R_series = 10.2;
    float R_ntc = 1.0;
    float R_t0 = 10.0;
    float Bn = 3380;

    float temperature = 0.0f;
    float vol = 0.0;

    if (adc_dev == NULL)
    {
        adc_dev = rt_device_find("adc");
        if (adc_dev == RT_NULL)
        {
            printf("device find error\n");
            return -1;
        }

        ret = rt_device_open(adc_dev, RT_DEVICE_OFLAG_RDWR);
        if (ret != RT_EOK)
        {
            printf("adc device open err\n");
            return -1;
        }
    }

    ///////////
    // get temperature
    // 1read
    uint32_t *p;
    p = (uint32_t *)(intptr_t)channel;
    ret = rt_device_control(adc_dev, ADC_CHN_ENABLE, (void *)p);
    if (ret != RT_EOK)
    {
        printf("adc device control err\n");
        return -1;
    }

    ret = rt_device_read(adc_dev, channel, (void *)&reg_value, sizeof(unsigned int));

    // 2get target
    vol = REF_VOL * reg_value / RESOLUTION;
    R_ntc = vol * R_series / (REF_VOL - vol);
    temperature = Bn / (log(R_ntc / R_t0) + Bn / 298.15) - 273.15;
    // printf("channel %d reg_value:0x%04x, voltage:%f, R_ntc:%f, temperature:%f *c\n", channel, reg_value, vol, R_ntc, temperature);

    *temp = temperature;
    return 0;
}

int sample_dv_dpu_update_temp(float temperature_obj)
{
    if (temperature_obj < -50 || temperature_obj > 100)
    {
        printf("obj temperature is invalid!\n");
        return -1;
    }

    // printf("start temperature rectify:ref_temp: %f, temperature:%f  \n", temperature.ref, temperature_obj);
    float diff_temp = temperature_obj - temperature.ref;
    // if (diff_temp < 3 && diff_temp > -3)
    //{
    //	printf("temperature is near refence temperature!there is no need to rectify \n");
    //	return 0;
    // }

    k_u16 image_width = dpu_dev_attr.dev_param.spp.width_speckle;
    k_u16 image_height = dpu_dev_attr.dev_param.spp.height_speckle;

    float *row_offset_ = dpu_dev_attr.dev_param.lpp.row_offset;
    float *col_offset_ = dpu_dev_attr.dev_param.lpp.col_offset;

    // 温度补偿量计算
    for (k_u16 r = 0; r < image_height; r++)
    {
        // 行偏差温度补偿
        float dalte_v_ = 0.f;
        float temp_vertical = (r - temperature.cy) * diff_temp * temperature.ky;
        float y_temp = temp_vertical + r;
        if (y_temp > 0.5 && y_temp < image_height - 0.5)
            dalte_v_ = temp_vertical;
        row_offset_[r] = dalte_v_;
    }

    for (k_u16 c = 0; c < image_width; c++)
    {
        // 列偏差温度补偿
        float dalte_u_ = 0.f;
        float temp_horizonal = (c - temperature.cx) * diff_temp * temperature.kx;
        float x_temp = temp_horizonal + c;
        if (x_temp > 0.5 && x_temp < image_width - 0.5)
            dalte_u_ = temp_horizonal;
        col_offset_[c] = dalte_u_;
    }

    kd_mpi_sys_mmz_flush_cache(dpu_dev_attr.dev_param.lpp.row_offset_phys, dpu_dev_attr.dev_param.lpp.row_offset, image_height * sizeof(float));
    kd_mpi_sys_mmz_flush_cache(dpu_dev_attr.dev_param.lpp.col_offset_phys, dpu_dev_attr.dev_param.lpp.col_offset, image_width * sizeof(float));

    k_s32 ret = kd_mpi_dpu_set_dev_attr(&dpu_dev_attr);
    if (ret)
    {
        printf("kd_mpi_dpu_set_dev_attr failed\n");
        printf("rectify failed\n");
        return -1;
        // goto err_dpu_delet;
    }
    printf("rectify success\n");

    return 0;
}

int sample_dv_dpu_init()
{
    k_s32 ret;

    /************************************************************
     * This part is the demo that actually starts to use DPU
     ***********************************************************/
    /* dpu init */
    dpu_init.start_num = 0;
    dpu_init.buffer_num = dpu_buf_cnt;
    ret = kd_mpi_dpu_init(&dpu_init);
    if (ret)
    {
        printf("kd_mpi_dpu_init failed\n");
        goto err_return;
    }

    /* parse file */
    ret = kd_mpi_dpu_parse_file(PARAM_PATH,
                                &dpu_dev_attr.dev_param,
                                &lcn_attr.lcn_param,
                                &ir_attr.ir_param,
                                &g_temp_space);
    // printf("g_temp_space.virt_addr:%p, g_temp_space.phys_addr:%lx\n",
    //     g_temp_space.virt_addr, g_temp_space.phys_addr);
    if (g_temp_space.virt_addr == NULL)
    {
        printf("g_temp_space.virt_addr is NULL\n");
        goto err_return;
    }
    if (ret)
    {
        printf("kd_mpi_dpu_parse_file failed\n");
        goto err_return;
    }

    /* set device attribute */
    dpu_dev_attr.mode = dpu_bind;
    dpu_dev_attr.tytz_temp_recfg = K_TRUE;
    dpu_dev_attr.align_depth_recfg = K_TRUE;
    dpu_dev_attr.param_valid = 123;

    ret = kd_mpi_dpu_set_dev_attr(&dpu_dev_attr);
    if (ret)
    {
        printf("kd_mpi_dpu_set_dev_attr failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_dev_attr success, flag_align %d\n", dpu_dev_attr.dev_param.spp.flag_align);

    /* set reference image */
    ret = kd_mpi_dpu_set_ref_image(REF_PATH);
    if (ret)
    {
        printf("kd_mpi_dpu_set_ref_image failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_ref_image success\n");

    /* set template image */
    ret = kd_mpi_dpu_set_template_image(&g_temp_space);
    if (ret)
    {
        printf("kd_mpi_dpu_set_template_image failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_template_image success\n");

    /* start dev */
    ret = kd_mpi_dpu_start_dev();
    if (ret)
    {
        printf("kd_mpi_dpu_start_dev failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_start_dev success\n");

    /* set chn attr */
    lcn_attr.chn_num = 0;
    lcn_attr.param_valid = 0;
    ir_attr.chn_num = 1;
    ir_attr.param_valid = 0;
    ret = kd_mpi_dpu_set_chn_attr(&lcn_attr, &ir_attr);
    if (ret)
    {
        printf("kd_mpi_dpu_set_chn_attr failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_set_chn_attr success\n");

    /* start channel 0 */
    ret = kd_mpi_dpu_start_chn(0);
    if (ret)
    {
        printf("kd_mpi_dpu_start_chn 0 failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_start_chn lcn success\n");

    /* start channel 1 */
    ret = kd_mpi_dpu_start_chn(1);
    if (ret)
    {
        printf("kd_mpi_dpu_start_chn 1 failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_start_chn ir success\n");

    return K_SUCCESS;

    /************************************************************
     * This part is used to stop the DPU
     ***********************************************************/
    ret = kd_mpi_dpu_stop_chn(0);
    if (ret)
    {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }
    ret = kd_mpi_dpu_stop_chn(1);
    if (ret)
    {
        printf("kd_mpi_dpu_stop_chn ir failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

err_dpu_dev:
    ret = kd_mpi_dpu_stop_dev();
    if (ret)
    {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_dev success\n");

err_dpu_delet:
    kd_mpi_dpu_delete();

err_return:
    return K_FAILED;
}

int sample_dv_dpu_delete()
{
    k_s32 ret;

    ret = kd_mpi_dpu_stop_chn(0);
    if (ret)
    {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }
    ret = kd_mpi_dpu_stop_chn(1);
    if (ret)
    {
        printf("kd_mpi_dpu_stop_chn ir failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

    ret = kd_mpi_dpu_stop_dev();
    if (ret)
    {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_dev success\n");

    kd_mpi_dpu_delete();

    return K_SUCCESS;
}

static k_s32 dma_dev_attr_init(k_dma_dev_attr_t *dev_attr)
{
    dev_attr->burst_len = 0;
    dev_attr->ckg_bypass = 0xff;
    dev_attr->outstanding = 7;

    return K_SUCCESS;
}

static k_s32 dma_chn_attr_init(k_dma_chn_attr_u attr[8])
{
    k_gdma_chn_attr_t *gdma_attr;

    gdma_attr = &attr[0].gdma_attr;
    gdma_attr->buffer_num = dma_buf_cnt;
    gdma_attr->rotation = DEGREE_90; // DEGREE_90;
    gdma_attr->x_mirror = K_FALSE;
    gdma_attr->y_mirror = K_FALSE;
    gdma_attr->width = width;
    gdma_attr->height = height;
    gdma_attr->src_stride[0] = width;
    gdma_attr->dst_stride[0] = height;
    gdma_attr->work_mode = (k_dma_mode_e)dpu_bind;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YUV_400_8BIT;

    gdma_attr = &attr[1].gdma_attr;
    gdma_attr->buffer_num = dma_buf_cnt;
    gdma_attr->rotation = DEGREE_90;
    gdma_attr->x_mirror = K_FALSE;
    gdma_attr->y_mirror = K_FALSE;
    gdma_attr->width = width;
    gdma_attr->height = height;
    gdma_attr->src_stride[0] = width;
    gdma_attr->dst_stride[0] = height;
    gdma_attr->src_stride[1] = width;
    gdma_attr->dst_stride[1] = height;
    gdma_attr->work_mode = DMA_UNBIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YVU_SEMIPLANAR_420_8BIT;

    return K_SUCCESS;
}

int sample_dv_dma_init()
{
    k_s32 ret;

    /************************************************************
     * This part is used to initialize the parameters of the dma.
     ***********************************************************/
    memset(chn_attr, 0, sizeof(k_dma_chn_attr_u) * DMA_MAX_CHN_NUMS);
    dma_dev_attr_init(&dma_dev_attr);
    dma_chn_attr_init(chn_attr);

    /************************************************************
     * This part is the demo that actually starts to use DMA
     ***********************************************************/
    ret = kd_mpi_dma_set_dev_attr(&dma_dev_attr);
    if (ret != K_SUCCESS)
    {
        printf("set dev attr error\r\n");
        goto err_return;
    }

    ret = kd_mpi_dma_start_dev();
    if (ret != K_SUCCESS)
    {
        printf("start dev error\r\n");
        goto err_return;
    }

    ret = kd_mpi_dma_set_chn_attr(0, &chn_attr[0]);
    if (ret != K_SUCCESS)
    {
        printf("set chn attr error\r\n");
        goto err_dma_dev;
    }
    ret = kd_mpi_dma_start_chn(0);
    if (ret != K_SUCCESS)
    {
        printf("start chn error\r\n");
        goto err_dma_dev;
    }

    ret = kd_mpi_dma_set_chn_attr(1, &chn_attr[1]);
    if (ret != K_SUCCESS)
    {
        printf("set chn attr error\r\n");
        goto err_dma_dev;
    }
    ret = kd_mpi_dma_start_chn(1);
    if (ret != K_SUCCESS)
    {
        printf("start chn error\r\n");
        goto err_dma_dev;
    }

    return K_SUCCESS;

    /************************************************************
     * This part is used to stop the DMA
     ***********************************************************/
    ret = kd_mpi_dma_stop_chn(0);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
    }

    ret = kd_mpi_dma_stop_chn(1);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
    }

err_dma_dev:
    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS)
    {
        printf("stop dev error\r\n");
    }

err_return:
    return K_FAILED;
}

int sample_dv_dma_delete()
{
    k_s32 ret;

    /************************************************************
     * This part is used to stop the DMA
     ***********************************************************/
    ret = kd_mpi_dma_stop_chn(0);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
    }

    ret = kd_mpi_dma_stop_chn(1);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
    }

    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS)
    {
        printf("stop dev error\r\n");
    }

    return K_SUCCESS;
}

static void dpu_release(void *pStream)
{
    datafifo_msg *pmsg = (datafifo_msg *)pStream;
    int j = 0;
    k_s32 ret;
    if (pmsg->msg_type == MSG_KEY_TYPE)
    {
        if (pmsg->dev_num == rgb_dev)
        {
            rgb_send_done = K_TRUE;
            // printf("%s>rgb time %ld ms\n", __func__, get_ticks()/27000);
        }
        else if (pmsg->dev_num == speckle_dev)
        {
            depth_send_done = K_TRUE;
            // printf("%s>depth time %ld ms\n", __func__, get_ticks()/27000);
        }
    }
}

k_u64 send_dpu_data_init(int chn)
{
    printf("send_dpu_data_init chn:%d \n", chn);
    if (chn >= DATAFIFO_CHN)
    {
        printf("open datafifo chn:%d error\n", chn);
        return -1;
    }
    k_s32 s32Ret = K_SUCCESS;
    s32Ret = kd_datafifo_open(&hDataFifo[chn], &dpu_writer_params);
    if (K_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    k_u64 phyAddr = 0;
    s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_GET_PHY_ADDR, &phyAddr);
    if (K_SUCCESS != s32Ret)
    {
        printf("get datafifo phy addr error:%x\n", s32Ret);
        return -1;
    }
    printf("send_dpu_data_init PhyAddr: %lx\n", phyAddr);
    s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, dpu_release);
    if (K_SUCCESS != s32Ret)
    {
        printf("set release func callback error:%x\n", s32Ret);
        return -1;
    }
    IsInitok[chn] = 1;
    return phyAddr;
}

int callwriteNULLtoflushdpu(int chn)
{
    if (chn >= DATAFIFO_CHN)
    {
        printf("callwriteNULLtoflushdpu chn:%d error\n", chn);
        return -1;
    }
    if (IsInitok[chn] == 1)
    {
        k_s32 s32Ret = K_SUCCESS;
        // call write NULL to flush
        s32Ret = kd_datafifo_write(hDataFifo[chn], NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
            return -1;
        }
    }
    return 0;
}

int send_dpu_data_to_little(int chn, k_char *buf)
{
    if (chn >= DATAFIFO_CHN)
    {
        printf("open datafifo_sen chn:%d error\n", chn);
        return -1;
    }
    if (IsInitok[chn] != 1)
    {
        // printf("not open datafifo chn:%d error\n", chn);
        return -2;
    }

    k_s32 s32Ret = K_SUCCESS;
    k_u32 availWriteLen = 0;
    int j = 0;

    // call write NULL to flush
    s32Ret = kd_datafifo_write(hDataFifo[chn], NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
        return -3;
    }

    s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
    if (K_SUCCESS != s32Ret)
    {
        printf("get available write len error:%x\n", s32Ret);
        return -4;
    }

    if (availWriteLen >= BLOCKLEN && availWriteLen <= (128 * BLOCKLEN))
    {
        datafifo_msg *pmsg = (datafifo_msg *)buf;

        s32Ret = kd_datafifo_write(hDataFifo[chn], buf);
        if (K_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
            return -5;
        }

        s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_WRITE_DONE, NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("write done error:%x\n", s32Ret);
            return -6;
        }
        // printf("send end datafifo[%d] phys_addr:0x%lx time[%ld]\n", chn, pmsg->pack[0].phys_addr, get_big_currtime());
    }
    return 0;
}

int send_dpu_data_deinit(int chn)
{
    if (chn >= DATAFIFO_CHN)
    {
        printf("send_dpu_data_deinit chn:%d error\n", chn);
        return -1;
    }
    k_s32 s32Ret = K_SUCCESS;
    // call write NULL to flush and release stream buffer.
    s32Ret = kd_datafifo_write(hDataFifo[chn], NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
        return -1;
    }
    printf(" kd_datafifo_close %lx\n", hDataFifo[chn]);
    kd_datafifo_close(hDataFifo[chn]);
    printf(" finish\n");
    IsInitok[chn] = 0;
    return 0;
}

static void release_image()
{
    k_s32 ret;

    ret = kd_mpi_vicap_dump_release(rgb_dev, rgb_chn, &rgb_buf[rgb_rp]);
    if (ret)
    {
        printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", rgb_dev, rgb_chn);
    }

    rgb_rp++;
    rgb_rp %= VICAP_OUTPUT_BUF_NUM;
}

static void release_speckle()
{
    k_s32 ret;

    ret = kd_mpi_vicap_dump_release(speckle_dev, speckle_chn, &speckle_buf[speckle_rp]);
    if (ret)
    {
        printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", speckle_dev, speckle_chn);
    }
    speckle_rp++;
    speckle_rp %= VICAP_OUTPUT_BUF_NUM;
}

static void image_dump()
{
    k_video_frame_info dump_info;
    k_s32 ret;
    k_u32 temp;

    if (rgb_dump_start_time == 0)
        rgb_dump_start_time = get_ticks() / 27000;

    // if (dpu_bind == DPU_UNBIND &&
    //     (image_mode == IMAGE_MODE_RGB_DEPTH || image_mode == IMAGE_MODE_RGB_IR))
    // {
    //     kd_mpi_vicap_3d_mode_crtl(K_FALSE);
    //     usleep(delay_us);
    // }

    ret = kd_mpi_vicap_dump_frame(rgb_dev, rgb_chn, VICAP_DUMP_YUV, &rgb_buf[rgb_wp], 100);

    // if (dpu_bind == DPU_UNBIND &&
    //     (image_mode == IMAGE_MODE_RGB_DEPTH || image_mode == IMAGE_MODE_RGB_IR))
    // {
    //     kd_mpi_vicap_3d_mode_crtl(K_TRUE);
    //     usleep(delay_us);
    // }

    if (ret)
    {
        // printf("sample_vicap, dev(%d) chn(%d) dump frame failed 0x%lx.\n", dev_num, chn_num, ret);
        return;
    }

    if (rgb_buf[rgb_wp].v_frame.phys_addr[0] == 0)
    {
        printf("rgb dump error: phys_addr is 0\n");
        return;
    }

    temp = rgb_wp;
    temp++;
    temp %= VICAP_OUTPUT_BUF_NUM;
    if (temp == rgb_rp)
    {
        printf("image buffer overflow\n");
        image_overflow = K_TRUE;
        ret = kd_mpi_vicap_dump_release(rgb_dev, rgb_chn, &rgb_buf[rgb_wp]);
        if (ret)
        {
            printf("sample_vicap, dev(%d) chn(%d) dump frame failed.\n", rgb_dev, rgb_chn);
        }
        return;
    }

    // printf("save rgb_wp %d, pts %ld, stc %ld\n", rgb_wp, rgb_buf[rgb_wp].v_frame.pts, get_ticks()/27000);

    rgb_wp++;
    rgb_wp %= VICAP_OUTPUT_BUF_NUM;

    rgb_total_cnt++;
    image_overflow = K_FALSE;

    // if (rgb_total_cnt % 30 == 0)
    // {
    //     printf("rgb_dump_count %d, Average FrameRate = %ld Fps\n", rgb_total_cnt, (rgb_total_cnt * 1000) / (get_ticks() / 27000 - rgb_dump_start_time));
    // }
    // printf("image frame cnt:%d,timestamp:%ld,frame_num:%d\n",rgb_total_cnt,rgb_buf[rgb_wp].v_frame.pts,rgb_buf[rgb_wp].v_frame.time_ref);
}

static void speckle_dump()
{
    k_video_frame_info dump_info;
    k_s32 ret;
    k_u32 temp;

    ret = kd_mpi_vicap_dump_frame(speckle_dev, speckle_chn, VICAP_DUMP_YUV, &speckle_buf[speckle_wp], 100);
    if (ret)
    {
        // printf("sample_vicap, dev(%d) chn(%d) dump frame failed 0x%lx.\n", dev_num, chn_num, ret);
        return;
    }

    if (speckle_buf[speckle_wp].v_frame.phys_addr[0] == 0)
    {
        printf("speckle dump error: phys_addr is 0\n");
        return;
    }

    // printf("save speckle_wp %d, pts %ld, %p, stc %ld\n", speckle_wp, speckle_buf[speckle_wp].v_frame.pts, &speckle_buf[speckle_wp], get_ticks()/27000);

    temp = speckle_wp;
    temp++;
    temp %= VICAP_OUTPUT_BUF_NUM;
    if (temp == speckle_rp)
    {
        printf("speckle buffer overflow\n");
        ret = kd_mpi_vicap_dump_release(speckle_dev, speckle_chn, &speckle_buf[speckle_wp]);
        if (ret)
        {
            printf("sample_vicap, dev(%d) chn(%d) release frame failed %d.\n", speckle_dev, speckle_chn, ret);
        }
        return;
    }

    speckle_wp++;
    speckle_wp %= VICAP_OUTPUT_BUF_NUM;
    if (speckle_wp == speckle_rp)
    {
        printf("speckle buffer overflow\n");
    }

    speckle_total_cnt++;
}

static void depth_dump()
{
    k_s32 ret;
    k_dpu_chn_result_u ir_result;
    k_bool get_ir = K_FALSE;

    if(dpu_dev_attr.dev_param.spp.flag_align)
        get_ir = K_TRUE;

    ret = kd_mpi_dpu_get_frame(0, &depth_buf[depth_wp], 100);
    if (ret)
	{
        printf("kd_mpi_dpu_get_frame lcn failed\n");
        return;
    }

    if(get_ir)
    {
        ret = kd_mpi_dpu_get_frame(1, &ir_result, 100);
        if (ret) {
            printf("kd_mpi_dpu_get_frame ir failed\n");
        }
        kd_mpi_dpu_release_frame();
    }

    depth_wp++;
    depth_wp %= VICAP_OUTPUT_BUF_NUM;
    if (depth_wp == depth_rp)
    {
        printf("depth buffer overflow\n");
    }

    // printf("save depth_wp %d, pts %ld, %p, stc %ld\n", depth_wp, depth_buf[depth_wp].lcn_result.pts, &depth_buf[depth_wp], get_ticks()/27000);

    depth_total_cnt++;
}

static void send_image(k_u64 depth_pts)
{
    k_s32 ret;
    k_u64 delta, prev_delta, prev_pts;
    int i, cnt=0;
    k_u32 temp_rp;

    memset(image_send_buf, 0, BLOCKLEN * 8);

    // printf("depth_pts %ld, rgb_wp %d, rgb_rp %d\n", depth_pts, rgb_wp, rgb_rp);
    // for(i=0; i<VICAP_OUTPUT_BUF_NUM; i++)
    // {
    //     printf("i %d, pts %ld\n", i, rgb_buf[i].v_frame.pts);
    // }

    //find the closest pts
    temp_rp = rgb_rp;
    while(rgb_wp != temp_rp)
    {
        if(depth_pts > rgb_buf[temp_rp].v_frame.pts)
            delta = depth_pts - rgb_buf[temp_rp].v_frame.pts;
        else
            delta = rgb_buf[temp_rp].v_frame.pts - depth_pts;
        // printf("temp_rp %d, pts %ld, delta %ld, prev_delta %ld\n", temp_rp, rgb_buf[temp_rp].v_frame.pts, delta, prev_delta);
        if(cnt > 0)
        {
            if(delta > prev_delta)
            {
                // printf("found matched pts %ld, cnt %d\n", prev_pts, cnt);
                break;
            }
        }
        prev_delta = delta;
        prev_pts = rgb_buf[temp_rp].v_frame.pts;

        temp_rp ++;
        temp_rp %= VICAP_OUTPUT_BUF_NUM;
        if(temp_rp == rgb_wp)
        {
            break;
        }
        cnt++;
    }
    for(i=0; i<cnt-1; i++)
    {
        release_image();
    }

    //send image
    {
        k_u32 data_size = 0;
        void *virt_addr = NULL;
        k_char filename[256];
        k_video_frame_info dma_get_info;
        int dataret = -1;

        data_size = rgb_buf[rgb_rp].v_frame.width * rgb_buf[rgb_rp].v_frame.height * 3 / 2;

        ret = kd_mpi_dma_send_frame(1, &rgb_buf[rgb_rp], 30);
        if (ret != K_SUCCESS)
        {
            printf("dma send frame error\r\n");
        }

        ret = kd_mpi_dma_get_frame(1, &dma_get_info, 30);
        if (ret != K_SUCCESS)
        {
            printf("dma get frame error\r\n");
        }

        k_u64 start_time = get_ticks() / 27000;
        datafifo_msg *pmsg = (datafifo_msg *)image_send_buf;
        pmsg->msg_type = MSG_KEY_TYPE;
        pmsg->dev_num = rgb_dev;
        pmsg->result_type = DPU_RESULT_TYPE_IMG;
        pmsg->upfunc = (k_u8 *)dpu_callback_attr[rgb_dev].pfn_data_cb;
        pmsg->puserdata = dpu_callback_attr[rgb_dev].p_private_data;

        rgb_send_done = K_FALSE;
        memcpy(&pmsg->dpu_result.img_result, &dma_get_info, sizeof(k_video_frame));
        pmsg->dpu_result.img_result.time_ref = dump_count;
        pmsg->dpu_result.img_result.pts = rgb_buf[rgb_rp].v_frame.pts;
        dataret = -1;
        pmsg->temperature = g_cur_temperature;
        dataret = send_dpu_data_to_little(rgb_dev, image_send_buf);
        // printf("send depth and rgb: delta pts %6ld, rgb_rp %d, time %6ld ms\n",
        //     (depth_pts-rgb_buf[rgb_rp].v_frame.pts), rgb_rp, get_ticks()/27000);
        if (dataret)
        {
            printf("send img error: %d\n", dataret);
        }
        while (!rgb_send_done)
        {
            callwriteNULLtoflushdpu(rgb_dev);
            usleep(1000);
        }
        // printf("send %d rgb pts %ld, take time %6ld ms\n", dump_count, rgb_buf[rgb_rp].v_frame.pts, (get_ticks()/27000-start_time));
        kd_mpi_dma_release_frame(1, &dma_get_info);
    }

    release_image();

    if(image_overflow)
    {
        while (rgb_wp != rgb_rp)
        {
            release_image();
        }
    }
}

static void dpu_unbind_dump()
{
    k_video_frame_info dma_get_info;
    k_dpu_chn_result_u lcn_result;
    k_dpu_chn_result_u ir_result;
    k_bool get_ir = K_FALSE;
    int dataret = -1;
    k_s32 ret;
    k_u64 depth_pts = 0;

    memset(depth_send_buf, 0, BLOCKLEN * 8);

    if (dpu_dev_attr.dev_param.spp.flag_align)
        get_ir = K_TRUE;

    if (dump_start_time == 0)
        dump_start_time = get_ticks() / 27000;

    while (speckle_wp != speckle_rp)
    {
        depth_pts = speckle_buf[speckle_rp].v_frame.pts;
        {
            k_u32 next_speckle_rp = speckle_rp + 1;
            next_speckle_rp %= VICAP_OUTPUT_BUF_NUM;
            //find the latest frame and discard old frame
            if (depth_pts == 0 || (next_speckle_rp != speckle_wp))
            {
                release_speckle();
                continue;
            }
        }

        ret = kd_mpi_dma_send_frame(0, &speckle_buf[speckle_rp], 30);
        if (ret != K_SUCCESS)
        {
            printf("dma send frame error\r\n");
        }

        ret = kd_mpi_dma_get_frame(0, &dma_get_info, 30);
        if (ret != K_SUCCESS)
        {
            printf("dma get frame error\r\n");
        }

        release_speckle();

        k_u64 start_time = get_ticks() / 27000;

        if (image_mode == IMAGE_MODE_RGB_IR ||
            image_mode == IMAGE_MODE_NONE_SPECKLE ||
            image_mode == IMAGE_MODE_NONE_IR)
        {
            datafifo_msg *pmsg = (datafifo_msg *)depth_send_buf;
            pmsg->msg_type = MSG_KEY_TYPE;
            pmsg->dev_num = speckle_dev;
            pmsg->result_type = DPU_RESULT_TYPE_IMG;
            pmsg->upfunc = (k_u8 *)dpu_callback_attr[speckle_dev].pfn_data_cb;
            pmsg->puserdata = dpu_callback_attr[speckle_dev].p_private_data;
            depth_send_done = K_FALSE;
            memcpy(&pmsg->dpu_result.img_result, &dma_get_info, sizeof(k_video_frame));
            pmsg->dpu_result.img_result.time_ref = dump_count;
            pmsg->dpu_result.img_result.pts = depth_pts;
            dataret = -1;
            pmsg->temperature = g_cur_temperature;

            dataret = send_dpu_data_to_little(speckle_dev, depth_send_buf);

            if (dataret)
            {
                printf("send IR error: %d\n", dataret);
            }

            while (!depth_send_done)
            {
                callwriteNULLtoflushdpu(speckle_dev);
                usleep(1000);
            }

            kd_mpi_dma_release_frame(0, &dma_get_info);
            // printf("send %d IR pts %ld take time %6ld ms\n", dump_count, depth_pts, (get_ticks()/27000-start_time));
        }
        else
        {
            ret = kd_mpi_dpu_send_frame(0, dma_get_info.v_frame.phys_addr[0], 30);
            if (ret)
            {
                printf("kd_mpi_dpu_send_frame failed, time %ld ms\n", get_ticks() / 27000);
            }

            if (get_ir)
            {
                ret = kd_mpi_dpu_send_frame(1, ir_frame.v_frame.phys_addr[0], 30);
                if (ret)
                {
                    printf("kd_mpi_dpu_send_frame ir failed\n");
                }
            }

            ret = kd_mpi_dpu_get_frame(0, &lcn_result, 100);
            if (ret)
            {
                printf("kd_mpi_dpu_get_frame failed\n");
            }

            if (get_ir)
            {
                ret = kd_mpi_dpu_get_frame(1, &ir_result, 100);
                if (ret)
                {
                    printf("kd_mpi_dpu_get_frame failed\n");
                }
                kd_mpi_dpu_release_frame();
            }

            datafifo_msg *pmsg = (datafifo_msg *)depth_send_buf;
            pmsg->msg_type = MSG_KEY_TYPE;
            pmsg->dev_num = speckle_dev;
            pmsg->result_type = DPU_RESULT_TYPE_LCN;
            pmsg->upfunc = (k_u8 *)dpu_callback_attr[speckle_dev].pfn_data_cb;
            pmsg->puserdata = dpu_callback_attr[speckle_dev].p_private_data;
            depth_send_done = K_FALSE;
            memcpy(&pmsg->dpu_result.lcn_result, &lcn_result, sizeof(k_dpu_chn_result_u));
            pmsg->dpu_result.lcn_result.time_ref = dump_count;
            pmsg->dpu_result.lcn_result.pts = depth_pts;
            dataret = -1;
            pmsg->temperature = g_cur_temperature;
            dataret = send_dpu_data_to_little(speckle_dev, depth_send_buf);

            kd_mpi_dma_release_frame(0, &dma_get_info);

            if (dataret)
            {
                printf("send depth error: %d\n", dataret);
                kd_mpi_dpu_release_frame();
            }

            while (!depth_send_done)
            {
                callwriteNULLtoflushdpu(speckle_dev);
                usleep(1000);
            }
            kd_mpi_dpu_release_frame();
            // printf("send %d depth pts %ld take time %6ld ms\n", dump_count, depth_pts, (get_ticks()/27000-start_time));
        }

        send_image(depth_pts);

        dump_count++;
        if (dump_count % 30 == 0)
        {
            printf("dump_count %d, Average FrameRate = %ld Fps\n", dump_count, (dump_count * 1000) / (get_ticks() / 27000 - dump_start_time));
        }
    }
}

static void dpu_bind_dump()
{
    k_dpu_chn_result_u lcn_result;
    k_bool get_ir = K_FALSE;
    int dataret = -1;
    k_s32 ret;
    k_u64 depth_pts=0;
    k_video_frame_info dma_get_info;

    memset(depth_send_buf, 0, BLOCKLEN * sizeof(k_char));

    if(dump_start_time == 0)
        dump_start_time = get_ticks()/27000;

    while(depth_wp != depth_rp && rgb_wp != rgb_rp)
    {
        k_bool found = K_FALSE;

		lcn_result = depth_buf[depth_rp];
		depth_pts = lcn_result.lcn_result.pts;
        {
            k_u32 next_depth_rp = depth_rp + 1;
            next_depth_rp %= VICAP_OUTPUT_BUF_NUM;
            //find the latest frame and drop old frames
            if (next_depth_rp != depth_wp)
            {
                kd_mpi_dpu_release_frame();
                depth_rp++;
                depth_rp %= VICAP_OUTPUT_BUF_NUM;
                continue;
            }
        }

        while(rgb_wp != rgb_rp)
        {
            k_u64 delta, rgb_pts;
            k_u32 temp;
            rgb_pts = rgb_buf[rgb_rp].v_frame.pts;
            delta = depth_pts > rgb_pts?(depth_pts - rgb_pts):(rgb_pts - depth_pts);
            temp = (rgb_rp + 1) % VICAP_OUTPUT_BUF_NUM;
            // printf("rgb_rp %d, rgb pts %ld, depth pts %ld, delta %ld\n", rgb_rp, rgb_pts, depth_pts, delta);
            if((delta < 100000) || (delta > 1000000) || (temp == rgb_wp))
            {
                found = K_TRUE;
                break;
            }
            release_image();
        }

        if(!found)
        {
            kd_mpi_dpu_release_frame();
            depth_rp++;
            depth_rp %= VICAP_OUTPUT_BUF_NUM;
            continue;
        }

        k_u64 start_time=get_ticks()/27000;

        datafifo_msg *pmsg = (datafifo_msg *)depth_send_buf;
        pmsg->msg_type = MSG_KEY_TYPE;
        pmsg->dev_num = speckle_dev;
        pmsg->result_type = DPU_RESULT_TYPE_LCN;
        pmsg->upfunc = (k_u8 *)dpu_callback_attr[speckle_dev].pfn_data_cb;
        pmsg->puserdata = dpu_callback_attr[speckle_dev].p_private_data;
        depth_send_done = K_FALSE;
        memcpy(&pmsg->dpu_result.lcn_result, &lcn_result, sizeof(k_dpu_chn_result_u));
        pmsg->dpu_result.lcn_result.time_ref = dump_count;
        pmsg->dpu_result.lcn_result.pts = depth_pts;
        pmsg->temperature = g_cur_temperature;

        dataret = -1;
        dataret = send_dpu_data_to_little(speckle_dev, depth_send_buf);
        if(dataret){
            printf("send depth error: %d\n", dataret);
            kd_mpi_dpu_release_frame();
        }

        while(!depth_send_done)
        {
            callwriteNULLtoflushdpu(speckle_dev);
            usleep(1000);
        }

        // printf("send depth take time %6ld ms\n", (get_ticks()/27000-start_time));
        kd_mpi_dpu_release_frame();

        k_u32 data_size = 0;
        void *virt_addr = NULL;
        k_char filename[256];

        data_size = rgb_buf[rgb_rp].v_frame.width * rgb_buf[rgb_rp].v_frame.height * 3 / 2;

        ret = kd_mpi_dma_send_frame(1, &rgb_buf[rgb_rp], 30);
        if (ret != K_SUCCESS)
        {
            printf("dma send frame error\r\n");
        }

        ret = kd_mpi_dma_get_frame(1, &dma_get_info, 30);
        if (ret != K_SUCCESS)
        {
            printf("dma get frame error\r\n");
        }

        start_time = get_ticks() / 27000;
        pmsg = (datafifo_msg *)image_send_buf;
        pmsg->msg_type = MSG_KEY_TYPE;
        pmsg->dev_num = rgb_dev;
        pmsg->result_type = DPU_RESULT_TYPE_IMG;
        pmsg->upfunc = (k_u8 *)dpu_callback_attr[rgb_dev].pfn_data_cb;
        pmsg->puserdata = dpu_callback_attr[rgb_dev].p_private_data;

        rgb_send_done = K_FALSE;
        memcpy(&pmsg->dpu_result.img_result, &dma_get_info, sizeof(k_video_frame));
        pmsg->dpu_result.img_result.time_ref = dump_count;
        pmsg->dpu_result.img_result.pts = rgb_buf[rgb_rp].v_frame.pts;
        dataret = -1;
        pmsg->temperature = g_cur_temperature;
        dataret = send_dpu_data_to_little(rgb_dev, image_send_buf);
        // printf("send depth and rgb: delta pts %6ld, time %6ld ms\n", delta, get_ticks()/27000);
        if (dataret)
        {
            printf("send img error: %d\n", dataret);
        }
        while (!rgb_send_done)
        {
            callwriteNULLtoflushdpu(rgb_dev);
            usleep(1000);
        }
        // printf("send rgb take time %6ld ms\n", (get_ticks()/27000-start_time));
        kd_mpi_dma_release_frame(1, &dma_get_info);

        release_image();

        dump_count++;
        if(dump_count % 30 == 0)
        {
            printf("dump_count %d, Average FrameRate = %ld Fps\n", dump_count, (dump_count*1000)/(get_ticks()/27000-dump_start_time));
        }
    }
}

static void *dump_thread(void *arg)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;

    while (!exiting)
    {
        if (dpu_bind == DPU_BIND)
        {
            dpu_bind_dump();
        }
        else
        {
            dpu_unbind_dump();
        }
        usleep(10000);
    }
    exiting = K_FALSE;
    return arg;
}

static void *dump_rgb_thread(void *arg)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;

    while (!image_exiting)
    {
        if (image_mode < IMAGE_MODE_NONE_SPECKLE)
        {
            image_dump();
        }
        else
            usleep(100000);
    }

    while (rgb_wp != rgb_rp)
    {
        release_image();
    }

    image_exiting = K_FALSE;
    return arg;
}

static void *dump_speckle_thread(void *arg)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;

    while (!speckle_exiting)
    {
        if(dpu_bind == DPU_UNBIND)
            speckle_dump();
        else
            depth_dump();
    }

    while (speckle_wp != speckle_rp)
    {
        release_speckle();
    }

    while(depth_wp != depth_rp)
    {
        kd_mpi_dpu_release_frame();
        depth_rp++;
        depth_rp %= VICAP_OUTPUT_BUF_NUM;
    }

    speckle_exiting = K_FALSE;
    return arg;
}

static void *adc_thread(void *arg)
{
    printf("%s\n", __FUNCTION__);
    float temp = -300.0; // 初始变量建议定义到正常范围之外，如果没有正常赋值，不会影响计算。
    float old_temp = -300.0;
    // char input = 'a';
    while (!exiting)
    {
        if (!sample_adc(&temp))
        {
            // printf("%f\n", temp - old_temp);
            if (temp - old_temp > 5 || temp - old_temp < -5)
            {
                sample_dv_dpu_update_temp(temp);
                old_temp = temp;
            }
        }
        g_cur_temperature = temp;

        fd_set rfds;
        struct timeval tv;
        int retval;

        FD_ZERO(&rfds);
        FD_SET(0, &rfds); // 监视标准输入流

#if 0
		tv.tv_sec = 1; // 设置等待时间为5秒
		tv.tv_usec = 0;
#else
        tv.tv_sec = 0; // wait 500 ms
        tv.tv_usec = 500 * 1000;
#endif

        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
        {
            perror("select()");
        }
        else if (retval > 0)
        {
            FD_ISSET(0, &rfds);
        }
        else
        {
            // printf("Timeout reached\n");
        }

        // k_u32 display_ms = 1000;//get temperature per 1s
        // usleep(1000 * display_ms);
    }
    printf("adc exit\n");

    return arg;
}

static k_s32 sample_dv_bind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn dma_mpp_chn;
    k_mpp_chn dpu_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = speckle_dev;
    vi_mpp_chn.chn_id = speckle_chn;
    dma_mpp_chn.mod_id = K_ID_DMA;
    dma_mpp_chn.dev_id = 0;
    dma_mpp_chn.chn_id = 0;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 0;

    ret = kd_mpi_sys_bind(&vi_mpp_chn, &dma_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_bind(&dma_mpp_chn, &dpu_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_bind(&dma_mpp_chn, &dpu_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);
    return ret;
}

static k_s32 sample_dv_unbind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn dma_mpp_chn;
    k_mpp_chn dpu_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = speckle_dev;
    vi_mpp_chn.chn_id = speckle_chn;
    dma_mpp_chn.mod_id = K_ID_DMA;
    dma_mpp_chn.dev_id = 0;
    dma_mpp_chn.chn_id = 0;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 0;

    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &dma_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_unbind(&dma_mpp_chn, &dpu_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_unbind(&dma_mpp_chn, &dpu_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    return ret;
}

k_s32 kd_mapi_dpu_init(k_dpu_info_t *init)
{
    k_s32 ret = 0;

    ir_frame.v_frame.phys_addr[0] = init->ir_phys_addr;
    adc_en = init->adc_en;
    rgb_dev = init->rgb_dev;
    rgb_chn = init->rgb_chn;
    speckle_dev = init->speckle_dev;
    speckle_chn = init->speckle_chn;
    width = init->width;
    height = init->height;
    dma_buf_cnt = init->dma_buf_cnt;
    dpu_buf_cnt = init->dpu_buf_cnt;
    image_mode = init->mode;
    delay_us = init->delay_ms * 1000;
    dpu_bind = init->dpu_bind;
    memcpy(&temperature, &init->temperature, sizeof(k_dpu_temperature_t));

    ir_buf_size = ((width * height * 3 / 2 + 0x3ff) & ~0x3ff);
    ir_frame.v_frame.virt_addr[0] = (k_u64)kd_mpi_sys_mmap_cached(ir_frame.v_frame.phys_addr[0], ir_buf_size);
    if (ir_frame.v_frame.virt_addr[0] == 0)
    {
        printf("%s>mmap error\n", __func__);
        return -1;
    }

    if(dpu_bind == DPU_BIND)
    {
        if((image_mode != IMAGE_MODE_RGB_DEPTH) && (image_mode != IMAGE_MODE_NONE_DEPTH))
        {
            printf("%s>dpu_bind mode error: image_mode %d\n", __func__, image_mode);
            return -1;
        }
    }

    printf("rgb_dev %d, rgb_chn %d, speckle_dev %d, speckle_chn %d\n", rgb_dev, rgb_chn, speckle_dev, speckle_chn);
    printf("adc_en %d, dpu_bind %d, image_mode %d, delay_ms %d\n", adc_en, dpu_bind, image_mode, init->delay_ms);
    printf("dpu_buf_cnt %d, dma_buf_cnt %d\n", dpu_buf_cnt, dma_buf_cnt);

    if (dpu_bind == DPU_BIND)
    {
        sample_dv_bind();
    }

    ret = sample_dv_dpu_init();
    if (ret)
    {
        printf("sample_dv_dpu_init failed\n");
        return -1;
    }
    printf("sample_dv_dpu_init ok\n");

    ret = sample_dv_dma_init();
    if (ret)
    {
        printf("sample_dma_init failed\n");
        return -1;
    }
    printf("sample_dma_init ok\n");

    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_start_grab()
{
    printf("%s\n", __FUNCTION__);

    kd_mpi_vicap_set_dump_reserved(rgb_dev, rgb_chn, K_TRUE);
    kd_mpi_vicap_set_dump_reserved(speckle_dev, speckle_chn, K_TRUE);

    pthread_create(&image_tid, NULL, dump_rgb_thread, NULL);
    pthread_create(&speckle_tid, NULL, dump_speckle_thread, NULL);
    pthread_create(&output_tid, NULL, dump_thread, NULL);

    // struct sched_param param;
    // param.sched_priority = 3;
    // pthread_setschedparam(image_tid, SCHED_FIFO, &param);

    if (adc_en)
    {
        pthread_create(&adc_tid, NULL, adc_thread, NULL);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_stop_grab()
{
    k_s32 ret;

    exiting = K_TRUE;
    image_exiting = K_TRUE;

    if (output_tid)
    {
        while (exiting)
        {
            usleep(10000);
        }
        pthread_cancel(output_tid);
        pthread_join(output_tid, NULL);
    }

    if (image_tid)
    {
        while (image_exiting)
        {
            usleep(10000);
        }
        pthread_cancel(image_tid);
        pthread_join(image_tid, NULL);
    }

    if (speckle_tid)
    {
        while (speckle_exiting)
        {
            usleep(10000);
        }
        pthread_cancel(speckle_tid);
        pthread_join(speckle_tid, NULL);
    }

    if (adc_tid)
    {
        pthread_cancel(adc_tid);
        pthread_join(adc_tid, NULL);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_close()
{
    int ret = 0;

    ret = sample_dv_dma_delete();
    if (ret)
    {
        printf("sample_dma_delete failed\n");
        return 0;
    }

    ret = sample_dv_dpu_delete();
    if (ret)
    {
        printf("sample_dpu_delete failed\n");
        return 0;
    }

    kd_mpi_sys_munmap((void *)ir_frame.v_frame.virt_addr[0], ir_buf_size);

    if (dpu_bind == DPU_BIND)
    {
        sample_dv_unbind();
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_registercallback(k_u32 dev_num, kd_dpu_callback_s *pst_dpu_cb)
{
    printf("%s>dev_num %d\n", __func__, dev_num);
    if (dev_num >= VICAP_DEV_ID_MAX)
    {
        mapi_dpu_error_trace("dpu chn %d error\n", dev_num);
        return -1;
    }
    CHECK_MAPI_DPU_NULL_PTR("pst_dpu_cb", pst_dpu_cb);

    dpu_callback_attr[dev_num].pfn_data_cb = pst_dpu_cb->pfn_data_cb;
    dpu_callback_attr[dev_num].p_private_data = pst_dpu_cb->p_private_data;

    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_unregistercallback(k_u32 dev_num, kd_dpu_callback_s *pst_dpu_cb)
{
    printf("%s>dev_num %d\n", __func__, dev_num);
    if (dev_num >= VICAP_DEV_ID_MAX)
    {
        mapi_dpu_error_trace("dpu chn %d error\n", dev_num);
        return -1;
    }
    CHECK_MAPI_DPU_NULL_PTR("pst_dpu_cb", pst_dpu_cb);

    dpu_callback_attr[dev_num].pfn_data_cb = NULL;
    dpu_callback_attr[dev_num].p_private_data = 0;

    return K_SUCCESS;
}
