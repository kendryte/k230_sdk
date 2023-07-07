/*
* Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
* Description: file soc_types.h.
* Author: CompanyName
* Create: 2021-4-3
*/
/**
* @file soc_types.h
*
* 数据类型说明
*/

#ifndef SOC_TYPES_H
#define SOC_TYPES_H

#include <soc_types_base.h>
#include <soc_errno.h>


/* linux错误码 */
#define OAL_SUCC                0
#define OAL_EFAIL               1   /* 内核通用错误返回值 -1 */
#define OAL_EIO                 5   /* I/O error */
#define OAL_ENOMEM              12  /* Out of memory */
#define OAL_EFAUL               14  /* Bad address */
#define OAL_EBUSY               16  /* Device or resource busy */
#define OAL_ENODEV              19  /* No such device */
#define OAL_EINVAL              22  /* Invalid argument */

#define oal_reference(data) ((void)(data))


#endif // SOC_TYPES_H

