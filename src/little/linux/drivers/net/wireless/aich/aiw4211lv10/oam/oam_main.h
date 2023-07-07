/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: Header file for oam_main.c.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __OAM_MAIN_H__
#define __OAM_MAIN_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define OAM_BEACON_HDR_LEN              24
#define OAM_TIMER_MAX_LEN               36
#define OAM_PRINT_CRLF_NUM              20              /* 输出换行符的个数 */

#define OAM_FEATURE_NAME_ABBR_LEN       12              /* 特性名称缩写最大长度 */

#define DEV_OM_FRAME_DELIMITER          0x7e
#define DEV_OM_MSG_TYPE_LOG             1

/*****************************************************************************
  10 函数声明
*****************************************************************************/
td_s32 oam_main_init(td_void);
td_void oam_main_exit(td_void);
td_u32 oam_get_log_level(td_void);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of oam_main */
