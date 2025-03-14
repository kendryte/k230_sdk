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
#include <linux/delay.h>
extern int ddr_init_board(void);
int ddr_init_training(void)
{
	#if defined(CONFIG_TARGET_K230_FPGA)
		return 0; //fpga not need init;
	#endif
	if( (readl((const volatile void __iomem *)0x980001bcULL) & 0x1 ) != 0 ){
		return 0; //have init ,not need reinit;
	}
	ddr_init_board();
	return 0;
}
sysctl_boot_mode_e sysctl_boot_get_boot_mode(void)
{
	return SYSCTL_BOOT_SDIO0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{

    return 0;
}
#endif
