/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <common.h>
#include <asm/types.h>
#include <asm/asm.h>
#include <asm/csr.h>
#include <common.h>
#include <cpu_func.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <command.h>
#include <spl.h>
#include <asm/cache.h>
#include <linux/delay.h>
#include "platform.h"

static inline void improving_cpu_performance(void)
{
	/* Set cpu regs */
	csr_write(CSR_MCOR, 0x70013);
	csr_write(CSR_MCCR2, 0xe0000009);
	csr_write(CSR_MHCR, 0x11ff); //Enable L1 Cache
	csr_write(CSR_MXSTATUS, 0x638000);
	csr_write(CSR_MHINT, 0x6e30c);

	csr_write(CSR_SMPEN, 0x1);
}

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	// disable_interrupts();

	cache_flush();
	icache_disable();
	dcache_disable();

    // csi_l2cache_flush_invalid();
	asm volatile(".long 0x0170000b\n":::"memory");

	improving_cpu_performance();

	return 0;
}

void harts_early_init(void)
{
	/*enable mtimer clk*/
    writel(0x1, (volatile void __iomem *)0x91108020);//CPU0
	writel(0x1, (volatile void __iomem *)0x91108030);//CPU1
	// enable stc0 
	writel(0x69, (volatile void __iomem *)0x91108000);

	record_boot_time_info_to_sram("et");

	writel(0x80199805, (void*)0x91100004);
    
    writel(0x0, (void*)SYSCTL_PWR_BASE_ADDR + 0x158);

// This address space only allows write access by the k230_burntool.
#ifndef CONFIG_CMD_DFU
	csr_write(pmpaddr0, 0x24484dff);//start addr：0x24484c00<<2=0x91213000 len=1<<9 * 8 = 4KB
	csr_write(pmpaddr1, 0x244851ff);//start addr：0x24485000<<2=0x91214000 len=1<<9 * 8 = 4KB
	csr_write(pmpcfg0, 0x9999);
#endif

	//improving_cpu_performance();
}
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1; //BOOT_DEVICE_SPI
}

typedef void (*func_app_entry)(void);
static int k230_boot_baremetal(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	static	ulong	boot_address , boot_size, boot_cpu;

	if (argc < 4)
		return CMD_RET_USAGE;

	boot_cpu = hextoul(argv[1], NULL);
	boot_address = hextoul(argv[2], NULL);
	boot_size = hextoul(argv[3], NULL);

	flush_cache(boot_address, boot_size);
	printf("boot_cpu = %ld boot_address = 0x%lx boot_size=0x%lx\n", boot_cpu, boot_address, boot_size);
	if(boot_cpu)
	{
		writel(boot_address, (void*)0x91102104ULL);//cpu1_hart_rstvec
		udelay(100);
		writel(0x10001000,(void*)0x9110100c);
		udelay(100);
		writel(0x10001,(void*)0x9110100c);
		udelay(100);
		writel(0x10000,(void*)0x9110100c);
	}
	else
	{
		func_app_entry app_entry = (void *)(long)boot_address;
		app_entry();
	}

    return 0;
}

U_BOOT_CMD_COMPLETE(
	boot_baremetal, 4, 1, k230_boot_baremetal,
	"boot_baremetal",
	"\n boot_baremetal cpu addr size\n", NULL
);


int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	printf("reboot system\n");
	#ifndef CONFIG_SPL_BUILD
	writel(0x10001, (void*)SYSCTL_BOOT_BASE_ADDR+0x60);
	#endif 
	while(1);
}
