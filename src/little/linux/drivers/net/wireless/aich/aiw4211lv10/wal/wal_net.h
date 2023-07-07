/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: Header file for oal_util.h.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __WAL_NET_H__
#define __WAL_NET_H__

#include <linux/netdevice.h>
#include "oal_net.h"
#include "oal_netbuf.h"
#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 宏定义 */
#define ETHER_ADDR_LEN          6
#define WLAN_MAC_ADDR_LEN       6
#define ether_is_multicast(_a) (*(_a) & 0x01)

/* inline 函数定义 */
static inline oal_net_device_stru *wal_net_alloc_netdev(td_u32 sizeof_priv, const td_char *netdev_name, td_void *set_up)
{
    oal_net_device_stru *tmp_netdev = TD_NULL;

    if ((netdev_name == TD_NULL) || (set_up == TD_NULL)) {
        return TD_NULL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
    tmp_netdev = alloc_netdev(sizeof_priv, netdev_name, NET_NAME_UNKNOWN, set_up);
#else
    tmp_netdev = alloc_netdev(sizeof_priv, netdev_name, set_up);
#endif
    if (tmp_netdev == TD_NULL) {
        oam_error_log0("oal_net_alloc_netdev:: netdev is NULL");
    }
    return tmp_netdev;
}

static inline td_void wal_net_free_netdev(oal_net_device_stru *netdev)
{
    if (netdev == TD_NULL) {
        return;
    }
    free_netdev(netdev);
}

/* 函数声明 */
td_s32 netdev_register(td_void);
td_void netdev_unregister(td_void);
td_s32 wal_rx_data_proc(oal_netbuf_stru *netbuf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __WAL_NET_H__ */
