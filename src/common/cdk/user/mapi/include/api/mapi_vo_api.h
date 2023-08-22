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
#ifndef __MAPI_VO_H__
#define __MAPI_VO_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_vo_api.h"
#include "k_vo_comm.h"


#define K_MAPI_ERR_VO_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_VO_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_VO_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_VO_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_VO_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_VO_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_MAPI_ERR_VO_INVALID_DEVID    K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_MAPI_ERR_VO_INVALID_CHNID    K_MAPI_DEF_ERR(K_MAPI_MOD_VO, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_VVI*/
/** @{ */  /** <!-- [ MAPI_VVI] */

// rst display 
k_s32 kd_mapi_set_backlight(void);

/**
 * @brief Reset the display subsystem
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - Should be used at the very beginning if needed
 */
k_s32 kd_mapi_vo_reset(void);


/**
 * @brief dsi enters hs mode to send commands
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - color bar display black and white gray
 */
k_s32 kd_mapi_dsi_set_test_pattern(void);


/**
 * @brief set dsi attr
 *
 * @param [in] attr Information about the dsi properties
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mapi_dsi_set_attr(k_vo_dsi_attr *attr);


/**
 * @brief turn off dsi
 *
 * @param [in] enable dsi enable state
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - Can only be turned off when the corresponding writeback is turned on
 */
k_s32 kd_mapi_dsi_enable(k_u32 enable);


/**
 * @brief dsi send command
 *
 * @param [in] data dsi data to send
 * @param [in] cmd_len Data length
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - You must configure lp mode or hs mode before sending
 */
k_s32 kd_mapi_dsi_send_cmd(k_u8 *data, k_s32 cmd_len);


/**
 * @brief dsi configures the rate of phy
 *
 * @param [in] attr Information about the phy properties
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - You must configure lp mode or hs mode before sending
 */
k_s32 kd_mapi_set_mipi_phy_attr(k_vo_mipi_phy_attr *attr);


/**
 * @brief dsi read command
 *
 * @param [in] rx_buf dsi data to send
 * @param [in] cmd_len Data length
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - You must configure lp mode or hs mode before reading
 */
k_s32 kd_mapi_dsi_read_pkg(k_u8 addr, k_u16 cmd_len, k_u32 *rv_data);


/**
 * @brief turn on vo
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - After the configuration is complete, vo is finally enabled
 */
k_s32 kd_mapi_vo_enable(void);


/**
 * @brief turn off vo
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - Can only be turned off when the corresponding vo is turned on
 */
k_s32 kd_mapi_vo_disable(void);


/**
 * @brief vo initialization
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mapi_vo_init(void);


/**
 * @brief set vo timing
 *
 * @param [in] attr Information about the vo properties
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The timing of dsi needs to match the timing of vo, otherwise the image will be wrong
 */
k_s32 kd_mapi_vo_set_dev_param(k_vo_pub_attr *attr);


/**
 * @brief user defined clock
 *
 * @param [in] sync_info Information about the clock
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The configured clock must meet the clock tree requirements
 * - must be enabled :kd_mpi_vo_set_user_sync_info
 */
k_s32 kd_mapi_vo_set_user_sync_info(k_u32 pre_div,k_u32 clk_en);


/**
 * @brief turn on layer
 *
 * @param [in] layer channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The number of channels cannot be greater than K_MAX_VO_LAYER_NUM
 */
k_s32 kd_mapi_vo_enable_video_layer(k_vo_layer layer);


/**
 * @brief turn off layer
 *
 * @param [in] layer channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The number of channels cannot be greater than K_MAX_VO_LAYER_NUM
 * - Can only be turned off when the corresponding layer is turned on
 */
k_s32 kd_mapi_vo_disable_video_layer(k_vo_layer layer);


/**
 * @brief Set video layer properties
 *
 * @param [in] layer channel number
 * @param [in] attr Information about the video layer properties
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The number of channels cannot be greater than K_MAX_VO_LAYER_NUM
 */
k_s32 kd_mapi_vo_set_video_layer_attr(k_vo_layer layer, k_vo_video_layer_attr *attr);


k_s32 kd_mapi_vo_set_layer_priority(k_vo_layer layer, k_s32 priority);


/**
 * @brief Set video layer properties
 *
 * @param [in] layer channel number
 * @param [in] attr Information about the video layer properties
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The number of channels cannot be greater than K_MAX_VO_OSD_NUM
 */
k_s32 kd_mapi_vo_set_video_osd_attr(k_vo_osd layer, k_vo_video_osd_attr *attr);


/**
 * @brief turn on osd layer
 * @param [in] layer channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The number of channels cannot be greater than K_MAX_VO_OSD_NUM
 */
k_s32 kd_mapi_vo_osd_enable(k_vo_osd layer);


/**
 * @brief turn off osd layer
 * @param [in] layer channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - The number of channels cannot be greater than K_MAX_VO_OSD_NUM
 * - Can only be turned off when the corresponding layer is turned on
 */
k_s32 kd_mapi_vo_osd_disable(k_vo_osd layer);


/**
 * @brief picture frame
 *
 * @param [in] frame Information about the draw frame properties
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mapi_vo_draw_frame(k_vo_draw_frame *frame);


/**
 * @brief turn on writeback
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - You need to configure writeback first, and then start
 */
k_s32 kd_mapi_vo_enable_wbc(void);


/**
 * @brief turn off writeback
 * @param [in] layer channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - Can only be turned off when the corresponding writeback is turned on
 */
k_s32 kd_mapi_vo_disable_wbc(void);


/**
 * @brief Set writeback parameters
 *
 * @param [in] vf_info Information about the writeback to be inserted
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - Currently the writeback data format can only be yuv420
 * - target_size: needs to be a valid size for vo output
 */
k_s32 kd_mapi_vo_set_wbc_attr(k_vo_wbc_attr *attr);


/**
 * @brief Insert a frame into the channel of the VO
 *
 * @param [in] chn_num channel number
 * @param [in] vf_info Information about the video frame to be inserted
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - The physical address of the video frame should come from a vb block
 * - The size of the video frame should not exceed the size of the vb block
 */
k_s32 kd_mapi_vo_chn_insert_frame(k_u32 chn_num, k_video_frame_info *vf_info);


/**
 * @brief Dump a video frame from the channel of the vo
 *
 * @param [in] chn_num channel number
 * @param [out] vf_info Video frame information obtained
 * @param [in] timeout_ms
 * - -1 indicates blocking mode.
 * - 0 indicates non-blocking mode.
 * - Greater than 0 indicates timeout mode, and the timeout time is measured in milliseconds(ms).
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - Before calling this function, you must first call ::kd_mpi_vo_chn_dump_frame
 * - When milli_sec is set to -1, it means blocking mode, and the program waits until
 *   the image is obtained before returning
 * - If milli_sec is greater than 0, it means non-blocking mode. The unit of the parameter
 *   is milliseconds, which refers to the timeout time. If no image is obtained within this time,
 *   the timeout will return
 * - The obtained physical address information comes from the VideoBuffer used inside MPP,
 *   so after using it, you must call the ::kd_mpi_vo_chn_dump_release interface to release its memory
 */
k_s32 kd_mapi_vo_chn_dump_frame(k_u32 chn_num, k_video_frame_info *vf_info, k_u32 timeout_ms);


/**
 * @brief Release dumped video frame
 *
 * @param [in] chn_num channel number
 * @param [in] vf_info Video frame information obtained by ::kd_mpi_vo_chn_dump_frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 * - If the channel number does not belong to a started pipe, the mpi returns failure
 * - If ::kd_mpi_vo_chn_dump_frame has not been called before, ,mpi will return failure
 * - Only the video frame obtained by ::kd_mpi_vo_chn_dump_frame can be released
 */
k_s32 kd_mapi_vo_chn_dump_release(k_u32 chn_num, const k_video_frame_info *vf_info);

/** @} */ /** <!-- ==== MAPI_VVI End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_VVI_API_H__ */

