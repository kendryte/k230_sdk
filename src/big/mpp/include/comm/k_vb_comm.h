/**
 * @file k_vb_comm.h
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
#ifndef __K_VB_COMM_H__
#define __K_VB_COMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

#include "k_errno.h"
#include "k_module.h"
#include "k_type.h"

#define MAX_MMZ_NAME_LEN               16

#define VB_INVALID_POOLID              (-1U)
#define VB_INVALID_HANDLE              (-1U)

#define MAX_VB_BLK_SIZE         0xFFFF0000UL

#define VB_SINGLE_MAX_BLKS      256  /**<The maximum number of block a VB pool can have. FIEXD!*/
#define VB_MAX_POOLS            256  /**< Defines the maximum number of VB pools. FIEXD! */
#define VB_MAX_COMM_POOLS       16   /**< Maximum number of common VB pools. FIEXD! */
#define VB_MAX_MOD_COMM_POOLS   16   /**< Maximum number of module VB pools. FIEXD! */
#define VB_MAX_BLKS             (VB_MAX_POOLS * VB_SINGLE_MAX_BLKS) /**< Defines the maximum number of VB blocks. FIEXD! */

#define VB_MAX_USER             VB_UID_BUTT     /**< The module ID for using the VB pool must be less than the value defined by this macro*/

/**
 * @brief Defines the enumeration type of the module ID that uses the VB pool.
 *
 */
typedef enum
{
    VB_UID_VI = 0,
    VB_UID_VENC = 1,
    VB_UID_VDEC = 2,
    VB_UID_VO = 3,
    VB_UID_USER = 4,
    VB_UID_AI = 5,
    VB_UID_AREC = 6,
    VB_UID_AENC = 7,
    VB_UID_ADEC = 8,
    VB_UID_AO = 9,
    VB_UID_V_VI = 10, /**< virtual vi user id for vb*/
    VB_UID_V_VO = 11,   /** < virtual vo user id for vb*/
    VB_UID_DMA  = 12,
    VB_UID_DPU  = 13,
    VB_UID_NONAI_2D  = 14,
    VB_UID_BUTT = 15,
} k_vb_uid;

#define POOL_OWNER_COMMON    (-1) /**< Generall common pool use this owner id, module common pool use VB_UID as owner id*/
#define POOL_OWNER_PRIVATE   (-2) /**< Private pool use this owner id */

/**
 * @brief Define VB kernel state virtual address mapping mode
 *
 */
typedef enum
{
    VB_REMAP_MODE_NONE = 0, /**< no remap */
    VB_REMAP_MODE_NOCACHE = 1, /**< no cache remap */
    VB_REMAP_MODE_CACHED = 2, /**< cache remap, if you use this mode, you should flush cache by yourself */
    VB_REMAP_MODE_BUTT
} k_vb_remap_mode;

/**
 * @brief Defines the VB pool attributes.
 *
 * @note
 * - blk_size of each VB should be calculated based on the image width and height, pixel
 *   format, data bit width, and the compression status
 * - The memory for a VB pool sources from an idle MMZ. A VB pool consists of multiple
 *   buffers of the same size. If the VB pool to be created is larger than the idle memory, the
 *   VB pool fails to be created.
 * - Ensure that the entered name of the DDR SDRAM exists. Otherwise, the memory fails to
 *   be allocated. If the array mmz_name is memset to 0, the VP pool is created in an
 *   anonymous DDR SDRAM
 * @see kd_mpi_vb_create_pool
 */
typedef struct
{
    k_u64 blk_size; /**< Size of each VB block*/
    k_u32 blk_cnt; /**< Number of blocks*/
    k_vb_remap_mode mode; /**< Mapping mode of the kernel mode virtual addresses of the VB*/
    char mmz_name[MAX_MMZ_NAME_LEN];/**< Name of the MMZ that allocates the memory for the current VB pool*/
} k_vb_pool_config;

/**
 * @brief Defines attributes of the VB pool.
 *
 * @note The entire structure must be cleared to 0 before assigning a value
 *
 * @see kd_mpi_vb_set_config
 */
typedef struct
{
    k_u32 max_pool_cnt; /**< max count of pools [0, ::VB_MAX_POOLS]*/
    k_vb_pool_config comm_pool[VB_MAX_COMM_POOLS]; /**< Attributes of the common VB pool*/
} k_vb_config;

/**
 * @brief Defines attributes of the mod VB pool
 *
 */
typedef struct
{
    k_vb_uid uid;   /**< ID of the module that uses the module common VB pool*/
    k_vb_config mod_config; /**< :: k_vb_config*/
} k_vb_mod_config;

/**
 * @brief VB pool infomation
 *
 */
typedef struct
{
    k_u32 pool_id; /**< pool Id*/
    k_u32 blk_cnt; /**< Number of blocks*/
    k_u64 blk_size; /**< Size of each VB block*/
    k_u64 pool_size; /**< Size of VB pool*/
    k_u64 pool_phys_addr; /**< Physical address of VB pool*/
    k_vb_remap_mode remap_mode; /**< Mapping mode of the kernel mode virtual addresses of the VB*/
    void *pool_kvirt_addr; /**< kernel virtual address of VB pool*/
    char mmz_name[MAX_MMZ_NAME_LEN]; /**< Name of the MMZ where the current VB POOL is located */
} k_vb_pool_info;


#define VB_SUPPLEMENT_JPEG_MASK        (1U << 0) /** <JPEG DCF mask for the auxiliary VB information */
#define VB_SUPPLEMENT_ISP_INFO_MASK    (1U << 1) /** <ISP INFO mask for the VB information  */
#define VB_SUPPLEMENT_UNSPPORT_MASK    ~(VB_SUPPLEMENT_JPEG_MASK | VB_SUPPLEMENT_ISP_INFO_MASK)

/**
 * @brief Defines the structure of the additional VB information.
 *
 * @note Four types of additional information are supported currently. For details, see the description of
 * ::kd_mpi_vb_set_supplement_config
 * @see kd_mpi_vb_get_supplement_config
 */
typedef struct
{
    k_u32 supplement_config; /**<Control of the auxiliary information*/
} k_vb_supplement_config;

/**
 * @brief Current status of VB pool
 *
 */
typedef struct
{
    k_bool is_comm_pool; /** <Is it a common pool*/
    k_u32 blk_cnt; /**< Number of blocks*/
    k_u32 free_blk_cnt; /**< Number of remaining available blcok*/
} k_vb_pool_status;

/**
 * @note high 16 bit is pool id, low 16bit is blk id
 *
 */
typedef k_u32 k_vb_blk_handle;

#define K_ERR_VB_NULL_PTR             K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_VB_NOMEM                K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_VB_NOBUF                K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_VB_UNEXIST              K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_VB_ILLEGAL_PARAM        K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_VB_NOTREADY             K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_VB_BUSY                 K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_ERR_VB_NOT_PERM             K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_VB_2MPOOLS              K_DEF_ERR(K_ID_VB, K_ERR_LEVEL_ERROR, K_ERR_BUTT + 1)

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
