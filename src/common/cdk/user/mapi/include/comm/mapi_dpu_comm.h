/**
 * @file mapi_dpu_comm.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-09-19
 *
 * @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
 *
 */
#ifndef __MAPI_DPU_COMM_H__
#define __MAPI_DPU_COMM_H__

#include "mapi_log.h"
#include "k_dpu_comm.h"
#include "k_vo_comm.h"
#include "k_vicap_comm.h"
#include "k_venc_comm.h"

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define mapi_dpu_emerg_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_FATAL, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define mapi_dpu_alter_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_ALERT, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define mapi_dpu_crit_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_CRIT, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define mapi_dpu_error_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_ERROR, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define mapi_dpu_warn_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_WARNING, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define mapi_dpu_notic_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_NOTICE, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define mapi_dpu_debug_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_DEBUG, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define mapi_dpu_info_trace(fmt, ...)                                                                                                 \
        do {                                                                                                                           \
            kendyrte_mapi_log_printf(K_MAPI_MOD_DPU, K_ERR_LEVEL_INFO, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define DPU_RET_MPI_TO_MAPI(ret) (((ret) & 0x0000FFFF) | (K_MAPI_ERR_APPID | ((K_MAPI_MOD_DPU) << 16 )))

typedef union
{
    k_video_frame img_result; /**< Result of rgb */
    struct{
        k_dpu_chn_lcn_result_t lcn_result;  /**< Result of the speckle channel. */;
        void * lcn_virt_addr;
    };
    k_venc_stream stream;
} k_dpu_result_u;

typedef enum
{
    DPU_RESULT_TYPE_IMG,
    DPU_RESULT_TYPE_LCN,
    DPU_RESULT_TYPE_BUTT
} k_dpu_result_type;

typedef enum
{
    IMAGE_MODE_RGB_DEPTH=0,  /*sensor 0: RGB,  sensor 1: speckle*/
    IMAGE_MODE_RGB_IR,       /*sensor 0: RGB,  sensor 1: IR*/
    IMAGE_MODE_IR_DEPTH,     /*sensor 0: IR,   sensor 1: speckle*/
    IMAGE_MODE_NONE_SPECKLE, /*sensor 0: none, sensor 1: speckle*/
    IMAGE_MODE_NONE_IR,      /*sensor 0: none, sensor 1: IR*/
    IMAGE_MODE_NONE_DEPTH,   /*sensor 0: none, sensor 1: speckle*/
    IMAGE_MODE_BUTT,
} k_dpu_image_mode;

typedef struct kd_datafifo_msg
{
    k_u16 msg_type;
    k_u16 dev_num;
    k_dpu_result_type result_type;
    k_u8 *upfunc;
    k_u8 *puserdata;
    k_dpu_result_u dpu_result;
    float temperature;
}datafifo_msg;

typedef struct
{
    float ref;
    float cx;
    float cy;
    float kx;
    float ky;
}k_dpu_temperature_t;

typedef struct
{
    k_u32 rgb_dev;
    k_u32 rgb_chn;
    k_u32 speckle_dev;
    k_u32 speckle_chn;
    k_u32 dev_cnt;
    k_u64 ir_phys_addr;
    k_u32 width;
    k_u32 height;
    k_u32 dpu_buf_cnt;
    k_u32 dma_buf_cnt;
    k_bool adc_en;
    k_dpu_temperature_t temperature;
    k_dpu_image_mode mode;
    k_u32 delay_ms;   /*3d_mode_crtl delay time*/
    k_dpu_mode_e dpu_bind;
} k_dpu_info_t;

typedef struct
{
    k_dpu_result_u dpu_result;
    float temperature;
}kd_dpu_data_s;

typedef k_s32 (*pfn_dpu_dataproc)(k_u32 dev_num, kd_dpu_data_s *p_dpu_data, k_u8 *p_private_data);

typedef struct
{
    pfn_dpu_dataproc pfn_data_cb;
    k_u8 *p_private_data;
}kd_dpu_callback_s;

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif
