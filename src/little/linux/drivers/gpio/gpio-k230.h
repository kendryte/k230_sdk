/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef GPIO_K230_H
#define GPIO_K230_H

#define K230_MAX_GPIOS		1
#define K230_GPIOA_BASE_ADDR		(0x9140B000)
#define K230_GPIOB_BASE_ADDR		(0x9140C000)

struct k230_port_property {
	struct fwnode_handle *fwnode;
	unsigned int	idx;
	unsigned int	id;
	unsigned int	ngpio;
	unsigned int	gpio_base;
	int		irq[K230_MAX_GPIOS];
	bool		irq_shared;
};

struct k230_platform_data {
	struct k230_port_property *properties;
	unsigned int nports;
};

#endif
