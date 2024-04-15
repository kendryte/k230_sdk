/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>
#include <string.h>
#include <dfs_fs.h>

#if CONFIG_ENABLE_USB_DEVICE
#include "usbd_core.h"
#include "cdc_acm_msc_template.c"
#endif

int main(void)
{
    printf("RT-SMART Hello RISC-V\n");
#ifdef RT_USING_SDIO
    while (mmcsd_wait_cd_changed(100) != MMCSD_HOST_PLUGED);
    if (dfs_mount("sd", "/sdcard", "elm", 0, 0) != 0)
        rt_kprintf("Dir /sdcard mount failed!\n");
#endif
#if CONFIG_ENABLE_USB_DEVICE
    extern void cdc_acm_msc_init(void);
    cdc_acm_msc_init();
    rt_thread_delay(10);
#endif
    msh_exec("/bin/init.sh", 13);
    return 0;
}
