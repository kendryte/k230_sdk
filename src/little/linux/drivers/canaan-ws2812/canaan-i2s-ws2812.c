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
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/delay.h>

#include "canaan-i2s-ws2812.h"
#define CANAAN_DRIVER_NAME	"canaan-ws2812"
#define WSIOC_RESET     _IOW('W', 0x10, int)

#define I2S_CHANNEL_0   0
#define I2S_CHANNEL_1   1
#define I2S_WORDLEN     32

typedef struct _ws2812_t {
    void __iomem    *base;
    struct device *dev;
	struct clk *i2sclk;
    struct miscdevice mdev;
    int 				err;
    struct dma_chan		*dma_lch;
    struct scatterlist *sg;

    struct mutex lock;
} ws2812_t;

/* ws2812 use 24-bits G-R-B of color data */
static unsigned int rgb_to_ws2812(unsigned char color)
{
	int i;
	unsigned char v = 0;
	unsigned int ret = 0;

	for (i = 3; i >= 0; i--)
	{
		v = (color >> (i * 2)) & 0x3;

		if (v == 0x3) //11
		{
			ret |= 0xee << (8*i);
		} else
		if (v == 0x2)	//10
		{
			ret |= 0xe8 << (8*i);
		} else
		if (v == 0x1)	//01
		{
			ret |= 0x8e << (8*i);
		} else
		if (v == 0x0)	//00
		{
			ret |= 0x88 << (8*i);
		}
	}
	return ret;
}

static void i2s_set_enable(const void *reg, bool enable)
{
    i2s_t *i2s = (i2s_t*)reg;
    if (enable)
        i2s->ier |= 0x1;
    else
        i2s->ier &= 0x0;
}

static void i2s_set_mask_interrupt(const void *reg, int channel_num,
                                  int rx_available_int, int rx_overrun_int,
                                  int tx_empty_int, int tx_overrun_int)
{
    i2s_t *i2s = (i2s_t*)reg;
    unsigned int data = i2s->channel[channel_num].imr;

    if (rx_available_int == 1)
        data |= 1 << 0; //Mask RX FIFO Data Available interrupt
    else
        data &= ~(1 << 0);  //unmask

    if (rx_overrun_int == 1)
        data |= 1 << 1;  //Mask RX FIFO Overrun interrupt
    else
        data &= ~(1 << 1); //unmask

    if (tx_empty_int == 1)
        data |= 1 << 4; //Mask TX FIFO Empty interrupt
    else
        data &= ~(1 << 4); //unmask

    if (tx_overrun_int == 1)
        data |= 1 << 5; //Mask TX FIFO Overrun interrupt
    else
        data &= ~(1 << 5); //unmask

    i2s->channel[channel_num].imr = data;
}

static void i2s_transmit_channel_enable(const void *reg, int channel_num, bool enable)
{
    i2s_t *i2s = (i2s_t*)reg;
    unsigned int data = i2s->channel[channel_num].ter;

    if (enable)
        data |= 1 << 0;
    else
        data &= ~(1 << 0);
    
    i2s->channel[channel_num].ter = data;
}

static void i2s_transimit_enable(const void *reg, int channel_num)
{
    i2s_t *i2s = (i2s_t*)reg;

    i2s->iter = 1;

    i2s_transmit_channel_enable(reg, channel_num, 1);
    /* Transmit channel enable */
}

static int i2s_tx_channel_enable(const void *reg, int channel_num, bool enable)
{
    if (channel_num < 0 || channel_num > 1)
        return -EINVAL;

    i2s_set_enable(reg, 1);

    if (enable)
    {
        i2s_set_mask_interrupt(reg, channel_num, 1, 1, 1, 1);
        i2s_transimit_enable(reg, channel_num);
    }
    else
    {
        i2s_transmit_channel_enable(reg, channel_num, 0);
    }

    return 0;
}

static int i2s_transmit_dma_enable(const void *reg, bool enable)
{
    i2s_t *i2s = (i2s_t*)reg;
    unsigned int data = i2s->ccr;

    if (enable)
        data |= 1 << 8;
    else
        data &= 0 << 8;

    i2s->ccr = data;
    return 0;
}

static void  i2s_tx_dma_enable(const void *reg, bool enable)
{
    if (enable)
        i2s_transmit_dma_enable(reg, 1);
    else
        i2s_transmit_dma_enable(reg, 0);
}

static void i2s_master_configure(const void *reg, i2s_word_select_cycles_t word_select_size,
                          i2s_sclk_gating_cycles_t gating_cycles,
                          i2s_work_mode_t word_mode)
{
    i2s_t *i2s = (i2s_t*)reg;
    unsigned int data = i2s->ccr;

    data &= ~(0xff);
    data |= word_select_size << 3;

    data |= gating_cycles;

    data |= word_mode << 5;

    i2s->ccr = data;

    i2s->cer = 1;
}

static void i2s_set_tx_threshold(const void *reg,
             i2s_fifo_threshold_t threshold,
             int channel_num)
{
    i2s_t *i2s = (i2s_t*)reg;

    i2s->channel[channel_num].tfcr = threshold;

}

static void i2s_tx_channel_cfg(const void *reg, int channel_num, unsigned int word_length, i2s_fifo_threshold_t trigger_level, i2s_work_mode_t word_mode)
{
    i2s_t *i2s = (i2s_t*)reg;
    i2s_word_length_t i2s_word_len = RESOLUTION_32_BIT;
    if (word_length == 32)
    {
        i2s_word_len = RESOLUTION_32_BIT;
    }
    else if (word_length == 24)
    {
        i2s_word_len = RESOLUTION_24_BIT;
    }
    else if (word_length == 16)
    {
        i2s_word_len = RESOLUTION_16_BIT;
    }

    if (channel_num < 0 || channel_num > 1)
    {
        printk("i2s channel error.\n");
        return;
    }

    /*tx channel config*/
    /* disable channel transmit */
    i2s_transmit_channel_enable(reg, channel_num, 0);

    /* flush tx fifo */
    i2s->txffr = 1;

    /* flush channel individual fifo */
    i2s->channel[channel_num].tff = 1;

    /* set tx word length */
    i2s->channel[channel_num].tcr = i2s_word_len;

    i2s_master_configure(reg, SCLK_CYCLES_32, NO_CLOCK_GATING, word_mode);
    i2s_set_tx_threshold(reg, trigger_level, channel_num);
    i2s_transmit_channel_enable(reg, channel_num, 1);
}

static void audio_i2s_out_init(const void *reg, bool enable, int word_len)
{
    audio_out_data_width_e out_word_len = AUDIO_OUT_TYPE_32BIT;
    volatile void *audio_out_reg = (void*)(reg + 0xc00);
    volatile void *audio_in_reg = (void*)(reg + 0x400);
    unsigned int data;

    if (32 == word_len)
    {
        out_word_len = AUDIO_OUT_TYPE_32BIT;
    }
    else if (24 == word_len)
    {
        out_word_len = AUDIO_OUT_TYPE_24BIT;
    }
    else if (16 == word_len)
    {
        out_word_len = AUDIO_OUT_TYPE_16BIT;
    }

    data = readl(audio_in_reg);
    data |= (1 << 5);
    writel(data, audio_in_reg);

    data = readl(audio_out_reg);
    data |= (out_word_len << 1 | AUDIO_OUT_MODE_I2S << 5 | (enable ? 1 : 0));
    writel(data, audio_out_reg);
}

static void k230_i2s_init(const void *reg, int i2s_tx_num, int word_len)
{
    audio_i2s_out_init(reg, true, 32);

    /* reset all of output channel */
    i2s_tx_channel_enable(reg, I2S_CHANNEL_0, false);
    i2s_tx_channel_enable(reg, I2S_CHANNEL_1, false);
    i2s_tx_dma_enable(reg, false);
    i2s_tx_channel_cfg(reg, I2S_CHANNEL_0, I2S_WORDLEN, TRIGGER_LEVEL_4, STANDARD_MODE);
}

static int k230_ws2812_dma_init(ws2812_t *ws2812)
{
    struct dma_slave_config dma_conf;
	int err;

	memset(&dma_conf, 0, sizeof(dma_conf));

	dma_conf.direction = DMA_MEM_TO_DEV;
    dma_conf.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dma_conf.dst_addr = 0x9140f1C8U;
    dma_conf.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;


	ws2812->dma_lch = dma_request_slave_channel(ws2812->dev, "tx");
	if (!ws2812->dma_lch) {
		dev_err(ws2812->dev, "Couldn't acquire a slave DMA channel.\n");
		return -EBUSY;
	}

	err = dmaengine_slave_config(ws2812->dma_lch, &dma_conf);
	if (err) {
		dma_release_channel(ws2812->dma_lch);
		ws2812->dma_lch = NULL;
		dev_err(ws2812->dev, "Couldn't configure DMA slave.\n");
		return err;
	}

	return 0;

}

enum dma_status k230_ws2812_dma_sync_wait(struct dma_chan *chan, dma_cookie_t cookie)
{
	enum dma_status status;
	unsigned long dma_sync_wait_timeout = jiffies + msecs_to_jiffies(1000);

	dma_async_issue_pending(chan);
	do {
		status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
		if (time_after_eq(jiffies, dma_sync_wait_timeout)) {
			dev_err(chan->device->dev, "%s: timeout!\n", __func__);
			return DMA_ERROR;
		}
		if (status != DMA_IN_PROGRESS)
			break;
		usleep_range(1000, 2000);
	} while (1);

	return status;
}

static int k230_ws2812_xmit_dma(ws2812_t *ws2812,
			       struct scatterlist *sg, int length, int mdma)
{
	struct dma_async_tx_descriptor *in_desc;
	dma_cookie_t cookie;
	static u32 count = 0;
	int err;
    count++;

	in_desc = dmaengine_prep_slave_sg(ws2812->dma_lch, sg, 1,
					  DMA_MEM_TO_DEV, DMA_PREP_INTERRUPT |
					  DMA_CTRL_ACK);
	if (!in_desc) {
		dev_err(ws2812->dev, "dmaengine_prep_slave error\n");
		return -ENOMEM;
	}

	cookie = dmaengine_submit(in_desc);
	err = dma_submit_error(cookie);
	if (err)
		return -ENOMEM;

    err = k230_ws2812_dma_sync_wait(ws2812->dma_lch, cookie);

	if (err) {
		dev_err(ws2812->dev, "DMA Error %i\n", err);
		dmaengine_terminate_all(ws2812->dma_lch);
		return err;
	}

    return 0;
}

static void i2s_transmit_data_dma(ws2812_t *ws2812, int channel_num, void *buf, int len)
{
    
    dma_addr_t dma_handle;
    uint8_t *dma_virt_addr;

    ws2812->sg->length = len;
    dma_virt_addr = dma_alloc_coherent(ws2812->dev, len, &dma_handle, GFP_KERNEL);
    ws2812->sg->dma_address = dma_handle;
    memcpy(dma_virt_addr, buf, len);

    k230_ws2812_xmit_dma(ws2812, ws2812->sg, 1, 0);
    dma_free_coherent(ws2812->dev, len, dma_virt_addr, dma_handle);
}

static int ws2812_open(struct inode *inode, struct file *file)
{
    ws2812_t *ws2812 = container_of(file->private_data,
					      ws2812_t, mdev);

    k230_i2s_init(ws2812->base, I2S_CHANNEL_0, 32);
    /* enable i2s channel */
    i2s_tx_channel_enable(ws2812->base, I2S_CHANNEL_0, true); //channel0 output enable
    i2s_tx_dma_enable(ws2812->base, true);

	return 0;
}

static int ws2812_release(struct inode *inode, struct file *file)
{
    ws2812_t *ws2812 = container_of(file->private_data,
					      ws2812_t, mdev);

    /* enable i2s channel */
    i2s_tx_dma_enable(ws2812->base, false);
    i2s_tx_channel_enable(ws2812->base, I2S_CHANNEL_0, false); //channel0 output enable

	return 0;
}

static ssize_t ws2812_write(struct file *file, const char __user *buf, size_t size, loff_t *loff)
{
    unsigned char *data;
    unsigned int *ws2812_data;
    int i;
    size_t ret = size;
    ws2812_t *ws2812 = container_of(file->private_data, ws2812_t, mdev);

	if (!mutex_trylock(&ws2812->lock)) {
		return -EBUSY;
	}
	data = kmalloc(size, GFP_KERNEL);
	if (!data)
    {
		ret = -ENOMEM;
        goto exit0;
    }

    if (copy_from_user(data, buf, size))
    {
        ret = -EFAULT;
        goto exit1;
    }

    if (size % 2)
    {
        ws2812_data = kmalloc(size * 4 + 4, GFP_KERNEL);
    } else {
        ws2812_data = kmalloc(size * 4, GFP_KERNEL);
    }

	if (!ws2812_data)
    {
		ret = -ENOMEM;
        goto exit1;
    }

    for (i = 0; i < size; i++)
    {
        ws2812_data[i] = rgb_to_ws2812(data[i]);
    }

    if (size % 2)
    {
        ws2812_data[i] = 0x0;
        i2s_transmit_data_dma(ws2812, I2S_CHANNEL_0, (void*)ws2812_data, size * 4 + 4);
    } else {
        i2s_transmit_data_dma(ws2812, I2S_CHANNEL_0, (void*)ws2812_data, size * 4);
    }

    kfree(ws2812_data);
exit1:
    kfree(data);
exit0:
    mutex_unlock(&ws2812->lock);
    return ret;
}

static long ws2812_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static const struct file_operations ws2812_fops = {
	.owner = THIS_MODULE,
	.open = ws2812_open,
    .release = ws2812_release,
    .write = ws2812_write,
    .unlocked_ioctl = ws2812_ioctl,
};

static int k230_ws2812_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;
    struct miscdevice *misc;

    ws2812_t *ws2812;

    ws2812 = devm_kzalloc(dev, sizeof(*ws2812), GFP_KERNEL);
	if (!ws2812)
		return -ENOMEM;

    ws2812->base = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(ws2812->base))
        return PTR_ERR(ws2812->base);

    ws2812->i2sclk = devm_clk_get(&pdev->dev, "i2sclk");
    clk_prepare_enable(ws2812->i2sclk);
    if (IS_ERR(ws2812->i2sclk)) {
		ws2812->i2sclk = devm_clk_get(&pdev->dev, NULL);
		if (IS_ERR(ws2812->i2sclk)) {
			ret = PTR_ERR(ws2812->i2sclk);
			if (ret != -EPROBE_DEFER)
				dev_err(&pdev->dev, "Can't get i2s bus clk: %d\n",
					ret);
			return ret;
		}
	}
    ws2812->dev = dev;
    ws2812->sg = devm_kzalloc(ws2812->dev, sizeof(struct scatterlist), GFP_KERNEL);

    mutex_init(&ws2812->lock);
	dev_set_drvdata(&pdev->dev, ws2812);
    
    k230_ws2812_dma_init(ws2812);
    
    

    misc = &ws2812->mdev;
    memset(misc, 0, sizeof(*misc));
    misc->minor = MISC_DYNAMIC_MINOR;
    misc->name = "ws2812";
    misc->fops = &ws2812_fops;
    misc->parent = dev;

	ret = misc_register(misc);
	if(ret < 0){
		printk("ws2812 register cdev failed!\n");
		return ret;
	}

    misc->this_device = dev;

    return 0;
}

static const struct of_device_id k230_of_match[] = {
	{ .compatible = "canaan,i2s-ws2812", .data = (void *)0},
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, k230_of_match);

static struct platform_driver k230_i2s_ws2812_driver = {
	.driver		= {
		.name	= CANAAN_DRIVER_NAME,
		.of_match_table = k230_of_match,
	},
	.probe		= k230_ws2812_probe,
};
module_platform_driver(k230_i2s_ws2812_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Canaan K230 ws2812 driver");
MODULE_ALIAS("platform:" CANAAN_DRIVER_NAME);
