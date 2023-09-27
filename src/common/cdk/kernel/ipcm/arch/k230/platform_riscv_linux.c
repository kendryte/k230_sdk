#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#include "device_config.h"
#include "ipcm_platform.h"

void __iomem *mail_box_reg_base = NULL;

static void mailbox_write_reg(uint32_t reg, uint32_t value)
{
    iowrite32(value, mail_box_reg_base + reg);
}

static uint32_t mailbox_read_reg(uint32_t reg)
{
    return ioread32(mail_box_reg_base + reg);
}

void __interrupt_trigger__(int cpu, int irq)
{
    mailbox_write_reg(CPU2DSP_INT_SET0, 0);
}

extern int irq_callback(int irq, void *data);

static irqreturn_t ipcm_irq(int irq_num, void *data)
{
	irq_callback(irq_num, NULL);
	mailbox_write_reg(DSP2CPU_INT_CLEAR0, 0);
	return IRQ_HANDLED;
}

static int ipcm_probe(struct platform_device *pdev)
{
	int ret;
	uint32_t value;
	int irq = 0;

	mail_box_reg_base = ioremap(MAILBOX_REG_BASE, 0x10000);

	value = mailbox_read_reg(CPU2DSP_INT_EN0);

	irq = platform_get_irq(pdev, 0);

	ret = devm_request_irq(&pdev->dev, irq, ipcm_irq, 0,
			       "ipcm_irq", mail_box_reg_base);

	mailbox_write_reg(CPU2DSP_INT_EN0, 0x1 << 16 | 0x1 << 0);
	mailbox_write_reg(DSP2CPU_INT_EN0, 0x1 << 16 | 0x1 << 0);

	if(ret < 0) {
		iounmap(mail_box_reg_base);
		//devm_iounmap(&pdev->dev, mail_box_reg_base);
		printk("request irq for ipcm failed.get:%d,ret:%d\n",
			   irq, ret);
		return 0;
	}

	pr_info("%s OK\n", __func__);
	return 0;
}

static int ipcm_remove(struct platform_device *pdev)
{
	devm_iounmap(&pdev->dev, mail_box_reg_base);
	return 0;
}

static const struct of_device_id ipcm_match[] = {
	{ .compatible = "canaan,ipcm-interrupt"},
	{},
};

static struct platform_driver ipcm_driver = {
	.driver     = {
		.name   = "ipcm-interrupt",
		.of_match_table = ipcm_match,
	},
	.probe      = ipcm_probe,
	.remove     = ipcm_remove,
};

int __arch_init__(void)
{
	int ret;
	ret = platform_driver_register(&ipcm_driver);
	return ret;
}

void __arch_free__(void)
{
	platform_driver_unregister(&ipcm_driver);
}

void __barrier__(void)
{
}
