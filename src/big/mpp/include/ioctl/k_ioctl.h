/*
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

#ifndef __K_IOCTL_H__
#define __K_IOCTL_H__

#include "sys/ioctl.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define K_IOC_TYPE_SYS      'S'
#define K_IOC_TYPE_VB       'B'
#define K_IOC_TYPE_LOG      'L'

#define K_IOC_TYPE_VI       'I'
#define K_IOC_TYPE_VPROC    'P'
#define K_IOC_TYPE_VREC     'R'
#define K_IOC_TYPE_VENC     'E'
#define K_IOC_TYPE_VDEC     'D'
#define K_IOC_TYPE_VO       'O'
#define K_IOC_TYPE_VVI      'I'
#define K_IOC_TYPE_VVO      'O'
#define K_IOC_TYPE_DMA      'M'
#define K_IOC_TYPE_DPU      'U'

#define K_IOC_TYPE_AI       'i'
#define K_IOC_TYPE_AREC     'r'
#define K_IOC_TYPE_AENC     'e'
#define K_IOC_TYPE_ADEC     'd'
#define K_IOC_TYPE_AO       'o'

#define K_IOC_TYPE_VICAP    'C'
#define K_IOC_TYPE_SENSOR   'S'
#define K_IOC_TYPE_ISP      'I'
#define K_IOC_TYPE_DEWARP   'W'

#define K_IOC_TYPE_USER_MMZ 'M'
#define K_IOC_TYPE_PM       'p'
#define K_IOC_TYPE_NONAI_2D 'N'

#define K_IOC_OUT         0x40000000UL    /* copy out parameters */
#define K_IOC_IN          0x80000000UL    /* copy in parameters */
#define K_IOC_INOUT       (K_IOC_IN|K_IOC_OUT)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
