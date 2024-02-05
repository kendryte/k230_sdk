/**
 * @file mapi_nonai_2d_api.h
 * @author  ()
 * @brief mapi of nonai_2d module
 * @version 1.0
 * @date 2023-03-22
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
#ifndef __MAPI_NONAI_2D_H__
#define __MAPI_NONAI_2D_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_nonai_2d_api.h"
#include "k_nonai_2d_comm.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_NONAI_2D*/
/** @{ */  /** <!-- [ MAPI_NONAI_2D] */

/**
 * @brief Create nonai_2d channel
 *
 * @param [in] chn_num  channel number
 * @param [in] attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note MJPEG rc attributes are different from h264 and h265
 *
 */

k_s32 kd_mapi_nonai_2d_init(k_u32 chn_num, k_nonai_2d_chn_attr *attr);

/**
 * @brief Destory channel
 *
 * @param [in] chn_num channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create/stop channel
 */
k_s32 kd_mapi_nonai_2d_deinit(k_u32 chn_num);


/**
 * @brief Start channel
 *
 * @param [in] chn_num channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create channel
 */
k_s32 kd_mapi_nonai_2d_start(k_u32 chn_num);

/**
 * @brief Stop channel
 *
 * @param [in] chn_num  channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel
 */
k_s32 kd_mapi_nonai_2d_stop(k_u32 chn_num);



/** @} */ /** <!-- ==== MAPI_NONAI_2D End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_NONAI_2D_API_H__ */

