/*
1. canaan,k510_pm_domains.h (/home/yangfan/maix2/k510_linux_kernel/include/dt-bindings/soc)
2. power_consumer.dtsi (/home/yangfan/maix2/k510_buildroot/k510_crb_lp3_v1_2_defconfig/build/linux-4.17/arch/riscv/boot/dts/canaan/k510_common)
3. k510_pm_domains.c (/home/yangfan/maix2/k510_linux_kernel/drivers/soc/canaan)
    1) 编写power_provider.dtsi文件；
    2) 编写canaan,k230_pm_domains.h文件，该文件定义了k230中所有power domain的编号；
    3) 编写k230_pm_domains.c文件
        a. provider在DTS中定义power domain有关的device tree node，并在provider的初始化接口（一般是driver的probe）
           中，定义、初始化并注册所有的power domain；
        b. 将所有的domain，以struct generic_pm_domain（PM domain framework提供的）的形式抽象出来，
           并填充数据结构中需要由provider提供的内容（一般有三个，分别是name、power on/off）；
        c. 调用pm_genpd_init，初始化struct generic_pm_domain变量中其余的内容；
        c. 调用__of_genpd_add_provider接口，将所有的domain添加到kernel中，同时提供一个根据DTS node
           获得对应的domain指针的回调函数；

*/
/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/pm_domain.h>
#include <linux/platform_device.h>
#include <linux/canaan-hardlock.h>

#include <dt-bindings/soc/canaan,k230_pm_domains.h>

// #define K230_PM_DEBUG
#undef K230_PM_DEBUG

typedef enum {
    REG_PM_PWR_EN,
    REG_PM_PWR_STAT,
    REG_PM_REPAIR_EN,
    REG_PM_REPAIR_STAT,
    REG_PM_ARRAY_SIZE,
} K230_PM_REG_E;

struct k230_pm_domain {
    struct generic_pm_domain dm;
    u16 *reg_offset;
    u16 pwr_on_bit;
    u16 pwr_on_wen_bit;
    u16 pwr_off_bit;
    u16 pwr_off_wen_bit;
    bool repair_enable;
    u16 repair_bit;
    u16 repair_wen_bit;
    bool soft_control_enable;
    bool hardlock;
};

#define PM_PWR_EN(pd) ((pd)->reg_offset[REG_PM_PWR_EN])
#define PM_PWR_STAT(pd) ((pd)->reg_offset[REG_PM_PWR_STAT])
#define PM_PWR_REPAIR_EN(pd) ((pd)->reg_offset[REG_PM_REPAIR_EN])
#define PM_PWR_REPAIR_STAT(pd) ((pd)->reg_offset[REG_PM_REPAIR_STAT])

static void __iomem *sysctl_power_base;
static u32 hardlock_disp;
static u32 hardlock_disp_cpu0;
static u32 hardlock_disp_cpu1;

static int k230_power_on(struct generic_pm_domain *domain)
{
    struct k230_pm_domain *pd = (struct k230_pm_domain *)domain;
    unsigned long loop = 1000;  //需要review
    u32 val;
    unsigned long flags;

#ifndef K230_PM_DEBUG
    if(false == pd->soft_control_enable) {
        pr_info("[K230_POWER]:skip power on %s\n", domain->name);
        return 0;
    }

    if (pd->hardlock) {
        local_irq_save(flags);
        while (hardlock_lock(hardlock_disp));
        hardlock_lock(hardlock_disp_cpu0);
    }

    if (readl(sysctl_power_base + PM_PWR_STAT(pd)) & BIT(pd->pwr_on_bit))
        goto out;

    val = BIT(pd->pwr_on_bit) | BIT(pd->pwr_on_wen_bit) | BIT(pd->pwr_off_wen_bit);
    writel(val, sysctl_power_base + PM_PWR_EN(pd));

    if(true == pd->repair_enable) {
        val = BIT(pd->repair_bit) | BIT(pd->repair_wen_bit);
        writel(val, sysctl_power_base + PM_PWR_REPAIR_EN(pd));

        do {
            udelay(1);
            // val = readl(sysctl_power_base + PM_PWR_REPAIR_STAT(pd)) & BIT(pd->repair_bit);   //在510中，repair_bit跟repair status寄存器bit一一对应，但在230中没有
            val = readl(sysctl_power_base + PM_PWR_REPAIR_STAT(pd)) & 0x7;
        } while (--loop && !val);

        if (!loop) {
            pr_err("[K230_POWER]:Error: %s %s repair fail\n", __func__, domain->name);
            return -EIO;
        }

        pr_debug("[K230_POWER]:poweron %s\n", domain->name);
    }

    loop = 1000;
    do {
        udelay(1);
        val = readl(sysctl_power_base + PM_PWR_STAT(pd)) & BIT(pd->pwr_on_bit);
    } while (--loop && !val);
out:
    if (pd->hardlock) {
        hardlock_unlock(hardlock_disp);
        local_irq_restore(flags);
    }

    if (!loop) {
        pr_err("[K230_POWER]:Error: %s %s power on fail\n", __func__, domain->name);
        return -EIO;
    }
#endif

    return 0;
}

static int k230_power_off(struct generic_pm_domain *domain)
{
    struct k230_pm_domain *pd = (struct k230_pm_domain *)domain;
    unsigned long loop = 1000;
    u32 val;
    unsigned long flags;

#ifndef K230_PM_DEBUG
    if(false == pd->soft_control_enable) {
        pr_info("[K230_POWER]:skip power off %s\n", domain->name);
        return 0;
    }

    if (pd->hardlock) {
        local_irq_save(flags);
        while (hardlock_lock(hardlock_disp));
        if (hardlock_lock(hardlock_disp_cpu1))
            goto out;
        hardlock_unlock(hardlock_disp_cpu1);
    }

    if (readl(sysctl_power_base + PM_PWR_STAT(pd)) & BIT(pd->pwr_off_bit))
        goto out;

    val = BIT(pd->pwr_off_bit) | BIT(pd->pwr_on_wen_bit) | BIT(pd->pwr_off_wen_bit);
    writel(val, sysctl_power_base + PM_PWR_EN(pd));

    do {
        udelay(1);
        val = readl(sysctl_power_base + PM_PWR_STAT(pd)) & BIT(pd->pwr_off_bit);
    } while (--loop && !val);
out:
    if (pd->hardlock) {
        hardlock_unlock(hardlock_disp_cpu0);
        hardlock_unlock(hardlock_disp);
        local_irq_restore(flags);
    }

    if (!loop) {
        pr_err("[K230_POWER]:Error: %s %s power off fail\n", __func__, domain->name);
        return -EIO;
    }
#endif

    return 0;
}


int k230_pd_probe(struct platform_device *pdev,
    struct generic_pm_domain **k230_pm_domains,
    int domain_num)
{
    struct genpd_onecell_data *genpd_data;
    struct resource *res;
    int i;

    genpd_data = devm_kzalloc(&pdev->dev, sizeof(*genpd_data), GFP_KERNEL);
    if (!genpd_data)
        return -ENOMEM;

    genpd_data->domains = k230_pm_domains;
    genpd_data->num_domains = domain_num;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    sysctl_power_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(sysctl_power_base))
        return PTR_ERR(sysctl_power_base);

    k230_pm_domains[K230_PM_DOMAIN_CPU1]->flags |= GENPD_FLAG_ALWAYS_ON;
    k230_pm_domains[K230_PM_DOMAIN_AI]->flags |= GENPD_FLAG_ALWAYS_ON;
    k230_pm_domains[K230_PM_DOMAIN_VPU]->flags |= GENPD_FLAG_ALWAYS_ON;
    k230_pm_domains[K230_PM_DOMAIN_DPU]->flags |= GENPD_FLAG_ALWAYS_ON;

    for (i = 0; i < domain_num; ++i) {
        k230_pm_domains[i]->power_on = k230_power_on;
        k230_pm_domains[i]->power_off = k230_power_off;

        pm_genpd_init(k230_pm_domains[i], NULL, i != K230_PM_DOMAIN_DISP ? false : true);
    }

    of_genpd_add_provider_onecell(pdev->dev.of_node, genpd_data);

    of_property_read_u32_index(pdev->dev.of_node, "hardlock", 0, &hardlock_disp);
    of_property_read_u32_index(pdev->dev.of_node, "hardlock", 1, &hardlock_disp_cpu0);
    of_property_read_u32_index(pdev->dev.of_node, "hardlock", 2, &hardlock_disp_cpu1);

    dev_info(&pdev->dev, "powerdomain init ok\n");

    return 0;
}

static u16 k230_offsets[K230_PM_DOMAIN_MAX][REG_PM_ARRAY_SIZE] = {
    {0x18,  0x1c,  0x18,  0x160},
    {0x28,  0x2c,  0x28,  0x160},
    {0x3c,  0x40,  0x3c,  0x160},
    {0x7c,  0x80,  0x7c,  0x160},
    {0x108, 0x10c, 0x108, 0x160},
};

static struct k230_pm_domain cpu1_domain = {
    .dm = {
        .name       = "cpu1_domain",
    },
    .pwr_on_bit     = 1,
    .pwr_on_wen_bit = 17,
    .pwr_off_bit    = 0,
    .pwr_off_wen_bit= 16,
    .repair_enable  = false,
    .reg_offset     = k230_offsets[K230_PM_DOMAIN_CPU1],
    .soft_control_enable = true,    //该字段需要need review
    .hardlock       = false,
};

static struct k230_pm_domain ai_domain = {
    .dm = {
        .name       = "ai_domain",
    },
    .pwr_on_bit     = 1,
    .pwr_on_wen_bit = 17,
    .pwr_off_bit    = 0,
    .pwr_off_wen_bit= 16,
    .repair_enable  = true,
    .repair_bit     = 4,
    .repair_wen_bit = 20,
    .reg_offset     = k230_offsets[K230_PM_DOMAIN_AI],
    .soft_control_enable = true,
    .hardlock       = false,
};

static struct k230_pm_domain disp_domain = {
    .dm = {
        .name       = "disp_domain",
    },
    .pwr_on_bit     = 1,
    .pwr_on_wen_bit = 17,
    .pwr_off_bit    = 0,
    .pwr_off_wen_bit= 16,
    .repair_enable  = false,
    .reg_offset     = k230_offsets[K230_PM_DOMAIN_DISP],
    .soft_control_enable = true,
    .hardlock       = true,
};

static struct k230_pm_domain vpu_domain = {
    .dm = {
        .name       = "vpu_domain",
    },
    .pwr_on_bit     = 1,
    .pwr_on_wen_bit = 17,
    .pwr_off_bit    = 0,
    .pwr_off_wen_bit= 16,
    .repair_enable  = false,
    .reg_offset     = k230_offsets[K230_PM_DOMAIN_VPU],
    .soft_control_enable = true,
    .hardlock       = false,
};

static struct k230_pm_domain dpu_domain = {
    .dm = {
        .name       = "dpu_domain",
    },
    .pwr_on_bit     = 1,
    .pwr_on_wen_bit = 17,
    .pwr_off_bit    = 0,
    .pwr_off_wen_bit= 16,
    .repair_enable  = false,
    .reg_offset     = k230_offsets[K230_PM_DOMAIN_DPU],
    .soft_control_enable = true,
    .hardlock       = false,
};

static struct generic_pm_domain *k230_pm_domains[] = {
    [K230_PM_DOMAIN_CPU1]   = &cpu1_domain.dm,
    [K230_PM_DOMAIN_AI]     = &ai_domain.dm,
    [K230_PM_DOMAIN_DISP]   = &disp_domain.dm,
    [K230_PM_DOMAIN_VPU]    = &vpu_domain.dm,
    [K230_PM_DOMAIN_DPU]    = &dpu_domain.dm,
};

static int k230_power_domain_probe(struct platform_device *pdev)
{
    return k230_pd_probe(pdev, k230_pm_domains, ARRAY_SIZE(k230_pm_domains));
}

static const struct of_device_id k230_pm_domain_matches[] = {
    { .compatible = "canaan, k230-sysctl-power", },
    {  },
};

static struct platform_driver k230_power_domain_driver = {
    .driver = {
        .name = "k230-powerdomain",
        .of_match_table = k230_pm_domain_matches,
    },
    .probe = k230_power_domain_probe,
};

static int __init k230_power_domain_init(void)
{
    return platform_driver_register(&k230_power_domain_driver);
}

subsys_initcall(k230_power_domain_init);