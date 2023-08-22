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

#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pwm.h>
#include <linux/module.h>
#include <linux/clk.h>

typedef struct
{
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
} k230_pwm_t;

struct k230_pwm_chip {
	struct	device	*dev;
	struct pwm_chip chip;
    struct clk *clk;
    void __iomem *addr;
	uint32_t channel;
};

static inline struct k230_pwm_chip *to_canaan_pwm_chip(struct pwm_chip *c)
{
	return container_of(c, struct k230_pwm_chip, chip);
}

static int k230_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
			     int duty_ns, int period_ns)
{
    struct k230_pwm_chip *k230_chip = to_canaan_pwm_chip(chip);
    k230_pwm_t *reg = (k230_pwm_t*)k230_chip->addr;
    int channel = pwm->hwpwm;
    uint64_t period, pulse, pwmcmpx_max;
    uint32_t pwmscale = 0;
    uint64_t pwm_clock = clk_get_rate(k230_chip->clk);

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

    if (channel > 2)
        reg = (k230_pwm_t *)((void*)reg + 0x40);

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
    case 3:
        reg->pwmcmp1 = (reg->pwmcmp0 - (pulse >> pwmscale));
        break;
    case 4:
        reg->pwmcmp2 = (reg->pwmcmp0 - (pulse >> pwmscale));
        break;
    case 5:
        reg->pwmcmp3 = (reg->pwmcmp0 - (pulse >> pwmscale));
        break;
    default:
        break;
    }

    return 0;
}

static int k230_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    struct k230_pwm_chip *k230_chip = to_canaan_pwm_chip(chip);
    k230_pwm_t *reg = (k230_pwm_t*)k230_chip->addr;
    
    int channel = pwm->hwpwm;

    if (channel > 2)
    {
        reg = (k230_pwm_t *)((void*)reg + 0x40);
    }
    reg->pwmcfg |= (1 << 12);

    return 0;
}

static void k230_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    struct k230_pwm_chip *k230_chip = to_canaan_pwm_chip(chip);
    k230_pwm_t *reg = (k230_pwm_t*)k230_chip->addr;
    
    int channel = pwm->hwpwm;

    if (channel > 2)
    {
        reg = (k230_pwm_t *)((void*)reg + 0x40);
    }
    reg->pwmcfg &= ~(1 << 12);

    return;
}

static const struct pwm_ops k230_pwm_ops = {
	.config = k230_pwm_config,
	.enable = k230_pwm_enable,
	.disable = k230_pwm_disable,
	.owner = THIS_MODULE,
};

static int k230_pwm_probe(struct platform_device *pdev)
{
    struct k230_pwm_chip *k230_pwm;
    int ret;
    k230_pwm_t *reg;

	k230_pwm = devm_kzalloc(&pdev->dev, sizeof(*k230_pwm), GFP_KERNEL);
	if (k230_pwm == NULL)
		return -ENOMEM;
	k230_pwm->dev = &pdev->dev;

	k230_pwm->addr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(k230_pwm->addr))
		return PTR_ERR(k230_pwm->addr);

    k230_pwm->clk = devm_clk_get(&pdev->dev, "pwm");
    if (IS_ERR(k230_pwm->clk)) {
		k230_pwm->clk = devm_clk_get(&pdev->dev, NULL);
		if (IS_ERR(k230_pwm->clk)) {
			ret = PTR_ERR(k230_pwm->clk);
			if (ret != -EPROBE_DEFER)
				dev_err(&pdev->dev, "Can't get bus clk: %d\n",
					ret);
			return ret;
		}
	}

	k230_pwm->chip.dev = &pdev->dev;
	k230_pwm->chip.ops = &k230_pwm_ops;
	k230_pwm->chip.base = -1;
	k230_pwm->chip.npwm = 6;

    reg = (k230_pwm_t*)k230_pwm->addr;
    reg->pwmcmp0 = 0;
    reg->pwmcmp1 = 0;
    reg->pwmcmp2 = 0;
    reg->pwmcmp3 = 0;
    reg = (k230_pwm_t*)((void*)reg + 0x40);
    reg->pwmcmp0 = 0;
    reg->pwmcmp1 = 0;
    reg->pwmcmp2 = 0;
    reg->pwmcmp3 = 0;

	ret = pwmchip_add(&k230_pwm->chip);
	if (ret < 0)
		return ret;
	platform_set_drvdata(pdev, k230_pwm);

	return 0;
}

static int k230_pwm_remove(struct platform_device *pdev)
{
	struct k230_pwm_chip *k230_pwm = platform_get_drvdata(pdev);
	int ret;

	ret = pwmchip_remove(&k230_pwm->chip);
	if (ret < 0)
		return ret;

	return 0;
}

static const struct of_device_id k230_pwm_of_match[] = {
	{ .compatible = "canaan,k230-pwm", },
	{ }
};

static struct platform_driver k230_pwm_driver = {
	.driver = {
		.name = "k230-pwm",
		.of_match_table = k230_pwm_of_match,
	},
	.probe = k230_pwm_probe,
	.remove = k230_pwm_remove,
};
module_platform_driver(k230_pwm_driver);

MODULE_AUTHOR("Canaan SDK Team");
MODULE_DESCRIPTION("Canaan Kendyte K230 chip PWM Driver");
MODULE_LICENSE("GPL");
