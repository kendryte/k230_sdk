/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: oal_wakelock.h 的头文件
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __OAL_LINUX_WAKE_LOCK_H__
#define __OAL_LINUX_WAKE_LOCK_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_mutex.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 STRUCT定义
*****************************************************************************/
typedef struct _oal_wakelock_stru_ {
    unsigned long           lock_count;     /* 持有wakelock锁的次数 */
    unsigned long           locked_addr;    /* the locked address */
} oal_wakelock_stru;

/* inline 函数定义 */
static inline td_void oal_wake_lock_init(oal_wakelock_stru *pst_wakelock, char *name)
{
    aich_unref_param(pst_wakelock);
    aich_unref_param(name);
}

static inline td_void oal_wake_lock_exit(oal_wakelock_stru *pst_wakelock)
{
    aich_unref_param(pst_wakelock);
}

static inline void oal_wake_lock(oal_wakelock_stru *pst_wakelock)
{
    aich_unref_param(pst_wakelock);
}

static inline  void oal_wake_unlock(oal_wakelock_stru *pst_wakelock)
{
    aich_unref_param(pst_wakelock);
}

static inline unsigned long oal_wakelock_active(oal_wakelock_stru *pst_wakelock)
{
    aich_unref_param(pst_wakelock);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_wakelock.h */

