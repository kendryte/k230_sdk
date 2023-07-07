/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-14     JasonHu      first version
 */

#ifndef __I386_H__
#define __I386_H__

#include <rtdef.h>

#define EFLAGS_MBS    (1 << 1)
#define EFLAGS_IF_1   (1 << 9)
#define EFLAGS_IF_0 0
#define EFLAGS_IOPL_3 (3 << 12)
#define EFLAGS_IOPL_1 (1 << 12)
#define EFLAGS_IOPL_0 (0 << 12)

#define EFLAGS_IF (EFLAGS_IF_1)

/* cr0 bit 31 is page enable bit, 1: enable MMU, 0: disable MMU */
#define CR0_PG  (1 << 31)

rt_inline rt_uint8_t inb(int port)
{
    rt_uint8_t data;
    __asm__ __volatile__("inb %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

rt_inline rt_uint16_t inw(int port)
{
    rt_uint16_t data;
    __asm__ __volatile__("inw %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

rt_inline rt_uint32_t inl(int port)
{
    rt_uint32_t data;
    __asm__ __volatile__("inl %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

rt_inline void outb(int port, rt_uint8_t data)
{
    __asm__ __volatile__("outb %0,%w1" : : "a" (data), "d" (port));
}

rt_inline void outw(int port, rt_uint16_t data)
{
    __asm__ __volatile__("outw %0,%w1" : : "a" (data), "d" (port));
}

rt_inline void outl(int port, rt_uint32_t data)
{
    __asm__ __volatile__("outl %0,%w1" : : "a" (data), "d" (port));
}

rt_inline rt_uint8_t read_cmos(int reg)
{
    outb(0x70, reg);
    return (rt_uint8_t) inb(0x71);
}

#define io_delay()  \
    __asm__ __volatile__ ("pushal \n\t"\
                "mov $0x3F6, %dx \n\t" \
                "inb %dx, %al \n\t"    \
                "inb %dx, %al \n\t"    \
                "inb %dx, %al \n\t"    \
                "inb %dx, %al \n\t"    \
                "popal")

rt_inline void ltr(rt_uint32_t selector)
{
    __asm__ __volatile__("ltr %w0" : : "q" (selector));
}

rt_uint32_t read_cr0(void);
rt_uint32_t read_cr2(void);
void write_cr0(rt_uint32_t value);
void write_cr3(rt_uint32_t pgdir);

rt_inline void rt_hw_cpu_pause(void)
{
    __asm__ __volatile__ ("pause");
}

#endif  /* __I386_H__ */
