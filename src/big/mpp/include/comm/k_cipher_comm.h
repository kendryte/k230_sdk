/**
 * @file k_cipher_comm.h
 * @author
 * @brief sha256 hash algorithm
 * @version 1.0
 * @date 2022-09-01
 *
 * @copyright
 * Copyright (C) 2021 HiSilicon (Shanghai) Technologies CO., LIMITED.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef __K_CIPHER_COMM_H__
#define __K_CIPHER_COMM_H__

#include "k_type.h"
#include "k_errno.h"
#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

/** \addtogroup     SHA256 */
/** @{ */ /** <!-- [SHA256] */

#define SHA256_BLOCK_SIZE   64  // SHA256 block size, byte
#define SHA256_HASH_SIZE    32  // SHA256 output a 32 byte digest, byte

typedef struct {
    k_u8 data[SHA256_BLOCK_SIZE];
    k_u32 hash[8];
    k_u32 datalen;
    k_u32 bits[2];
} k_sha256_ctx;

/** @} */ /** <!-- ==== SHA256 End ==== */

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif
