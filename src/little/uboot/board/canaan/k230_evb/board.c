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
	#define USB_IDPULLUP0 		(1<<4)
	#define USB_DMPULLDOWN0 	(1<<8)
	#define USB_DPPULLDOWN0 	(1<<9)

    u32 usb0_otg_en_gpio52_dir = readl((void*)(GPIO_BASE_ADDR1 + 0x4));
    usb0_otg_en_gpio52_dir |= 1 << (52 - 32);
    writel(usb0_otg_en_gpio52_dir, (void*)(GPIO_BASE_ADDR1 + 0x4));

    u32 usb0_otg_en_gpio52_data = readl((void*)(GPIO_BASE_ADDR1 + 0x0));
    usb0_otg_en_gpio52_data |= 1 << (52 - 32);
    writel(usb0_otg_en_gpio52_data, (void*)(GPIO_BASE_ADDR1 + 0x0));

    u32 usb0_test_ctl3 = readl((void*)USB0_TEST_CTL3);
	u32 usb1_test_ctl3 = readl((void*)USB1_TEST_CTL3);

	usb0_test_ctl3 |= USB_IDPULLUP0;
	usb1_test_ctl3 |= USB_IDPULLUP0;

	writel(usb0_test_ctl3, (void*)USB0_TEST_CTL3);
	writel(usb1_test_ctl3, (void*)USB1_TEST_CTL3);

    #define SD_HOST_REG_VOL_STABLE      (1<<4)
    #define SD_CARD_WRITE_PROT          (1<<6)
    u32 sd0_ctrl = readl((void*)SD0_CTRL);
    sd0_ctrl |= SD_HOST_REG_VOL_STABLE | SD_CARD_WRITE_PROT;
    writel(sd0_ctrl, (void*)SD0_CTRL);
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