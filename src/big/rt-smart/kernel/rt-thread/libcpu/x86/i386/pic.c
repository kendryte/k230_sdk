/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-16     JasonHu      first version
 */

#include "pic.h"
#include <interrupt.h>

void rt_hw_pic_init(void)
{
    /* mask all interrupts */
    outb(PIC_MASTER_CTLMASK,  0xff);
    outb(PIC_SLAVE_CTLMASK,   0xff);

    outb(PIC_MASTER_CTL,      0x11);
    outb(PIC_MASTER_CTLMASK,  0x20);
    outb(PIC_MASTER_CTLMASK,  1 << 2);
    outb(PIC_MASTER_CTLMASK,  0x01);

    outb(PIC_SLAVE_CTL,       0x11);
    outb(PIC_SLAVE_CTLMASK,   0x28);
    outb(PIC_SLAVE_CTLMASK,   2);
    outb(PIC_SLAVE_CTLMASK,   0x01);

    /* mask all interrupts */
    outb(PIC_MASTER_CTLMASK,  0xff);
    outb(PIC_SLAVE_CTLMASK,   0xff);
}

void rt_hw_pic_enable(int irq)
{
    if (irq < 8) /* clear master */
    {
        outb(PIC_MASTER_CTLMASK, inb(PIC_MASTER_CTLMASK) & ~(1 << irq));
    }
    else /* clear irq 2 first, then clear slave */
    {
        outb(PIC_MASTER_CTLMASK, inb(PIC_MASTER_CTLMASK) & ~(1 << PIC_SLAVE_CONNECT_IRQ));
        outb(PIC_SLAVE_CTLMASK, inb(PIC_SLAVE_CTLMASK) & ~ (1 << (irq - 8)));
    }
}

void rt_hw_pic_disable(int irq)
{
    if(irq < 8) /* set master */
    {
        outb(PIC_MASTER_CTLMASK, inb(PIC_MASTER_CTLMASK) | (1 << irq));
    }
    else /* set slave */
    {
        outb(PIC_SLAVE_CTLMASK, inb(PIC_SLAVE_CTLMASK) | (1 << (irq - 8)));
    }
}

void rt_hw_pic_ack(int irq)
{
    if (irq >= 8) /* slaver */
    {
        outb(PIC_SLAVE_CTL,  PIC_EIO);
    }
    outb(PIC_MASTER_CTL,  PIC_EIO);
}
