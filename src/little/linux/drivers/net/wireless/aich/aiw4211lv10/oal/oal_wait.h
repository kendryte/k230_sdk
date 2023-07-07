/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: oal_wait.h 的头文件
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __OAL_WAIT_H__
#define __OAL_WAIT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <linux/wait.h>
#include <linux/sched.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 STRUCT定义
*****************************************************************************/
/* new fix: */
#if (_PRE_KERVER == _PRE_KERVER_4D9)
typedef wait_queue_t         oal_wait_queue_stru;
#else
typedef wait_queue_entry_t   oal_wait_queue_stru;
#endif
typedef wait_queue_head_t    oal_wait_queue_head_stru;

/*****************************************************************************
  3 宏定义
*****************************************************************************/
#define aich_wait_queue_wake_up_interrupt(_pst_wq)     wake_up_interruptible(_pst_wq)

#define aich_wait_queue_wake_up(_pst_wq)     wake_up(_pst_wq)

#define aich_interruptible_sleep_on(_pst_wq) interruptible_sleep_on(_pst_wq)

#define aich_wait_queue_init_head(_pst_wq)   init_waitqueue_head(_pst_wq)

#define aich_wait_event_interruptible_timeout(_st_wq, _condition, _timeout) \
    wait_event_interruptible_timeout(_st_wq, _condition, _timeout)

#define aich_wait_event_timeout(_st_wq, _condition, _timeout) \
    wait_event_timeout(_st_wq, _condition, _timeout)

#define aich_wait_event_interruptible(_st_wq, _condition) \
    wait_event_interruptible(_st_wq, _condition)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_wait.h */

