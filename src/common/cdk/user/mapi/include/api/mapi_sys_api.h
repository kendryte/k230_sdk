/**
 * @file mapi_sys_api.h
 * @author  ()
 * @brief mapi of sys module
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
#ifndef __MAPI_SYS_H__
#define __MAPI_SYS_H__

/** \addtogroup     MAPI_SYSTEM_CTRL*/
/** @{ */  /** <!-- [MAPI_SYSTEM_CTRL] */

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "k_vb_comm.h"

#define K_MAPI_ERR_SYS_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_SYS, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_SYS_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_SYS, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_SYS_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_SYS, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_SYS_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_SYS, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_SYS_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_SYS, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_SYS_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_SYS, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** media config param */
typedef struct {
    k_vb_supplement_config vb_supp;
    k_vb_config vb_config;
} k_mapi_media_config_t;

/** media init attribute */
typedef struct {
    k_mapi_media_config_t media_config;
} k_mapi_media_attr_t;

/**
 * @brief Initialize the system resources and establish the message
 * communication pipeline between the dual cores.
 * In order to establish a dual-core connection, the services running
 * on each OS need to call this interface during  initialization to
 * establish the connection before they can do inter-core communication
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note This function needs to be called before the ::kd_mapi_media_init call
 */
k_s32 kd_mapi_sys_init(void);

/**
 * @brief Disconnect the dual-core communication
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note When ::kd_mapi_sys_init is not called,
 * the call to this mapi returns success
 */
k_s32 kd_mapi_sys_deinit(void);

/**
 * @brief Initialize multimedia resources, currently mainly initialize vb module
 *
 * @param [in] media_attr Pointer to the media attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_media_init(const k_mapi_media_attr_t *media_attr);

/**
 * @brief Deinitializes the resources of the media-related modules.
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_media_deinit(void);

/**
 * @brief Creates a VB pool
 *
 * @param [in] config Pointer to the configuration attribute parameter of the VB pool
 * @return k_s32
 * @retval VB_INVALID_POOLID A VB pool fails to be created
 * @retval "Other value" valid pool ID
 */
k_s32 kd_mapi_vb_create_pool(k_vb_pool_config *config);

/**
 * @brief Destroys a VB pool
 *
 * @param [in] pool_id VB pool ID Value
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - If the VB pool to be destroyed does not exist, K_ERR_VB_UNEXIST is returned.
 * - Only the pool created by ::kd_mpi_vb_create_pool can be destroyed
 */
k_s32 kd_mapi_vb_destory_pool(k_u32 pool_id);

/**
 * @brief Allocates the MMZ memory in user mode(cached).
 *
 * @param [out] phys_addr Physical address of a buffer
 * @param [out] virt_addr Virtual address of a buffer
 * @param [in] len Buffer length
 * @param [in] name Buffer name
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - The virtual address and physical address of the buffer have been mapped
 * - After the memory is allocated, run cat /proc/media-mem to check whether the
 *   buffer is successfully allocated
 */
k_s32 kd_mapi_alloc_buffer(k_u64 *phys_addr, void **virt_addr, k_u32 len, const k_char *name);

/**
 * @brief Releases the MMZ memory in user mode.
 *
 * @param [in] phys_addr Physical address of a buffer
 * @param [in] virt_addr Virtual address of a buffer
 * @param [in] len Buffer length
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note After the memory is released, run cat /proc/media-mem to check whether the buffer is
 *       successfully released.
 *
 */
k_s32 kd_mapi_free_buffer(k_u64 phys_addr, void *virt_addr, k_u32 len);

/**
 * @brief Obtains a VB block in user mode
 *
 * @param [out] pool_id ID of the VB pool where the VB block resides
 * @param [out] phys_addr Physical address of the VB block
 * @param [in] blk_size Size of the VB block
 * @param [in] mmz_name Name of the DDR SDRAM where a VB pool resides
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - To obtain a VB block of a specified size from any common VB pool, set the second
 *   parameter blkSize to the required VB block size and specify the DDR SDRAM from
 *   which the VB block is obtained. If the specified DDR SDRAM does not have a common
 *   VB pool, no VB block can be obtained. If mmzName is NULL, it indicates that VB
 *   blocks are obtained in the common VB pool of the DDR SDRAM without an existing
 *   name.
 * - During media initialization, if a VB pool is created in an unnamed DDR SDRAM, the
 *   VB block of a specified size is obtained from any common VB pool. In this case, set
 *   mmzName to NULL
 */
k_s32 kd_mapi_sys_get_vb_block(k_u32 *pool_id, k_u64 *phys_addr, k_u64 blk_size, const char* mmz_name);

/**
 * @brief Obtains a VB block in user mode
 *
 * @param [in] pool_id ID of the VB pool where the VB block resides
 * @param [out] phys_addr Physical address of the VB block
 * @param [in] blk_size Size of the VB block
 * @param [in] mmz_name Name of the DDR SDRAM where a VB pool resides
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - To obtain a VB block of a specified size from any common VB pool, set the second
 *   parameter blkSize to the required VB block size and specify the DDR SDRAM from
 *   which the VB block is obtained. If the specified DDR SDRAM does not have a common
 *   VB pool, no VB block can be obtained. If mmzName is NULL, it indicates that VB
 *   blocks are obtained in the common VB pool of the DDR SDRAM without an existing
 *   name.
 * - During media initialization, if a VB pool is created in an unnamed DDR SDRAM, the
 *   VB block of a specified size is obtained from any common VB pool. In this case, set
 *   mmzName to NULL
 */
k_s32 kd_mapi_sys_get_vb_block_from_pool_id(k_u32 pool_id, k_u64 *phys_addr, k_u64 blk_size, const char* mmz_name);

/**
 * @brief Releases a VB block in user mode
 *
 * @param [in] phys_addr Physical address of the VB block
 * @param [in] blk_size Size of the VB block
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note If obtained VB blocks are used up, this MAPI should be called to release VB blocks
 */
k_s32 kd_mapi_sys_release_vb_block(k_u64 phys_addr,  k_u64 blk_size);

/** @} */ /** <!-- ==== MAPI_SYSTEM_CTRL End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_SYS_API_H__ */

