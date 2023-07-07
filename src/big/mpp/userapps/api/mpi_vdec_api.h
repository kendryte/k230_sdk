/**
 * @file mpi_vdec_api.h
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
#ifndef __MPI_VDEC_API_H__
#define __MPI_VDEC_API_H__

#include "k_type.h"
#include "k_vdec_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VDEC */
/** @{ */ /** <!-- [VDEC] */

/**
 * @brief Create a decode channel
 *
 * @param [in] chn_num  channel number
 * @param [in] attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note none
 *
 */
k_s32 kd_mpi_vdec_create_chn(k_u32 chn_num, k_vdec_chn_attr *attr);

/**
 * @brief Start a decode channel
 *
 * @param [in] chn_num Channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create channel
 */
k_s32 kd_mpi_vdec_start_chn(k_u32 chn_num);

/**
 * @brief Stop a decode channel
 *
 * @param [in] chn_num Channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel
 */
k_s32 kd_mpi_vdec_stop_chn(k_u32 chn_num);

/**
 * @brief Destroy a decode channel
 *
 * @param [in] chn_num Channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create/stop channel
 */
k_s32 kd_mpi_vdec_destroy_chn(k_u32 chn_num);


/**
 * @brief Send input stream
 *
 * @param [in] chn_num Channel number
 * @param [in] stream Input stream
 * @param [in] milli_sec wait time, -1 means wait forever
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel.
 */
k_s32 kd_mpi_vdec_send_stream(k_u32 chn_num, k_vdec_stream *stream, k_s32 milli_sec);

/**
 * @brief Get decoded frame
 *
 * @param [in] chn_num Channel number
 * @param [in] frame Output frame
 * @param [in] supplement Some extra information
 * @param [in] milli_sec wait time, -1 means wait forever
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel.
 */
k_s32 kd_mpi_vdec_get_frame(k_u32 chn_num, k_video_frame_info *frame, k_vdec_supplement_info *supplement, k_s32 milli_sec);

/**
 * @brief Release decoded frame
 *
 * @param [in] chn_num Channel number
 * @param [in] frame Output frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after get frame
 */
k_s32 kd_mpi_vdec_release_frame(k_u32 chn_num, k_video_frame_info *frame);

/**
 * @brief Query decode status
 *
 * @param [in] chn_num Channel number
 * @param [in] status Decode channel status
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create channel
 */
k_s32 kd_mpi_vdec_query_status(k_u32 chn_num, k_vdec_chn_status *status);

/**
 * @brief close vdec handle
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note close vdec device
 *
 */
k_s32 kd_mpi_vdec_close_fd();

/**
 * @brief set rotation
 *
 * @param [in] chn_num Channel number
 * @param [in] rotation rotation parameter
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It can be set on the fly
 */
k_s32 kd_mpi_vdec_set_rotation(k_u32 chn_num, k_rotation rotation);

k_s32 kd_mpi_vdec_set_downscale(k_u32 chn_num, const k_vdec_downscale *downscale);

/** @} */ /** <!-- ==== VDEC End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

