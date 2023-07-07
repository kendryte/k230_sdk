/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-11-12     quanzhao     first version
 */
#ifndef __UNET_ETH_H__
#define __UNET_ETH_H__

#include <rtthread.h>

#include "unet.h"

/* ethernet driver commands */
#define NIOCTL_GADDR        0x01
#define NIOCTL_RX           0x02
#define NIOCTL_TX           0x03

/* eth flag with auto_linkup or phy_linkup */
#define ETHIF_LINK_AUTOUP   0x0000
#define ETHIF_LINK_PHYUP    0x0100

struct eth_device
{
    /* inherit from rt_device */
    struct rt_device parent;

    /* network interface for unet */
    void *netif;
    struct rt_semaphore *tx_ack;    /* created dynamically */

    rt_uint16_t flags;
    rt_uint8_t  link_changed;
    rt_uint8_t  link_status;

    /* eth device interface */
    rt_size_t   (*eth_rx)(rt_device_t dev, void *buf);
    rt_err_t    (*eth_tx)(rt_device_t dev, void *buf, rt_uint32_t length);
};

#ifdef __cplusplus
extern "C" {
#endif

    rt_err_t eth_device_ready(struct eth_device* dev);
    rt_err_t eth_device_init(struct eth_device * dev, const char *name);
    rt_err_t eth_device_init_with_flag(struct eth_device *dev, const char *name, rt_uint16_t flag);
    //rt_err_t eth_device_linkchange(struct eth_device* dev, rt_bool_t up);
    void eth_device_linkchange(struct eth_device* dev, rt_bool_t up);

    int eth_system_device_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __UNET_ETH_H__ */

