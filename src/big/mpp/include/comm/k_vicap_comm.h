/**
 * @file k_vicap_comm.h
 * @author
 * @brief
 * @version 1.0
 * @date 2023-03-20
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
#ifndef __K_VICAP_COMM_H__
#define __K_VICAP_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#include "k_sensor_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VICAP */
/** @{ */ /** <!-- [VICAP] */

#define VICAP_MAX_DEV_NUMS        (3)
#define VICAP_MAX_CHN_NUMS        (3)

#define VICAP_MIN_FRAME_COUNT     (3)
#define VICAP_MAX_FRAME_COUNT     (10)

#define VICAP_MCM_FRAME_COUNT     (4)

#define VICAP_IMG_MIN_WIDTH 320
#define VICAP_IMG_MIN_HEIGH 240

#define VICAP_IMG_MAX_WIDTH 2960
#define VICAP_IMG_MAX_HEIGH 2160

#define VICAP_GPIO_IRQ_CTRL_PIN_MAX 63

#define VICAP_ALIGN_1K 0x400
#define VICAP_ALIGN_UP(addr, size)	(((addr)+((size)-1U))&(~((size)-1U)))

/**
 * @brief Defines the CSI NUM
 *
 */
typedef enum {
    VICAP_CSI0 = 1,
    VICAP_CSI1 = 2,
    VICAP_CSI2 = 3,
} k_vicap_csi_num;

typedef enum {
    VICAP_IPI1 = 1,
    VICAP_IPI2 = 2,
    VICAP_IPI3 = 3,
} k_vicap_vi_ipi;

typedef enum {
    VICAP_PLL0_CLK_DIV4 = 5,
    VICAP_PLL1_CLK_DIV3 = 8,
    VICAP_PLL1_CLK_DIV4 = 9,
} k_vicap_mclk_sel;

typedef enum {
    VICAP_MCLK0 = 1,
    VICAP_MCLK1 = 2,
    VICAP_MCLK2 = 3,
} k_vicap_mclk_id;

typedef struct {
    k_vicap_mclk_id id;
    k_vicap_mclk_sel mclk_sel;
    k_u8 mclk_div;
    k_u8 mclk_en;
} k_vicap_mclk;

/**
 * @brief Defines the MIPI LANEs
 *
 */
typedef enum {
    VICAP_MIPI_1LANE = 0,
    VICAP_MIPI_2LANE = 1,
    VICAP_MIPI_4LANE = 3,
} k_vicap_mipi_lanes;

/**
 * @brief Defines the MIPI CSI PHY freq
 *
 */
typedef enum {
	VICAP_MIPI_PHY_800M  = 1,
	VICAP_MIPI_PHY_1200M = 2,
	VICAP_MIPI_PHY_1600M = 3,
} k_vicap_mipi_phy_freq;

/**
 * @brief Defines the MIPI CSI data type
 *
 */
typedef enum {
    VICAP_CSI_DATA_TYPE_RAW8   = 0x2A,
    VICAP_CSI_DATA_TYPE_RAW10 = 0x2B,
    VICAP_CSI_DATA_TYPE_RAW12 = 0x2C,
    VICAP_CSI_DATA_TYPE_RAW16 = 0x2E,
    VICAP_CSI_DATA_TYPE_YUV422_8 = 0x1E,
} k_vicap_csi_data_type;

/**
 * @brief Defines the MIPI CSI work mode
 *
 */
typedef enum {
    VICAP_CSI_CAMERA_MODE  = 0,
    VICAP_CSI_CONTROL_MODE = 1,
} k_vicap_csi_work_mode;

/**
 * @brief Defines the vi flash light mode
 *
 */
typedef enum {
    VICAP_FLASH_FOLLOW_STROBE = 0,
    VICAP_FLASH_FOLLOW_STROBE_BASE_PWM = 1,
    VICAP_FLASH_NORMAL_PWM = 2,
    VICAP_FLASH_DISABLE = 3,  /**< disable flash light*/
} k_vicap_vi_flash_mode;

/**
 * @brief Defines the dvp port of VICAP
 *
 */
typedef enum {
    VICAP_VI_DVP_PORT0 = 0,
    VICAP_VI_DVP_PORT1 = 1,
    VICAP_VI_DVP_PORT2 = 2,
    VICAP_VI_DVP_PORT_MAX,
} k_vicap_vi_dvp_port;

/**
 * @brief Defines the data source of VICAP
 *
 */
typedef enum {
    VICAP_SOURCE_CSI0 = 0,   /**< vicap acquire data from the csi0*/
    VICAP_SOURCE_CSI1 = 1,   /**< vicap acquire data from the csi1*/
    VICAP_SOURCE_CSI1_FS_TR0 = 2,   /**<vicap acquire data from the csi1 for flash trigger 0*/
    VICAP_SOURCE_CSI1_FS_TR1 = 3,   /**<vicap acquire data from the csi0 for flash trigger 1*/
    VICAP_SOURCE_CSI2 = 4,   /**< vicap acquire data from the csi2*/
} k_vicap_data_source;

/**
 * @brief Defines the HDR mode of VICAP
 *
 */
typedef enum {
    VICAP_VCID_HDR_2FRAME = 0,
    VICAP_VCID_HDR_3FRAME = 1,
    VICAP_SONY_HDR_3FRAME = 2,
    VICAP_SONY_HDR_2FRAME = 3,
    VICAP_LINERA_MODE     = 4,
} k_vicap_hdr_mode;

/**
 * @brief Defines the first frame selcect for VI triger
 *
 */
typedef enum {
    VICAP_VI_FIRST_FRAME_FS_TR0 = 0,
    VICAP_VI_FIRST_FRAME_FS_TR1 = 1,
} k_vicap_vi_first_frame_sel;

/**
 * @brief Defines the flash control mode of VICAP
 *
 */
typedef struct {
    k_bool is_3d_mode;
    k_u32 sensor_sel;
    k_vicap_vi_flash_mode flash_mode;
    k_vicap_vi_first_frame_sel first_frame;    /**< the first frame source select. 0: VICAP_SOURCE_CSI1_FS_TR0, 1: VICAP_SOURCE_CSI1_FS_TR1*/
    k_u16 glitch_filter;    /**< filter the glitch of strobe signal. Effective value 0~65535 cycles, default: 0 cycle*/
} k_vicap_flash_ctrl;

/**
 * @brief Defines the vi ipi attr of VICAP
 *
 */
typedef struct {
    k_vicap_csi_work_mode work_mode;
    k_vicap_csi_data_type data_type;
    k_bool is_csi_sync_event;
    k_u32 hsa;
    k_u32 hbp;
} k_vicap_vi_ipi_attr;

/**
 * @brief Defines vi attributes of VICAP
 *
 */
typedef struct {
    k_vicap_csi_num csi_num;
    k_vicap_mipi_lanes mipi_lanes;
    k_vicap_csi_data_type data_type;
    k_vicap_mipi_phy_freq phy_freq;
    k_vicap_hdr_mode hdr_mode;
    k_vicap_vi_dvp_port dvp_port;
    k_vicap_data_source source;
    k_vicap_flash_ctrl ctrl_mode;
} k_vicap_vi_attr;

/**
 * @brief Defines the work mode of VICAP
 *
 */
typedef enum {
    VICAP_WORK_ONLINE_MODE,
    VICAP_WORK_OFFLINE_MODE,
    VICAP_WORK_LOAD_IMAGE_MODE,
    VICAP_WORK_ONLY_MCM_MODE,
} k_vicap_work_mode;

/**
 * @brief Defines the input type of VICAP
 *
 */
typedef enum {
    VICAP_INPUT_TYPE_SENSOR,
    VICAP_INPUT_TYPE_IMAGE,
} k_vicap_input_type;

/**
 * @brief Defines the bayer pattern of raw image
 *
 */
typedef k_sensor_bayer_pattern k_vicap_image_pattern;

/**
 * @brief Defines the image window of a VICAP
 *
 */
typedef struct {
    k_u16    h_start;
    k_u16    v_start;
    k_u16    width;
    k_u16    height;
} k_vicap_window;

/**
 * @brief Defines device id of VICAP
 *
 */
typedef enum {
    VICAP_DEV_ID_0 = 0,
    VICAP_DEV_ID_1 = 1,
    VICAP_DEV_ID_2 = 2,
    VICAP_DEV_ID_MAX,
} k_vicap_dev;

/**
 * @brief Defines the channel id of VICAP
 *
 */
typedef enum {
    VICAP_CHN_ID_0 = 0,
    VICAP_CHN_ID_1 = 1,
    VICAP_CHN_ID_2 = 2,
    VICAP_CHN_ID_MAX,
} k_vicap_chn;

/**
 * @brief Defines the dump data format of VICAP
 *
 */
typedef enum {
    VICAP_DUMP_YUV = 0,
    VICAP_DUMP_RGB = 1,
    VICAP_DUMP_RAW = 2,
    VICAP_DUMP_YUV444 = 3,
} k_vicap_dump_format;

/**
 * @brief Defines the database parse mode of VICAP
 *
 */
typedef enum {
    VICAP_DATABASE_PARSE_XML_JSON = 0,
    VICAP_DATABASE_PARSE_HEADER = 1,
} k_vicap_database_parse_mode;

/**
 * @brief Defines the fill light ctrl state of VICAP
 *
 */
typedef enum {
    VICAP_FILL_LIGHT_CTRL_NORMAL = 0,
    VICAP_FILL_LIGHT_CTRL_IR = 100,
    VICAP_FILL_LIGHT_CTRL_SPECKLE,
    VICAP_FILL_LIGHT_CTRL_MAX = 255,
} k_vicap_fill_light_ctrl_state;


typedef union {
    struct {
        k_u32 ae_enable : 1;      /**< bit 0: 0-disable 1-enable */
        k_u32 af_enable : 1;      /**< bit 1 */
        k_u32 ahdr_enable : 1;    /**< bit 2 */
        k_u32 awb_enable : 1;     /**< bit 3 */
        k_u32 ccm_enable : 1;     /**< bit 4 */
        k_u32 compress_enable : 1; /**< bit 5 */
        k_u32 expand_enable : 1;  /**< bit 6 */
        k_u32 cnr_enable : 1;     /**< bit 7 */
        k_u32 ynr_enable : 1;     /**< bit 8 */
        k_u32 cproc_enable : 1;   /**< bit 9 */
        k_u32 dci_enable : 1;     /**< bit 10 */
        k_u32 demosaic_enable : 1; /**< bit 11 */
        k_u32 dg_enable : 1;      /**< bit 12 */
        k_u32 dpcc_enable : 1;    /**< bit 13 */
        k_u32 dpf_enable : 1;     /**< bit 14 */
        k_u32 ee_enable : 1;      /**< bit 15 */
        k_u32 gc_enable : 1;      /**< bit 16 */
        k_u32 ge_enable : 1;      /**< bit 17 */
        k_u32 gtm_enable : 1;     /**< bit 18 */
        k_u32 lsc_enable : 1;     /**< bit 19 */
        k_u32 lut3d_enable : 1;   /**< bit 20 */
        k_u32 pdaf_enable : 1;    /**< bit 21 */
        k_u32 rgbir_enable : 1;   /**< bit 22 */
        k_u32 wb_enable : 1;      /**< bit 23 */
        k_u32 wdr_enable : 1;     /**< bit 24 */
        k_u32 dnr3_enable : 1;    /**< bit 25 */
        k_u32 dnr2_enable : 1;    /**< bit 26 */
        k_u32 roi_enable : 1;     /**< bit 27 */
        k_u32 reserved_enable : 4;/**< bit 28:31 */
    } bits;
    k_u32 data;
} k_vicap_isp_pipe_ctrl;

/**
 * @brief Defines the CSI NUM of VICAP device
 *
 */
typedef struct {
    const char *sensor_name;
    k_u16 width;
    k_u16 height;
    k_vicap_csi_num csi_num;  /**< CSI NUM that the sensor connects to*/
    k_vicap_mipi_lanes mipi_lanes;  /**< MIPI lanes that the sensor connects to*/
    k_vicap_data_source source_id; /**<source id that the sensor used to*/
    k_bool is_3d_sensor;

    k_vicap_mipi_phy_freq phy_freq;
    k_vicap_csi_data_type data_type;
    k_vicap_hdr_mode hdr_mode;
    k_vicap_vi_flash_mode flash_mode;
    k_vicap_vi_first_frame_sel first_frame;
    k_u16 glitch_filter;
    k_vicap_sensor_type sensor_type;
} k_vicap_sensor_info;

typedef struct {
    const char *sensor_string;
    k_vicap_sensor_type sensor_type;
} k_vicap_sensor_type_map;

/**
 * @brief Defines the attributes of a VICAP channel
 *
 */
typedef struct {
    k_vicap_window out_win;
    k_vicap_window crop_win;
    k_vicap_window scale_win;
    k_bool crop_enable;
    k_bool scale_enable;
    k_bool chn_enable;
    k_pixel_format pix_format;
    k_u32 buffer_num;
    k_u32 buffer_size;
    k_u8 alignment; // 0: 1 byte, 1: 2 byte ... 12: 4096 byte
    k_u8 fps; // 0: original FPS
} k_vicap_chn_attr;


typedef enum {
    VICAP_MIRROR_NONE = 0,
    VICAP_MIRROR_HOR = 1,
    VICAP_MIRROR_VER = 2,
    VICAP_MIRROR_BOTH = 3,
} k_vicap_mirror;


typedef struct {
    k_vicap_sensor_type sensor_type;
    k_vicap_mirror mirror;
}k_vicap_mirror_mode;




/**
 * @brief Defines the attributes of a VICAP device
 *
 */
typedef struct {
    k_vicap_window acq_win;
    k_vicap_work_mode mode;
    k_vicap_input_type input_type;
    k_vicap_image_pattern image_pat;
    k_vicap_isp_pipe_ctrl pipe_ctrl;
    k_u32 cpature_frame;
    k_vicap_sensor_info sensor_info;
    k_bool dw_enable;
    k_bool dev_enable;
    k_u32 buffer_num;
    k_u32 buffer_size;
    k_vicap_mirror mirror;
} k_vicap_dev_attr;

/**
 * @brief Defines the attributes of a VICAP sensor info
 *
 */
typedef struct {
    k_vicap_dev dev_num;
    k_vicap_chn chn_num;
    k_s32 sensor_fd;
} k_vicap_sensor_attr;


typedef enum
{
    STC_27M_CLK = 0,
    STC_1M_CLK = 1,
    STC_90K_CLK = 2,
}k_vicap_stcmode;

typedef struct
{
    k_vicap_stcmode stc_mode;
    k_bool is_clear;
    k_bool is_open;
    k_bool load_new_val;
    k_vicap_hdr_mode mode;
}k_vicap_timerstamp;

typedef struct
{
    k_u8 m;
    k_u8 n;
    k_vicap_hdr_mode mode;
}k_vicap_drop_frame;

typedef enum {
    VICAP_SLAVE_ID0 = 1,
    VICAP_SLAVE_ID1 = 2,
} k_vicap_slave_id;


typedef struct {
    k_u32 hs_delay;
    k_u32 vs_cycle;
    k_u32 hs_cycle;
    k_u32 vs_high;
    k_u32 hs_high;
}k_vicap_slave_info;


typedef struct {
    k_bool hs_enable;
    k_bool vs_enable;
}k_vicap_slave_enable;

/**
 * @brief Defines the VICAP vb info
 *
 */
typedef struct {
    k_u64 phys_addr[3];
    void *virt_addr;
    k_s32 pool_id;
    k_s32 dev_num;
    k_s32 chn_num;
    k_u32 width;
    k_u32 height;
    k_pixel_format format;
    k_u8 alignment;
    k_u8 fill_light_state;
    k_u32 frame_num;
    k_u64 timestamp;
} k_vicap_vb_info;

/**
 * @brief Defines the VICAP attr info
 *
 */
typedef struct {
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_vicap_sensor_type sensor_type;
    k_u32 out_width;
    k_u32 out_height;
    k_pixel_format pixel_format;
    k_vicap_isp_pipe_ctrl pipe_ctrl;
    k_bool dw_en;
    k_bool crop_en;
    k_u32 buf_size;
} k_vicap_attr_info;

/**
 * @brief Defines the VICAP dev set info
 *
 */
typedef struct {
    k_vicap_dev vicap_dev;
    k_vicap_work_mode mode;
    k_vicap_sensor_type sensor_type;
    k_vicap_isp_pipe_ctrl pipe_ctrl;
    k_bool dw_en;
    k_u32 buffer_num;
    k_u32 buffer_size;
} k_vicap_dev_set_info;

/**
 * @brief Defines the VICAP chn set info
 *
 */
typedef struct {
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_bool crop_en;
    k_bool scale_en;
    k_bool chn_en;
    k_u32 out_width;
    k_u32 out_height;
    k_u32 crop_h_start;
    k_u32 crop_v_start;
    k_pixel_format pixel_format;
    k_u32 buf_size;
    k_u32 buffer_num;
    k_u8 alignment;
    k_u8 fps;
} k_vicap_chn_set_info;

/**
 * @brief Defines the VICAP set mcm
 *
 */
typedef struct {
    k_u32 width;
    k_u32 height;
    k_u32 format;
    k_u32 stride;
}k_vicap_mcm_cfg;

/**
 * @brief Defines the VICAP set mcm
 *
 */
typedef struct {
    k_vicap_chn vicap_chn;
    k_vicap_mcm_cfg cfg;

}k_vicap_mcm_chn_cfg;

/**
 * @brief Defines the VICAP set mcm
 *
 */
typedef struct {
    k_vicap_mcm_cfg cfg;
    k_vicap_chn mcm_chn;
    k_u32 buff_num;
}k_vicap_mcm_chn_attr;

/**
 * @brief Defines the dump data format of VICAP
 *
 */
typedef enum {
    VICAP_MCM_RAW8 = 0,
    VICAP_MCM_RAW10 = 1,
    VICAP_MCM_RAW12 = 2,
    VICAP_MCM_RAW14 = 3,
    VICAP_MCM_RAW16 = 4,
    VICAP_MCM_RAW24 = 5,
} k_vicap_mcm_format;


typedef struct
{
    k_u32 chn_num;
    k_u32 timeout_ms;
    k_video_frame_info info;
} k_vicap_mcm_chn_vf_info;

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
#define K_ERR_VICAP_OPT_ALREADY_WRITE             K_DEF_ERR(K_ID_VICAP, K_ERR_LEVEL_ERROR, K_ERR_OPT_ALREADY_WRITE)



/** @} */ /** <!-- ==== VICAP End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
