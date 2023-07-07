/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: Header file for oal_util.h.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __OAL_UTIL_H__
#define __OAL_UTIL_H__

/* 头文件包含 */
#include <linux/string.h>
#include <linux/version.h>
#include "soc_types_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 宏定义包含 */
#define OAL_RET_ADDR          __builtin_return_address(0)

#define oal_bug_on(_con)        BUG_ON(_con)
#define oal_warn_on(condition)  WARN_ON(condition)

#define oal_likely(_expr)       likely(_expr)
#define oal_unlikely(_expr)     unlikely(_expr)

#define oal_round_down          round_down

#ifndef IS_ALIGNED
#define oal_is_aligned(val, align)  (((td_u32)(val) & ((align) - 1)) == 0)
#else
#define oal_is_aligned  IS_ALIGNED
#endif

/* inline 函数定义 */
static inline td_char *aich_strrchr(const td_char *file, td_char c)
{
    td_char *p = TD_NULL;
    return (((p = strrchr(file, c)) != TD_NULL) ? (p + 1) : (TD_NULL));
}

/* 函数声明 */
td_s32 oal_atoi(const td_char *c_string);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_util.h */
