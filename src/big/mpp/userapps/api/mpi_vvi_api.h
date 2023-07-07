/**
 * @file mpi_vvi_api.h
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
#ifndef __MPI_VVI_API_H__
#define __MPI_VVI_API_H__

#include "k_type.h"
#include "k_vvi_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VVI */
/** @{ */ /** <!-- [VVI] */

/**
 * @brief Set the channel attributes of the virtual VI
 *
 * @param [in] chn_num  channel number
 * @param [in] attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note After the pipe to which the channel belongs is started, the channel attributes cannot be set
 *
 */
k_s32 kd_mpi_vvi_set_chn_attr(k_u32 chn_num, k_vvi_chn_attr *attr);

/**
 * @brief Set the device attributes of the virtual VI
 *
 * @param [in] dev_num
 * @param [in] attr
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_vvi_set_dev_attr(k_u32 dev_num, k_vvi_dev_attr *attr);

/**
 * @brief Start a pipe
 *
 * @param [in] dev_num Device number of pipe
 * @param [in] chn_num Channel number of pipe
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the device number or channel number passed in already belongs to a started pipe,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_vvi_start_pipe(k_u32 dev_num, k_u32 chn_num);

/**
 * @brief Stop a pipe
 *
 * @param [in] dev_num Device number of pipe
 * @param [in] chn_num Channel number of pipe
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the device number or channel number passed in no longer
 *   belongs to any of the started pipes, the mpi will return a failure
 * - For a closed pipe, calling this mpi returns success
 */
k_s32 kd_mpi_vvi_stop_pipe(k_u32 dev_num, k_u32 chn_num);

/**
 * @brief Insert a frame into the channel of the virtual VI
 *
 * @param [in] chn_num channel number
 * @param [in] vf_info Information about the video frame to be inserted
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @see kd_mpi_vvi_chn_remove_pic
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - The physical address of the video frame should come from a vb block
 * - The size of the video frame should not exceed the size of the vb block
 * - After inserting a video frame, all frames obtained in the virtual VI
 *   interrupt are discarded until inserted frame is removed
 */
k_s32 kd_mpi_vvi_chn_insert_pic(k_u32 chn_num, k_video_frame_info *vf_info);

/**
 * @brief Remove the video frames inserted into the virtual VI channel
 *
 * @param [in] chn_num channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @see kd_mpi_vvi_chn_insert_pic
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - The mpi returns success if the inserted frame for this channel has been removed or has not yet been inserted
 */
k_s32 kd_mpi_vvi_chn_remove_pic(k_u32 chn_num);

/**
 * @brief Make a request to dump video frames to the virtual VI channel
 *
 * @param [in] chn_num channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - If the request fails, you need to call ::kd_mpi_vvi_chn_dump_release first to release the frame
 */
k_s32 kd_mpi_vvi_chn_dump_request(k_u32 chn_num);

/**
 * @brief Dump a video frame from the channel of the virtual VI
 *
 * @param [in] chn_num channel number
 * @param [out] vf_info Video frame information obtained
 * @param [in] timeout
 * - -1 indicates blocking mode.
 * - 0 indicates non-blocking mode.
 * - Greater than 0 indicates timeout mode, and the timeout time is measured in milliseconds(ms).
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - Before calling this function, you must first call ::kd_mpi_vvi_chn_dump_request
 * - When milli_sec is set to -1, it means blocking mode, and the program waits until
 *   the image is obtained before returning
 * - If milli_sec is greater than 0, it means non-blocking mode. The unit of the parameter
 *   is milliseconds, which refers to the timeout time. If no image is obtained within this time,
 *   the timeout will return
 * - The obtained physical address information comes from the VideoBuffer used inside MPP,
 *   so after using it, you must call the ::kd_mpi_vvi_chn_dump_release interface to release its memory
 */
k_s32 kd_mpi_vvi_chn_dump_frame(k_u32 chn_num, k_video_frame_info *vf_info, k_u32 milli_sec);

/**
 * @brief Release dumped video frame
 *
 * @param [in] chn_num channel number
 * @param [in] vf_info Video frame information obtained by ::kd_mpi_vvi_chn_dump_frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - If ::kd_mpi_vvi_chn_dump_request has not been called before, ,mpi will return failure
 * - Only the video frame obtained by ::kd_mpi_vvi_chn_dump_frame can be released
 */
k_s32 kd_mpi_vvi_chn_dump_release(k_u32 chn_num, const k_video_frame_info *vf_info);

/** @} */ /** <!-- ==== VVI End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
