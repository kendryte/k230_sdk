/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/canaan-hardlock.h>

#define CANAAN_DRIVER_NAME	"hard-lock"

struct canaan_hardlock {
    void __iomem *base;
    char used[HARDLOCK_MAX];
};

static struct canaan_hardlock *hardlock;

int hardlock_lock(int num)
{
    if (num < 0 || num > HARDLOCK_MAX)
        return -1;

    if (!readl(hardlock->base + num * 0x4))
    {
        pr_debug("hardlock-%d locked\n", num);
        return 0;
    }
    pr_debug("hardlock-%d is busy\n", num);
    return -1;
}
EXPORT_SYMBOL(hardlock_lock);

int hardlock_unlock(int num)
{
    if (num < 0 || num > HARDLOCK_MAX)
        return -1;

    if (readl(hardlock->base + num * 0x4))
    {
        writel(0x0, hardlock->base + num * 0x4);
        pr_debug("hardlock-%d unlock\n", num);
        return 0;
    }
    pr_err("hardlock-%d unlock failed\n", num);
    return -1;
}
EXPORT_SYMBOL(hardlock_unlock);

int request_lock(int num)
{
    if (num < 0 || num > HARDLOCK_MAX)
        return -1;

    if (!hardlock->used[num])
    {
        hardlock->used[num] = 1;
        return 0;
    }

    pr_err("request hardlock failed, hardlock-%d is used\n", num);
    return -1;
}
EXPORT_SYMBOL(request_lock);

static int canaan_hardlock_probe(struct platform_device *pdev)
{
    struct resource *reg;

    hardlock = devm_kzalloc(&pdev->dev, sizeof(*hardlock), GFP_KERNEL);
    if (!hardlock)
        return -ENOMEM;

    reg = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    hardlock->base = 0xa0 + devm_ioremap_resource(&pdev->dev, reg); /* hardlock offset */
    if (IS_ERR(hardlock->base))
        return PTR_ERR(hardlock->base);

    platform_set_drvdata(pdev, hardlock);
    printk( "Canaan Hard Lock Driver init.\n");
    return 0;
}

static const struct of_device_id canaan_hardlock_match[] = {
    { .compatible = "canaan,k230-hardlock"},
    { }
};

static struct platform_driver canaan_hardlock_driver = {
    .driver = {
        .name = CANAAN_DRIVER_NAME,
        .of_match_table = of_match_ptr(canaan_hardlock_match),
    },
    .probe = canaan_hardlock_probe,
};

static int __init hardlock_init(void)
{
    int ret;
	ret = platform_driver_register(&canaan_hardlock_driver);
	if (ret < 0)
		return ret;

	return 0;
}
subsys_initcall(hardlock_init);
