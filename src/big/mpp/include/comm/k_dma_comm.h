/**
 * @file k_dma_comm.h
 * @author
 * @brief
 * @version 1.0
 * @date 2022-09-07
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
#ifndef __K_DMA_COMM_H__
#define __K_DMA_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     DMA */
/** @{ */ /** <!-- [DMA] */

#define DMA_INT_NUM             (16+124)

#define DMA_MAX_DEV_NUMS        (1)     /**< The maximum number of the DMA device. FIXED! */
#define GDMA_MAX_CHN_NUMS       (4)     /**< The maximum number of the GDMA channel. FIXED! */
#define SDMA_MAX_CHN_NUMS       (4)     /**< The maximum number of the SDMA channel. FIXED！ */
#define DMA_MAX_CHN_NUMS        (GDMA_MAX_CHN_NUMS + SDMA_MAX_CHN_NUMS) /**< The maximum number of the DMA channel. FIXED! */
#define DMA_LINKLIST_BYTE       (24)    /**< The number of bytes occupied by a linked list of DMA. */


/**
 * @brief Defines the working mode of the dma
 *
 */
typedef enum
{
    DMA_BIND,           /**< DMA binding mode */
    DMA_UNBIND,         /**< DMA unbinding MODE*/
} k_dma_mode_e;

/**
 * @brief Defines the rotation mode of the gdma
 *
 */
typedef enum
{
    DEGREE_0,       /**< Rotate 0 degrees */
    DEGREE_90,      /**< Rotate 90 degrees */
    DEGREE_180,     /**< Rotate 180 degrees */
    DEGREE_270,     /**< Rotate 270 degrees */
} k_gdma_rotation_e;

/**
 * @brief Defines the 1d or 2d mode for sdma
 *
 */
typedef enum
{
    DIMENSION1,     /**< One dimensional mode */
    DIMENSION2,     /**< Two dimensional mode */
} k_sdma_data_mode_e;

typedef enum
{
    DMA_PIXEL_FORMAT_RGB_444 = 0,
    DMA_PIXEL_FORMAT_RGB_555,
    DMA_PIXEL_FORMAT_RGB_565,
    DMA_PIXEL_FORMAT_RGB_888,

    DMA_PIXEL_FORMAT_BGR_444,
    DMA_PIXEL_FORMAT_BGR_555,
    DMA_PIXEL_FORMAT_BGR_565,
    DMA_PIXEL_FORMAT_BGR_888,

    DMA_PIXEL_FORMAT_ARGB_1555,
    DMA_PIXEL_FORMAT_ARGB_4444,
    DMA_PIXEL_FORMAT_ARGB_8565,
    DMA_PIXEL_FORMAT_ARGB_8888,

    DMA_PIXEL_FORMAT_ABGR_1555,
    DMA_PIXEL_FORMAT_ABGR_4444,
    DMA_PIXEL_FORMAT_ABGR_8565,
    DMA_PIXEL_FORMAT_ABGR_8888,

    DMA_PIXEL_FORMAT_YVU_PLANAR_420_8BIT,       // add 8BIT
    DMA_PIXEL_FORMAT_YVU_PLANAR_420_10BIT,      //
    DMA_PIXEL_FORMAT_YVU_PLANAR_420_16BIT,      //
    DMA_PIXEL_FORMAT_YVU_PLANAR_444_8BIT,       // add 8BIT
    DMA_PIXEL_FORMAT_YVU_PLANAR_444_10BIT,      //

    DMA_PIXEL_FORMAT_YUV_PLANAR_420_8BIT,       //
    DMA_PIXEL_FORMAT_YUV_PLANAR_420_10BIT,      //
    DMA_PIXEL_FORMAT_YUV_PLANAR_420_16BIT,      //

    DMA_PIXEL_FORMAT_YVU_SEMIPLANAR_420_8BIT,   // add 8BIT
    DMA_PIXEL_FORMAT_YVU_SEMIPLANAR_420_10BIT,  //
    DMA_PIXEL_FORMAT_YVU_SEMIPLANAR_420_16BIT,  //

    DMA_PIXEL_FORMAT_YUV_SEMIPLANAR_420_8BIT,   // add 8BIT
    DMA_PIXEL_FORMAT_YUV_SEMIPLANAR_420_10BIT,  //
    DMA_PIXEL_FORMAT_YUV_SEMIPLANAR_420_16BIT,  //

    DMA_PIXEL_FORMAT_YUV_400_8BIT,              /// 8BIT
    DMA_PIXEL_FORMAT_YUV_400_10BIT,             ///
    DMA_PIXEL_FORMAT_YUV_400_12BIT,             ///
    DMA_PIXEL_FORMAT_YUV_400_16BIT,             ///

    DMA_PIXEL_FORMAT_YUV_PACKED_444_8BIT,       ///
    DMA_PIXEL_FORMAT_YUV_PACKED_444_10BIT,      ///

    /* SVP data format */
    DMA_PIXEL_FORMAT_BGR_888_PLANAR,
} k_pixel_format_dma_e;

/**
 * @brief Defines the attributes of the gdma channel
 *
 */
typedef struct
{
    k_u8 buffer_num;                        /**< Number of channel buffers */
    k_gdma_rotation_e rotation;             /**< Rotate mode */
    k_bool x_mirror;                        /**< Horizontal mirror */
    k_bool y_mirror;                        /**< Vertical mirror */
    k_u16 width;                            /**< Image width in pixels */
    k_u16 height;                           /**< Image height in pixels */
    k_u16 src_stride[3];                    /**< Source stride */
    k_u16 dst_stride[3];                    /**< Destination address */
    k_dma_mode_e work_mode;                 /**< Binding or unbound mode */
    k_pixel_format_dma_e pixel_format;      /**< Pixel width */
} k_gdma_chn_attr_t;

/**
 * @brief Defines the attributes of the sdma channel
 *
 */
typedef struct
{
    k_u8 buffer_num;                        /**< Number of channel buffers */
    k_u32 line_size;                        /**< In 1d mode, it represents the total data length; In 2d mode, it represents the length of a single row of data. */
    k_u16 line_num;                         /**< In 1d mode, it is invalid; In 2d mode, it represents the number of rows. */
    k_u16 line_space;                       /**< In 1d mode, it is invalid; In 2d mode, it represents the space between rows. */
    k_sdma_data_mode_e data_mode;           /**< 1d mode or 2d mode. */
    k_dma_mode_e work_mode;                 /**< Binding or unbound mode. */
} k_sdma_chn_attr_t;

/**
 * @brief Defines the attributes of the dma channel
 *
 */
typedef union
{
    k_gdma_chn_attr_t gdma_attr;            /**< gdma attribute, i.e. attributes of channel 0~3 */
    k_sdma_chn_attr_t sdma_attr;            /**< sdma attribute, i.e. attributes of channel 4~7 */
} k_dma_chn_attr_u;

/**
 * @brief Define the attributes of the dma device
 */
typedef struct
{
    k_u8 burst_len;                         /**< burst length of the dma device */
    k_u8 outstanding;                       /**< outstanding of the dma device */
    k_bool ckg_bypass;                      /**< clock gate bypass of the dma device */
} k_dma_dev_attr_t;

typedef struct
{
    k_u32 time_ref;
    k_u64 pts;                              /**< time stamp */
    k_u32 in_pool_id;                       /**< Input vb pool id */
    k_u32 out_pool_id;                      /**< Output vb pool id */
    k_u64 virt_src_addr[3];                 /**< virtual source address */
    k_u64 phys_src_addr[3];                 /**< physical source address */
    k_u64 virt_dst_addr[3];                 /**< virtual destination address */
    k_u64 phys_dst_addr[3];                 /**< physical destination address */
    k_video_frame_info vf_info;             /**< video information */
} k_dma_frame_info;

#define K_ERR_DMA_INVALID_DEVID     K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_DMA_INVALID_CHNID     K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_DMA_ILLEGAL_PARAM     K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_DMA_EXIST             K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_DMA_UNEXIST           K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_DMA_NULL_PTR          K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_DMA_NOT_CONFIG        K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_DMA_NOT_SUPPORT       K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_DMA_NOT_PERM          K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_DMA_NOMEM             K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_DMA_NOBUF             K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_DMA_BUF_EMPTY         K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_DMA_BUF_FULL          K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_DMA_NOTREADY          K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_DMA_BADADDR           K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_DMA_BUSY              K_DEF_ERR(K_ID_DMA, K_ERR_LEVEL_ERROR, K_ERR_BUSY)

/** @} */ /** <!-- ==== DMA End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif