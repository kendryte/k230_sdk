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
#ifdef PKG_CHERRYUSB_HOST
#include "usbh_core.h"
#endif

int main(void)
{
    printf("RT-SMART Hello RISC-V.\n");

#ifdef PKG_CHERRYUSB_HOST
    char *usb_base = NULL;
#ifdef CHERRYUSB_HOST_USING_USB1
    usb_base = (char *)rt_ioremap((void *)0x91540000UL, 0x10000);
#else
    usb_base = (char *)rt_ioremap((void *)0x91500000UL, 0x10000);
#endif
    usbh_initialize(0, (uint32_t)usb_base);
#endif

#ifdef RT_USING_SDIO
    while (mmcsd_wait_cd_changed(100) != MMCSD_HOST_PLUGED);
    if (dfs_mount("sd", "/sdcard", "elm", 0, 0) != 0)
        rt_kprintf("Dir /sdcard mount failed!\n");
#endif

    msh_exec("/bin/init.sh", 13);

    return 0;
}
