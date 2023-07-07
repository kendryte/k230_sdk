/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: oal_util.c.
 * Author: CompanyName
 * Create: 2021-08-04
 */

/******************************************************************************
  1 头文件包含
******************************************************************************/
#include "oal_util.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/*****************************************************************************
2 函数实现
*****************************************************************************/
td_s32 oal_atoi(const td_char *c_string)
{
    td_s32 l_ret = 0;
    td_s32 flag = 0;

    for (;; c_string++) {
        switch (*c_string) {
            case '0' ... '9':
                l_ret = 10 * l_ret + (*c_string - '0'); /* 10:十进制数 */
                break;
            case '-':
                flag = 1;
                break;
            case ' ':
                continue;
            default:
                return ((flag == 0) ? l_ret : (-l_ret));
        }
    }
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

