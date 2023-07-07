/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: oam log interface's implementation.(in rom).
 * Author: CompanyName
 * Create: 2021-08-04
 */

/*****************************************************************************
    1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PRINT   printk

td_void oal_print_nlogs(
        const td_char* pfile_name,
        const td_char* pfuc_name,
        td_u16         us_line_no,
        void*          pfunc_addr,
        td_u8          clog_level,
        td_u8          uc_param_cnt,
        td_char*       fmt,
        td_u32 p1, td_u32 p2, td_u32 p3, td_u32 p4)
{
    td_char buffer[OAM_PRINT_FORMAT_LENGTH] = {0};
    td_char* level = NULL;

    aich_unref_param(pfile_name);
    aich_unref_param(pfuc_name);
    aich_unref_param(pfunc_addr);

    if (clog_level > oam_get_log_level()) {
        return;
    }
    switch (clog_level) {
        case OAM_LOG_LEVEL_INFO:
            level = "[INFO]";
            break;
        case OAM_LOG_LEVEL_WARNING:
            level = "[WARN]";
            break;
        case OAM_LOG_LEVEL_ERROR:
            level = "[ERROR]";
            break;
        default:
            break;
    }

    if (level == TD_NULL) {
        return;
    }

    if (snprintf_s(buffer, OAM_PRINT_FORMAT_LENGTH, OAM_PRINT_FORMAT_LENGTH - 1,
        "%s:%d:%s \r\n", level, us_line_no, fmt) == -1) {
        return;
    }

    switch (uc_param_cnt) {
        case 0: /* case 0 param_cnt */
            printk(buffer);
            break;
        case 1: /* case 1 param_cnt */
            printk(buffer, p1);
            break;
        case 2: /* case 2 param_cnt */
            printk(buffer, p1, p2);
            break;
        case 3: /* case 3 param_cnt */
             printk(buffer, p1, p2, p3);
            break;
        case 4: /* case 4 param_cnt */
             printk(buffer, p1, p2, p3, p4);
            break;
        default:
            break;
    }
    return;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

