/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

/**********************************************************************************************************************************
*  Canaan clock tree driver design description:
*  1. osc24m are automatically registered by clk-fixed-rate.c                   -->compatile is "fixed-clock"               parent is sysctl_root
*  2. pll0/1/2/3 are registered by cannan_k230_pll_clk_setup                    -->compatile is "canaan,k230-pll"           parent is sysctl_root
*  3. pll0/1/2/3 div2/3/4 are automatically registered by clk-fixed-factor.c    -->compatile is "fixed-factor-clock"        parent is sysctl_root
*
*  5. composite clks are reigstered by cannan_k230_clk_composite_setup          -->compatile is "canaan,k230-clk-composite" parent is sysctl_clk
*     k230-clk-composite consider the follow ops:
*     ______________________________________
*     |OPS | mux | rate | gate | read-only |
*     ——————————————————————————————————————
*          | Y/N | Y/N  | Y/N  | Y/N |
*     ——————————————————————————————————————
***********************************************************************************************************************************/
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define CLK_IS_BASIC    BIT(5)
#define K230_CLK_DEBUG

#undef K230_CLK_DEBUG
static DEFINE_SPINLOCK(k230_clk_spinlock);
/**********************************************************************************************************************
* K230 composite clk register
*
* 1. rate_calc_method = 0: div(denominator) is fixed, mul(numerator) is changeble.
*                          rate_div_mask = 0, divide = 1/rate_div_max;
*                          rate_div_mask = 1, divide = 2/rate_div_max;
*                          rate_div_mask = 2, divide = 3/rate_div_max;
* 2. rate_calc_method = 1: div(denominator) is changeble, mul(numerator) is fixed.
*                          rate_div_mask = 0, divide = 1/1;
*                          rate_div_mask = 1, divide = 1/2;
*                          rate_div_mask = 2, divide = 1/3;(denominator 3: less than rate_div_max)
* 3. rate_calc_method = 2: div(denominator) and mul(numerator) are changeable.
*                          rate_mul_mask = mul, rate_div_mask = div
**********************************************************************************************************************/
struct k230_clk_composite {
    struct clk_hw   hw;
    struct clk_lookup *cl;                      /* clock lookup by clk_get() api, if don't regist, please use __clk_lookup*/
    struct clk_ops  *ops;

    /* gate */
    void __iomem    *gate_reg;
    uint32_t        gate_bit;
    uint32_t        gate_bit_reverse;           /* if set to 0, gate_bit=0 indicate gate; if set to 1, gate_bit=1 indicate gate */

    /* mux */
    void __iomem    *mux_reg;
    uint32_t        mux_shift;
    uint32_t        mux_mask;

    /* rate */
    void __iomem    *rate_reg;                 /* the reg of numerator */
    /* config info */
    uint32_t        rate_mul_min;               /* The multiple min value */
    uint32_t        rate_mul_max;               /* The multiple max value */
    uint32_t        rate_div_min;               /* The divide min value */
    uint32_t        rate_div_max;               /* The divide max value */
    uint32_t        rate_calc_method;           /* The method, please see above NOTE */
    /* register info */
    uint32_t        rate_mul_shift;
    uint32_t        rate_mul_mask;              /* please ignore this field when rate_mul_mask=0 */
    uint32_t        rate_div_shift;
    uint32_t        rate_div_mask;              /* please ignore this field when rate_div_mask=0 */
    uint32_t        rate_write_enable_bit;

    /* rate1 */
    void __iomem    *rate_reg_1;                 /* the reg of numerator */
    /* config info */
    uint32_t        rate_mul_min_1;               /* The multiple min value */
    uint32_t        rate_mul_max_1;               /* The multiple max value */
    uint32_t        rate_div_min_1;               /* The divide min value */
    uint32_t        rate_div_max_1;               /* The divide max value */
    /* register info */
    uint32_t        rate_mul_shift_1;
    uint32_t        rate_mul_mask_1;              /* please ignore this field when rate_mul_mask=0 */
    uint32_t        rate_div_shift_1;
    uint32_t        rate_div_mask_1;              /* please ignore this field when rate_div_mask=0 */
    uint32_t        rate_write_enable_bit_1;

    /* read-only attribute */
    uint32_t        composite_read_only;        /* composite_read_only control the read-write attribute of clock. 1 means read-only, 0 means read-write. */

    spinlock_t      *composite_spinlock;

#ifdef CONFIG_DEBUG_FS
    struct dentry   *debugfs;
#endif
};

#define to_k230_clk_composite(_hw) container_of(_hw, struct k230_clk_composite, hw)

//calculate the frequency division factor:
// 0: 1/12, 2/12, 3/12, .......
// 1: 1/1, 1/2, 1/3, .......    
// 2: x/y, .......
static unsigned long k230_clk_composite_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    uint32_t reg_mul;
    uint32_t reg_div;
    uint32_t rate_mul;
    uint32_t rate_div;
    unsigned long flags;

    /* no divider */
    if(NULL == k230_composite->rate_reg) {
        return parent_rate;
    }

    switch(k230_composite->rate_calc_method) {
        /* only mul can be changeable 1/12,2/12,3/12...*/
        case 0: {
            spin_lock_irqsave(k230_composite->composite_spinlock,flags);
            reg_div = (readl(k230_composite->rate_reg) >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
            spin_unlock_irqrestore(k230_composite->composite_spinlock,flags);      
            rate_mul = reg_div + 1;
            rate_div = k230_composite->rate_div_max;
            break;
        }
        /* only div can be changeable, 1/1,1/2,1/3...*/
        case 1: {
            spin_lock_irqsave(k230_composite->composite_spinlock,flags);
            reg_div = (readl(k230_composite->rate_reg) >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
            spin_unlock_irqrestore(k230_composite->composite_spinlock,flags);
            rate_mul = k230_composite->rate_mul_max;
            rate_div = reg_div + 1;
            break;
        }
        /* mul and div can be changeable. */
        case 2: {
            if(NULL == (k230_composite->rate_reg_1))
            {
                spin_lock_irqsave(k230_composite->composite_spinlock,flags);
                reg_mul = (readl(k230_composite->rate_reg) >> k230_composite->rate_mul_shift) & k230_composite->rate_mul_mask;
                reg_div = (readl(k230_composite->rate_reg) >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
                spin_unlock_irqrestore(k230_composite->composite_spinlock,flags);
            }
            else
            {
                spin_lock_irqsave(k230_composite->composite_spinlock,flags);
                reg_mul = (readl(k230_composite->rate_reg_1) >> k230_composite->rate_mul_shift_1) & k230_composite->rate_mul_mask_1;
                reg_div = (readl(k230_composite->rate_reg) >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
                spin_unlock_irqrestore(k230_composite->composite_spinlock,flags);
            }
            rate_mul = reg_mul;
            rate_div = reg_div;
            break;
        }
        default: {
            pr_err("[K230_CLK]:clk %s recalc rate error.",clk_hw_get_name(hw));
            return 0;
        }
    }
#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:clk %s recalc rate %d", clk_hw_get_name(hw),(uint32_t)(parent_rate * rate_mul / rate_div));
#endif
    return parent_rate * rate_mul / rate_div;
}

#if 0
/* POW(x, y): x^y */
static unsigned long POW(int a, int n) {
    unsigned int c = a;
	if(n < 0)
	    return 0;
    else if(n == 0)
		return 1;
	else {
	    while(1) {
            n--;
            if(n == 0)
                return c;
            else
                c *= a;
	    }
	}
}
#endif

/* Find the approximate value based on the parent node and the current node */
static int k230_clk_find_approximate(struct k230_clk_composite *k230_composite,
                                            uint32_t mul_min, 
                                            uint32_t mul_max, 
                                            uint32_t div_min, 
                                            uint32_t div_max, 
                                            uint32_t method,
                                            unsigned long rate, 
                                            unsigned long parent_rate,
                                            uint32_t *div, 
                                            uint32_t *mul)
{
    long abs_min;
    long abs_current;
    /* we used interger to instead of float */
    long perfect_divide;

    uint32_t a,b;
    int i = 0;
    int j = 0;
    int n = 0;
    unsigned long mul_ulong,div_ulong;

    volatile uint32_t codec_clk[9] = {
        2048000,
        3072000,
        4096000,
        6144000,
        8192000,
        11289600,
        12288000,
        24576000,
        49152000
    };
    volatile uint32_t codec_div[9][2] = {
        {3125, 16},
        {3125, 24},
        {3125, 32},
        {3125, 48},
        {3125, 64},
        {15625, 441},
        {3125, 96},
        {3125, 192},
        {3125, 384}
    };

    volatile uint32_t pdm_clk[20] = {
        128000,
        192000,
        256000,
        384000,
        512000,
        768000,
        1024000,
        1411200,
        1536000,
        2048000,
        2822400,
        3072000,
        4096000,
        5644800,
        6144000,
        8192000,
        11289600,
        12288000,
        24576000,
        49152000
    };
    volatile uint32_t pdm_div[20][2] = {
        {3125, 1},
        {6250, 3},
        {3125, 2},
        {3125, 3},
        {3125, 4},
        {3125, 6},
        {3125, 8},
        {125000, 441},
        {3125, 12},
        {3125, 16},
        {62500, 441},
        {3125, 24},
        {3125, 32},
        {31250, 441},
        {3125, 48},
        {3125, 64},
        {15625, 441},
        {3125, 96},
        {3125, 192},
        {3125, 384}
    };

    switch(method) {
        /* only mul can be changeable 1/12,2/12,3/12...*/
        case 0: {
            perfect_divide = (long)((parent_rate*1000) / rate);
            abs_min = abs(perfect_divide - (long)(((long)div_max*1000)/(long)mul_min));

            for(i = mul_min + 1; i <= mul_max; i++) {
                abs_current = abs(perfect_divide - (long)((long)((long)div_max*1000)/(long)i));
                if(abs_min > abs_current ) {
                    abs_min = abs_current;
                    *mul = i;
                }
            }

           *div = div_min;
            break;
        }
        /* only div can be changeable, 1/1,1/2,1/3...*/
        case 1: {
            perfect_divide = (long)((parent_rate*1000) / rate);
            abs_min = abs(perfect_divide - (long)(((long)div_min*1000)/(long)mul_max));
            *div = div_min;

            for(i = div_min + 1; i <= div_max; i++) {
                abs_current = abs(perfect_divide - (long)((long)((long)i*1000)/(long)mul_max));
                if(abs_min > abs_current ) {
                    abs_min = abs_current;
                    *div = i;
                }
            }
            *mul = mul_min;
            break;
        }
        /* mul and div can be changeable. */
        case 2: {
            if((0x91100038 == (k230_composite->rate_reg)) || (0x9110003c == (k230_composite->rate_reg)))
            {
                for(j=0;j<9;j++)
                {
                    if(0 == (rate - codec_clk[j]))
                    {
                        *div = codec_div[j][0];
                        *mul = codec_div[j][1];
                    }
                }
            }
            else if((0x91100034 == (k230_composite->rate_reg)) || (0x91100040 == (k230_composite->rate_reg)))
            {
                for(j=0;j<20;j++)
                {
                    if(0 == (rate - pdm_clk[j]))
                    {
                        *div = pdm_div[j][0];
                        *mul = pdm_div[j][1];
                    }
                }
            }
            else
            {
                return -1;
            }
            break;
        }

        default:{
		    pr_err("[canaan_clk_err]:k230_clk_find_approximate method error!\n");
            return -1;
        }
    }
#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:find approximate mul %d, div %d",*mul,*div);
#endif
    return 0;
}

/* calc round rate and set */
static int k230_clk_composite_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    uint32_t mul=0;
    uint32_t div=0;
    uint32_t reg=0;
    uint32_t reg_1=0;
    unsigned long flags;

    if(NULL == k230_composite->rate_reg) {
        return -1;
    }

    if((rate > parent_rate) || (0 == rate) || (0 == parent_rate)) {
        return -1;
    }

    if(1 == k230_composite->composite_read_only) {
        return -1;
    }

    if(k230_clk_find_approximate(k230_composite, 
                              k230_composite->rate_mul_min, 
                              k230_composite->rate_mul_max, 
                              k230_composite->rate_div_min,
                              k230_composite->rate_div_max,
                              k230_composite->rate_calc_method,
                              rate,
                              parent_rate,
                              &div,
                              &mul)){
        pr_err("[canaan_clk_err]:clk %s set rate error!\n", clk_hw_get_name(hw));
        return -1;
    }

    if(NULL == (k230_composite->rate_reg_1))
    {
        spin_lock_irqsave(k230_composite->composite_spinlock,flags);
        reg = readl(k230_composite->rate_reg);
        reg &= ~((k230_composite->rate_div_mask) << (k230_composite->rate_div_shift));
        if(k230_composite->rate_calc_method == 2) {
            reg &= ~((k230_composite->rate_mul_mask) << (k230_composite->rate_mul_shift));
        }
        reg |= (1 << k230_composite->rate_write_enable_bit);
        #if 0
        if(k230_composite->rate_calc_method == 1)
        {
            reg |= ((div - 1) & k230_composite->rate_div_mask) << (k230_composite->rate_div_shift);
        }else
        {
            reg |= (div & k230_composite->rate_div_mask) << (k230_composite->rate_div_shift);
        }
        if(k230_composite->rate_calc_method == 2) {
            reg |= (mul & k230_composite->rate_mul_mask) << (k230_composite->rate_mul_shift);
        }
        #endif
        #if 1
        if(k230_composite->rate_calc_method == 1)
        {
            reg |= ((div - 1) & k230_composite->rate_div_mask) << (k230_composite->rate_div_shift);
        }
        else if(k230_composite->rate_calc_method == 0)
        {
            reg |= ((mul - 1) & k230_composite->rate_div_mask) << (k230_composite->rate_div_shift);
        }
        else
        {
            reg |= (mul & k230_composite->rate_mul_mask) << (k230_composite->rate_mul_shift);
            reg |= (div & k230_composite->rate_div_mask) << (k230_composite->rate_div_shift);
        }
        #endif
        writel(reg, k230_composite->rate_reg);
        spin_unlock_irqrestore(k230_composite->composite_spinlock,flags);
    }
    else
    {
        spin_lock_irqsave(k230_composite->composite_spinlock,flags);
        reg = readl(k230_composite->rate_reg);
        reg_1 = readl(k230_composite->rate_reg_1);
        reg &= ~((k230_composite->rate_div_mask) << (k230_composite->rate_div_shift));
        reg_1 &= ~((k230_composite->rate_mul_mask_1) << (k230_composite->rate_mul_shift_1));
        reg_1 |= (1 << k230_composite->rate_write_enable_bit_1);

        reg_1 |= (mul & k230_composite->rate_mul_mask_1) << (k230_composite->rate_mul_shift_1);
        reg |= (div & k230_composite->rate_div_mask) << (k230_composite->rate_div_shift);

        writel(reg, k230_composite->rate_reg);
        writel(reg_1, k230_composite->rate_reg_1);
        spin_unlock_irqrestore(k230_composite->composite_spinlock,flags);
    }
#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:clk_set_rate %s rate %d, parent_rate %d",clk_hw_get_name(hw),(int32_t)rate,(uint32_t)parent_rate);
#endif
    return 0;
}

static long k230_clk_composite_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    return rate;
}

static int k230_clk_composite_is_enable(struct clk_hw *hw)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    uint32_t composite_gate_value;
    unsigned long flags;

    spin_lock_irqsave(k230_composite->composite_spinlock, flags);
    composite_gate_value = readl(k230_composite->gate_reg);
    spin_unlock_irqrestore(k230_composite->composite_spinlock, flags);

    if(((composite_gate_value >> k230_composite->gate_bit) & 0x1) ^ k230_composite->gate_bit_reverse) {
#ifdef K230_CLK_DEBUG
        pr_info("[K230_CLK]:clk %s enable status YES!",clk_hw_get_name(hw));
#endif
        /*xor = 1,clock enable */
        return 1;
    } else {
#ifdef K230_CLK_DEBUG
        pr_info("[K230_CLK]:clk %s enable status NO!",clk_hw_get_name(hw));
#endif
        return 0;
    }
}

static int k230_clk_composite_enable(struct clk_hw *hw)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    uint32_t composite_gate_value;
    unsigned long flags;

    if(1 == k230_composite->composite_read_only) {
        return -1;
    }

    spin_lock_irqsave(k230_composite->composite_spinlock, flags);
    composite_gate_value = readl(k230_composite->gate_reg);
    if(k230_composite->gate_bit_reverse == 0) {
        composite_gate_value |= (1 << k230_composite->gate_bit);
    } else {
        composite_gate_value &= ~(1 << k230_composite->gate_bit);
    }
    writel(composite_gate_value,k230_composite->gate_reg);
    spin_unlock_irqrestore(k230_composite->composite_spinlock, flags);

#ifdef K230_CLK_DEBUG
    spin_lock_irqsave(k230_composite->composite_spinlock, flags);
    composite_gate_value = readl(k230_composite->gate_reg);
    spin_unlock_irqrestore(k230_composite->composite_spinlock, flags);
    pr_info("[K230_CLK]:enable clk %s addr 0x%08x reg_value 0x%0x",
           clk_hw_get_name(hw),
           (int)(long)k230_composite->gate_reg, 
           composite_gate_value);
#endif
    return 0;
}

static void k230_clk_composite_disable(struct clk_hw *hw)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    uint32_t composite_gate_value;
    unsigned long flags;

    if(1 == k230_composite->composite_read_only) {
        pr_err("[canaan_clk_err]:clk %s is read-only!\n", clk_hw_get_name(hw));
        return;
    }

    spin_lock_irqsave(k230_composite->composite_spinlock, flags);
    composite_gate_value = readl(k230_composite->gate_reg);
    if(k230_composite->gate_bit_reverse == 1) {
        composite_gate_value |= (1 << k230_composite->gate_bit);
    } else {
        composite_gate_value &= ~(1 << k230_composite->gate_bit);
    }
    writel(composite_gate_value,k230_composite->gate_reg);
    spin_unlock_irqrestore(k230_composite->composite_spinlock, flags);

#ifdef K230_CLK_DEBUG
        spin_lock_irqsave(k230_composite->composite_spinlock, flags);
        composite_gate_value = readl(k230_composite->gate_reg);
        spin_unlock_irqrestore(k230_composite->composite_spinlock, flags);
        pr_info("[K230_CLK]:disable clk %s addr 0x%08x reg_value 0x%0x",
               clk_hw_get_name(hw),
               (int)(long)k230_composite->gate_reg, 
               composite_gate_value);
#endif
}

static int k230_clk_composite_set_parent(struct clk_hw *hw, u8 index)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    uint32_t composite_mux_value;
    unsigned long flags;

    if(1 == k230_composite->composite_read_only) {
        return -1;
    }

    composite_mux_value = (k230_composite->mux_mask & index) << k230_composite->mux_shift;

    spin_lock_irqsave(k230_composite->composite_spinlock, flags);
    writel(composite_mux_value,k230_composite->mux_reg);
    spin_unlock_irqrestore(k230_composite->composite_spinlock, flags);

#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:set parent %s clk mux addr 0x%08x value 0x%08x shift %d mask 0x%08x return %d",
                                                                   clk_hw_get_name(hw),
                                                                   (int)(long)k230_composite->mux_reg, 
                                                                   (int)composite_mux_value,
                                                                   k230_composite->mux_shift,
                                                                   k230_composite->mux_mask,
                                                                   (composite_mux_value >> k230_composite->mux_shift) & k230_composite->mux_mask);
#endif
    return 0;
}

static u8 k230_clk_composite_get_parent(struct clk_hw *hw)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    uint32_t composite_mux_value;
    unsigned long flags;

    spin_lock_irqsave(k230_composite->composite_spinlock, flags);
    composite_mux_value = readl(k230_composite->mux_reg);
    spin_unlock_irqrestore(k230_composite->composite_spinlock, flags);

#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:get parent %s clk mux addr 0x%08x value 0x%08x shift %d mask 0x%08x return %d",
                                                                   clk_hw_get_name(hw),
                                                                   (int)(long)k230_composite->mux_reg, 
                                                                   (int)composite_mux_value,
                                                                   k230_composite->mux_shift,
                                                                   k230_composite->mux_mask,
                                                                   (composite_mux_value >> k230_composite->mux_shift) & k230_composite->mux_mask);
#endif
    return (composite_mux_value >> k230_composite->mux_shift) & k230_composite->mux_mask;
}

static int k230_clk_composite_init(struct clk_hw *hw)
{
    if(k230_clk_composite_is_enable(hw)) {
#ifdef K230_CLK_DEBUG
        pr_info("[K230_CLK]:%s hardware auto enable, set software enable!",clk_hw_get_name(hw));
#endif
        clk_prepare_enable(hw->clk);
    }
    return 0;
}

static int k230_debugfs_clk_composite_init(struct clk_hw *hw);

static struct clk *_of_cannan_k230_clk_composite_setup(struct device_node *node)
{
    struct device_node *parent_sys_clk = NULL;
    void __iomem *base_reg = NULL;
    const char **parent_name;
    struct k230_clk_composite *k230_clk_composite = NULL;
    struct clk_init_data init;
    struct clk_hw *hw;
    bool mux,rate,gate,only_read;
    struct clk_ops *p_clk_ops = NULL;
    uint32_t composite_reg_offset;
    uint32_t composite_reg_offset_1;

    /* parent is sysctl_clk, get address from dts */
    parent_sys_clk = of_get_parent(node);
    if(NULL == parent_sys_clk) {
        pr_err("[canaan_clk_err]:Could not get %s parent node.\n", node->name);
        return NULL;
    }

    /* remap io base address */
    base_reg = of_iomap(parent_sys_clk, 0);
    if(NULL == base_reg) {
        pr_err("[canaan_clk_err]:Could not iomap %s node.\n", node->name);
        return NULL;
    }

    /* kzalloc composite clock struct */
    k230_clk_composite = kzalloc(sizeof(struct k230_clk_composite), GFP_KERNEL);
    if(NULL == k230_clk_composite) {
        pr_err("[canaan_clk_err]:Could not kzalloc sruct k230_clk_composite.\n");
        goto out_unmap;
    }

    if(of_property_read_u32(node, "clk-parent-reg-offset",                  &composite_reg_offset)) {
        mux = false;
    } else {
        mux = true;
        k230_clk_composite->mux_reg = (void __iomem *)((uint64_t)base_reg + (uint64_t)composite_reg_offset);

        if((of_property_read_u32(node, "clk-parent-reg-value-shift",        &k230_clk_composite->mux_shift))            ||
           (of_property_read_u32(node, "clk-parent-reg-value-mask",         &k230_clk_composite->mux_mask))) {
                pr_err("[canaan_clk_err]:Parse composite node %s mux error.\n",     node->name);
                goto free_k230_clk_composite;
           }
    }

    if((of_property_read_u32(node, "clk-rate-reg-offset",                    &composite_reg_offset))) {
        rate = false;
    } else {
        rate = true;
        k230_clk_composite->rate_reg = (void __iomem *)((uint64_t)base_reg + (uint64_t)composite_reg_offset);

        if(true == (of_property_read_bool(node, "clk-rate-reg-offset_1")))
        {
            of_property_read_u32(node, "clk-rate-reg-offset_1",                    &composite_reg_offset_1);
            k230_clk_composite->rate_reg_1 = (void __iomem *)((uint64_t)base_reg + (uint64_t)composite_reg_offset_1);
            if((of_property_read_u32(node, "clk-rate-conf-div-min",             &k230_clk_composite->rate_div_min))         ||
                (of_property_read_u32(node, "clk-rate-conf-div-max",             &k230_clk_composite->rate_div_max))         ||
                (of_property_read_u32(node, "clk-rate-calc-method",              &k230_clk_composite->rate_calc_method))     ||
                (of_property_read_u32(node, "clk-rate-reg-div-value-shift",      &k230_clk_composite->rate_div_shift))       ||
                (of_property_read_u32(node, "clk-rate-reg-div-value-mask",       &k230_clk_composite->rate_div_mask))        ||
                (of_property_read_u32(node, "clk-rate-conf-mul-min_1",             &k230_clk_composite->rate_mul_min_1))         ||
                (of_property_read_u32(node, "clk-rate-conf-mul-max_1",             &k230_clk_composite->rate_mul_max_1))         ||
                (of_property_read_u32(node, "clk-rate-reg-mul-value-shift_1",      &k230_clk_composite->rate_mul_shift_1))       ||
                (of_property_read_u32(node, "clk-rate-reg-mul-value-mask_1",       &k230_clk_composite->rate_mul_mask_1))        ||
                (of_property_read_u32(node, "clk-rate-reg-write-enable-bit_1",     &k230_clk_composite->rate_write_enable_bit_1))) {
                        pr_err("[canaan_clk_err]:Parse composite node %s rate error.\n",     node->name);
                        goto free_k230_clk_composite;
           } 
        }
        else
        {
            if((of_property_read_u32(node, "clk-rate-conf-mul-min",             &k230_clk_composite->rate_mul_min))         ||
                (of_property_read_u32(node, "clk-rate-conf-mul-max",             &k230_clk_composite->rate_mul_max))         ||
                (of_property_read_u32(node, "clk-rate-conf-div-min",             &k230_clk_composite->rate_div_min))         ||
                (of_property_read_u32(node, "clk-rate-conf-div-max",             &k230_clk_composite->rate_div_max))         ||
                (of_property_read_u32(node, "clk-rate-calc-method",              &k230_clk_composite->rate_calc_method))     ||
                (of_property_read_u32(node, "clk-rate-reg-mul-value-shift",      &k230_clk_composite->rate_mul_shift))       ||
                (of_property_read_u32(node, "clk-rate-reg-mul-value-mask",       &k230_clk_composite->rate_mul_mask))        ||
                (of_property_read_u32(node, "clk-rate-reg-div-value-shift",      &k230_clk_composite->rate_div_shift))       ||
                (of_property_read_u32(node, "clk-rate-reg-div-value-mask",       &k230_clk_composite->rate_div_mask))        ||
                (of_property_read_u32(node, "clk-rate-reg-write-enable-bit",     &k230_clk_composite->rate_write_enable_bit))) {
                        pr_err("[canaan_clk_err]:Parse composite node %s rate error.\n",     node->name);
                        goto free_k230_clk_composite;
           }
        }
    }

    if(of_property_read_u32(node, "clk-gate-reg-offset",                    &composite_reg_offset)) {
        gate = false;
    } else {
        gate = true;
        k230_clk_composite->gate_reg = (void __iomem *)((uint64_t)base_reg + (uint64_t)composite_reg_offset);

        if((of_property_read_u32(node, "clk-gate-reg-bit-enable",           &k230_clk_composite->gate_bit))             ||
           (of_property_read_u32(node, "clk-gate-reg-bit-reverse",          &k230_clk_composite->gate_bit_reverse))) {
                pr_err("[canaan_clk_err]:Parse composite node %s gate error.\n",     node->name);
                goto free_k230_clk_composite;
           }
    }

    /* parse the read-only */
    if(of_property_read_u32(node, "read-only",                              &k230_clk_composite->composite_read_only)) {
        only_read = false;
        pr_err("[canaan_clk_err]:Parse composite node %s read-only error.\n",     node->name);
        goto free_k230_clk_composite;
    }
    else {
        only_read = true;
    }


    if((true != mux) && (true != rate) && (true != gate) && (true != only_read)) {
        pr_err("[canaan_clk_err]:Parse composite node %s error.\n",    node->name);
        goto free_k230_clk_composite;
    }

    p_clk_ops = kzalloc(sizeof(struct clk_ops), GFP_KERNEL);

    if(NULL == p_clk_ops) {
        pr_err("[canaan_clk_err]:kzalloc composite clk ops %s error.\n",    node->name);
        goto free_k230_clk_composite;
    }

    if(true == mux) {
        p_clk_ops->set_parent   = k230_clk_composite_set_parent;
        p_clk_ops->get_parent   = k230_clk_composite_get_parent;
    }
    if(true == rate) {
        p_clk_ops->recalc_rate  = k230_clk_composite_recalc_rate;
        p_clk_ops->set_rate     = k230_clk_composite_set_rate;
        p_clk_ops->round_rate   = k230_clk_composite_round_rate;
    }
    if(true == gate) {
        p_clk_ops->is_enabled   = k230_clk_composite_is_enable;
        p_clk_ops->enable       = k230_clk_composite_enable;
        p_clk_ops->disable      = k230_clk_composite_disable;
        p_clk_ops->init         = k230_clk_composite_init;
    }

    k230_clk_composite->composite_spinlock = &k230_clk_spinlock;

    init.num_parents = of_clk_get_parent_count(node);
    parent_name = kcalloc(init.num_parents, sizeof(const char *), GFP_KERNEL);
    if (!parent_name) {
        goto free_ops;
    }
    init.num_parents = of_clk_parent_fill(node, parent_name,init.num_parents);
    init.parent_names = parent_name;
    init.name = node->name;
    init.ops = p_clk_ops;
    init.flags = CLK_IS_BASIC/*|flags*/;
    k230_clk_composite->hw.init = &init;

    hw = &k230_clk_composite->hw;
    if(clk_hw_register(NULL, hw)){
        goto free_parent_names;
    }

    if(of_clk_add_provider(node, of_clk_src_simple_get, hw->clk)){
        goto unregister_hw_clk;
    }

    k230_clk_composite->cl = clkdev_create(hw->clk, node->name, NULL);
    if(NULL == k230_clk_composite->cl) {
        goto unregister_provider;
    }

#ifdef CONFIG_DEBUG_FS
    k230_debugfs_clk_composite_init(hw);
#endif

    return hw->clk;

unregister_provider:
    of_clk_del_provider(node);
unregister_hw_clk:
    clk_unregister(hw->clk);
free_parent_names:
    kfree(parent_name);
free_ops:
    kfree(p_clk_ops);
free_k230_clk_composite:
    kfree(k230_clk_composite);
out_unmap:
    iounmap(base_reg);

    return NULL;
}

#if 0
void __init of_cannan_k230_clk_composite_setup(struct device_node *node)
{
    _of_cannan_k230_clk_composite_setup(node);
}

CLK_OF_DECLARE(k230_composite, "canaan,k230-clk-composite", of_cannan_k230_clk_composite_setup);
#endif

static int canaan_k230_clk_composite_probe(struct platform_device *pdev)
{
    struct clk *clk;
    /*
     * This function is not executed when of_fixed_clk_setup
     * succeeded.
     */
#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:register clock node %s",pdev->dev.of_node->name);
#endif
    clk = _of_cannan_k230_clk_composite_setup(pdev->dev.of_node);
    if (IS_ERR(clk))
        return PTR_ERR(clk);

    platform_set_drvdata(pdev, clk);

#ifdef K230_CLK_DEBUG     
    pr_info("[K230_CLK]:k230_clk_composite_probe ok!"); 
#endif

    return 0;
}

static const struct of_device_id of_canaan_k230_clk_composite_ids[] = {
	{ .compatible = "canaan,k230-clk-composite" },
	{ }
};
MODULE_DEVICE_TABLE(of, of_canaan_k230_clk_composite_ids);

static struct platform_driver of_canaan_k230_clk_composite_driver = {
    .driver = {
        .name = "k230-clk-composite",
        .of_match_table = of_canaan_k230_clk_composite_ids,
    },
    .probe = canaan_k230_clk_composite_probe,
};

#if 0
builtin_platform_driver(of_canaan_k230_clk_composite_driver);
#else
static int __init canaan_k230_clk_composite_init_driver(void)
{
    return platform_driver_register(&of_canaan_k230_clk_composite_driver);
}
arch_initcall(canaan_k230_clk_composite_init_driver);
#endif


/**********************************************************************************************************************
*
* K230 pll clk register
*
**********************************************************************************************************************/
struct k230_clk_pll {
    struct clk_hw       hw;
    struct clk_lookup   *cl;                    /* clock lookup by clk_get() api, if don't regist, please use __clk_lookup*/
    struct clk_ops      *pll_ops;

    /* pll output freqency = osc24m * (fb_div+1) / (ref_div+1) / (out_div+1) */
    void __iomem    *pll_reg_divide;
    uint32_t        pll_out_div_shift;
    uint32_t        pll_out_div_mask;
    uint32_t        pll_ref_div_shift;
    uint32_t        pll_ref_div_mask;
    uint32_t        pll_fb_div_shift;
    uint32_t        pll_fb_div_mask;

    /* if bypass=true, output freqency = osc */
    void __iomem    *pll_reg_bypass;
    uint32_t        pll_bypass_enable_bit;

    /* pll output can be gated */
    void __iomem    *pll_reg_gate;
    uint32_t        pll_gate_enable_bit;
    uint32_t        pll_gate_write_enable_bit;

    /* pll lock status */
    void __iomem    *pll_reg_lock;
    uint32_t        pll_lock_status_bit;

    /* read-only attribute */
    uint32_t        pll_read_only;                  /* pll_read_only control the read-write attribute of clock. 1 means read-only, 0 means read-write. */

    spinlock_t      *pll_spinlock;

#ifdef CONFIG_DEBUG_FS
    struct dentry   *debugfs;
#endif
};

/* from member(hw) and member addr(_hw) to get k230_clk_pll base addr */
#define to_k230_clk_pll(_hw) container_of(_hw, struct k230_clk_pll, hw)

static unsigned long k230_clk_pll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    struct k230_clk_pll *k230_pll = to_k230_clk_pll(hw);
    unsigned long int rate;
    uint32_t pll_divide_value;
    uint32_t fb_div,ref_div,out_div;
    unsigned long flags;

    /* lock dual cores */
    spin_lock_irqsave(k230_pll->pll_spinlock,flags);

    /* If bypass, pll out freqency = parent freqency(osc24m) */
    if((readl(k230_pll->pll_reg_bypass) & (1 << k230_pll->pll_bypass_enable_bit))) {
        // spin_unlock(k230_pll->pll_spinlock);
        spin_unlock_irqrestore(k230_pll->pll_spinlock,flags);
        return parent_rate;
    } else {
        /* query pll lock status */
        if(!(readl(k230_pll->pll_reg_lock) & (1 << k230_pll->pll_lock_status_bit)))
        {
            spin_unlock_irqrestore(k230_pll->pll_spinlock,flags);
            pr_err("[ERROR K230_CLK]:pll %s is unlock.\n", clk_hw_get_name(hw));
            return 0;
        } else {
             /* pll output freqency = osc24m * (fb_div+1) / (ref_div+1) / (out_div+1) */
            pll_divide_value = readl(k230_pll->pll_reg_divide);
            fb_div = ((pll_divide_value >> k230_pll->pll_fb_div_shift) & k230_pll->pll_fb_div_mask) + 1;
            ref_div = ((pll_divide_value >> k230_pll->pll_ref_div_shift) & k230_pll->pll_ref_div_mask) + 1;
            out_div = ((pll_divide_value >> k230_pll->pll_out_div_shift) & k230_pll->pll_out_div_mask) + 1;
            rate = parent_rate * fb_div / ref_div / out_div;
            spin_unlock_irqrestore(k230_pll->pll_spinlock, flags);
#ifdef K230_CLK_DEBUG
            pr_info("[K230_CLK]:pll %s recalc rate %ld.\n", clk_hw_get_name(hw), rate);
#endif
            return rate;
        }
    }
}

static int k230_clk_pll_is_enable(struct clk_hw *hw)
{
    struct k230_clk_pll *k230_pll = to_k230_clk_pll(hw);
    uint32_t pll_gate_value;
    unsigned long flags;

    spin_lock_irqsave(k230_pll->pll_spinlock, flags);
    pll_gate_value = readl(k230_pll->pll_reg_gate);
    spin_unlock_irqrestore(k230_pll->pll_spinlock, flags);

    if(pll_gate_value & (1 << k230_pll->pll_gate_enable_bit)) {
#ifdef K230_CLK_DEBUG
        pr_info("[K230_CLK]:pll %s enable status YES.\n", clk_hw_get_name(hw));
#endif
        return 1;   /* enable */
    } else {
#ifdef K230_CLK_DEBUG
        pr_info("[K230_CLK]:pll %s enable status NO.\n", clk_hw_get_name(hw));
#endif
        return 0;   /* disable */
    }
}

static int k230_clk_pll_enable(struct clk_hw *hw)
{
    struct k230_clk_pll *k230_pll = to_k230_clk_pll(hw);
    uint32_t pll_gate_value;
    unsigned long flags;

    if(1 == k230_pll->pll_read_only) {
        pr_err("[canaan_clk_err]:clk %s is read-only!\n", clk_hw_get_name(hw));
        return -1;
    }

    /* lock dual cores */
    spin_lock_irqsave(k230_pll->pll_spinlock, flags);
    pll_gate_value = readl(k230_pll->pll_reg_gate);

    pll_gate_value |= (1 << k230_pll->pll_gate_enable_bit);
    pll_gate_value |= (1 << k230_pll->pll_gate_write_enable_bit);

    writel(pll_gate_value,k230_pll->pll_reg_gate);
    spin_unlock_irqrestore(k230_pll->pll_spinlock, flags);
#ifdef K230_CLK_DEBUG
            pr_info("[K230_CLK]:pll %s set enable.\n", clk_hw_get_name(hw));
#endif
    return 0;
}

static void k230_clk_pll_disable(struct clk_hw *hw)
{
    struct k230_clk_pll *k230_pll = to_k230_clk_pll(hw);
    uint32_t pll_gate_value;
    unsigned long flags;

    if(1 == k230_pll->pll_read_only) {
        pr_err("[canaan_clk_err]:clk %s is read-only!\n", clk_hw_get_name(hw));
        return;
    }

    /* lock dual cores */
    spin_lock_irqsave(k230_pll->pll_spinlock, flags);
    pll_gate_value = readl(k230_pll->pll_reg_gate);

    pll_gate_value &= ~(1 << k230_pll->pll_gate_enable_bit);
    pll_gate_value |= (1 << k230_pll->pll_gate_write_enable_bit);

    writel(pll_gate_value,k230_pll->pll_reg_gate);
    spin_unlock_irqrestore(k230_pll->pll_spinlock, flags);
#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:pll %s set disable.\n", clk_hw_get_name(hw));
#endif
}

static int k230_clk_pll_init(struct clk_hw *hw) {
    if(k230_clk_pll_is_enable(hw)) {
#ifdef K230_CLK_DEBUG
        pr_info("[K230_CLK]:%s hardware auto enable, set software enable!",clk_hw_get_name(hw));
#endif
        clk_prepare_enable(hw->clk);
    }
    return 0;
}

const struct clk_ops k230_clk_pll_ops = {
    .init           = k230_clk_pll_init,
    .recalc_rate    = k230_clk_pll_recalc_rate,
    .is_enabled     = k230_clk_pll_is_enable,
    .enable         = k230_clk_pll_enable,
    .disable        = k230_clk_pll_disable,
};
EXPORT_SYMBOL_GPL(k230_clk_pll_ops);

#ifdef CONFIG_DEBUG_FS
static int k230_debugfs_clk_pll_init(struct clk_hw *hw);
#endif

static struct clk * _of_canaan_k230_pll_clk_setup(struct device_node *node)
{
    struct device_node *parent_sys_boot = NULL;
    void __iomem *base_reg = NULL;
    const char *parent_name;
    struct k230_clk_pll *k230_clk_pll = NULL;
    struct clk_init_data init;
    struct clk_hw *hw;
    uint32_t pll_reg_offset;

    /* parent is sysctl_boot, get address from dts */
    parent_sys_boot = of_get_parent(node);
    if(NULL == parent_sys_boot) {
        pr_err("[canaan_clk_err]:Could not get %s parent node.\n", node->name);
        return NULL;
    }

    /* remap io base address */
    base_reg = of_iomap(parent_sys_boot, 0);
    if(NULL == base_reg) {
        pr_err("[canaan_clk_err]:Could not iomap %s node.\n", node->name);
        return NULL;
    }

    /* kzalloc pll clock struct */
    k230_clk_pll = kzalloc(sizeof(struct k230_clk_pll), GFP_KERNEL);
    if(NULL == k230_clk_pll) {
        pr_err("[canaan_clk_err]:Could not kzalloc sruct k230_clk_pll.\n");
        goto out_unmap;
    }

    /* parse divide */
    if((of_property_read_u32(node, "clk-divide-reg-offset",                 &pll_reg_offset))                               ||
       (of_property_read_u32(node, "clk-divide-out-reg-value-shift",        &k230_clk_pll->pll_out_div_shift))              || 
       (of_property_read_u32(node, "clk-divide-out-reg-value-mask",         &k230_clk_pll->pll_out_div_mask))               ||  
       (of_property_read_u32(node, "clk-divide-ref-reg-value-shift",        &k230_clk_pll->pll_ref_div_shift))              || 
       (of_property_read_u32(node, "clk-divide-ref-reg-value-mask",         &k230_clk_pll->pll_ref_div_mask))               || 
       (of_property_read_u32(node, "clk-divide-fb-reg-value-shift",         &k230_clk_pll->pll_fb_div_shift))               || 
       (of_property_read_u32(node, "clk-divide-fb-reg-value-mask",          &k230_clk_pll->pll_fb_div_mask))) {
        pr_err("[canaan_clk_err]:parse k230 pll divider error.\n");
        goto free_k230_clk_pll;
    } else {
        k230_clk_pll->pll_reg_divide = (void __iomem *)((uint64_t)base_reg + (uint64_t)pll_reg_offset);
    }

    /* parse bypass, bypass belongs to divide */
    if((of_property_read_u32(node, "clk-divide-bypass-reg-offset",          &pll_reg_offset))                               ||
       (of_property_read_u32(node, "clk-divide-bypass-reg-enable-bit",      &k230_clk_pll->pll_bypass_enable_bit))) {
        pr_err("[canaan_clk_err]:parse k230 pll bypass error.\n");
        goto free_k230_clk_pll;
    } else {
        k230_clk_pll->pll_reg_bypass = (void __iomem *)((uint64_t)base_reg + (uint64_t)pll_reg_offset);
    }

    /* parse gate */
    if((of_property_read_u32(node, "clk-gate-reg-offset",                   &pll_reg_offset))                               ||
       (of_property_read_u32(node, "clk-gate-reg-enable-bit",               &k230_clk_pll->pll_gate_enable_bit))            ||
       (of_property_read_u32(node, "clk-gate-reg-write-enable-bit",         &k230_clk_pll->pll_gate_write_enable_bit))) {
        pr_err("[canaan_clk_err]:parse k230 pll gate error.\n");
        goto free_k230_clk_pll;
    } else {
        k230_clk_pll->pll_reg_gate = (void __iomem *)((uint64_t)base_reg + (uint64_t)pll_reg_offset);
    }

    /* parse lock */
    if((of_property_read_u32(node, "clk-lock-reg-offset",                   &pll_reg_offset))                               ||
       (of_property_read_u32(node, "clk-lock-reg-status_bit",               &k230_clk_pll->pll_lock_status_bit)))
    {
        pr_err("[canaan_clk_err]:parse k230 pll lock error.\n");
        goto free_k230_clk_pll;
    } else {
        k230_clk_pll->pll_reg_lock = (void __iomem *)((uint64_t)base_reg + (uint64_t)pll_reg_offset);
    }

    /* parse read-only */
    if(of_property_read_u32(node, "read-only",                              &k230_clk_pll->pll_read_only)) {
        pr_err("[canaan_clk_err]:parse k230 pll read-only error.\n");
        goto free_k230_clk_pll;
    }

    k230_clk_pll->pll_spinlock = &k230_clk_spinlock;
    parent_name = of_clk_get_parent_name(node, 0);  /* parent is osc24m */

    init.name = node->name;
    init.ops = &k230_clk_pll_ops;
    init.flags = CLK_IS_BASIC/*|flags*/;
    init.parent_names = &parent_name;
    init.num_parents = 1;                           /* #clock-cells = 0, only one parent */
    k230_clk_pll->hw.init = &init;

    hw = &k230_clk_pll->hw;
    if(clk_hw_register(NULL, hw)) {
        goto free_k230_clk_pll;
    }

    if(of_clk_add_provider(node, of_clk_src_simple_get, hw->clk)) {
        goto unregister_hw_clk;
    }

    k230_clk_pll->cl = clkdev_create(hw->clk, node->name, NULL);
    if(NULL == k230_clk_pll->cl) {
        goto unregister_provider;
    }

#ifdef CONFIG_DEBUG_FS
    k230_debugfs_clk_pll_init(hw);
#endif
    return hw->clk;

unregister_provider:
    of_clk_del_provider(node);
unregister_hw_clk:
    clk_unregister(hw->clk);
free_k230_clk_pll:
    kfree(k230_clk_pll);
out_unmap:
    iounmap(base_reg);

    return NULL;
}

static int canaan_k230_pll_clk_probe(struct platform_device *pdev)
{
    struct clk *clk;

    /*
     * This function is not executed when of_fixed_clk_setup
     * succeeded.
     */
#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:register clock node %s",pdev->dev.of_node->name);
#endif

    clk = _of_canaan_k230_pll_clk_setup(pdev->dev.of_node);
    if (IS_ERR(clk))
        return PTR_ERR(clk);

    platform_set_drvdata(pdev, clk);

#ifdef K230_CLK_DEBUG
    pr_info("[K230_CLK]:k230_clk_pll_probe ok!");
#endif

    return 0;
}

static const struct of_device_id of_canaan_k230_pll_clk_ids[] = {
    { .compatible = "canaan,k230-pll" },
    { }
};
MODULE_DEVICE_TABLE(of, of_canaan_k230_pll_clk_ids);

static struct platform_driver of_canaan_k230_pll_clk_driver = {
    .driver = {
        .name = "k230-pll",
        .of_match_table = of_canaan_k230_pll_clk_ids,
    },
    .probe = canaan_k230_pll_clk_probe,
};
#if 0
builtin_platform_driver(of_canaan_k230_pll_clk_driver);
#else
static int __init canaan_k230_clk_pll_init_driver(void)
{
    return platform_driver_register(&of_canaan_k230_pll_clk_driver);
}
arch_initcall(canaan_k230_clk_pll_init_driver);
#endif


/*-----------------------------------------------------debugfs--------------------------------------------------------*/
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>

static struct dentry *canaan_clk_debug_dir = NULL;

/*---------------------------------------------------------------
 * for composite clks debug 
 * gate/rate/mux/.....
 *
 *
 *---------------------------------------------------------------*/
static int k230_debugfs_composite_enable_get(void *data, u64 *val)
{
    struct clk_hw *hw   = (struct clk_hw *)data;
    struct clk *clk     = NULL;

    clk = clk_get(NULL, clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        *val = -ENOENT;
        return 0;
    }

    if(false == __clk_is_enabled(clk)) {
        *val = 0;
    } else {
        *val = 1;
    }

    clk_put(clk);

    return 0;
}

static int k230_debugfs_composite_enable_set(void *data, u64 val)
{
    struct clk_hw *hw = (struct clk_hw *)data;
    struct clk *clk     = NULL;

    clk = clk_get(NULL, clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        return 0;
    }

    if(0 == val) {
        clk_disable(clk);
    } else {
        clk_enable(clk);
    }

    clk_put(clk);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(composite_enable_fops, k230_debugfs_composite_enable_get, k230_debugfs_composite_enable_set,"%llu\n");

static int k230_debugfs_composite_rate_get(void *data, u64 *val)
{
    struct clk_hw *hw   = (struct clk_hw *)data;
    struct clk *clk     = NULL;

    clk = clk_get(NULL,clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        *val = -ENOENT;
        return 0;
    }

    *val = clk_get_rate(clk);

    clk_put(clk);

    return 0;
}

static int k230_debugfs_composite_rate_set(void *data, u64 val)
{
    struct clk_hw *hw   = (struct clk_hw *)data;
    struct clk *clk     = NULL;

    clk = clk_get(NULL,clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        return 0;
    }

    clk_set_rate(clk,val);

    clk_put(clk);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(composite_rate_fops, k230_debugfs_composite_rate_get, k230_debugfs_composite_rate_set,"%llu\n");

static int k230_debugfs_composite_mux_get(void *data, u64 *val)
{
    struct clk_hw *hw       = (struct clk_hw *)data;
    struct clk *clk         = NULL;
    struct clk *clk_parent  = NULL;
    int i;

    clk = clk_get(NULL,clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        *val = 88880000;
        return 0;
    }

    clk_parent = clk_get_parent(clk);

    if(NULL == clk_parent) {
        *val = 88880001;
        goto end;
    }

    for(i = 0; i < clk_hw_get_num_parents(hw);i++) {
        if(0 == strcmp(clk_hw_get_name(clk_hw_get_parent_by_index(hw,i)),__clk_get_name(clk_parent))) {
            *val = i;
            goto end;
        }
    }

    *val = 88880002;
end:
    clk_put(clk);

    return 0;
}

static int k230_debugfs_composite_mux_set(void *data, u64 val)
{
    struct clk_hw *hw           = (struct clk_hw *)data;
    struct clk *clk             = NULL;
    struct clk_hw *clk_parent   = NULL;

    clk = clk_get(NULL,clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        return 0;
    }

    if(clk_hw_get_num_parents(hw) <= 1) {
        goto end;
    }

    if(val > clk_hw_get_num_parents(hw)) {
        goto end;
    }

    clk_parent = clk_hw_get_parent_by_index(hw,val);
    if(NULL == clk_parent) {
        goto end;
    }

    clk_set_parent(clk, clk_parent->clk);
end:
    clk_put(clk);
    return 0;
}


DEFINE_SIMPLE_ATTRIBUTE(composite_mux_fops, k230_debugfs_composite_mux_get, k230_debugfs_composite_mux_set,"%llu\n");

static int composite_show_register_show(struct seq_file *s, void *data)
{
    struct clk_hw *hw = s->private;
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);
    int reg_value;
    int reg_value_1;
    int mux_field_value = 0;
    int div_field_value = 0;
    int mul_field_value = 0;
    int i;

    seq_printf(s, "1. GATE info:\n");
    seq_printf(s,"-------------------------------------------------------------------------------\n");
    if(k230_composite->gate_reg != NULL) {
        reg_value = readl(k230_composite->gate_reg);
        seq_printf(s, "   reg_offset:      0x%08x\n",((int)(long)k230_composite->gate_reg) & 0xFFF);
        seq_printf(s, "   reg_value :      0x%08x\n",reg_value);
        seq_printf(s, "   gate_bit  :      %d\n",k230_composite->gate_bit);
        seq_printf(s, "   invert_bit:      %s\n",k230_composite->gate_bit_reverse ? "TRUE":"FALSE");
        seq_printf(s, "   clk_enable:      %s\n",((reg_value & (0x1 << k230_composite->gate_bit)) ^ k230_composite->gate_bit_reverse) ? "YES":"NO");
    } else {
        seq_printf(s, "   does not support gate!\n");
    } 
    seq_printf(s,"-------------------------------------------------------------------------------\n\n");

    seq_printf(s, "2. mux info:\n");
    seq_printf(s,"-------------------------------------------------------------------------------\n");
    if(k230_composite->mux_reg != NULL) {
        reg_value = readl(k230_composite->mux_reg);
        seq_printf(s, "   reg_offset:      0x%08x\n",((int)(long)k230_composite->mux_reg) & 0xFFF);
        seq_printf(s, "   reg_value :      0x%08x\n",reg_value);
        seq_printf(s, "   mask      :      0x%08x\n",k230_composite->mux_mask);
        seq_printf(s, "   shift     :      %d\n",k230_composite->mux_shift);
        mux_field_value = (reg_value >> k230_composite->mux_shift) & k230_composite->mux_mask;
        seq_printf(s, "   mux_value :      %d\n",mux_field_value);
        seq_printf(s, "   parent    :      %s\n",clk_hw_get_name(clk_hw_get_parent_by_index(hw,mux_field_value)));
        seq_printf(s, "   parents   :\n");
        for(i = 0; i < clk_hw_get_num_parents(hw); i++) {
            seq_printf(s, "                      %d:%s\n", i,clk_hw_get_name(clk_hw_get_parent_by_index(hw,i)));
        }
    } else {
        seq_printf(s, "   parent    :      %s\n",clk_hw_get_name(clk_hw_get_parent_by_index(hw,0)));
        seq_printf(s, "   mux is fixed!\n");
    }
    seq_printf(s,"-------------------------------------------------------------------------------\n\n");

    seq_printf(s, "3. rate info:\n");
    if(k230_composite->rate_reg != NULL) {
        reg_value = readl(k230_composite->rate_reg);
        seq_printf(s, "   reg_offset:      0x%08x\n",((int)(long)k230_composite->rate_reg) & 0xFFF);
        seq_printf(s, "   reg_value :      0x%08x\n",reg_value);
        if(k230_composite->rate_calc_method == 0)
        {
            div_field_value = (reg_value >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
            seq_printf(s, "   only multi changable\n");
            seq_printf(s, "   mul/div   :      0x%08x/0x%08x\n",div_field_value + 1 , k230_composite->rate_div_max);
        } else if(k230_composite->rate_calc_method == 1) {
            div_field_value = (reg_value >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
            seq_printf(s, "   only div changable\n");
            seq_printf(s, "   mul/div   :      0x%08x/0x%08x\n",k230_composite->rate_mul_max, div_field_value+1);
        } else {
            if(k230_composite->rate_reg_1 != NULL) {
                reg_value_1 = readl(k230_composite->rate_reg_1);
                div_field_value = (reg_value >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
                mul_field_value = (reg_value_1 >> k230_composite->rate_mul_shift_1) & k230_composite->rate_mul_mask_1;
                seq_printf(s, "   div&mul changable\n");
                seq_printf(s, "   mul/div   :      0x%08x/0x%08x\n",mul_field_value, div_field_value);
            }
            else{
                div_field_value = (reg_value >> k230_composite->rate_div_shift) & k230_composite->rate_div_mask;
                mul_field_value = (reg_value >> k230_composite->rate_mul_shift) & k230_composite->rate_mul_mask;
                seq_printf(s, "   div&mul changable\n");
                seq_printf(s, "   mul/div   :      0x%08x/0x%08x\n",mul_field_value, div_field_value);
            }
        }
    } else {
        seq_printf(s, "   rate is fixed!\n");
    }
    seq_printf(s,"-------------------------------------------------------------------------------\n\n");
    return 0;
}
DEFINE_SHOW_ATTRIBUTE(composite_show_register);

static int k230_debugfs_clk_composite_init(struct clk_hw *hw)
{
    struct k230_clk_composite *k230_composite = to_k230_clk_composite(hw);

    if(NULL == canaan_clk_debug_dir) {
        canaan_clk_debug_dir = debugfs_create_dir("canaan_clk",NULL);
        if(NULL == canaan_clk_debug_dir) return -ENOMEM;
    }

    k230_composite->debugfs = debugfs_create_dir(clk_hw_get_name(hw), canaan_clk_debug_dir);

    if (!k230_composite->debugfs) return -ENOMEM;

    if (!debugfs_create_file("enable", S_IRUGO | S_IWUGO, k230_composite->debugfs,hw, &composite_enable_fops)) {
        return -ENOMEM;
    }

    if (!debugfs_create_file("rate", S_IRUGO | S_IWUGO, k230_composite->debugfs,hw, &composite_rate_fops)) {
        return -ENOMEM;
    }

    if (!debugfs_create_file("mux", S_IRUGO | S_IWUGO, k230_composite->debugfs,hw, &composite_mux_fops)) {
        return -ENOMEM;
    }

    if (!debugfs_create_file("reg_info", 0444, k230_composite->debugfs,hw, &composite_show_register_fops)) {
        return -ENOMEM;
    }

    return 0;
}

/*---------------------------------------------------------------
 * for pll clks debug 
 *
 * gate/rate
 *
 *---------------------------------------------------------------*/
static int k230_debugfs_pll_enable_get(void *data, u64 *val)
{
    struct clk_hw *hw   = (struct clk_hw *)data;
    struct clk *clk     = NULL;

    clk = clk_get(NULL,clk_hw_get_name(hw));

    if(ERR_PTR(-ENOENT) == clk) {
        *val = -ENOENT;
        return 0;
    }

    if(false == __clk_is_enabled(clk)) {
        *val = 0;
    } else {
        *val = 1;
    }

    clk_put(clk);

    return 0;
}

static int k230_debugfs_pll_enable_set(void *data, u64 val)
{
    struct clk_hw *hw = (struct clk_hw *)data;
    struct clk *clk     = NULL;

    clk = clk_get(NULL,clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        return 0;
    }

    if(0 == val) {
        clk_disable(clk);
    } else {
        clk_enable(clk);
    }

    clk_put(clk);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pll_enable_fops, k230_debugfs_pll_enable_get, k230_debugfs_pll_enable_set,"%llu\n");

static int k230_debugfs_pll_rate_get(void *data, u64 *val)
{
    struct clk_hw *hw   = (struct clk_hw *)data;
    struct clk *clk     = NULL;

    clk = clk_get(NULL,clk_hw_get_name(hw));
    if(ERR_PTR(-ENOENT) == clk) {
        *val = -ENOENT;
        return 0;
    }

    *val = clk_get_rate(clk);

    clk_put(clk);

    return 0;
}

static int k230_debugfs_pll_rate_set(void *data, u64 val)
{
    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pll_rate_fops, k230_debugfs_pll_rate_get, k230_debugfs_pll_rate_set,"%llu\n");

static int pll_show_register_show(struct seq_file *s, void *data)
{
    struct clk_hw *hw = s->private;
    struct k230_clk_pll *k230_pll = to_k230_clk_pll(hw);
    int reg_value;
    int fb_div = 0;
    int ref_div = 0;
    int out_div = 0;

    seq_printf(s, "1. rate info:\n");
    seq_printf(s, "-------------------------------------------------------------------------------\n");
    seq_printf(s, "   reg_offset:      0x%08x\n",((int)(long)k230_pll->pll_reg_divide) & 0xFFF);
    reg_value = readl(k230_pll->pll_reg_divide);
    seq_printf(s, "   reg_value :      0x%08x\n",reg_value);
    out_div = (reg_value >> k230_pll->pll_out_div_shift) & k230_pll->pll_out_div_mask;
    ref_div = (reg_value >> k230_pll->pll_ref_div_shift) & k230_pll->pll_ref_div_mask;
    fb_div = (reg_value >> k230_pll->pll_fb_div_shift) & k230_pll->pll_fb_div_mask;
    seq_printf(s, "   out_div   :      %d\n",out_div);
    seq_printf(s, "   ref_div   :      %d\n",ref_div);
    seq_printf(s, "   fb_div    :      %d\n",fb_div);
    seq_printf(s, "   freq = 24M * (fb_div+1) / (ref_div+1) / (out_div+1) = %d\n",24000000*(fb_div+1)/(ref_div+1)/(out_div+1));
    seq_printf(s,"-------------------------------------------------------------------------------\n\n");

    
    seq_printf(s, "2. status info:\n");
    seq_printf(s, "-------------------------------------------------------------------------------\n");
    reg_value = readl(k230_pll->pll_reg_bypass);
    seq_printf(s, "   bypass_reg:      0x%08x\n",((int)(long)k230_pll->pll_reg_bypass) & 0xFFF);
    seq_printf(s, "   bypass_val:      0x%08x\n", reg_value);
    seq_printf(s, "   bypass_sta:      %s\n",(reg_value & (1 << k230_pll->pll_bypass_enable_bit)) ? "BYPASS ENABLE":"BYPASS DISABLE");

    reg_value = readl(k230_pll->pll_reg_gate);
    seq_printf(s, "   gate_reg  :      0x%08x\n",((int)(long)k230_pll->pll_reg_gate) & 0xFFF);
    seq_printf(s, "   gate_val  :      0x%08x\n", reg_value);
    seq_printf(s, "   gate_statu:      %s\n",(reg_value & (1 << k230_pll->pll_gate_enable_bit)) ? "CLK ENABLE":"CLK DISABLE");
    seq_printf(s,"-------------------------------------------------------------------------------\n\n");
    return 0;
}
DEFINE_SHOW_ATTRIBUTE(pll_show_register);

static int k230_debugfs_clk_pll_init(struct clk_hw *hw)
{
    struct k230_clk_pll *k230_pll = to_k230_clk_pll(hw);

    if(NULL == canaan_clk_debug_dir) {
        canaan_clk_debug_dir = debugfs_create_dir("canaan_clk",NULL);
        if(NULL == canaan_clk_debug_dir) return -ENOMEM;
    }

    k230_pll->debugfs = debugfs_create_dir(clk_hw_get_name(hw), canaan_clk_debug_dir);

    if (!k230_pll->debugfs) return -ENOMEM;

    if (!debugfs_create_file("enable", S_IRUGO | S_IWUGO, k230_pll->debugfs,hw, &pll_enable_fops)) {
        return -ENOMEM;
    }

    if (!debugfs_create_file("rate", S_IRUGO | S_IWUGO, k230_pll->debugfs,hw, &pll_rate_fops)) {
        return -ENOMEM;
    }

    if (!debugfs_create_file("reg_info", 0444, k230_pll->debugfs, hw, &pll_show_register_fops)) {
        return -ENOMEM;
    }

    return 0;
}
#endif

/* There are two methods for clock configuration.Please used methord B for drivers.

    Method A (for example:debufs):
    1. clk = clk_get(NULL, clk_name);       ---> clk_name is defined in the clock_provider.dtsi
    2. clk_set_rate(clk,rate); clk_disable(clk); clk_set_parent(clk,clk_parent); etc.
    3. clk_put(clk);

    Method B (for example:emac and other drivers):
    1. clock_consumer.dtsi:
        &emac {
            clocks      = <&refclk>, <&refclk>, <&refclk>, <&refclk>, <&refclk>, <&refclk>;         -->clocks is defined in the clock_provider.dtsi
            clock-names = "hclk", "pclk", "ether_clk", "tx_clk", "rx_clk", "tsu_clk";               -->clock-names only index
        };
    2. C code:
        clk = clk_get(device, clk_name);     ---> device is platform_device->device, clk_name is clocks-names item(hclk, pclk etc.)
        clk_set_rate(clk,rate); clk_disable(clk); clk_set_parent(clk,clk_parent); etc.
        clk_put(clk);
*/
