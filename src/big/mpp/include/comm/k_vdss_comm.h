/**
 * @file k_vvi_comm.h
 * @author
 * @brief
 * @version 1.0
 * @date 2022-09-01
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
#ifndef __K_VDSS_COMM_H__
#define __K_VDSS_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VVI */
/** @{ */ /** <!-- [VVI] */

#define VDSS_MAX_DEV_NUMS        (3)
#define VDSS_MAX_CHN_NUMS        (3)

#define VDSS_MAX_FRAME_COUNT     (10)


#define K_ERR_VICAP_INVALID_DEVID     K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_VICAP_INVALID_CHNID     K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_VICAP_ILLEGAL_PARAM     K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_VICAP_EXIST             K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_VICAP_UNEXIST           K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_VICAP_NULL_PTR          K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_VICAP_NOT_CONFIG        K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_VICAP_NOT_SUPPORT       K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_VICAP_NOT_PERM          K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_VICAP_NOMEM             K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_VICAP_NOBUF             K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_VICAP_BUF_EMPTY         K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_VICAP_BUF_FULL          K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_VICAP_NOTREADY          K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_VICAP_BADADDR           K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_VICAP_BUSY              K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_BUSY)


typedef enum
{
    CSI0 = 1,
    CSI1 = 2,
    CSI2 = 3,
}k_vicap_csi;


typedef enum
{
    IPI1 = 1,
    IPI2 = 2,
    IPI3 = 3,
}k_vicap_ipi;


typedef enum
{
    MIPI_1LAN = 0,
    MIPI_2LAN = 1,
    MIPI_4LAN = 3,
}k_vicap_lan_num;


typedef enum
{
    SINGLE_SLAVE_MODE_SENSOR0 = 1,
    SINGLE_SLAVE_MODE_SENSOR1 = 2,
    DOUBLE_SLAVE_MODE = 3,

}k_vicap_slave_mode;


typedef enum
{
    STROBE_INTR = 0,
    FLASH_TRIGGER0_INTR = 1,
    FLASH_TRIGGER1_INTR = 2,
    FLASH_TRIGGER0_FRAME_MISS = 3,
    FLASH_TRIGGER1_FRAME_MISS = 4,
    FLASH_TRIGGER0_STROBE_MISS = 5,
    FLASH_TRIGGER1_STROBE_MISS = 6,

}k_vicap_vi_irq;

typedef enum
{
    RAW8  = 0x2a,
    RAW10 = 0x2b,
    RAW12 = 0x2c,
    RAW16 = 0x2e,
}k_vicap_csi_data_type;


typedef enum
{
    CAMERA_MODE  = 0,
    CONTROL_MODE = 1
}k_vicap_csi_work_mode;


typedef enum
{
    DVP_CSI0 = 0,
    DVP_CSI1 = 1,
    DVP_CSI1_FLASE_TRIGGER0 = 2,
    DVP_CSI1_FLASE_TRIGGER1 = 3,
    DVP_CSI2 = 4,

}k_vicap_dvp_mux;


typedef enum
{
    FOLLOW_STROBE_MODE = 0,
    FOLLOW_STROBE_BASE_PWM_MODE = 1,
    PWM_MODE = 2,
    NORMAL_FOLLOW_STROBE_MODE = 3,
    NORMAL_FOLLOW_STROBE_BASE_PWM_MODE = 4,
    CLOSE_3D_MODE = 5,
}k_vicap_3d_mode_type;


typedef enum
{
    STC_27M_CLK = 0,
    STC_1M_CLK = 1,
    STC_90K_CLK = 2,
}k_vicap_stcmode;


typedef enum
{
    VCID_HDR_2FRAME = 0,
    VCID_HDR_3FRAME = 1,
    SONY_HDR_3FRAME = 2,
    SONY_HDR_2FRAME = 3,
    LINERA_MODE = 4,
}k_vicap_hdr_mode;


typedef struct
{
    k_vicap_csi_work_mode mode;
    k_vicap_csi_data_type type;
    k_bool is_csi_sync_enent;
    k_u32 hsa;
    k_u32 hbp;
}k_vicap_ipi_attr;


typedef struct
{
    k_u32 hsa;
    k_u32 hbp;
    k_u32 hfp;
    k_u32 hline;

    k_u32 vsa;
    k_u32 vbp;
    k_u32 vfp;
    k_u32 vact;
}k_vicap_ipi_timing;

typedef struct
{
    k_vicap_stcmode stc_mode;
    k_bool is_clear;
    k_bool load_new_val;
    k_vicap_hdr_mode mode;
}k_vicap_timerstamp;


typedef struct {
    k_bool err_phy0_control_lan0;
    k_bool err_phy0_control_lan1;
    k_bool err_phy0_lp1_contention;
    k_bool err_phy0_lp0_contention;

    k_bool err_phy1_control_lan0;
    k_bool err_phy1_control_lan1;
    k_bool err_phy1_lp1_contention;
    k_bool err_phy1_lp0_contention;

    k_bool err_phy2_control_lan0;
    k_bool err_phy2_control_lan1;
    k_bool err_phy2_lp1_contention;
    k_bool err_phy2_lp0_contention;

    k_bool flash_trigger0_strobe_miss;
    k_bool flash_trigger1_strobe_miss;
    k_bool flash_trigger0_frame_miss;
    k_bool flash_trigger1_frame_miss;

}k_vicap_err_status;


typedef struct {

    k_u32 first_frame;
    k_u32 glitch;
    k_bool strobe_inv;
    k_bool flash_trigger0_inv;
    k_bool flash_trigger1_inv;
    k_vicap_3d_mode_type type;

}k_vicap_3d_mode;


typedef struct {

    k_vicap_slave_mode slave_mode;

    k_u32 sensor0_vsa_cycle;
    k_u32 sensor0_hsa_cycle;
    k_u32 sensor0_vs_high_cycle;
    k_u32 sensor0_hs_high_cycle;
    k_u32 sensor0_hs_dly_cycle;

    k_u32 sensor1_vsa_cycle;
    k_u32 sensor1_hsa_cycle;
    k_u32 sensor1_vs_high_cycle;
    k_u32 sensor1_hs_high_cycle;
    k_u32 sensor1_hs_dly_cycle;

}k_vicap_slave;

typedef enum {
	MIPI_800M = 1,
	MIPI_1200M = 2,
	MIPI_1600M = 3,
}k_vicap_phy_freq;


typedef struct{
    k_vicap_phy_freq freq;
    k_vicap_lan_num lan_num;
}k_phy_attr;


typedef struct{
    k_vicap_ipi_attr attr[3];
    k_vicap_hdr_mode mode;
}k_csi_attr;

typedef struct {
    k_vicap_csi csi;
    k_u32 sensor_type;
    k_phy_attr phy_attr;
    k_vicap_hdr_mode mode;
    k_vicap_csi_data_type dev_format[3];
    k_vicap_3d_mode_type type;
    k_u32 bind_dvp;
}k_vi_attr;


/**
 * @brief Defines the attributes of a VVI channel
 *
 */
typedef struct {
    k_u32 enable;
    k_u32 height;
    k_u32 width;
    k_pixel_format format;  /**< Currently only ARGB is supported*/
}k_vicap_chn_attr;


typedef struct {
    k_u32 dev_num;
    k_u32 sensor_type;
    k_u32 height;
    k_u32 width;
    k_vi_attr artr;
}k_vicap_dev_attr;


typedef struct
{
    k_u32 dev_num;
    k_u32 chn_num;
    k_u32 timeout_ms;
    k_video_frame_info info;
} k_vdss_chn_vf_info;

/** @} */ /** <!-- ==== VVI End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
