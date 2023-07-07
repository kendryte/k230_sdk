/**
 * @file k_mmz_comm.h
 * @author
 * @brief Mmz user device error code and public data types
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
#ifndef __K_MMZ_COMM_H__
#define __K_MMZ_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

/**
 * @brief Defines information about a virtual memory
 *
 * @see kd_mpi_sys_get_virmem_info
 */
typedef struct
{
    k_u64  phy_addr; /**< Physical address corresponding to the virtual address */
    k_bool cached;  /**< Cached mapping indicator */
} k_sys_virmem_info;

/* mmz userdev error code */
#define K_ERR_MMZ_USERDEV_INVALID_DEVID     K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_MMZ_USERDEV_INVALID_CHNID     K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_MMZ_USERDEV_ILLEGAL_PARAM     K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_MMZ_USERDEV_EXIST             K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_MMZ_USERDEV_UNEXIST           K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_MMZ_USERDEV_NULL_PTR          K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_MMZ_USERDEV_NOT_CONFIG        K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_MMZ_USERDEV_NOT_SUPPORT       K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_MMZ_USERDEV_NOT_PERM          K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_MMZ_USERDEV_NOMEM             K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_MMZ_USERDEV_NOBUF             K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_MMZ_USERDEV_BUF_EMPTY         K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_MMZ_USERDEV_BUF_FULL          K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_MMZ_USERDEV_NOTREADY          K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_MMZ_USERDEV_BADADDR           K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_MMZ_USERDEV_BUSY              K_DEF_ERR(K_ID_MMZ_USER_DEV, K_ERR_LEVEL_ERROR, K_ERR_BUSY)

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */
#endif
