/**
 * @file mapi_vvi_api.h
 * @author  ()
 * @brief mapi of virtual video inputdeo input module
 * @version 1.0
 * @date 2022-10-18
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
#ifndef __MAPI_VVI_H__
#define __MAPI_VVI_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_vvi_api.h"
#include "k_vvi_comm.h"

#define K_MAPI_ERR_VVI_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_VVI, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_VVI_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_VVI, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_VVI_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_VVI, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_VVI_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_VVI, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_VVI_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_VVI, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_VVI_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_VVI, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_VVI*/
/** @{ */  /** <!-- [ MAPI_VVI] */

/**
 * @brief Start a virtual video input PIPE unit.
 *
 * @param [in] dev Virtual vi device id
 * @param [in] chn Virtual vi channel id
 * @param [in] dev_attr Virtual vi dev attribute
 * @param [in] chn_attr Virtual vi chn attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note If the device number or channel number passed in already
 *       belongs to a started pipe, the mpi will return a failure
 */
k_s32 kd_mapi_vvi_start_pipe(k_u32 dev, k_u32 chn, k_vvi_dev_attr *dev_attr, k_vvi_chn_attr *chn_attr);

/**
 * @brief Stop a virtual video input PIPE unit.
 *
 * @param [in] dev Virtual vi device id
 * @param [in] chn Virtual vi channel id
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the device number or channel number passed in no longer belongs to any of the started pipes,
 *   the mpi will return a failure
 * - For a closed pipe, calling this mpi returns success
 */
k_s32 kd_mapi_vvi_stop_pipe(k_u32 dev, k_u32 chn);

/**
 * @brief Insert a frame into virtual video input
 *
 * @param [in] dev Virtual vi device id
 * @param [in] chn Virtual vi channel id
 * @param [in] vf_info Information about the frame to insert
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - The physical address of the video frame should come from a vb block
 * - The size of the video frame should not exceed the size of the vb block
 * - After inserting a video frame, all frames obtained in the virtual VI interrupt are discarded until inserted frame is removed
 */
k_s32 kd_mapi_vvi_insert_pic(k_u32 dev, k_u32 chn, k_video_frame_info *vf_info);

/**
 * @brief Remove the video frames inserted into the virtual VI
 *
 * @param [in] dev Virtual vi device id
 * @param [in] chn Virtual vi channel id
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - The mapi returns success if the inserted frame for this channel has been removed or has not yet been inserted
 */
k_s32 kd_mapi_vvi_remove_pic(k_u32 dev, k_u32 chn);

/**
 * @brief Dump a video frame from the channel of the virtual VI
 *
 * @param [in] dev Virtual vi device id
 * @param [in] chn Virtual vi channel id
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
 * - Before calling ::kd_mapi_vvi_release_frame, calling this mapi repeatedly will get the same video frame
 */
k_s32 kd_mapi_vvi_dump_frame(k_u32 dev, k_u32 chn, k_video_frame_info *vf_info, k_s32 milli_sec);

/**
 * @brief Release dumped video frame
 *
 * @param [in] dev Virtual vi device id
 * @param [in] chn Virtual vi channel id
 * @param [in] vf_info Video frame information obtained by ::kd_mapi_vvi_dump_frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - Only the video frame obtained by ::kd_mapi_vvi_dump_frame can be released
 */
k_s32 kd_mapi_vvi_release_frame(k_u32 dev, k_u32 chn, const k_video_frame_info *vf_info);

/**
 * @brief bind vvi to vvo
 *
 * @param [in] vvi_dev  virtual video input device id
 * @param [in] vvi_chn  virtual video input channel id
 * @param [in] vvo_dev  virtual video output channel id
 * @param [in] vvo_chn  virtual video output channel id
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - VVI must be initialized first
 * - Repeatedly binding the same input source to the same VVO returns success,
 *   repeatedly binding different input sources to the same VVO return error code
 */
k_s32 kd_mapi_vvi_bind_vvo(k_u32 vvi_dev, k_u32 vvi_chn, k_u32 vvo_dev, k_u32 vvo_chn);

/**
 * @brief unbind vvi to vvo
 *
 * @param [in] vvi_dev  virtual video input device id
 * @param [in] vvi_chn  virtual video input channel id
 * @param [in] vvo_dev  virtual video output channel id
 * @param [in] vvo_chn  virtual video output channel id
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note Repeat unbundling returns success
 */
k_s32 kd_mapi_vvi_unbind_vvo(k_u32 vvi_dev, k_u32 vvi_chn, k_u32 vvo_dev, k_u32 vvo_chn);


/** @} */ /** <!-- ==== MAPI_VVI End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_VVI_API_H__ */

