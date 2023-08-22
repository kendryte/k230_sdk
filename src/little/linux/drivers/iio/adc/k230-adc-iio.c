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

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include<linux/delay.h>
#include <linux/iio/iio.h>

#define ADC_MAX_CHANNEL     6
#define ADC_MAX_DMA_CHN     3

typedef struct
{
    uint32_t trim_reg;
    uint32_t cfg_reg;
    uint32_t mode_reg;
    uint32_t thsd_reg;
    uint32_t dma_intr_reg;
    uint32_t data_reg[ADC_MAX_CHANNEL];
    uint32_t data_dma[ADC_MAX_DMA_CHN];
} adc_reg_t;

struct k230_adc_data {
	struct device *dev;
	struct clk *clk;
	void __iomem *addr;
	int channel;
};

#define K230_ADC_CHANNEL(index) {			\
	.type = IIO_VOLTAGE,					\
	.channel = index,					\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	\
	.datasheet_name = "CH##index",				\
	.indexed = 1,						\
}

static const struct iio_chan_spec k230_channels[] = {
	K230_ADC_CHANNEL(0),
	K230_ADC_CHANNEL(1),
	K230_ADC_CHANNEL(2),
	K230_ADC_CHANNEL(3),
	K230_ADC_CHANNEL(4),
	K230_ADC_CHANNEL(5),
};

static int k230_adc_conversion(struct k230_adc_data *info)
{
    uint32_t data;
	adc_reg_t *adc_regs = (adc_reg_t*)info->addr;
    uint32_t channel = info->channel;

    if (channel >= ADC_MAX_CHANNEL)
    {
        return -EINVAL;
    }

    writel(channel, &adc_regs->cfg_reg);

    data = readl(&adc_regs->cfg_reg);
    data |= 0x10;
    writel(data, &adc_regs->cfg_reg);

    mdelay(1);

    data = readl(&adc_regs->data_reg[channel]);

    return data;
}

static int k230_adc_read_raw(struct iio_dev *indio_dev,
			       struct iio_chan_spec const *chan,
			       int *val, int *val2, long mask)
{
	struct k230_adc_data *info = iio_priv(indio_dev);

    info->channel = chan->channel;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
        *val = k230_adc_conversion(info);
		return IIO_VAL_INT;
	default:
		return -EINVAL;
    }
}

static const struct iio_info k230_iio_info = {
	.read_raw = &k230_adc_read_raw,
	.write_raw = NULL,
};

static void k230_adc_init(struct k230_adc_data *info)
{
	adc_reg_t *adc_regs = (adc_reg_t*)info->addr;
	uint32_t data;

	data = readl(&adc_regs->trim_reg);
	data &= ~(0x1);
    writel(data, &adc_regs->trim_reg);

    data = readl(&adc_regs->trim_reg);
    data |= 0x1;
    writel(data, &adc_regs->trim_reg);

    data = readl(&adc_regs->trim_reg);
    data |= (0x1 << 20);
    writel(data, &adc_regs->trim_reg);

	udelay(150);

    data &= ~(0x1 << 20);
    writel(data, &adc_regs->trim_reg);

    writel(0x0, &adc_regs->mode_reg);
}

static int k230_adc_probe(struct platform_device *pdev)
{
    struct k230_adc_data *k230_data;
	struct device *dev = &pdev->dev;
	struct iio_dev *indio_dev;
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(struct k230_adc_data));
	if (!indio_dev) {
		dev_err(&pdev->dev, "Failed allocating iio device\n");
		return -ENOMEM;
	}

	k230_data = iio_priv(indio_dev);
	k230_data->dev = dev;

	k230_data->addr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(k230_data->addr))
		return PTR_ERR(k230_data->addr);

	k230_data->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(k230_data->clk)) {
		ret = PTR_ERR(k230_data->clk);
		dev_err(dev, "Failed getting clock, err = %d\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(k230_data->clk);
	if (ret) {
		dev_err(k230_data->dev,
			"Could not prepare or enable clock.\n");
		return ret;
	}

	platform_set_drvdata(pdev, indio_dev);

    k230_adc_init(k230_data);

	indio_dev->name = dev_name(dev);
	indio_dev->info = &k230_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = k230_channels;
	indio_dev->num_channels = ARRAY_SIZE(k230_channels);
	ret = devm_iio_device_register(dev, indio_dev);
	if (ret)
		dev_err(dev, "could not register iio (ADC)");

	return ret;
}

static const struct of_device_id k230_adc_of_match[] = {
	{ .compatible = "canaan,k230-adc", },
	{ }
};
MODULE_DEVICE_TABLE(of, k230_adc_of_match);


static struct platform_driver k230_adc_driver = {
	.probe = k230_adc_probe,
	.driver = {
		.name = "k230-adc",
		.of_match_table = k230_adc_of_match,
	},
};
module_platform_driver(k230_adc_driver);

MODULE_AUTHOR("Canaan SDK Team");
MODULE_DESCRIPTION("Canaan Kendyte K230 chip ADC Driver");
MODULE_LICENSE("GPL");
