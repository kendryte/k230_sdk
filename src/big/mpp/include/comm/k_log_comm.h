/**
 * @file k_log_comm.h
 * @author
 * @brief cmpi log device error code
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
#ifndef __K_LOG_COMM_H__
#define __K_LOG_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

typedef struct
{
    k_mod_id  mod_id;
    k_s32  level;
    k_char  mod_name[16];
} k_log_level_conf;

#define K_ERR_LOG_INVALID_DEVID     K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_LOG_INVALID_CHNID     K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_LOG_ILLEGAL_PARAM     K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_LOG_EXIST             K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_LOG_UNEXIST           K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_LOG_NULL_PTR          K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_LOG_NOT_CONFIG        K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_LOG_NOT_SUPPORT       K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_LOG_NOT_PERM          K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_LOG_NOMEM             K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_LOG_NOBUF             K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_LOG_BUF_EMPTY         K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_LOG_BUF_FULL          K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_LOG_NOTREADY          K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_LOG_BADADDR           K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_LOG_BUSY              K_DEF_ERR(K_ID_LOG, K_ERR_LEVEL_ERROR, K_ERR_BUSY)

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif
