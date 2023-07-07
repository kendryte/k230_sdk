/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-16     JasonHu      first version
 */

#ifndef __PIC_H__
#define __PIC_H__

#define PIC_MASTER_CTL      0x20    /* I/O port for interrupt controller         <Master> */
#define PIC_MASTER_CTLMASK  0x21    /* setting bits in this port disables ints   <Master> */
#define PIC_SLAVE_CTL       0xa0    /* I/O port for second interrupt controller  <Slave>  */
#define PIC_SLAVE_CTLMASK   0xa1    /* setting bits in this port disables ints   <Slave>  */

#define PIC_EIO             0x20    /* end of IO port */

#define PIC_SLAVE_CONNECT_IRQ 2    /* irq2 connected to slaver pic */

void rt_hw_pic_init();
void rt_hw_pic_enable(int irq);
void rt_hw_pic_disable(int irq);
void rt_hw_pic_ack(int irq);

#endif  /* __PIC_H__ */
