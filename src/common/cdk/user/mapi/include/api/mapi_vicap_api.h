/**
 * @file mapi_vicap_api.h
 * @author  ()
 * @brief mapi of virtual video inputdeo input module
 * @version 1.0
 * @date 2023-04-11
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
#ifndef __MAPI_VICAP_H__
#define __MAPI_VICAP_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_vicap_api.h"
#include "k_vicap_comm.h"

#define K_MAPI_ERR_VICAP_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_VICAP_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_VICAP_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_VICAP_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_VICAP_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_VICAP_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_MAPI_ERR_VICAP_INVALID_DEVID    K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_MAPI_ERR_VICAP_INVALID_CHNID    K_MAPI_DEF_ERR(K_MAPI_MOD_VICAP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_VICAP*/
/** @{ */  /** <!-- [ MAPI_VICAP] */

/**
 * @brief Dump a video frame from the channel of the VICAP
 *
 * @param [in] sensor_attr sensor_attr.dev_num VICAP device id
 * @param [out] sensor_fd sensor_attr.sensor_fd VICAP open sensor fd
 * - -1 indicates blocking mode.
 * - 0 indicates non-blocking mode.
 * - Greater than 0 indicates timeout mode, and the timeout time is measured in milliseconds(ms).
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - Before calling ::kd_mapi_vicap_release_frame, calling this mapi repeatedly will get the same video frame
 */
k_s32 kd_mapi_vicap_get_sensor_fd(k_vicap_sensor_attr *sensor_attr);

/**
 * @brief Dump a video frame from the channel of the VICAP
 *
 * @param [in] dev vicap device id
 * @param [in] chn vicap channel id
 * @param [in] foramt dump format
 * @param [out] vf_info Video frame information obtained
 * @param [in] milli_sec
 * - -1 indicates blocking mode.
 * - 0 indicates non-blocking mode.
 * - Greater than 0 indicates timeout mode, and the timeout time is measured in milliseconds(ms).
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - Before calling ::kd_mapi_vicap_release_frame, calling this mapi repeatedly will get the same video frame
 */
k_s32 kd_mapi_vicap_dump_frame(k_vicap_dev dev_num, k_vicap_chn chn_num, k_vicap_dump_format foramt, k_video_frame_info *vf_info, k_u32 milli_sec);

/**
 * @brief Release dumped video frame
 *
 * @param [in] dev vicap device id
 * @param [in] chn vicap channel id
 * @param [in] vf_info Video frame information obtained by ::kd_mapi_vicap_dump_frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - Only the video frame obtained by ::kd_mapi_vicap_dump_frame can be released
 */
k_s32 kd_mapi_vicap_release_frame(k_vicap_dev dev_num, k_vicap_chn chn_num, const k_video_frame_info *vf_info);

/**
 * @brief Vicap get sensor info
 *
 * @param [in] sensor_info vicap sensor info
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - sensor_info->sensor_type need given before use this function
 */
k_s32 kd_mapi_vicap_get_sensor_info(k_vicap_sensor_info *sensor_info);

/**
 * @brief Vicap set dev attr
 *
 * @param [in] dev_info vicap dev attr info
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 */
k_s32 kd_mapi_vicap_set_dev_attr(k_vicap_dev_set_info dev_info);

/**
 * @brief Vicap set chn attr
 *
 * @param [in] chn_info vicap chn attr info
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - Need use after kd_mapi_vicap_set_dev_attr
 */
k_s32 kd_mapi_vicap_set_chn_attr(k_vicap_chn_set_info chn_info);

/**
 * @brief Vicap init and start stream
 *
 * @param [in] vicap_dev vicap device id
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 */
k_s32 kd_mapi_vicap_start(k_vicap_dev vicap_dev);

/**
 * @brief Vicap deinit and stop stream
 *
 * @param [in] vicap_dev vicap device id
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 */
k_s32 kd_mapi_vicap_stop(k_vicap_dev vicap_dev);

/**
 * @brief vicap set vi drop frame
 *
 * @param [in] csi csi num
 * @param [in] k_vicap_drop_frame Description of lost frame information
 * @param [in] enable enable drop frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mapi_vicap_set_vi_drop_frame(k_vicap_csi_num csi, k_vicap_drop_frame *frame, k_bool enable);

/**
 * @brief vicap set mclk
 *
 * @param [in] id mclk id
 * @param [in] sel pll clk div select
 * @param [in] mclk_div mclk div
 * @param [in] mclk_div mclk enable
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */

k_s32 kd_mapi_vicap_set_mclk(k_vicap_mclk_id id, k_vicap_mclk_sel sel, k_u8 mclk_div, k_u8 mclk_en);

/** @} */ /** <!-- ==== MAPI_VICAP End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_VICAP_API_H__ */
