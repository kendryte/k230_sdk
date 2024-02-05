/**
 * @file k_module.h
 * @author
 * @brief Data types associated with the mpi module
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
#ifndef __K_MODULE__
#define __K_MODULE__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

#include "k_type.h"

/**
 * @brief mpi module ID
 *
 */
typedef enum
{
    K_ID_CMPI           = 0,    /**< common module platform interface           */
    K_ID_LOG            = 1,    /**< mpi device log                             */
    K_ID_MMZ            = 2,    /**< media memory zone                          */
    K_ID_MMZ_USER_DEV   = 3,    /**< media memory zone user used                */
    K_ID_VB             = 4,    /**< video buffer device                        */
    K_ID_SYS            = 5,    /**< system contrl device                       */

    K_ID_VI             = 6,    /**< video in device                            */
#if 0
    K_ID_VPROC          = 7,    /**< video proc device                          */
    K_ID_VREC           = 8,    /**< video recognize device                     */
#endif
    K_ID_VENC           = 9,    /**< video encoding device                      */
    K_ID_VDEC           = 10,   /**< video decoding device                      */
    K_ID_VO             = 11,   /**< video output device                        */

    K_ID_AI             = 12,   /**< audio input device                         */
    K_ID_AREC           = 13,   /**< audio recognize device                     */
    K_ID_AENC           = 14,   /**< audio encoding device                      */
    K_ID_ADEC           = 15,   /**< audio decoding device                      */
    K_ID_AO             = 16,   /**< audio output device                        */
    K_ID_DPU            = 17,   /**< depth Process Unit                         */

    K_ID_V_VI,                  /**< virtual video input device                 */
    K_ID_V_VO,                  /**< virtual video output device                */
    K_ID_DMA,                   /**< dma device                                 */
    K_ID_VICAP,                 /**< vicap device                               */
    K_ID_DW200,
    K_ID_PM,
    K_ID_NONAI_2D,             /**< non ai 2d device                           */
    K_ID_BUTT,                  /**< Invalid                                    */
} k_mod_id;

/**
 * @brief Defines channels of modules and devices
 *
 * @see
 * - kd_mpi_sys_bind
 * - kd_mpi_sys_unbind
 * - kd_mpi_sys_get_bind_by_dest
 */
typedef struct
{
    k_mod_id    mod_id; /**< Module ID  */
    k_s32       dev_id; /**< Device ID  */
    k_s32       chn_id; /**< Channel ID */
} k_mpp_chn;

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
