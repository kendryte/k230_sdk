/**
 * @file mpi_venc_api.h
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
#ifndef __MPI_VENC_API_H__
#define __MPI_VENC_API_H__

#include "k_type.h"
#include "k_venc_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VENC */
/** @{ */ /** <!-- [VENC] */

/**
 * @brief Create encode channel
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
k_s32 kd_mpi_venc_create_chn(k_u32 chn_num, k_venc_chn_attr *attr);

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
k_s32 kd_mpi_venc_start_chn(k_u32 chn_num);

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
k_s32 kd_mpi_venc_stop_chn(k_u32 chn_num);

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
k_s32 kd_mpi_venc_destroy_chn(k_u32 chn_num);

/**
 * @brief Send frame to venc
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
k_s32 kd_mpi_venc_send_frame(k_u32 chn_num, k_video_frame_info *frame, k_s32 milli_sec);

/**
 * @brief Get stream from venc
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
k_s32 kd_mpi_venc_get_stream(k_u32 chn_num, k_venc_stream *stream, k_s32 milli_sec);

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
k_s32 kd_mpi_venc_release_stream(k_u32 chn_num, k_venc_stream *stream);

/**
 * @brief Query encode status
 *
 * @param [in] chn_num Channel number
 * @param [in] status Encode channel status
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create channel
 */
k_s32 kd_mpi_venc_query_status(k_u32 chn_num, k_venc_chn_status *status);

/**
 * @brief close venc handle
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note close venc device
 *
 */
k_s32 kd_mpi_venc_close_fd();


/**
 * @brief Set venc rotaion
 *
 * @param [in] chn_num Channel number
 * @param [in] rotation Venc rotation
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_venc_set_rotation(k_u32 chn_num, const k_rotation rotation);

/**
 * @brief Get venc rotaion
 *
 * @param [in] chn_num Channel number
 * @param [in] rotation Pointer of venc rotation
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_venc_get_rotation(k_u32 chn_num, k_rotation *rotation);

/**
 * @brief Set venc mirror
 *
 * @param [in] chn_num Channel number
 * @param [in] mirror Venc mirror
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_venc_set_mirror(k_u32 chn_num, const k_venc_mirror mirror);

/**
 * @brief Get venc rotaion
 *
 * @param [in] chn_num Channel number
 * @param [in] mirror Pointer of venc mirror
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_venc_get_mirror(k_u32 chn_num, k_venc_mirror *mirror);

/**
 * @brief Enable venc IDR frame
 *
 * @param [in] chn_num Channel number
 * @param [in] idr_enable Enable IDR frame or not
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_venc_enable_idr(k_u32 chn_num, k_bool idr_enable);

/**
 * @brief Request instant IDR frame
 *
 * @param [in] chn_num Channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Should be called after create channel, before start channel
 */
k_s32 kd_mpi_venc_request_idr(k_u32 chn_num);

/**
 * @brief Set H.265 Sample Adaptive Offset attribution
 *
 * @param [in] chn_num Channel number
 * @param [in] h265_sao Pointer of SAO attribution
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Should be called after create channel, before start channel
 */
k_s32 kd_mpi_venc_set_h265_sao(k_u32 chn_num, const k_venc_h265_sao *h265_sao);

/**
 * @brief Get H.265 Sample Adaptive Offset attribution
 *
 * @param [in] chn_num Channel number
 * @param [out] h265_sao Pointer of SAO attribution
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_venc_get_h265_sao(k_u32 chn_num, k_venc_h265_sao *h265_sao);

/**
 * @brief Enable H.264/H.265 deblocking
 *
 * @param [in] chn_num Channel number
 * @param [in] dblk_en Enable deblocking or not
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Should be called after create channel, before start channel
 */
k_s32 kd_mpi_venc_set_dblk(k_u32 chn_num, const k_bool dblk_en);

/**
 * @brief Get H.264/H.265 deblocking status
 *
 * @param [in] chn_num Channel number
 * @param [out] dblk_en Pointer of deblocking status
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_venc_get_dblk(k_u32 chn_num, k_bool *dblk_en);

/**
 * @brief Set H.264 entropy parameters
 *
 * @param [in] chn_num Channel number
 * @param [in] h264_entropy Pointer of H.264 entropy structure
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Should be called after create channel, before start channel
 */
k_s32 kd_mpi_venc_set_h264_entropy(k_u32 chn_num, const k_venc_h264_entropy *h264_entropy);

/**
 * @brief Get H.264 entropy parameters
 *
 * @param [in] chn_num Channel number
 * @param [out] h264_entropy Pointer of H.264 entropy structure
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_venc_get_h264_entropy(k_u32 chn_num, k_venc_h264_entropy *h264_entropy);

/**
 * @brief Set H.265 entropy parameters
 *
 * @param [in] chn_num Channel number
 * @param [in] h265_entropy Pointer of H.265 entropy structure
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Should be called after create channel, before start channel
 */
k_s32 kd_mpi_venc_set_h265_entropy(k_u32 chn_num, const k_venc_h265_entropy *h265_entropy);

/**
 * @brief Get H.265 entropy parameters
 *
 * @param [in] chn_num Channel number
 * @param [out] h265_entropy Pointer of H.265 entropy structure
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_venc_get_h265_entropy(k_u32 chn_num, k_venc_h265_entropy *h265_entropy);

/**
 * @brief Start 2d channel
 *
 * @param [in] chn_num Channel number of 2D
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_venc_start_2d_chn(k_u32 chn_num);

/**
 * @brief Stop 2d channel
 *
 * @param [in] chn_num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_stop_2d_chn(k_u32 chn_num);

/**
 * @brief Send frame to 2d
 *
 * @param [in] chn_num Channel number
 * @param [in] frame Input frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_send_2d_frame(k_u32 chn_num, k_video_frame_info *frame);

/**
 * @brief Get frame from 2d
 *
 * @param [in] chn_num Channel number
 * @param [in] frame Output frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_get_2d_frame(k_u32 chn_num, k_video_frame_info *frame);

/**
 * @brief Set calculation mode of 2D
 *
 * @param [in] chn_num Channel number
 * @param [in] mode Pointer of calculation mode
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_set_2d_mode(k_u32 chn_num, k_venc_2d_calc_mode mode);

/**
 * @brief Get calculation mode of 2D
 *
 * @param [in] chn_num Channel number
 * @param [out] mode Pointer of calculation mode
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_get_2d_mode(k_u32 chn_num, k_venc_2d_calc_mode *mode);

/**
 * @brief Set parameters of 2D CSC
 *
 * @param [in] chn_num Channel number
 * @param [in] fmt Pointer of output format
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_set_2d_csc_param(k_u32 chn_num, const k_venc_2d_csc_attr *attr);

/**
 * @brief Get parameters of 2D CSC
 *
 * @param [in] chn_num Channel number
 * @param [out] fmt Pointer of output format
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_get_2d_csc_param(k_u32 chn_num, k_venc_2d_csc_attr *attr);

/**
 * @brief Set parameters of 2D OSD
 *
 * @param [in] chn_num Channel number
 * @param [in] index Index of attribution, should in range [1, K_VENC_MAX_2D_OSD_REGION_NUM)
 * @param [in] attr Pointer to attribute of OSD
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @see K_VENC_MAX_2D_OSD_REGION_NUM
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_set_2d_osd_param(k_u32 chn_num, k_u8 index, const k_venc_2d_osd_attr *attr);

/**
 * @brief Get parameters of 2d OSD
 *
 * @param [in] chn_num Channel number
 * @param [in] index Index of attribution.
 * @param [out] attr Pointer to attribute of 2d, should in range [1, K_VENC_MAX_2D_OSD_REGION_NUM)
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @see K_VENC_MAX_2D_OSD_REGION_NUM
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_get_2d_osd_param(k_u32 chn_num, k_u8 index, k_venc_2d_osd_attr *attr);

/**
 * @brief Set parameters of 2D border
 *
 * @param [in] chn_num Channel number
 * @param [in] index Index of attribution, should in range [1, K_VENC_MAX_2D_BORDER_NUM)
 * @param [in] attr Pointer to attribute of OSD
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @see K_VENC_MAX_2D_BORDER_NUM
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_set_2d_border_param(k_u32 chn_num, k_u8 index, const k_venc_2d_border_attr *attr);

/**
 * @brief Get parameters of 2D border
 *
 * @param [in] chn_num Channel number
 * @param [in] index Index of attribution, should in range [1, K_VENC_MAX_2D_BORDER_NUM)
 * @param [out] attr Pointer to attribute of OSD
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @see K_VENC_MAX_2D_BORDER_NUM
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_get_2d_border_param(k_u32 chn_num, k_u8 index, k_venc_2d_border_attr *attr);

/**
 * @brief Set parameters of 2D border
 *
 * @param [in] chn_num Channel number
 * @param [in] coef Pointer of custom CSC coefficent
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_set_2d_custom_coef(k_u32 chn_num, const k_s16 *coef);

/**
 * @brief Get parameters of 2D border
 *
 * @param [in] chn_num Channel number
 * @param [out] coef Pointer of custom CSC coefficent
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_get_2d_custom_coef(k_u32 chn_num, const k_s16 *coef);
/**
 * @brief Get color gamut of 2D
 *
 * @param [in] chn_num Channel number
 * @param [in] color_gamut Pointer of color gamut enumeration
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_get_2d_color_gamut(k_u32 chn_num, k_venc_2d_color_gamut *color_gamut);

/**
 * @brief Set color gamut of 2D
 *
 * @param [in] chn_num Channel number
 * @param [out] color_gamut Enumeration of color gamut
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not started,
 *       the mpi will return a failure
 */
k_s32 kd_mpi_venc_set_2d_color_gamut(k_u32 chn_num, const k_venc_2d_color_gamut color_gamut);
/**
 * @brief Attach 2d to venc
 *
 * @param [in] chn_num Channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the channel number passed in has not created, the mpi will return a failure
 * - kd_mpi_venc_attach_2d should be called after kd_mpi_venc_create_chn if needed.
 * - kd_mpi_venc_attach_2d should be called before kd_mpi_venc_start_chn if needed.
 */
k_s32 kd_mpi_venc_attach_2d(k_u32 chn_num);

/**
 * @brief Detach 2d from venc
 *
 * @param [in] chn_num Channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note If the channel number passed in has not created,
 *       the mpi will return a failure
 * @note kd_mpi_venc_detach_2d should be called before venc destroy if needed.
 */
k_s32 kd_mpi_venc_detach_2d(k_u32 chn_num);
/** @} */ /** <!-- ==== VENC End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
