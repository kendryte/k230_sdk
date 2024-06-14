/**
 * @file k_errno.h
 * @author
 * @brief Basic mpi error code definition
 * @version 1.0
 * @date 2022-08-31
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
#ifndef __K_ERRORNO_H__
#define __K_ERRORNO_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

typedef enum kERR_LEVEL_E
{
    K_ERR_LEVEL_DEBUG   = 0,    /**< debug-level                                  */
    K_ERR_LEVEL_INFO,           /**< informational                                */
    K_ERR_LEVEL_NOTICE,         /**< normal but significant condition             */
    K_ERR_LEVEL_WARNING,        /**< warning conditions                           */
    K_ERR_LEVEL_ERROR,          /**< error conditions                             */
    K_ERR_LEVEL_CRIT,           /**< critical conditions                          */
    K_ERR_LEVEL_ALERT,          /**< action must be taken immediately             */
    K_ERR_LEVEL_FATAL,          /**< just for compatibility with previous version */
    K_ERR_LEVEL_BUTT
} K_ERR_LEVEL_E;

typedef enum
{
    K_ERR_INVALID_DEVID = 1,    /**< invlalid device ID                                                               */
    K_ERR_INVALID_CHNID = 2,    /**< invlalid channel ID                                                              */
    K_ERR_ILLEGAL_PARAM = 3,    /**< at lease one parameter is illagal eg, an illegal enumeration value               */
    K_ERR_EXIST         = 4,    /**< resource exists                                                                  */
    K_ERR_UNEXIST       = 5,    /**< resource unexists                                                                */
    K_ERR_NULL_PTR      = 6,    /**< using a NULL point                                                               */
    K_ERR_NOT_CONFIG    = 7,    /**< try to enable or initialize system, device or channel, before configing attribute*/
    K_ERR_NOT_SUPPORT   = 8,    /**< operation or type is not supported by NOW                                        */
    K_ERR_NOT_PERM      = 9,    /**< operation is not permitted eg, try to change static attribute                    */
    K_ERR_NOMEM         = 12,   /**< failure caused by malloc memory                                                  */
    K_ERR_NOBUF         = 13,   /**< failure caused by malloc buffer                                                  */
    K_ERR_BUF_EMPTY     = 14,   /**< no data in buffer                                                                */
    K_ERR_BUF_FULL      = 15,   /**< no buffer for new data                                                           */
    K_ERR_NOTREADY      = 16,   /**< System is not ready,maybe not initialed or loaded.                               */
    K_ERR_BADADDR       = 17,   /**< bad address, eg. used for copy_from_user & copy_to_user                          */
    K_ERR_BUSY          = 18,   /**< resource is busy, eg. destroy a venc chn without unregister it                   */
    K_ERR_OPT_ALREADY_WRITE          = 19,   /**< resource is busy, eg. destroy a venc chn without unregister it                   */

    K_ERR_BUTT          = 63,   /**< 64 maxium code, private error code of all modules must be greater than it           */
} K_ERR_CODE_E;

#define K_ERR_APPID  (0x80000000L + 0x20000000L)

#define K_DEF_ERR( module, level, errid) \
        ((int)( (K_ERR_APPID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

/* return ok */
#define K_ERR_OK    0

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __K_ERRORNO_H__ */
