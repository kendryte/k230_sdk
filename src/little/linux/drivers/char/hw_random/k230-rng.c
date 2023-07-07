/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/hw_random.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>

#define PUF_REG_TRNG_OFFSET			0x2a0

struct k230_rng {
	void __iomem *base;
	struct hwrng rng;
	struct device *dev;
};

#define to_k230_rng(p)	container_of(p, struct k230_rng, rng)


static int k230_rng_read(struct hwrng *rng, void *buf, size_t max, bool wait)
{
	struct k230_rng *k230rng = to_k230_rng(rng);
	int retval = 0;

	while(max >= sizeof(u32))
	{
		*(u32 *)buf = readl(k230rng->base + PUF_REG_TRNG_OFFSET);

		retval += sizeof(u32);
		buf += sizeof(u32);
		max -= sizeof(u32);
	}

	return retval;
}

static int k230_rng_probe(struct platform_device *pdev)
{
	struct k230_rng *rng;
	struct resource *res;
	int ret;

	rng = devm_kzalloc(&pdev->dev, sizeof(*rng), GFP_KERNEL);
	if (!rng)
		return -ENOMEM;

	rng->dev = &pdev->dev;
	platform_set_drvdata(pdev, rng);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	rng->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(rng->base))
		return PTR_ERR(rng->base);

	rng->rng.name = pdev->name;
	rng->rng.read = k230_rng_read;

	ret = devm_hwrng_register(&pdev->dev, &rng->rng);
	if (ret) {
		dev_err(&pdev->dev, "failed to register hwrng\n");
		return ret;
	}

	dev_info(&pdev->dev, "K230 TRNG driver register!\n");

	return 0;
}

static const struct of_device_id k230_rng_dt_ids[] = {
	{ .compatible = "canaan,k230-rng" },
	{ }
};
MODULE_DEVICE_TABLE(of, k230_rng_dt_ids);

static struct platform_driver k230_rng_driver = {
	.probe		= k230_rng_probe,
	.driver		= {
		.name	= "k230-rng",
		.of_match_table = of_match_ptr(k230_rng_dt_ids),
	},
};

module_platform_driver(k230_rng_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Canaan K230 random number generator driver");
