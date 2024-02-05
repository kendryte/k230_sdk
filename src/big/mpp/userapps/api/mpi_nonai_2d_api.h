/**
 * @file mpi_nonai_2d_api.h
 * @author
 * @brief Defines APIs related to virtual video input device
 * @version 1.0
 * @date 2022-09-22
 *
 * @copyright
 * Copyright (c), Canaan Bright Sight Co., Ltd
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
#ifndef __MPI_NONAI_2D_API_H__
#define __MPI_NONAI_2D_API_H__

#include "k_type.h"
#include "k_nonai_2d_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     NONAI_2D */
/** @{ */ /** <!-- [NONAI_2D] */

/**
 * @brief Create encode channel
 *
 * @param [in] chn_num  channel number
 * @param [in] attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_nonai_2d_create_chn(k_u32 chn_num, k_nonai_2d_chn_attr *attr);

/**
 * @brief Start channel
 *
 * @param [in] dev_num
 * @param [in] attr
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create channel
 */
k_s32 kd_mpi_nonai_2d_start_chn(k_u32 chn_num);

/**
 * @brief Stop channel
 *
 * @param [in] dev_num
 * @param [in] attr
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel
 */
k_s32 kd_mpi_nonai_2d_stop_chn(k_u32 chn_num);

/**
 * @brief Destory channel
 *
 * @param [in] dev_num
 * @param [in] attr
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create/stop channel
 */
k_s32 kd_mpi_nonai_2d_destroy_chn(k_u32 chn_num);

/**
 * @brief Send frame to nonai_2d
 *
 * @param [in] chn_num Channel number
 * @param [in] stream Input frame
 * @param [in] milli_sec wait time, -1 means wait forever
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel.
 */
k_s32 kd_mpi_nonai_2d_send_frame(k_u32 chn_num, k_video_frame_info *frame, k_s32 milli_sec);

/**
 * @brief Get stream from nonai_2d
 *
 * @param [in] chn_num Channel number
 * @param [in] stream Output stream
 * @param [in] milli_sec wait time, -1 means wait forever
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel. Stream type can be frame or header.
 */
k_s32 kd_mpi_nonai_2d_get_frame(k_u32 chn_num, k_video_frame_info *frame, k_s32 milli_sec);

/**
 * @brief Release encoded stream
 *
 * @param [in] chn_num Channel number
 * @param [in] frame Output stream
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after get stream
 */
k_s32 kd_mpi_nonai_2d_release_frame(k_u32 chn_num, k_video_frame_info *frame);

/**
 * @brief close nonai_2d handle
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note close nonai_2d device
 *
 */
k_s32 kd_mpi_nonai_2d_close();

/** @} */ /** <!-- ==== NONAI_2D End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
