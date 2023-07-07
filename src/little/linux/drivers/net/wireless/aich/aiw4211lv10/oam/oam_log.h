/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: oam log interface's header file.(in rom).
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __OAM_LOG_H__
#define __OAM_LOG_H__

#include "soc_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  10 函数声明
*****************************************************************************/
td_u32 oam_log_level_set(td_u32 log_level);

td_void oal_print_nlogs(
        const td_char* pfile_name,
        const td_char* pfuc_name,
        td_u16         us_line_no,
        void*          pfunc_addr,
        td_u8          clog_level,
        td_u8          uc_param_cnt,
        td_char*       fmt,
        td_u32 p1, td_u32 p2, td_u32 p3, td_u32 p4);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif /* end of oam_log.h */
