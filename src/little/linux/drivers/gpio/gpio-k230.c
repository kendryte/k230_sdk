// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022, Canaan Bright Sight Co., Ltd
 * Copyright (c) 2011 Jamie Iles
 *
 * All enquiries to support@picochip.com
 * based on gpio-dwapb.c
 */
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/gpio/driver.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/reset.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/canaan-hardlock.h>
#include "gpio-k230.h"
#include "gpiolib.h"

#define GPIO_SWPORTA_DR		0x00
#define GPIO_SWPORTA_DDR	0x04
#define GPIO_SWPORTB_DR		0x0c
#define GPIO_SWPORTB_DDR	0x10
#define GPIO_SWPORTC_DR		0x18
#define GPIO_SWPORTC_DDR	0x1c
#define GPIO_SWPORTD_DR		0x24
#define GPIO_SWPORTD_DDR	0x28
#define GPIO_INTEN		0x30
#define GPIO_INTMASK		0x34
#define GPIO_INTTYPE_LEVEL	0x38
#define GPIO_INT_POLARITY	0x3c
#define GPIO_INTSTATUS		0x40
#define GPIO_PORTA_DEBOUNCE	0x48
#define GPIO_PORTA_EOI		0x4c
#define GPIO_EXT_PORTA		0x50
#define GPIO_EXT_PORTB		0x54
#define GPIO_EXT_PORTC		0x58
#define GPIO_EXT_PORTD		0x5c
#define GPIO_INTTYPE_BOTHEDGE	0x68

#define K230_DRIVER_NAME	"k230-gpio"
#define K230_MAX_PORTS		4

#define GPIO_EXT_PORT_STRIDE	0x04 /* register stride 32 bits */
#define GPIO_SWPORT_DR_STRIDE	0x0c /* register stride 3*32 bits */
#define GPIO_SWPORT_DDR_STRIDE	0x0c /* register stride 3*32 bits */

#define GPIO_REG_OFFSET_V2	1

#define GPIO_INTMASK_V2		0x44
#define GPIO_INTTYPE_LEVEL_V2	0x34
#define GPIO_INT_POLARITY_V2	0x38
#define GPIO_INTSTATUS_V2	0x3c
#define GPIO_PORTA_EOI_V2	0x40

#define K230_NR_CLOCKS		2

struct k230_gpio;
static int hardlock;
static bool hardlock_requested = false;

#ifdef CONFIG_PM_SLEEP
/* Store GPIO context across system-wide suspend/resume transitions */
struct k230_context {
	u32 data;
	u32 dir;
	u32 ext;
	u32 int_en;
	u32 int_mask;
	u32 int_type;
	u32 int_pol;
	u32 int_deb;
	u32 wake_en;
};
#endif

struct k230_gpio_port_irqchip {
	struct irq_chip		irqchip;
	unsigned int		nr_irqs;
	unsigned int		irq[K230_MAX_GPIOS];
};

struct k230_gpio_port {
	struct gpio_chip	gc;
	struct k230_gpio_port_irqchip *pirq;
	struct k230_gpio	*gpio;
#ifdef CONFIG_PM_SLEEP
	struct k230_context	*ctx;
#endif
	unsigned int		idx;
	unsigned int		id;
};
#define to_k230_gpio(_gc) \
	(container_of(_gc, struct k230_gpio_port, gc)->gpio)

struct k230_gpio {
	struct	device			*dev;
	struct device_node 		*node;
	struct device_node 		*irq_parent;
	struct irq_domain 		*parent;
	void __iomem			*regs;
	struct k230_gpio_port	*ports;
	unsigned int			nr_ports;
	unsigned int			flags;
	struct reset_control	*rst;
	struct clk_bulk_data	clks[K230_NR_CLOCKS];
};

static void __iomem			*iomem_gpio_a;
static void __iomem			*iomem_gpio_b;
static bool k230_mapped 	= false;

static inline u32 gpio_reg_v2_convert(unsigned int offset)
{
	switch (offset) {
	case GPIO_INTMASK:
		return GPIO_INTMASK_V2;
	case GPIO_INTTYPE_LEVEL:
		return GPIO_INTTYPE_LEVEL_V2;
	case GPIO_INT_POLARITY:
		return GPIO_INT_POLARITY_V2;
	case GPIO_INTSTATUS:
		return GPIO_INTSTATUS_V2;
	case GPIO_PORTA_EOI:
		return GPIO_PORTA_EOI_V2;
	}

	return offset;
}

static inline u32 gpio_reg_convert(struct k230_gpio *gpio, unsigned int offset)
{
	if (gpio->flags & GPIO_REG_OFFSET_V2)
		return gpio_reg_v2_convert(offset);

	return offset;
}

static inline u32 k230_read(struct k230_gpio *gpio, unsigned int offset)
{
	struct gpio_chip *gc	= &gpio->ports[0].gc;
	void __iomem *reg_base	= gpio->regs;

	return gc->read_reg(reg_base + gpio_reg_convert(gpio, offset));
}

static inline void k230_write(struct k230_gpio *gpio, unsigned int offset,
			       u32 val)
{
	struct gpio_chip *gc	= &gpio->ports[0].gc;
	void __iomem *reg_base	= gpio->regs;

	gc->write_reg(reg_base + gpio_reg_convert(gpio, offset), val);
}

static void k230_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct k230_gpio *gpio = to_k230_gpio(gc);
	u32 val;
	unsigned long flags;
	int offset = gc->base;
	if (offset >= 32)
		offset -= 32;

	val = BIT(offset);
	spin_lock_irqsave(&gc->bgpio_lock, flags);
	k230_write(gpio, GPIO_PORTA_EOI, val);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	if (irqd_is_activated(d))
		irq_chip_eoi_parent(d);
}

static void k230_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct k230_gpio *gpio = to_k230_gpio(gc);
	unsigned long flags;
	u32 val;
	unsigned int offset = gc->base;
	if (offset >= 32)
		offset -= 32;

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));
	val = k230_read(gpio, GPIO_INTMASK) | BIT(offset);
	k230_write(gpio, GPIO_INTMASK, val);
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
}

static void k230_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct k230_gpio *gpio = to_k230_gpio(gc);
	unsigned long flags;
	u32 val;
	unsigned int offset = gc->base;
	if (offset >= 32)
		offset -= 32;
	irq_chip_unmask_parent(d);

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));
	val = k230_read(gpio, GPIO_INTMASK) & ~BIT(offset);
	k230_write(gpio, GPIO_INTMASK, val);
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
}

static void k230_irq_enable(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct k230_gpio *gpio = to_k230_gpio(gc);
	unsigned long flags;
	u32 val;

	unsigned int offset = gc->base;
	if (offset >= 32)
		offset -= 32;
	irq_chip_enable_parent(d);

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));
	val = k230_read(gpio, GPIO_INTEN);
	val |= BIT(offset);
	k230_write(gpio, GPIO_INTEN, val);
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
}

static void k230_irq_disable(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct k230_gpio *gpio = to_k230_gpio(gc);
	unsigned long flags;
	u32 val;
	unsigned int offset = gc->base;
	if (offset >= 32)
		offset -= 32;

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));
	val = k230_read(gpio, GPIO_INTEN);
	val &= ~BIT(offset);
	k230_write(gpio, GPIO_INTEN, val);
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
	irq_chip_disable_parent(d);
}

static void k230_irq_eoi(struct irq_data *d)
{
	irq_chip_eoi_parent(d);
}

static int k230_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct k230_gpio *gpio = to_k230_gpio(gc);
	unsigned long level, polarity, flags;
	u32 bit;
	unsigned int offset = gc->base;
	if (offset >= 32)
		offset -= 32;
	bit = offset;

	if (type & ~IRQ_TYPE_SENSE_MASK)
		return -EINVAL;

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));

	if (type == IRQ_TYPE_EDGE_BOTH)
	{
		level = k230_read(gpio, GPIO_INTTYPE_BOTHEDGE);
		level &= ~BIT(bit);
		level |= BIT(bit);

		k230_write(gpio, GPIO_INTTYPE_BOTHEDGE, level);
	} else {
		level = k230_read(gpio, GPIO_INTTYPE_LEVEL);
		polarity = k230_read(gpio, GPIO_INT_POLARITY);

		switch (type) {
		case IRQ_TYPE_EDGE_RISING:
			level |= BIT(bit);
			polarity |= BIT(bit);
			break;
		case IRQ_TYPE_EDGE_FALLING:
			level |= BIT(bit);
			polarity &= ~BIT(bit);
			break;
		case IRQ_TYPE_LEVEL_HIGH:
			level &= ~BIT(bit);
			polarity |= BIT(bit);
			break;
		case IRQ_TYPE_LEVEL_LOW:
			level &= ~BIT(bit);
			polarity &= ~BIT(bit);
			break;
		}

		k230_write(gpio, GPIO_INTTYPE_LEVEL, level);
		k230_write(gpio, GPIO_INT_POLARITY, polarity);
	}
	hardlock_unlock(hardlock);

	if (type & IRQ_TYPE_LEVEL_MASK)
		irq_set_handler_locked(d, handle_level_irq);
	else if (type & IRQ_TYPE_EDGE_BOTH)
		irq_set_handler_locked(d, handle_edge_irq);

	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int k230_irq_set_wake(struct irq_data *d, unsigned int enable)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct k230_gpio *gpio = to_k230_gpio(gc);
	struct k230_context *ctx = gpio->ports[0].ctx;
	irq_hw_number_t bit = irqd_to_hwirq(d);

	if (enable)
		ctx->wake_en |= BIT(bit);
	else
		ctx->wake_en &= ~BIT(bit);

	return 0;
}
#endif

static int k230_gpio_set_debounce(struct gpio_chip *gc,
				   unsigned offset, unsigned debounce)
{
	struct k230_gpio_port *port = gpiochip_get_data(gc);
	struct k230_gpio *gpio = port->gpio;
	unsigned long flags, val_deb;
	unsigned long mask;
	offset = gc->base;
	if (offset >= 32)
		offset -= 32;
	mask = BIT(offset);
	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));
	val_deb = k230_read(gpio, GPIO_PORTA_DEBOUNCE);
	val_deb |= mask;
	k230_write(gpio, GPIO_PORTA_DEBOUNCE, val_deb);
	hardlock_unlock(hardlock);

	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
	return 0;
}

static int k230_gpio_set_config(struct gpio_chip *gc, unsigned offset,
				 unsigned long config)
{
	u32 debounce;
	debounce = pinconf_to_config_argument(config);
	return k230_gpio_set_debounce(gc, offset, debounce);
}

static int k230_convert_irqs(struct k230_gpio_port_irqchip *pirq,
			      struct k230_port_property *pp)
{
	pirq->irq[0] = pp->irq[0];
	pirq->nr_irqs = 1;
	return pirq->nr_irqs ? 0 : -ENOENT;
}

static int k230_gpio_child_to_parent_hwirq(struct gpio_chip *gc,
					     unsigned int child,
					     unsigned int child_type,
					     unsigned int *parent,
					     unsigned int *parent_type)
{
	*parent_type = IRQ_TYPE_NONE;
	*parent = gc->base + 32;
	return 0;
}

static void k230_configure_irqs(struct k230_gpio *gpio,
				 struct k230_gpio_port *port,
				 struct k230_port_property *pp)
{
	struct k230_gpio_port_irqchip *pirq;
	struct gpio_chip *gc = &port->gc;
	struct gpio_irq_chip *girq;

	pirq = devm_kzalloc(gpio->dev, sizeof(*pirq), GFP_KERNEL);
	if (!pirq)
		return;

	if (k230_convert_irqs(pirq, pp)) {
		dev_warn(gpio->dev, "no IRQ for port%d\n", pp->idx);
		goto err_kfree_pirq;
	}

	gc->owner = THIS_MODULE;
	gc->parent = gpio->dev;
	girq = &gc->irq;
	girq->fwnode = of_node_to_fwnode(gpio->node);
	girq->parent_domain = gpio->parent;
	girq->child_to_parent_hwirq = k230_gpio_child_to_parent_hwirq;
	girq->handler = handle_bad_irq;
	girq->default_type = IRQ_TYPE_NONE;

	port->pirq = pirq;
	pirq->irqchip.name = K230_DRIVER_NAME;
	pirq->irqchip.irq_ack = k230_irq_ack;
	pirq->irqchip.irq_mask = k230_irq_mask;
	pirq->irqchip.irq_unmask = k230_irq_unmask;
	pirq->irqchip.irq_set_type = k230_irq_set_type;
	pirq->irqchip.irq_enable = k230_irq_enable;
	pirq->irqchip.irq_disable = k230_irq_disable;
	pirq->irqchip.irq_eoi = k230_irq_eoi;
#ifdef CONFIG_PM_SLEEP
	pirq->irqchip.irq_set_wake = k230_irq_set_wake;
#endif

	girq->chip = &pirq->irqchip;

	return;

err_kfree_pirq:
	devm_kfree(gpio->dev, pirq);
}

static int k230_gpio_get(struct gpio_chip *gc, unsigned int gpio)
{
	gpio = gc->base;
	if (gpio >= 32)
		gpio -= 32;

	return !!(gc->read_reg(gc->reg_dat) & BIT(gpio));
}

static void k230_gpio_set_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	unsigned long mask;
	unsigned long flags;
	gpio = gc->base;

	if (gpio >= 32)
			gpio -= 32;
	mask = BIT(gpio);
	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));
	if (val) {
		gc->bgpio_data = gc->read_reg(gc->reg_dir_out);
		gc->bgpio_data |= mask;
		gc->write_reg(gc->reg_dir_out, gc->bgpio_data);
		gc->bgpio_data = gc->read_reg(gc->reg_set);
		gc->bgpio_data |= mask;
		gc->write_reg(gc->reg_set, gc->bgpio_data);
	} else {
		gc->bgpio_data = gc->read_reg(gc->reg_set);
		gc->bgpio_data &= ~mask;
		gc->write_reg(gc->reg_set, gc->bgpio_data);
	}
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
}

static int k230_gpio_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	unsigned long flags;
	unsigned int dir;

	spin_lock_irqsave(&gc->bgpio_lock, flags);

	gpio = gc->base;
	if (gpio >= 32)
		gpio -= 32;

	dir = ~BIT(gpio);

	while(hardlock_lock(hardlock));
	if (gc->reg_dir_in)
		gc->write_reg(gc->reg_dir_in, ~gc->bgpio_dir);
	if (gc->reg_dir_out) {
		gc->bgpio_dir = gc->read_reg(gc->reg_dir_out);
		gc->bgpio_dir &= dir;
		gc->write_reg(gc->reg_dir_out, gc->bgpio_dir);
	}
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	return 0;
}

static int k230_gpio_get_dir(struct gpio_chip *gc, unsigned int gpio)
{
	gpio = gc->base;
	if (gpio >= 32)
		gpio -= 32;
	/* Return 0 if output, 1 if input */
	if (gc->bgpio_dir_unreadable) {
		if (gc->bgpio_dir & BIT(gpio))
			return GPIO_LINE_DIRECTION_OUT;
		return GPIO_LINE_DIRECTION_IN;
	}

	if (gc->reg_dir_out) {
		if (gc->read_reg(gc->reg_dir_out) & BIT(gpio))
			return GPIO_LINE_DIRECTION_OUT;
		return GPIO_LINE_DIRECTION_IN;
	}

	if (gc->reg_dir_in)
		if (!(gc->read_reg(gc->reg_dir_in) & BIT(gpio)))
			return GPIO_LINE_DIRECTION_OUT;

	return GPIO_LINE_DIRECTION_IN;
}

static int k230_dir_out_val_first(struct gpio_chip *gc, unsigned int gpio,
				   int val)
{
	unsigned long flags;
	unsigned int dir;
	gc->set(gc, gpio, val);

	gpio = gc->base;
	if (gpio >= 32)
		gpio -= 32;

	spin_lock_irqsave(&gc->bgpio_lock, flags);

	dir = BIT(gpio);
	while(hardlock_lock(hardlock));
	if (gc->reg_dir_in)
		gc->write_reg(gc->reg_dir_in, ~gc->bgpio_dir);
	if (gc->reg_dir_out) {
		gc->bgpio_dir = gc->read_reg(gc->reg_dir_out);
		gc->bgpio_dir |= dir;
		gc->write_reg(gc->reg_dir_out, gc->bgpio_dir);
	}
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	return 0;
}

static void k230_write_reg(void __iomem *reg, unsigned long data)
{
	writel(data, reg);
}

static unsigned long k230_read_reg(void __iomem *reg)
{
	return readl(reg);
}

static int k230_gpio_add_port(struct platform_device *pdev, struct k230_gpio *gpio,
			       struct k230_port_property *pp,
			       unsigned int offs)
{
	struct k230_gpio_port *port;
	void __iomem *dat, *set, *dirout;
	int err;

	port = &gpio->ports[offs];
	port->gpio = gpio;
	port->idx = pp->idx;

#ifdef CONFIG_PM_SLEEP
	port->ctx = devm_kzalloc(gpio->dev, sizeof(*port->ctx), GFP_KERNEL);
	if (!port->ctx)
		return -ENOMEM;
#endif

	dat = gpio->regs + GPIO_EXT_PORTA + pp->idx * GPIO_EXT_PORT_STRIDE;
	set = gpio->regs + GPIO_SWPORTA_DR + pp->idx * GPIO_SWPORT_DR_STRIDE;
	dirout = gpio->regs + GPIO_SWPORTA_DDR + pp->idx * GPIO_SWPORT_DDR_STRIDE;

	/* This registers 32 GPIO lines per port */
	err = bgpio_init(&port->gc, gpio->dev, 4, dat, set, NULL, dirout,
			 NULL, 0);
	if (err) {
		dev_err(gpio->dev, "failed to init gpio chip for port%d\n",
			port->idx);
		return err;
	}

/* set k230 gpiochip callback function, because it is special */
	port->gc.get = k230_gpio_get;
	port->gc.set = k230_gpio_set_set;
	port->gc.direction_input = k230_gpio_dir_in;
	port->gc.get_direction = k230_gpio_get_dir;
	port->gc.direction_output = k230_dir_out_val_first;
	port->gc.write_reg = k230_write_reg;
	port->gc.read_reg = k230_read_reg;

#ifdef CONFIG_OF_GPIO
	port->gc.of_node = to_of_node(pp->fwnode);
#endif
	port->gc.ngpio = pp->ngpio;
	port->gc.base = pp->gpio_base;
	port->gc.set_config = k230_gpio_set_config;

	if (of_property_read_u32(pdev->dev.of_node, "interrupts", &pp->irq[0])) {
		dev_err(gpio->dev, "get irq num failed!\n");
		return -EINVAL;
	}

	/* get gpio used hardlock num */
	if (!hardlock_requested) {
		if (of_property_read_u32(pdev->dev.of_node, "hardlock", &hardlock)) {
			dev_err(gpio->dev, "get used hardlock num failed!\n");
			return -EINVAL;
		}
		if (request_lock(hardlock))
		{
			dev_err(gpio->dev, "request hardlock %d failed!\n", hardlock);
			hardlock = -1;
		}
		hardlock_requested = true;
		dev_err(gpio->dev, "request hardlock %d success!\n", hardlock);
	}

	k230_configure_irqs(gpio, port, pp);

	err = devm_gpiochip_add_data(gpio->dev, &port->gc, port);
	if (err) {
		dev_err(gpio->dev, "failed to register gpiochip for port%d\n",
			port->idx);
		return err;
	}

	return 0;
}

static struct k230_platform_data *k230_gpio_get_pdata(struct device *dev)
{
	struct fwnode_handle *fwnode;
	struct k230_platform_data *pdata;
	struct k230_port_property *pp;
	int nports;
	int i;

	nports = device_get_child_node_count(dev);
	if (nports == 0)
		return ERR_PTR(-ENODEV);

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);

	pdata->properties = devm_kcalloc(dev, nports, sizeof(*pp), GFP_KERNEL);
	if (!pdata->properties)
		return ERR_PTR(-ENOMEM);

	pdata->nports = nports;

	i = 0;
	device_for_each_child_node(dev, fwnode)  {
		pp = &pdata->properties[i++];
		pp->fwnode = fwnode;

		if (fwnode_property_read_u32(fwnode, "reg-bank", &pp->idx) ||
		    pp->idx >= K230_MAX_PORTS) {
			dev_err(dev,
				"missing/invalid port index for port%d\n", i);
			fwnode_handle_put(fwnode);
			return ERR_PTR(-EINVAL);
		}

		if (fwnode_property_read_u32(fwnode, "ngpios", &pp->ngpio) &&
		    fwnode_property_read_u32(fwnode, "nr-gpios", &pp->ngpio)) {
			dev_info(dev,
				 "failed to get number of gpios for port%d\n",
				 i);
			pp->ngpio = K230_MAX_GPIOS;
		}

		if (fwnode_property_read_u32(fwnode, "id", &pp->id) ||
		    pp->idx > 1 || pp->idx < 0) {
			dev_err(dev,
				"invalid port num for port%d\n", i);
			fwnode_handle_put(fwnode);
			return ERR_PTR(-EINVAL);
		}

		pp->irq_shared	= false;
		pp->gpio_base	= pp->id;
	}

	return pdata;
}

static void k230_assert_reset(void *data)
{
	struct k230_gpio *gpio = data;

	reset_control_assert(gpio->rst);
}

static int k230_get_reset(struct k230_gpio *gpio)
{
	int err;

	gpio->rst = devm_reset_control_get_optional_shared(gpio->dev, NULL);
	if (IS_ERR(gpio->rst)) {
		dev_err(gpio->dev, "Cannot get reset descriptor\n");
		return PTR_ERR(gpio->rst);
	}

	err = reset_control_deassert(gpio->rst);
	if (err) {
		dev_err(gpio->dev, "Cannot deassert reset lane\n");
		return err;
	}

	return devm_add_action_or_reset(gpio->dev, k230_assert_reset, gpio);
}

static void k230_disable_clks(void *data)
{
	struct k230_gpio *gpio = data;

	clk_bulk_disable_unprepare(K230_NR_CLOCKS, gpio->clks);
}

static int k230_get_clks(struct k230_gpio *gpio)
{
	int err;

	/* Optional bus and debounce clocks */
	gpio->clks[0].id = "bus";
	gpio->clks[1].id = "db";
	err = devm_clk_bulk_get_optional(gpio->dev, K230_NR_CLOCKS,
					 gpio->clks);
	if (err) {
		dev_err(gpio->dev, "Cannot get APB/Debounce clocks\n");
		return err;
	}

	err = clk_bulk_prepare_enable(K230_NR_CLOCKS, gpio->clks);
	if (err) {
		dev_err(gpio->dev, "Cannot enable APB/Debounce clocks\n");
		return err;
	}

	return devm_add_action_or_reset(gpio->dev, k230_disable_clks, gpio);
}

static const struct of_device_id k230_of_match[] = {
	{ .compatible = "canaan,k230-apb-gpio", .data = (void *)0},
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, k230_of_match);

static int k230_gpio_probe(struct platform_device *pdev)
{
	struct k230_gpio *gpio;
	int err;
	struct device *dev = &pdev->dev;
	struct k230_platform_data *pdata = dev_get_platdata(dev);
	struct resource *r;

	if (!k230_mapped) {
		iomem_gpio_a = ioremap(K230_GPIOA_BASE_ADDR, 0x1000);
		iomem_gpio_b = ioremap(K230_GPIOB_BASE_ADDR, 0x1000);
		k230_mapped = true;
	}

	if (!pdata) {
		pdata = k230_gpio_get_pdata(dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}

	if (!pdata->nports)
		return -ENODEV;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	gpio->dev = &pdev->dev;
	gpio->node = pdev->dev.of_node;
	gpio->nr_ports = pdata->nports;
	gpio->irq_parent = of_irq_find_parent(gpio->node);
	if (!gpio->irq_parent) {
		dev_err(dev, "no IRQ parent node\n");
		return -ENODEV;
	}
	gpio->parent = irq_find_host(gpio->irq_parent);
	if (!gpio->parent) {
		dev_err(dev, "no IRQ parent domain\n");
		return -ENODEV;
	}

	err = k230_get_reset(gpio);
	if (err)
		return err;

	gpio->ports = devm_kcalloc(&pdev->dev, gpio->nr_ports,
				   sizeof(*gpio->ports), GFP_KERNEL);
	if (!gpio->ports)
		return -ENOMEM;

	gpio->ports[0].id = pdata->properties[0].id;

	/* for k230 special design */
	if (k230_mapped) {
		r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (r->start == K230_GPIOA_BASE_ADDR)
		{
			gpio->regs = iomem_gpio_a;
			pdata->properties[0].idx = 0;
		} else if (r->start == K230_GPIOB_BASE_ADDR) {
			gpio->regs = iomem_gpio_b;
			pdata->properties[0].idx = 0;
		}
	} else {
		dev_err(dev, "get device register address failed!\n");
		return -ENOMEM;
	}

	err = k230_get_clks(gpio);
	if (err)
		return err;

	gpio->flags = (uintptr_t)device_get_match_data(dev);

	err = k230_gpio_add_port(pdev, gpio, &pdata->properties[0], 0);
	if (err)
		return err;

	platform_set_drvdata(pdev, gpio);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int k230_gpio_suspend(struct device *dev)
{
	struct k230_gpio *gpio = dev_get_drvdata(dev);
	struct gpio_chip *gc	= &gpio->ports[0].gc;
	unsigned long flags;
	int i;

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	for (i = 0; i < gpio->nr_ports; i++) {
		unsigned int offset;
		unsigned int idx = gpio->ports[i].idx;
		struct k230_context *ctx = gpio->ports[i].ctx;

		offset = GPIO_SWPORTA_DDR + idx * GPIO_SWPORT_DDR_STRIDE;
		ctx->dir = k230_read(gpio, offset);

		offset = GPIO_SWPORTA_DR + idx * GPIO_SWPORT_DR_STRIDE;
		ctx->data = k230_read(gpio, offset);

		offset = GPIO_EXT_PORTA + idx * GPIO_EXT_PORT_STRIDE;
		ctx->ext = k230_read(gpio, offset);

		/* Only port A can provide interrupts */
		if (idx == 0) {
			ctx->int_mask	= k230_read(gpio, GPIO_INTMASK);
			ctx->int_en	= k230_read(gpio, GPIO_INTEN);
			ctx->int_pol	= k230_read(gpio, GPIO_INT_POLARITY);
			ctx->int_type	= k230_read(gpio, GPIO_INTTYPE_LEVEL);
			ctx->int_deb	= k230_read(gpio, GPIO_PORTA_DEBOUNCE);

			/* Mask out interrupts */
			while(hardlock_lock(hardlock));
			k230_write(gpio, GPIO_INTMASK, ~ctx->wake_en);
			hardlock_unlock(hardlock);
		}
	}
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	clk_bulk_disable_unprepare(K230_NR_CLOCKS, gpio->clks);

	return 0;
}

static int k230_gpio_resume(struct device *dev)
{
	struct k230_gpio *gpio = dev_get_drvdata(dev);
	struct gpio_chip *gc	= &gpio->ports[0].gc;
	unsigned long flags;
	int i, err;

	err = clk_bulk_prepare_enable(K230_NR_CLOCKS, gpio->clks);
	if (err) {
		dev_err(gpio->dev, "Cannot reenable APB/Debounce clocks\n");
		return err;
	}

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	while(hardlock_lock(hardlock));
	for (i = 0; i < gpio->nr_ports; i++) {
		unsigned int offset;
		unsigned int idx = gpio->ports[i].idx;
		struct k230_context *ctx = gpio->ports[i].ctx;

		offset = GPIO_SWPORTA_DR + idx * GPIO_SWPORT_DR_STRIDE;
		k230_write(gpio, offset, ctx->data);

		offset = GPIO_SWPORTA_DDR + idx * GPIO_SWPORT_DDR_STRIDE;
		k230_write(gpio, offset, ctx->dir);

		offset = GPIO_EXT_PORTA + idx * GPIO_EXT_PORT_STRIDE;
		k230_write(gpio, offset, ctx->ext);

		/* Only port A can provide interrupts */
		if (idx == 0) {
			k230_write(gpio, GPIO_INTTYPE_LEVEL, ctx->int_type);
			k230_write(gpio, GPIO_INT_POLARITY, ctx->int_pol);
			k230_write(gpio, GPIO_PORTA_DEBOUNCE, ctx->int_deb);
			k230_write(gpio, GPIO_INTEN, ctx->int_en);
			k230_write(gpio, GPIO_INTMASK, ctx->int_mask);

			/* Clear out spurious interrupts */
			k230_write(gpio, GPIO_PORTA_EOI, 0xffffffff);
		}
	}
	hardlock_unlock(hardlock);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(k230_gpio_pm_ops, k230_gpio_suspend,
			 k230_gpio_resume);

static struct platform_driver k230_gpio_driver = {
	.driver		= {
		.name	= K230_DRIVER_NAME,
		.pm	= &k230_gpio_pm_ops,
		.of_match_table = k230_of_match,
	},
	.probe		= k230_gpio_probe,
};

module_platform_driver(k230_gpio_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Canaan K230 GPIO driver");
MODULE_ALIAS("platform:" K230_DRIVER_NAME);
