/**
 * @file k_isp_comm.h
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
#ifndef __K_ISP_COMM_H__
#define __K_ISP_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"

#include "k_vicap_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     ISP */
/** @{ */ /** <!-- [ISP] */

#define ISP_MAX_DEV_NUMS        (3)
#define ISP_MAX_CHN_NUMS        (3)
#define ISP_MAX_FRAME_COUNT     (3)

#define ISP_IMG_MIN_WIDTH 320
#define ISP_IMG_MIN_HEIGH 240

#define ISP_IMG_MAX_WIDTH 4096
#define ISP_IMG_MAX_HEIGH 2160

#define ISP_AE_ROI_WINDOWS_MAX 25
#define ISP_AE_ROI_HOLD_FRAME 3
/**
 * @brief Defines the input picture info of isp
 *
 */
typedef struct {
    k_u32 loop_cnt;
    k_u32 frame_num;
    k_u32 width;
    k_u32 height;
    k_u32 layout;
    k_u32 format;
    char *picture_name;
} k_isp_input_picture_cfg;

/**
 * @brief Defines the channel id of isp
 *
 */
typedef enum {
    ISP_MAIN_CHN  = 0,
    ISP_SELF1_CHN = 1,
    ISP_SELF2_CHN = 2,
    ISP_RDMA_CHN = 3,
    ISP_CHN_ID_MAX = 3,
} k_isp_chn;

/**
 * @brief Defines the device id of isp
 *
 */
typedef enum {
    ISP_DEV_ID_0 = 0,
    ISP_DEV_ID_1 = 1,
    ISP_DEV_ID_2 = 2,
    ISP_DEV_ID_MAX,
} k_isp_dev;

/**
 * @brief Defines the mcm mode isp
 *
 */
typedef enum {
    ISP_MCM_ONLINE_MODE = 0,
    ISP_MCM_OFFLINE_MODE = 1,
} k_isp_mcm_mode;

/**
 * @brief Defines the data mode of isp
 *
 */
typedef enum {
    ISP_MI_DATAMODE_INVALID       = 0,
    ISP_MI_DATAMODE_DISABLED      = 1,
    ISP_MI_DATAMODE_JPEG          = 2,
    ISP_MI_DATAMODE_YUV444        = 3,
    ISP_MI_DATAMODE_YUV422        = 4,
    ISP_MI_DATAMODE_YUV420        = 5,
    ISP_MI_DATAMODE_YUV400        = 6,
    ISP_MI_DATAMODE_RGB888        = 7,
    ISP_MI_DATAMODE_RGB666        = 8,
    ISP_MI_DATAMODE_RGB565        = 9,
    ISP_MI_DATAMODE_RAW8          = 10,
    ISP_MI_DATAMODE_RAW12         = 11,
    ISP_MI_DATAMODE_DPCC          = 12,
    ISP_MI_DATAMODE_RAW10         = 13,
    ISP_MI_DATAMODE_RAW14         = 14,
    ISP_MI_DATAMODE_RAW16         = 15,
    ISP_MI_DATAMODE_RAW20         = 16,
    ISP_MI_DATAMODE_RAW24         = 17,
    ISP_MI_DATAMODE_META          = 18,
    ISP_MI_DATAMODE_MAX
} k_isp_mi_data_mode;

/**
 * @brief Defines the piexl format of isp
 *
 */
typedef enum {
    ISP_PIX_FMT_YUV422SP = 0,               /**< ISP output format: YUV422 Semi-Planar */
    ISP_PIX_FMT_YUV422I,                    /**< ISP output format: YUV422 Interleaved */
    ISP_PIX_FMT_YUV420SP,                   /**< ISP output format: YUV420 Semi-Planar */
    ISP_PIX_FMT_YUV444P,                    /**< ISP output format: YUV422 Planer */
    ISP_PIX_FMT_YUV444I,                    /**< ISP output format: YUV422 Interleaved */
    ISP_PIX_FMT_YUV400,                     /**< ISP output format: YUV400 Y only format */
    ISP_PIX_FMT_RGB888,                     /**< ISP output format: RGB888 Raster Scan*/
    ISP_PIX_FMT_RGB888P,                    /**< ISP output format: RGB888 Planar */
    ISP_PIX_FMT_RAW8,                       /**< ISP output format: Raw 8-bit */
    ISP_PIX_FMT_RAW10,                      /**< ISP output format: Raw 10bit */
    ISP_PIX_FMT_RAW10_ALIGNED_MODE0,        /**< ISP output format: Raw 10bit in align mode 0 */
    ISP_PIX_FMT_RAW10_ALIGNED_MODE1,        /**< ISP output format: Raw 10bit in align mode 1 */
    ISP_PIX_FMT_RAW12,                      /**< ISP output format: Raw 12-bit */
    ISP_PIX_FMT_RAW12_ALIGNED_MODE0,        /**< ISP output format: Raw 12-bit in align mode 0 */
    ISP_PIX_FMT_RAW12_ALIGNED_MODE1,        /**< ISP output format: Raw 12-bit in align mode 1 */
    ISP_PIX_FMT_RAW14,                      /**< ISP output format: Raw 14bit */
    ISP_PIX_FMT_RAW14_ALIGNED_MODE0,        /**< ISP output format: Raw 14bit in align mode 0 */
    ISP_PIX_FMT_RAW14_ALIGNED_MODE1,        /**< ISP output format: Raw 14bit in align mode 1 */
    ISP_PIX_FMT_RAW16,                      /**< ISP output format: Raw 16-bit */
    ISP_PIX_FMT_MAX                         /**< Total number of output formats */
} k_isp_output_pix_format;

/**
 * @brief Defines the data width of isp
 *
 */
typedef enum {
	ISP_PIXEL_YUV_BIT_INVALID = -1,
	ISP_PIXEL_YUV_8_BIT  = 0,
	ISP_PIXEL_YUV_10_BIT = 1,
	ISP_PIXEL_YUV_12_BIT = 2,
	ISP_PIXEL_YUV_BIT_MAX
} k_isp_output_bit_width;

/**
 * @brief Defines the data layout mode of isp
 *
 */
typedef enum {
    ISP_MI_DATASTORAGE_INVALID       = 0,
    ISP_MI_DATASTORAGE_PLANAR        = 1,
    ISP_MI_DATASTORAGE_SEMIPLANAR    = 2,
    ISP_MI_DATASTORAGE_INTERLEAVED   = 3,
    ISP_MI_DATASTORAGE_MAX
} k_isp_mi_data_layout;

/**
 * @brief Defines the data align mode of isp
 *
 */
typedef enum {
	ISP_MI_PIXEL_ALIGN_INVALID       = -1,
	ISP_MI_PIXEL_UN_ALIGN            = 0,
	ISP_MI_PIXEL_ALIGN_128BIT        = 1,
	ISP_MI_PIXEL_ALIGN_DOUBLE_WORD   = 1,
	ISP_MI_PIXEL_ALIGN_WORD          = 2,
	ISP_MI_PIXEL_ALIGN_16BIT         = 2,
	ISP_MI_PIXEL_ALIGN_MAX
} k_isp_mi_data_align_mode;


/**
 * @brief Defines the interface selection of isp
 *
 */
typedef enum {
    ISP_ITF_SELECT_INVALID   = 0,
    ISP_ITF_SELECT_PARALLEL  = 1,
    ISP_ITF_SELECT_SMIA      = 2,
    ISP_ITF_SELECT_MIPI      = 3,
    ISP_ITF_SELECT_HDR       = 4,
    ISP_ITF_SELECT_MAX
} k_isp_if_select;

/**
 * @brief Defines the sample edge of isp
 *
 */
typedef enum {
    ISP_SAMPLE_EDGE_INVALID   = 0,
    ISP_SAMPLE_EDGE_FALLING   = 1,
    ISP_SAMPLE_EDGE_RISING    = 2,
    ISP_SAMPLE_EDGE_MAX
} k_isp_sample_edge;

/**
 * @brief Defines the sync polarity of isp
 *
 */
typedef enum {
    ISP_POLARITY_INVALID  = 0,
    ISP_POLARITY_HIGH     = 1,
    ISP_POLARITY_LOW      = 2,
    ISP_POLARITY_MAX
} k_isp_sync_polarity;

/**
 * @brief Defines the bayer pattern of isp
 *
 */
typedef enum {
    ISP_BAYER_PATTERN_INVALID    = 0,
    ISP_BAYER_PATTERN_RGRGGBGB   = 1,
    ISP_BAYER_PATTERN_GRGRBGBG   = 2,
    ISP_BAYER_PATTERN_GBGBRGRG   = 3,
    ISP_BAYER_PATTERN_BGBGGRGR   = 4,
    ISP_BAYER_PATTERN_MAX
} k_isp_bayer_pattern;

/**
 * @brief Defines the color subsampling of isp
 *
 */
typedef enum {
    ISP_CONV422_INVALID        = 0,
    ISP_CONV422_COSITED        = 1,
    ISP_CONV422_INTERLEAVED    = 2,
    ISP_CONV422_NONCOSITED     = 3,
    ISP_CONV422_MAX
} k_isp_color_subsampling;

/**
 * @brief Defines the ccir sequence of isp
 *
 */
typedef enum {
    ISP_CCIR_SEQUENCE_INVALID    = 0,
    ISP_CCIR_SEQUENCE_YCbYCr     = 1,        /**< YCbYCr */
    ISP_CCIR_SEQUENCE_YCrYCb     = 2,        /**< YCrYCb */
    ISP_CCIR_SEQUENCE_CbYCrY     = 3,        /**< CbYCrY */
    ISP_CCIR_SEQUENCE_CrYCbY     = 4,        /**< CrYCbY */
    ISP_CCIR_SEQUENCE_MAX
} k_isp_ccir_sequence;

/**
 * @brief Defines the field selection of isp
 *
 */
typedef enum {
    ISP_FIELD_SELECTION_INVALID   = 0,
    ISP_FIELD_SELECTION_BOTH      = 1,
    ISP_FIELD_SELECTION_EVEN      = 2,
    ISP_FIELD_SELECTION_ODD       = 3,
    ISP_FIELD_SELECTION_MAX
} k_isp_filed_selection;

/**
 * @brief Defines the input data width of isp
 *
 */
typedef enum {
    ISP_INPUT_DATA_INVALID     = 0,
    ISP_INPUT_DATA_12BIT       = 1,
    ISP_INPUT_DATA_10BIT_ZZ    = 2,
    ISP_INPUT_DATA_10BIT_EX    = 3,
    ISP_INPUT_DATA_8BIT_ZZ     = 4,
    ISP_INPUT_DATA_8BIT_EX     = 5,
    ISP_INPUT_DATA_MAX
} k_isp_input_data_width;

/**
 * @brief Defines the input data mode of isp
 *
 */
typedef enum {
    ISP_DATA_MODE_INVALID     = 0,
    ISP_DATA_MODE_RAW         = 1,
    ISP_DATA_MODE_656         = 2,
    ISP_DATA_MODE_601         = 3,
    ISP_DATA_MODE_BAYER_RGB   = 4,
    ISP_DATA_MODE_DATA        = 5,
    ISP_DATA_MODE_RGB656      = 6,
    ISP_DATA_MODE_RAW656      = 7,
    ISP_DATA_MODE_MAX
} k_isp_input_data_mode;

/**
 * @brief Defines the mipi csi data type
 *
 */
typedef enum {
    ISP_DATA_TYPE_YUV420_8            = 0x18,
    ISP_DATA_TYPE_YUV420_10           = 0x19,
    ISP_DATA_TYPE_LEGACY_YUV420_8     = 0x1A,
    ISP_DATA_TYPE_YUV420_8_CSPS       = 0x1C,
    ISP_DATA_TYPE_YUV420_10_CSPS      = 0x1D,
    ISP_DATA_TYPE_YUV422_8            = 0x1E,
    ISP_DATA_TYPE_YUV422_10           = 0x1F,

    ISP_DATA_TYPE_RGB444              = 0x20,
    ISP_DATA_TYPE_RGB555              = 0x21,
    ISP_DATA_TYPE_RGB565              = 0x22,
    ISP_DATA_TYPE_RGB666              = 0x23,
    ISP_DATA_TYPE_RGB888              = 0x24,

    ISP_DATA_TYPE_RAW_6               = 0x28,
    ISP_DATA_TYPE_RAW_7               = 0x29,
    ISP_DATA_TYPE_RAW_8               = 0x2A,
    ISP_DATA_TYPE_RAW_10              = 0x2B,
    ISP_DATA_TYPE_RAW_12              = 0x2C,
    ISP_DATA_TYPE_RAW_14              = 0x2D,
} k_isp_mipi_data_type;

/**
 * @brief Defines the work mode isp
 *
 */
typedef enum {
    ISP_WORK_MODE_INVALID = 0,           /**< Invalid mode*/
    ISP_WORK_MODE_STREAM,                /**< Stream mode: one sensor input */
    ISP_WORK_MODE_MCM,                   /**< Reserved. */
    ISP_WORK_MODE_RDMA,                  /**< RDMA mode: input from DMA buffer */
    ISP_WORK_MODE_MAX
} k_isp_work_mode;

/**
 * @brief Defines the input type of isp
 *
 */
typedef enum {
    ISP_INPUT_TYPE_INVALID = 0,           /**< Invalid type */
    ISP_INPUT_TYPE_SENSOR,                /**< Sensor input */
    ISP_INPUT_TYPE_USER,                  /**< DMA buffer input */
    ISP_INPUT_TYPE_MAX
} k_isp_input_type;

/**
 * @brief Defines the buffer block type of isp
 *
 */
typedef enum {
    ISP_BUFQUE_NONBLOCK_TYPE     = 0,   /**< Non-blocking type */
    ISP_BUFQUE_TIMEOUT_TYPE,            /**< Time-blocking type*/
    ISP_BUFQUE_BLOCK_TYPE,              /**< Blocking type */
    ISP_BUFQUE_BLOCK_TYPE_MAX           /**< Blocking type max */
} k_isp_buf_block_type;

/**
 * @brief Defines the config mode of isp
 *
 */
typedef enum {
    ISP_CFG_MODE_MANUAL,            /**< Manual mode */
    ISP_CFG_MODE_AUTO,              /**< Auto mode */
    ISP_CFG_MODE_MAX
} k_isp_config_mode;

/**
 * @brief Defines the module config of isp
 *
 */
typedef struct {
    k_bool enable;
    k_bool use_cfg;
    k_bool use_cfg_ext;
    k_bool reset;
    k_bool support;
    k_bool status;
    k_bool auto_level;
    k_isp_config_mode mode;
} k_isp_module_cfg;


/**
 * @brief Defines the submodule config of isp
 *
 */
typedef struct {
    k_isp_module_cfg ae;
    k_isp_module_cfg awb;
    k_isp_module_cfg af;
    k_isp_module_cfg wb;
    k_isp_module_cfg wdr;
    k_isp_module_cfg hdr;
    k_isp_module_cfg dnr2;
    k_isp_module_cfg dnr3;
    k_isp_module_cfg bls;
    k_isp_module_cfg cac;
    k_isp_module_cfg cnr;
    k_isp_module_cfg lsc;
    k_isp_module_cfg dmsc;
    k_isp_module_cfg rgbir;
    k_isp_module_cfg ee;
    k_isp_module_cfg gc;
    k_isp_module_cfg ge;
    k_isp_module_cfg ca;
    k_isp_module_cfg dci;
    k_isp_module_cfg dpcc;
    k_isp_module_cfg dpf;
    k_isp_module_cfg ccm;
    k_isp_module_cfg cpd;
    k_isp_module_cfg filter;
    k_isp_module_cfg dg;
    k_isp_module_cfg pdaf;
    k_isp_module_cfg cproc;
    k_isp_module_cfg gtm;
    k_isp_module_cfg ynr;
    k_isp_module_cfg lut3d;
    k_isp_module_cfg exp;
    k_isp_module_cfg afm;
} k_isp_submodule_cfg;

/**
 * @brief Defines the attributes of a ISP channel
 *
 */
typedef struct {
    k_vicap_window out_win;
    k_vicap_window crop_win;
    k_vicap_window scale_win;

    k_isp_output_bit_width      bit_width;
    k_isp_output_pix_format     pix_format;

    k_u32 buffer_hold;
    k_u32 buffer_num;
    k_u32 buffer_size;
    k_u64 *vb_phys_addr;
    void **vb_virt_addr;

    k_u32 yraw_size;
    k_u32 uv_size;
    k_u32 v_size;
    k_isp_buf_block_type block_type;
    k_u32 wait_time;

    k_bool crop_enable;
    k_bool scale_enable;
    k_bool dw_enable __attribute__((deprecated));
    k_bool chn_enable;
    k_u8 alignment;
} k_isp_chn_attr;

/**
 * @brief Defines the attributes of a ISP device
 *
 */
typedef struct {
    k_vicap_window acq_win;
    k_vicap_window is_win;
    k_vicap_isp_pipe_ctrl pipe_ctrl;
    k_isp_submodule_cfg module_cfg;
    k_isp_work_mode work_mode;
    k_isp_input_type input_type;
    k_sensor_mode sensor_mode;
    k_s32 sensor_fd;
    char sensor_name[32];
    k_bool dw_enable;
    k_bool dev_enable;
    k_u32 buffer_num;
    k_u32 buffer_size;
    k_u64 *vb_phys_addr;
    void **vb_virt_addr;
    // k_vicap_mirror mirror;
} k_isp_dev_attr;

typedef struct {
    k_u16 hOffset;           /**< Horizontal start offset */
    k_u16 vOffset;           /**< Vertical start offset */
    k_u16 width;             /**< Width */
    k_u16 height;            /**< Height */
} k_isp_window;

typedef struct {
    k_isp_window window;     /**< Roi window */
    float weight;     /**< Weight */
} k_isp_roi_windows;

typedef struct {
    k_u8 roiNum;
    float                roiWeight;
    k_isp_roi_windows roiWindow[ISP_AE_ROI_WINDOWS_MAX];  /**< ROI windows */
} k_isp_ae_roi;

typedef struct {
    k_u32 roi_count;
    k_u32 roi_count_flag;
} k_isp_ae_roi_state;

/*
#define K_ERR_ISP_INVALID_DEVID     K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_ISP_INVALID_CHNID     K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_ISP_ILLEGAL_PARAM     K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_ISP_EXIST             K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_ISP_UNEXIST           K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_ISP_NULL_PTR          K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_ISP_NOT_CONFIG        K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_ISP_NOT_SUPPORT       K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_ISP_NOT_PERM          K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_ISP_NOMEM             K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_ISP_NOBUF             K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_ISP_BUF_EMPTY         K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_ISP_BUF_FULL          K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_ISP_NOTREADY          K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_ISP_BADADDR           K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_ISP_BUSY              K_DEF_ERR(K_ID_ISP, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
*/

/** @} */ /** <!-- ==== ISP End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
