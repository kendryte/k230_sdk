#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <regmap.h>
#include <asm/global_data.h>
#include <linux/io.h>
#include <linux/log2.h>
#include <linux/bitfield.h>

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC	1000000000L
#endif

DECLARE_GLOBAL_DATA_PTR;

struct pwm_kendryte_regs {
    volatile uint32_t pwmcfg;
    volatile uint32_t reserved0;
    volatile uint32_t pwmcount;
    volatile uint32_t reserved1;
    volatile uint32_t pwms;
    volatile uint32_t reserved2;
    volatile uint32_t reserved3;
    volatile uint32_t reserved4;
    volatile uint32_t pwmcmp0;
    volatile uint32_t pwmcmp1;
    volatile uint32_t pwmcmp2;
    volatile uint32_t pwmcmp3;
};

struct pwm_kendryte_priv {
	void __iomem *base;
	ulong freq;
};

static int pwm_kendryte_set_config(struct udevice *dev, uint channel,
				 uint period_ns, uint duty_ns)
{
	struct pwm_kendryte_priv *priv = dev_get_priv(dev);
	struct pwm_kendryte_regs *reg = priv->base;

	debug("%s: period_ns=%u, duty_ns=%u\n", __func__, period_ns, duty_ns);

    uint64_t period, pulse, pwmcmpx_max;
    uint32_t pwmscale = 0;
    uint64_t pwm_clock = priv->freq;

    period = (pwm_clock * period_ns) / NSEC_PER_SEC;
    pulse = (pwm_clock * duty_ns) / NSEC_PER_SEC;

    if (pulse > period)
    {
        return -EINVAL; 
    }

    if(period > ((1 << (15 + 16)) - 1LL))
    {
        return -EINVAL; 
    }

    pwmcmpx_max = (1 << 16) - 1;
    while ((period >> pwmscale) > pwmcmpx_max)
    {
        pwmscale++;
    }

    if(pwmscale > 0xf)
    {
        return -EINVAL;
    }

    reg->pwmcfg |= (1 << 9);  //default always mode
    reg->pwmcfg |= pwmscale;  //scale
    reg->pwmcmp0 = (period >> pwmscale);

    
    switch(channel)
    {
    case 0:
        reg->pwmcmp1 = (reg->pwmcmp0 - (pulse >> pwmscale));
        break;
    case 1:
        reg->pwmcmp2 = (reg->pwmcmp0 - (pulse >> pwmscale));
        break;
    case 2:
        reg->pwmcmp3 = (reg->pwmcmp0 - (pulse >> pwmscale));
        break;
    default:
        break;
    }
    debug("%s: channel=%u, reg->pwmcmp3=0x%x 0x%x\n", __func__, channel, reg->pwmcmp3, &(reg->pwmcmp3));
	return 0;
}

static int pwm_kendryte_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct pwm_kendryte_priv *priv = dev_get_priv(dev);
	struct pwm_kendryte_regs *reg = priv->base;

	debug("%s: Enable '%s'\n", __func__, dev->name);

	if (enable)
		reg->pwmcfg |= (1 << 12);
	else
		reg->pwmcfg &= ~(1 << 12);

	return 0;
}

static int pwm_kendryte_probe(struct udevice *dev)
{
	struct pwm_kendryte_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret = 0;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		debug("%s get clock fail!\n", __func__);
		return -EINVAL;
	}

	priv->freq = clk_get_rate(&clk);
    priv->base = dev_read_addr_ptr(dev);

	return 0;
}

static const struct pwm_ops pwm_kendryte_ops = {
	.set_config	= pwm_kendryte_set_config,
	.set_enable	= pwm_kendryte_set_enable,
};

static const struct udevice_id pwm_kendryte_ids[] = {
	{ .compatible = "kendryte,pwm"},
	{ }
};

U_BOOT_DRIVER(pwm_kendryte) = {
	.name	= "pwm_kendryte",
	.id	= UCLASS_PWM,
	.of_match = pwm_kendryte_ids,
	.ops	= &pwm_kendryte_ops,
	.probe		= pwm_kendryte_probe,
	.priv_auto	= sizeof(struct pwm_kendryte_priv),
};
