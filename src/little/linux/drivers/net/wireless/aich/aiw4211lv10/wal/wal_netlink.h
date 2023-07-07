/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: hcc driver implementatioin.
 * Author: CompanyName
 * Create: 2021-09-28
 */
#ifndef _WAL_NETLINK_H
#define _WAL_NETLINK_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_util.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_SOCCHANNEL_HDR_OPT
#define MAX_USER_DATA_LEN               412     /* 小于这个值的数据报文通过高优先级通道传输 */
#else
#define MAX_USER_DATA_LEN               384
#endif
#define MAX_USER_LONG_DATA_LEN          1500    /* 消息最大长度 */
/*****************************************************************************
  3 函数声明
*****************************************************************************/
td_s32 oal_netlink_init(td_void);

td_s32 oal_netlink_deinit(td_void);

td_s32 oal_send_user_msg(td_u8 *buf, td_u32 len);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif
