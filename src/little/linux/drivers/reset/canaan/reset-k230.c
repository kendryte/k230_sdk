/****************************************************
 * 1. 写provider.dtsi，组织reset模块的形式（标准格式：phandle+specifier），使得扫描device tree后生成的id格式统一
 * 2. 在dt-bindings目录下写头文件，将所有的模块按照一定的格式描述出来
 * 3. 编写驱动文件，按照不同的类型reset对应的模块。
 * 
 * id的格式：根据reset_consumer.dtsi中定义的格式类型，即
 *      ___________________________________________
 *      | phandle | offset | type | done | assert |
 *      ———————————————————————————————————————————
 * 经过xlate函数翻译之后，将reset属性翻译成id，其中id的形式如下：
 *      31     16|15  14|13   7|6      0
 *      ————————————————————————————————
 *      | offset | type | done | reset |
 *      ————————————————————————————————
 * id最终将提供给reset_control_ops使用
 */ 

/*
 * Copyright (c) 2016-2017 Linaro Ltd.
 * Copyright (c) 2022, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <dt-bindings/reset/canaan-k230-reset.h>

#define K230_RESET_DEBUG

struct k230_reset_controller {
    spinlock_t                  lock;
    void __iomem                *membase;
    struct reset_controller_dev rst;
};

#define to_k230_reset_controller(_rst) \
    container_of(_rst, struct k230_reset_controller, rst)

static int k230_reset_of_xlate(struct reset_controller_dev *rcdev, const struct of_phandle_args *reset_spec)
{
    u32 offset;
    u32 type;
    u32 done;
    u32 reset;

    offset  = (reset_spec->args[0] << K230_RESET_REG_OFFSET_SHIFT) & K230_RESET_REG_OFFSET_MASK;
    type    = (reset_spec->args[1] << K230_RESET_TYPE_SHIFT)       & K230_RESET_TYPE_MASK;
    done    = (reset_spec->args[2] << K230_RESET_DONE_BIT_SHIFT)   & K230_RESET_DONE_BIT_MASK;
    reset   = (reset_spec->args[3] << K230_RESET_ASSERT_BIT_SHIFT) & K230_RESET_ASSERT_BIT_MASK;

    return (offset | type | done | reset);
}

static int k230_reset(struct reset_controller_dev *rcdev, unsigned long id) 
{
    struct k230_reset_controller *rstc = to_k230_reset_controller(rcdev);
    unsigned long flags;
    u32 offset  = (id & K230_RESET_REG_OFFSET_MASK) >> K230_RESET_REG_OFFSET_SHIFT;
    u32 type    = (id & K230_RESET_TYPE_MASK)       >> K230_RESET_TYPE_SHIFT;
    u32 done    = (id & K230_RESET_DONE_BIT_MASK)   >> K230_RESET_DONE_BIT_SHIFT;
    u32 reset   = (id & K230_RESET_ASSERT_BIT_MASK) >> K230_RESET_ASSERT_BIT_SHIFT;
    u32 reg;

#ifdef K230_RESET_DEBUG
    pr_info("[K230_RESET]:k230_reset id = 0x%08x, offset = 0x%08x type = %d done = %d reset =%d",(int)id,offset,type,done,reset);
#endif

    spin_lock_irqsave(&rstc->lock, flags);
    switch(type) {
        case K230_RESET_TYPE_CPU: {
            /* clear done bit */
            reg = readl(rstc->membase+offset);
            reg |= (1 << done);
            reg |= (1 << (done + 0x10));   // note: write enable
            writel(reg, rstc->membase+offset);

            /* set reset bit */
            reg |= (1 << reset);
            reg |= (1 << (reset + 0x10));   // note: write enable
            writel(reg, rstc->membase+offset);

            udelay(10);

            /* clear reset bit */
            if(0xc == offset)
            {
                reg &= ~(1 << reset);
                reg &= (1 << (reset + 0x10));   // note: write enable
                writel(reg, rstc->membase+offset);
            }
            
            /* wait done bit set */
            while(1) {
                reg = readl(rstc->membase+offset);
                if(reg & (1 << done)) {
                    /* clear done and break */
                    writel(reg, rstc->membase+offset);
                    break;
                }
            }
            break;
        }
        case K230_RESET_TYPE_HW_AUTO_DONE: {
            /* clear done bit */
            reg = readl(rstc->membase+offset);
            reg |= (1 << done);
            writel(reg, rstc->membase+offset);

            /* set reset bit */
            reg = readl(rstc->membase+offset);

            reg |= (1 << reset);
            writel(reg, rstc->membase+offset);

            /* wait done bit set */
            while(1) {
                reg = readl(rstc->membase+offset);
                if(reg & (1 << done)) {
                    /* clear done and break */
                    writel(reg, rstc->membase+offset);
                    break;
                }
            }
            break;
        }
        case K230_RESET_TYPE_SW_SET_DONE: {
            /* set reset bit */
            reg = readl(rstc->membase+offset);
            if((0x20 == offset) || (0x24 == offset) || (0x80 == offset) || (0x64 == offset))
            {
                reg |= (0 << reset);            //special，复位：reset=0
            }
            else if((0x4 == offset) || (0xc == offset))
            {
                reg |= (1 << reset);
                reg |= (1 << (reset + 0x10));   //note: write enable
            }
            else
            {
                reg |= (1 << reset);
            }
            writel(reg, rstc->membase+offset);

            udelay(10);

            /* clear reset bit */
            if((0x4 != offset) && (0xc != offset))      //special，0x4, 0xc寄存器是自动清零
            {
                if(0xa8 == offset)
                {
                    reg &= ~(1 << reset);                   
                    writel(reg, rstc->membase+offset); 
                }
                else
                {
                    reg &= ~(0 << reset);                   
                    writel(reg, rstc->membase+offset);
                }
            }

            break;
        }
        default:{
            break;
        }
    }

    spin_unlock_irqrestore(&rstc->lock, flags);
    return 0;
}

static int k230_reset_assert(struct reset_controller_dev *rcdev, unsigned long id)
{
    struct k230_reset_controller *rstc = to_k230_reset_controller(rcdev);
    unsigned long flags;
    u32 offset  = (id & K230_RESET_REG_OFFSET_MASK) >> K230_RESET_REG_OFFSET_SHIFT;
    u32 type    = (id & K230_RESET_TYPE_MASK)       >> K230_RESET_TYPE_SHIFT;
    /*u32 done    = (id & K230_RESET_DONE_BIT_MASK) >> K230_RESET_DONE_BIT_SHIFT;*/
    u32 reset   = (id & K230_RESET_ASSERT_BIT_MASK) >> K230_RESET_ASSERT_BIT_SHIFT;
    u32 reg;

#ifdef K230_RESET_DEBUG
    pr_info("[K230_RESET]:k230_reset_assert id = 0x%08x",(int)id);
#endif

    // if(type == K230_RESET_TYPE_HW_AUTO_DONE) {
    //     pr_err("hardware auto done reset DOESNOT support reset assert!");
    // } else {
    //     spin_lock_irqsave(&rstc->lock, flags);
    //     reg = readl(rstc->membase+offset);
    //     /* set reset bit */
    //     reg |= (1 << reset);
    //     writel(reg, rstc->membase+offset);
    //     spin_unlock_irqrestore(&rstc->lock, flags);
    // }
    if(type == K230_RESET_TYPE_HW_AUTO_DONE)
    {
        pr_err("hardware auto done reset DOESNOT support reset assert!");
    }
    else if(type == K230_RESET_TYPE_CPU)
    {
        spin_lock_irqsave(&rstc->lock, flags);
        reg = readl(rstc->membase+offset);
        /* set reset bit */
        reg |= (1 << reset);
        reg |= (1 << (reset + 0x10));   // note: write enable
        writel(reg, rstc->membase+offset);
        spin_unlock_irqrestore(&rstc->lock, flags);
    }
    else
    {
        spin_lock_irqsave(&rstc->lock, flags);
        reg = readl(rstc->membase+offset);
        /* set reset bit */
        if((0x20 == offset) || (0x24 == offset) || (0x80 == offset) || (0x64 == offset))
        {
            reg |= (0 << reset);            //special，复位：reset=0
        }
        else if((0x4 == offset) || (0xc == offset))
        {
            reg |= (1 << reset);
            reg |= (1 << (reset + 0x10));   //note: write enable
        }
        else
        {
            reg |= (1 << reset);
        }
        writel(reg, rstc->membase+offset);
        spin_unlock_irqrestore(&rstc->lock, flags);
    }
    return 0;
}

static int k230_reset_deassert(struct reset_controller_dev *rcdev, unsigned long id)
{
    struct k230_reset_controller *rstc = to_k230_reset_controller(rcdev);
    unsigned long flags;
    u32 offset  = (id & K230_RESET_REG_OFFSET_MASK) >> K230_RESET_REG_OFFSET_SHIFT;
    u32 type    = (id & K230_RESET_TYPE_MASK)       >> K230_RESET_TYPE_SHIFT;
    u32 done    = (id & K230_RESET_DONE_BIT_MASK)   >> K230_RESET_DONE_BIT_SHIFT;
    u32 reset   = (id & K230_RESET_ASSERT_BIT_MASK) >> K230_RESET_ASSERT_BIT_SHIFT;
    u32 reg;

#ifdef K230_RESET_DEBUG
        pr_info("[K230_RESET]:k230_reset_deassert id = 0x%08x",(int)id);
#endif

    // if(type == K230_RESET_TYPE_HW_AUTO_DONE) {
    //     pr_err("hardware auto done reset DOESNOT support reset assert!");
    // } else {
    //     spin_lock_irqsave(&rstc->lock, flags);
    //     reg = readl(rstc->membase+offset);
    //     /* clear reset bit */
    //     reg &= ~(1 << reset);
    //     writel(reg, rstc->membase+offset);
    //     if(type == K230_RESET_TYPE_CPU) {
    //         /* check bit done */
    //         while(1) {
    //             reg = readl(rstc->membase+offset);
    //             if(reg & (1 << done)) {
    //                 /* clear done and break */
    //                 writel(reg, rstc->membase+offset);
    //                 break;
    //             }
    //         }
    //     }
    //     spin_unlock_irqrestore(&rstc->lock, flags);
    if(type == K230_RESET_TYPE_HW_AUTO_DONE)
    {
        pr_err("hardware auto done reset DOESNOT support reset assert!");
    }
    else if(type == K230_RESET_TYPE_CPU)
    {
        spin_lock_irqsave(&rstc->lock, flags);
        reg = readl(rstc->membase+offset);
        /* clear reset bit */
        if(0xc == offset)
        {
            reg &= ~(1 << reset);
            reg &= (1 << (reset + 0x10));   // note: write enable
            writel(reg, rstc->membase+offset);
        }
        
        /* wait done bit set */
        while(1) {
            reg = readl(rstc->membase+offset);
            if(reg & (1 << done)) {
                /* clear done and break */
                writel(reg, rstc->membase+offset);
                break;
            }
        }
        spin_unlock_irqrestore(&rstc->lock, flags);
    }
    else
    {
        spin_lock_irqsave(&rstc->lock, flags);
        reg = readl(rstc->membase+offset);
        /* clear reset bit */
        if((0x4 != offset) && (0xc != offset))      //special，0x4, 0xc寄存器是自动清零
        {
            if(0xa8 == offset)
            {
                reg &= ~(1 << reset);                   
                writel(reg, rstc->membase+offset); 
            }
            else
            {
                reg &= ~(0 << reset);                   
                writel(reg, rstc->membase+offset);
            }
        }
        spin_unlock_irqrestore(&rstc->lock, flags);
    }
    return 0;
}

static const struct reset_control_ops k230_reset_ops = {
    .reset          = k230_reset,
    .assert         = k230_reset_assert,
    .deassert       = k230_reset_deassert,
};

static int k230_reset_probe(struct platform_device *pdev)
{
    struct k230_reset_controller *rstc;
    struct resource *res;

    rstc = devm_kmalloc(&pdev->dev, sizeof(*rstc), GFP_KERNEL);
    if (!rstc) {
        pr_err("k230_reset_init dev_kmalloc error!");
        return -1;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    rstc->membase = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!rstc->membase) {
        pr_err("k230_reset_init devm_ioremap error!");
        return -1;
    }
    #ifdef K230_RESET_DEBUG
        pr_info("[K230_RESET]:sysctl reset phy addr 0x%08x",(int)res->start);
    #endif

    spin_lock_init(&rstc->lock);
    rstc->rst.owner = THIS_MODULE;
    rstc->rst.ops = &k230_reset_ops;
    rstc->rst.of_node = pdev->dev.of_node;
    rstc->rst.of_reset_n_cells = 4;
    rstc->rst.of_xlate = k230_reset_of_xlate;
    if(0 == reset_controller_register(&rstc->rst)) {
    #ifdef K230_RESET_DEBUG
        pr_info("[K230_RESET]:k230_reset_probe ok!");
    #endif
    } else {
        pr_info("[K230_RESET]:k230_reset_probe error!");
    }
    return 0;
}

void k230_reset_exit(struct k230_reset_controller *rstc)
{
    reset_controller_unregister(&rstc->rst);
}

static const struct of_device_id k230_reset_match[] = {
    { .compatible = "canaan,k230-sysctl-reset",},
    {},
};
MODULE_DEVICE_TABLE(of, k230_reset_match);

static struct platform_driver k230_reset_driver = {
    .probe = k230_reset_probe,
    .driver = {
        .name = "k230-sysctl-reset",
        .of_match_table = k230_reset_match,
    }
};

static int __init k230_reset_init(void)
{
    return platform_driver_register(&k230_reset_driver);
}
arch_initcall(k230_reset_init);

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:k230-sysctl-reset");
MODULE_DESCRIPTION("Canaan K230 Reset Driver");

/* how to reset device in the device driver:
    1. write reset attribute in reset_consumer.dtsi
    2. use device_reset(&platform_device->device) api to reset device.
    for example: please see emac driver and dtsi
*/
