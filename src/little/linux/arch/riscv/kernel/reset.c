// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <linux/reboot.h>
#include <linux/pm.h>
#include <asm/io.h>

static int k230_pmu_poweroff(void)
{
	void __iomem *pmu_base = ioremap(0x91000000, 0xb0);

	// write SOC_NORMAL_PD status, please refer to k230-pum.c
	writel(2, pmu_base + 0x3c);
	writel(1, pmu_base + 0x78);
	writel(0x3ff, pmu_base + 0x54);
	writel(0, pmu_base + 0x78);

	while(1);

	return 0;
}

static void default_power_off(void)
{
	while (1)
		wait_for_interrupt();
}

void (*pm_power_off)(void) = default_power_off;
EXPORT_SYMBOL(pm_power_off);

void machine_restart(char *cmd)
{
	do_kernel_restart(cmd);
	while (1);
}

void machine_halt(void)
{
	pm_power_off();
}

void machine_power_off(void)
{
	k230_pmu_poweroff();
	pm_power_off();
}
