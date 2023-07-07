/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: HMAC module HCC layer adaptation.
 * Author: CompanyName
 * Create: 2021-08-04
 */

/* 头文件包含 */
#include <linux/module.h>
#include "soc_types_base.h"
#include "oam_ext_if.h"
#include "wal_net.h"
#include "hcc_adapt.h"
#include "wal_netlink.h"

/* new fix: add module parameters */
#include <linux/moduleparam.h>
#include "soc_param.h"

int mmc = 1;
module_param(mmc, int, 0664);
MODULE_PARM_DESC(mmc, "mmc slot number for wifi sdio");

int gpio = 34;
module_param(gpio, int, 0664);
MODULE_PARM_DESC(gpio, "gpio number for wifi oob");


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 函数定义 */
/* insmod socchannel.ko 入口函数 */
static td_s32 __init wlan_init(void)
{
    td_s32 ret;
    oam_warning_log0("socchannel 2022-01-13 16:00:00");
    ret = oal_netlink_init();
    if (ret != EXT_SUCCESS) {
        oam_error_log1("wlan_init:: oal_netlink_init FAILED[%d]", ret);
        goto fail;
    }

    /* 维测初始化 */
    ret = oam_main_init();
    if (ret != EXT_SUCCESS) {
        oam_error_log1("wlan_init:: oam_main_init FAILED[%d]", ret);
        goto oam_main_init_fail;
    }

    ret = hcc_adapt_init();
    if (ret != EXT_SUCCESS) {
        goto hcc_host_init_fail;
    }

    ret = netdev_register();
    if (ret != EXT_SUCCESS) {
        oam_error_log0("wlan_init:: netdev_register failed");
        return ret;
    }
    
    //oam_warning_log0("wlan drv insmod SUCCESSFULLY");
    printk("aich wifi registered");
    return EXT_SUCCESS;

hcc_host_init_fail:
    oam_main_exit();
oam_main_init_fail:
    oal_netlink_deinit();
fail:
    return -EXT_FAILURE;
}

/* rmmdo socchannel.ko 入口函数 */
static void __exit wlan_deinit(void)
{
    netdev_unregister();
    hcc_adapt_exit();
    oam_main_exit();
    oal_netlink_deinit();
    return;
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Soc Wlan Driver");
MODULE_AUTHOR("CompanyName Wifi Team");
MODULE_VERSION("V1.0.0_000.20200902");

late_initcall(wlan_init);
module_exit(wlan_deinit);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
