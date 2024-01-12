/**
 * @file mapi_dpu_api.h
 * @author  ()
 * @brief mapi of dpu module
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
 */
#ifndef __MAPI_DPU_H__
#define __MAPI_DPU_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_dpu_api.h"
#include "k_dpu_comm.h"
#include "mapi_dpu_comm.h"

#define K_MAPI_ERR_DPU_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_DPU, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_DPU_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_DPU, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_DPU_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_DPU_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_DPU_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_DPU, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_DPU_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_DPU*/
/** @{ */  /** <!-- [ MAPI_DPU] */

/**
 * @brief Initialize the DPU
 * @param [in] init
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mapi_dpu_init(k_dpu_info_t *init);

/**
 * @brief Delete the DPU
 * @param [in] dev_cnt Number of device count
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mapi_dpu_close();

k_s32 kd_mapi_dpu_start_grab();
k_s32 kd_mapi_dpu_stop_grab();

k_s32 dpu_internuclear_fifo_create(k_u32 chn_num);
k_s32 dpu_internuclear_fifo_delete(k_u32 chn_num);
k_s32 kd_mapi_dpu_registercallback(k_u32 dev_num, kd_dpu_callback_s *pst_dpu_cb);
k_s32 kd_mapi_dpu_unregistercallback(k_u32 dev_num, kd_dpu_callback_s *pst_dpu_cb);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_DPU_API_H__ */