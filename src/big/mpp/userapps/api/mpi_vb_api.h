/**
 * @file mpi_vb_api.h
 * @author  ()
 * @brief  Defines APIs related to VB pool used
 * @version 1.0
 * @date 2022-09-06
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
#ifndef __MPI_VB_API_H__
#define __MPI_VB_API_H__

#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

/**
 * @brief Creates a VB pool
 *
 * @param [in] config Pointer to the configuration attribute parameter of the VB pool
 * @return k_s32
 * @retval VB_INVALID_POOLID A VB pool fails to be created
 * @retval "Other value" valid pool ID
 */
k_s32 kd_mpi_vb_create_pool(k_vb_pool_config *config);

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
k_s32 kd_mpi_vb_destory_pool(k_u32 pool_id);

/**
 * @brief User state get a vb block
 *
 * @param [in] pool_id VB pool ID value
 * @param [in] blk_size Buffer size
 * @param [in] mmz_name Name of the DDR SDRAM where a VB pool is located
 * @return k_vb_blk_handle
 * @retval "Other value" valid video block handle
 * @retval VB_INVALID_HANDLE A buffer fails to be get
 * @note
 * - After creating a VB pool, you can call this MPI to get a buffer
 *   from the VB pool. That is, you can set the first parameter Pool to
 *   the ID of the created VB pool, and the second parameter blk_size to the
 *   desired buffer size. Note that  the buffer size must be less than or equal to the
 *   size of the VB specified when  the VB pool is created. The
 *   mmz_name parameter is invalid when a buffer is obtained from a specified VB
 *   pool
 * - If you want to get a buffer with a specified size from a common VB pool,
 *   you can set the first parameter Pool to an invalid ID (VB_INVALID_POOLID), set the
 *   second parameter blk_size to a required size of the buffer, and specify the DDR SDRAM
 *   from whose common VB pools that buffers are obtained. If the specified
 *   DDR SDRAM does not have common VB pool, no buffer is obtained. If mmz_name
 *   is NULL, it indicates that buffers are obtained in the common VB pool of the
 *   anonymous DDR SDRAM.
 * - Occupying the public VB pool at the user level will result in a reduction of
 *   the public VB pool available to the kernel, so please use this MPI with
 *   caution
 */
k_vb_blk_handle kd_mpi_vb_get_block(k_u32 pool_id, k_u64 blk_size, const k_char *mmz_name);

/**
 * @brief Releases a vb block
 *
 * @param [in] block VB block handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @see kd_mpi_vb_destory_pool
 */
k_s32 kd_mpi_vb_release_block(k_vb_blk_handle block);

/**
 * @brief Get the VB block handle to the physical address of a buffer.
 *
 * @param [in] phys_addr  Physical address of a buffer
 * @return k_vb_blk_handle
 * @retval other  valid video block handle
 * @retval VB_INVALID_HANDLE A buffer fails to be get
 * @note The physical address must be the address of a valid buffer obtained from an
 *      MPP VB pool
 */
k_vb_blk_handle kd_mpi_vb_phyaddr_to_handle(k_u64 phys_addr);

/**
 * @brief Get the physical address of a vb block
 *
 * @param [in] block VB block handle
 * @return k_u64
 * @retval 0 Invalid physical address
 * @retval "not 0" valid physical address
 * @note The specified buffer must be a valid one obtained from a MPP VB pool
 */
k_u64 kd_mpi_vb_handle_to_phyaddr(k_vb_blk_handle block);

/**
 * @brief Gets the ID of the buffer pool where a vb block is located
 *
 * @param [in] block VB block handle
 * @return k_s32
 * @retval VB_INVALID_POOLID invalid pool ID
 * @retval "Other Value" valid pool ID
 * @note The specified buffer must be a valid one obtained from a MPP VB pool
 */
k_s32 kd_mpi_vb_handle_to_pool_id(k_vb_blk_handle block);

/**
 * @brief Inquire how many users are using the buffer
 *
 * @param [in] block VB block handle
 * @return k_s32
 * @retval K_FAILED Failed to get user counts
 * @retval "Other value" Count of used buffers
 */
k_s32 kd_mpi_vb_inquire_user_cnt(k_vb_blk_handle block);

/**
 * @brief Obtains the auxiliary information of the VB memory
 *
 * @param [in] block Handle to the buffer
 * @param [out] supplement Auxiliary information of the VB memory, such as  DCF information
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_vb_get_supplement_attr(k_vb_blk_handle block, k_video_supplement *supplement);

/**
 *
 * @brief Sets the additional information of the VB memory
 * @pre
 * <span style="color: green">Some information needs to be added at the end of the VB memory,
 * such as the DCF information, ISP statistics, and some real-time ISP parameters.
 * The additional information can be transferred among MPP modules along with the
 * VB memory. If the VB memory is obtained in user mode,
 * the additional information can be obtained as well</span>
 *
 * @param [in] supplement_config Structure for controlling the additional
 *                              information of the VB , which is used to
 *                              allocate memory for the additional information
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - The following four types of additional information of the VB are supported
 *   currently:
 *   -# DCF information, corresponding to the structure k_jpeg_dcf. The corresponding
 *      mask is VB_SUPPLEMENT_JPEG_MASK
 *   -# Real-time ISP information, corresponding to the structure k_isp_frame_info.
 *      The corresponding mask is VB_SUPPLEMENT_ISPINFO_MASK
 * - This MPI needs to be called before ::kd_mpi_vb_init is called so that the
 *   auxiliary information takes effect.
 * @todo Need to describe the order in which this MPI is called with ISP-related MPIs
 */
k_s32 kd_mpi_vb_set_supplement_config(const k_vb_supplement_config *supplement_config);

/**
 * @brief Obtains the additional information of the VB memory
 *
 * @param [out] supplement_config Structure for controlling the additional information of the
 *                  VB memory, which is used to allocate memory for the additional information
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_vb_get_supplement_config(k_vb_supplement_config *supplement_config);

/**
 * @brief Initializes VB pool
 *
 * @return k_s32
 * @note
 * - Before initializing a VB pool, you must set the attributes of the VB pool by calling
 *   ::kd_mpi_vb_set_config. Otherwise, the VB pool fails to be initialized
 * - This MPI can be called repeatedly, and a code indicating success is returned.
 * @see kd_mpi_vb_exit
 */
k_s32 kd_mpi_vb_init(void);

/**
 * @brief Deinitializes VB pool.
 *
 * @return k_s32
 * @note
 * - This MPI can be called repeatedly, and a code indicating success is returned
 * - After the VB pool is deinitialized, the configurations of the VB pool are retained
 * - Before exiting the VB pool, ensure that no VB in the VB pool is occupied. Otherwise,
 *   exit fails
 * @see kd_mpi_vb_init
 */
k_s32 kd_mpi_vb_exit(void);

/**
 * @brief Set the configs of VB pool
 *
 * @param [in] config Pointer to VB pool attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - The attributes of a VB pool can be set only before the system is initialized.
 *   Otherwise, an error code indicating failure is returned.
 * - The size of each buffer in the common VB pool varies according to the current picture
 *   pixel format and the picture compression mode (compressed or uncompressed). For
 *   details about the allocated size, see the description in k_vb_config
 * @see kd_mpi_vb_get_config
 */
k_s32 kd_mpi_vb_set_config(const k_vb_config *config);

/**
 * @brief Get the configs of VB pool
 *
 * @param [out] config
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note Before obtaining the attributes of VB pool, you must set the attributes of the VB
 * pool by ::kd_mpi_vb_set_config
 */
k_s32 kd_mpi_vb_get_config(k_vb_config *config);

/**
 * @brief Initializes a module common VB pool.
 *
 * @param [in] vb_uid ID of the module that uses the module common VB pool
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 *  - Before calling this MPI, you must initialize the common VB pool by calling
 *    ::kd_mpi_vb_init
 *  - Before initializing a common VB pool, you must set its attributes by calling
 *    ::kd_mpi_vb_set_mod_pool_config Otherwise, the VB pool fails to be initialized
 *  - This MPI can be called repeatedly, and no error code indicating failure is returned
 */
k_s32 kd_mpi_vb_init_mod_common_pool(k_vb_uid vb_uid);

/**
 * @brief Exits a module common VB pool
 *
 * @param [in] vb_uid
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - Call this MPI before calling ::kd_mpi_vb_exit. Otherwise, an error code
 *   indicating failure is returned
 * - this MPI can be called repeatedly, and no error code indicating failure is returned
 * - After a module common VB pool is exited, its settings are cleared.
 * - Before exiting the VB pool, ensure that no VB in the VB pool is occupied. Otherwise,
 *   exit fails
 * @see kd_mpi_vb_init_mod_common_pool
 */
k_s32 kd_mpi_vb_exit_mod_common_pool(k_vb_uid vb_uid);

/**
 * @brief Sets the attributes of a module common VB pool
 *
 * @param [in] vb_uid ID of the module that uses the module common VB pool
 * @param [in] config Pointer to the attributes of a module common VB pool
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - The module common VB pool must be configured as required. Otherwise, the memory is
 *   not sufficiently used
 * - If a VB has been created for the module, the error code K_ERR_VB_BUSY is returned
 *   when the VB is configured again.
 */
k_s32 kd_mpi_vb_set_mod_pool_config(k_vb_uid vb_uid, const k_vb_config *config);

/**
 * @brief Obtains the attributes of a module common VB pool
 *
 * @param [in] vb_uid ID of the module that uses the module common VB pool
 * @param [out] config Pointer to the attributes of a module common VB pool
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note Before calling this MPI, you must set the attributes of a common VB pool by
 * calling ::kd_mpi_vb_set_mod_pool_config
 * @see kd_mpi_vb_set_mod_pool_config
 */
k_s32 kd_mpi_vb_get_mod_pool_config(k_vb_uid vb_uid, k_vb_config *config);

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif