/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: Header file for hcc_task.c.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __HCC_TASK_H__
#define __HCC_TASK_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hcc_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    oal_kthread_stru            *hcc_kthread;
    oal_wait_queue_head_stru     hcc_wq;
    td_u8                        task_state;
    td_u8                        auc_resv[3];   /* resv 3 bytes */
} hcc_task_stru;

td_u32 hcc_task_init(hcc_handler_stru *hcc_handler);
td_void hcc_task_exit(td_void);

td_void hcc_task_sched(td_void);

td_void hcc_exit_task_thread(hcc_handler_stru* hcc);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of frw_task.h */

