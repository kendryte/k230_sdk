/**
 * @file msg_dpu.h
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

#ifndef __MSG_DPU_H__
#define __MSG_DPU_H__

#include "k_type.h"
#include "k_dpu_comm.h"
#include "mpi_dpu_api.h"
#include "mpi_sys_api.h"
#include "mapi_dpu_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum
{
    MSG_CMD_MEDIA_DPU_INIT,
    MSG_CMD_MEDIA_DPU_DELETE,
    MSG_CMD_MEDIA_DPU_PARSE_FILE,
    MSG_CMD_MEDIA_DPU_SET_DEV_ATTR,
    MSG_CMD_MEDIA_DPU_GET_DEV_ATTR,
    MSG_CMD_MEDIA_DPU_SET_REF_IMAGE,
    MSG_CMD_MEDIA_DPU_SET_PROCESSED_REF_IMAGE,
    MSG_CMD_MEDIA_DPU_SET_TEMPLATE_IMAGE,
    MSG_CMD_MEDIA_DPU_START_DEV,
    MSG_CMD_MEDIA_DPU_SET_CHN_ATTR,
    MSG_CMD_MEDIA_DPU_GET_CHN_ATTR,
    MSG_CMD_MEDIA_DPU_START_CHN,
    MSG_CMD_MEDIA_DPU_STOP_CHN,
    MSG_CMD_MEDIA_DPU_SEND_FRAME,
    MSG_CMD_MEDIA_DPU_GET_FRAME,
    MSG_CMD_MEDIA_DPU_RELEASE_FRAME,
    
    MSG_CMD_MEDIA_DPU_REGISTER_CALLBACK,
    MSG_CMD_MEDIA_DPU_UNREGISTER_CALLBACK,
    MSG_CMD_MEDIA_DPU_INIT_DATAFIFO,
    MSG_CMD_MEDIA_DPU_DELETE_DATAFIFO,
    MSG_CMD_MEDIA_DPU_START_GRAB,
    MSG_CMD_MEDIA_DPU_STOP_GRAB,
    MSG_CMD_MEDIA_DPU_CLOSE,
} msg_media_dpu_cmd_t;

typedef struct
{
    k_s32 chn_cnt       :2;
    char* param_path;
    char* ref_path;

    // k_dpu_info_t init;

} msg_dpu_attr_t;

typedef struct
{
    k_u32 dpu_chn;
    kd_dpu_callback_s callback_attr;
} msg_dpu_callback_attr;

typedef struct
{
    k_u32 fifo_chn;
    k_u64 phyAddr;
} msg_dpu_fifo_t;


/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_DPU_H__ */
