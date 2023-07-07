/**
 * @file mpi_sys_api.h
 * @author
 * @brief Defines APIs related to memory mmz requests and sys bindings
 * @version 1.0
 * @date 2022-08-31
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
#ifndef __MPI_SYS_API_H__
#define __MPI_SYS_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

#include "k_type.h"
#include "k_mmz_comm.h"
#include "k_sys_comm.h"
#include "k_log_comm.h"

/**
 * @brief Allocate MMZ memory in the user space
 *
 * @param [out] phy_addr Physical address pointer
 * @param [out] virt_addr Pointer to userspace virtual address pointer
 * @param [in] mmb String pointer to the name of the Mmb
 * @param [in] zone String pointer to MMZ zone name
 * @param [in] len Memory block size
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note The MMZ consists of multiple zones and each zone has multiple MMBs. You can call this
 *       MPI to allocate a memory block *mmb with the size of len in the *zone
 *       of the MMZ. In this case, the pointers that point to the physical address and user-mode virtual
 *       address are returned. If there is the anonymous zone in the MMZ, set *zone to null. If
 *        *mmb is set to null, the created MMB is named null.
 */
k_s32 kd_mpi_sys_mmz_alloc(k_u64 *phy_addr, void **virt_addr, const k_char *mmb, const k_char *zone, k_u32 len);

/**
 * @brief Allocates the MMZ supporting cache in the user space.
 *
 * @param [out] phy_addr Physical address pointer
 * @param [out] virt_addr Pointer to userspace virtual address pointer
 * @param [in] mmb String pointer to the name of the Mmb
 * @param [in] zone String pointer to MMZ zone name
 * @param [in] len Memory block size
 * @return k_s32 0:success "not 0" failed
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - The differences between kd_mpi_sys_mmz_alloc_cached and kd_mpi_sys_mmz_alloc are as follows:
 *  -# The memory that is allocated by calling kd_mpi_sys_mmz_alloc_cached supports cache.
 *  -# kd_mpi_sys_mmz_alloc_cached is recommended if the memory to be allocated will be frequently used. This improves the
 *     CPU read/write efficiency and system performance.
 * - When the CPU accesses the memory that is allocated by calling
 *   kd_mpi_sys_mmz_alloc_cached, the data in the memory will be stored in the cache.
 *   The hardware devices can access only the physical memory rather than
 *   the cache. In this case, ::kd_mpi_sys_mmz_flush_cache needs to be called to synchronize
 *   data.
 */
k_s32 kd_mpi_sys_mmz_alloc_cached(k_u64 *phy_addr, void **virt_addr, const k_char *mmb, const k_char *zone, k_u32 len);

/**
 * @brief Maps the memory storage address no cache
 *
 * @param [in] phy_addr Start address of the memory to be mapped
 * @param [in] size Number of mapped bytes
 * @return void*
 * @retval 0 Invalid address
 * @retval "not 0" Valid address
 * @note
 * - Only physical memory regions requested by the underlying using MMZ can be mapped.
 * - For memory areas belonging to VB, the mapping size cannot exceed the size of VB POOL.
 */
void *kd_mpi_sys_mmap(k_u64 phy_addr, k_u32 size);

/**
 * @brief Maps the memory storage address cached
 *
 * @param [in] phy_addr Start address of the memory to be mapped
 * @param [in] size Number of mapped bytes
 * @return void*
 * @retval 0 Invalid address
 * @retval "not 0" Valid address
 * @note
 * - Only physical memory regions requested by the underlying using MMZ can be mapped.
 * - For memory areas belonging to VB, the mapping size cannot exceed the size of VB POOL.
 */
void *kd_mpi_sys_mmap_cached(k_u64 phy_addr, k_u32 size);

/**
 * @brief munmap Unmaps the storage address
 *
 * @param [in] virt_addr Address returned after mmap is called
 * @param [in] size Length of mapped area, in bytes
 * @retval 0 success
 * @retval "not 0" failed
 */
k_s32 kd_mpi_sys_munmap(void *virt_addr, k_u32 size);

/**
 * @brief Copies data from the cache to the MMZ and invalidates data in the cache
 *
 * @param [in] phy_addr Start physical address for storing the data to be used
 * @param [in] virt_addr  Pointer to the start virtual address for storing the data to
 *                        be used. The value cannot be null
 * @param [in] size Amount of the data to be used
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - If the data in the cache is the latest data, you need to call this MPI to synchronize the
 *   data to the memory. This ensures that the hardware that cannot directly access the cache
 *   can obtain correct data.
 * - This MPI must be called if kd_mpi_sys_mmz_alloc_cached is called
 * - If phy_addr is set to 0, the entire cache is operated
 * - You need to ensure that the transferred parameters are valid
 * - Ensure that kd_mpi_sys_mmz_free is not called to release the memory being flushed
 *       when you are performing the flush operation. Otherwise, unpredictable exceptions may
 *       occur.
 */
k_s32 kd_mpi_sys_mmz_flush_cache(k_u64 phy_addr, void *virt_addr, k_u32 size);

/**
 * @brief Releases the MMZ in the user space
 *
 * @param [in] phy_addr Physical address
 * @param [in] virt_addr Pointer to the user space virtual address
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - The entered address must be a valid physical address. The pointer that points to the
 *   virtual address can be set to null
 * - You must not release the memory that is being flushed. Otherwise, unpredictable
 *   exceptions may occur
 */
k_s32 kd_mpi_sys_mmz_free(k_u64 phy_addr, void *virt_addr);

/**
 * @brief Obtains information about the corresponding memory (including the physical address and
 *        cached attributes) based on the virtual address.
 *
 * @param [in] virt_addr User-mode virtual address
 * @param [out] mem_info Information about the memory corresponding to the virtual address,
 *                       including the physical address and cached attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_sys_get_virmem_info(const void *virt_addr, k_sys_virmem_info *mem_info);

/**
 * @brief Binds a data source and a data receiver
 *
 * @param [in] src_chn Pointer to the source channel
 * @param [in] dest_chn Pointer to the destination channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note
 * - A data receiver can be bound only to one data source
 * - The binding establishes an association between a data source and a data receiver. After
 *   binding, the data generated by the data source is automatically transmitted to the data
 *   receiver.
 * @todo Description of the current list of bindings supported by the system
 * @todo Describe the meaning of the bindings
 */
k_s32 kd_mpi_sys_bind(k_mpp_chn *src_chn, k_mpp_chn *dest_chn);

/**
 * @brief Unbind a data source from a data receiver.
 *
 * @param [in] src_chn Pointer to the source channel
 * @param [in] dest_chn Pointer to the destination channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note If no bound source channel can be found for PstDestChn, a code indicating success is
 *       returned directly. If the bound source channel is found but it does not match src_chn, a
 *       code indicating failure is returned.
 */
k_s32 kd_mpi_sys_unbind(k_mpp_chn *src_chn, k_mpp_chn *dest_chn);

/**
 * @brief Obtains the information about a bound data source
 *
 * @param [out] dest_chn Pointer to the source channel
 * @param [in] src_chn Pointer to the destination channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_sys_get_bind_by_dest(k_mpp_chn *dest_chn, k_mpp_chn *src_chn);

/**
 * @brief Sets the log level
 * @param [in] conf Log level information structure
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 * @note When mod_name in conf is set to all, the log levels of all modules are set. Otherwise,
 *       only the log level of the module specified by mod_id is set.
 */
k_s32 kd_mpi_log_set_level_conf(const k_log_level_conf *conf);

/**
 * @brief Gets the log level
 * @param [out] conf Log level information structure conf->level conf->mod_name is out param
 * @param [in] conf Log level information structure conf->mod_id is in param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_log_get_level_conf(k_log_level_conf *conf);

/**
 * @brief Sets the waiting flag for reading the log
 *
 * @param [in] is_wait Flag indicating whether to wait when reading the log
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_log_set_wait_flag(k_bool is_wait);

/**
 * @brief Reads the log
 *
 * @param [out] buf Buffer pointer for storing the log
 * @param [in] size Size of the log to be read
 * @return k_s32
 * @retval "Values greater than or equal to 0" Size of the log successfully read
 */
k_s32 kd_mpi_log_read(k_char *buf, k_u32 size);

/**
 * @brief Closes the log
 * @return void
 */
void kd_mpi_log_close(void);

/**
 * @brief Whether to output the log directly to the console
 *
 * @param [in] is_console
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_log_set_console(k_bool is_console);

/**
 * @brief Query whether to output the log directly to the console
 *
 * @param [out] is_console
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_log_get_console(k_bool *is_console);


/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
