#include <asm/asm.h>
#include <asm/io.h>
#include <asm/types.h>
#include <lmb.h>
#include <cpu_func.h>
#include <stdio.h>
#include <common.h>
#include <command.h>
#include <image.h>
#include <gzip.h>
#include <asm/spl.h>
#include "sysctl.h"

#include <pufs_hmac.h>
#include <pufs_ecp.h>
#include <pufs_rt.h>
#include "pufs_sm2.h"
#include <pufs_sp38a.h>
#include <pufs_sp38d.h>
#include <linux/kernel.h>
// #include "sdk_autoconf.h"
// #include "k230_board_common.h"
#include <env_internal.h>

//必须实现；board_r 会调用；
int board_init(void)
{
    u32 usb0_otg_en_gpio52_dir = readl((void*)(GPIO_BASE_ADDR1 + 0x4));
    usb0_otg_en_gpio52_dir |= 1 << (52 - 32);
    writel(usb0_otg_en_gpio52_dir, (void*)(GPIO_BASE_ADDR1 + 0x4));

    u32 usb0_otg_en_gpio52_data = readl((void*)(GPIO_BASE_ADDR1 + 0x0));
    usb0_otg_en_gpio52_data |= 1 << (52 - 32);
    writel(usb0_otg_en_gpio52_data, (void*)(GPIO_BASE_ADDR1 + 0x0));

	return 0;
}

void quick_boot_board_init(void)
{
    u32 usb0_otg_en_gpio52_dir = readl((void*)(GPIO_BASE_ADDR1 + 0x4));
    usb0_otg_en_gpio52_dir |= 1 << (52 - 32);
    writel(usb0_otg_en_gpio52_dir, (void*)(GPIO_BASE_ADDR1 + 0x4));

    u32 usb0_otg_en_gpio52_data = readl((void*)(GPIO_BASE_ADDR1 + 0x0));
    usb0_otg_en_gpio52_data |= 1 << (52 - 32);
    writel(usb0_otg_en_gpio52_data, (void*)(GPIO_BASE_ADDR1 + 0x0));
	return 0;
}