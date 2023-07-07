/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-16     JasonHu      first version
 */

#include "gate.h"
#include "segment.h"
#include "interrupt.h"
#include <rtdef.h>

struct rt_hw_gate
{
    rt_uint16_t offset_low, selector;
    rt_uint8_t datacount;
    rt_uint8_t attributes;      /* P(1) DPL(2) DT(1) TYPE(4) */
    rt_uint16_t offset_high;
};
typedef struct rt_hw_gate rt_hw_gate_t;

typedef void (*rt_hw_intr_entry_t)(void);

extern void rt_hw_intr_entry0x00(void);
extern void rt_hw_intr_entry0x01(void);
extern void rt_hw_intr_entry0x02(void);
extern void rt_hw_intr_entry0x03(void);
extern void rt_hw_intr_entry0x04(void);
extern void rt_hw_intr_entry0x05(void);
extern void rt_hw_intr_entry0x06(void);
extern void rt_hw_intr_entry0x07(void);
extern void rt_hw_intr_entry0x08(void);
extern void rt_hw_intr_entry0x09(void);
extern void rt_hw_intr_entry0x0a(void);
extern void rt_hw_intr_entry0x0b(void);
extern void rt_hw_intr_entry0x0c(void);
extern void rt_hw_intr_entry0x0d(void);
extern void rt_hw_intr_entry0x0e(void);
extern void rt_hw_intr_entry0x0f(void);
extern void rt_hw_intr_entry0x10(void);
extern void rt_hw_intr_entry0x11(void);
extern void rt_hw_intr_entry0x12(void);
extern void rt_hw_intr_entry0x13(void);
extern void rt_hw_intr_entry0x14(void);
extern void rt_hw_intr_entry0x15(void);
extern void rt_hw_intr_entry0x16(void);
extern void rt_hw_intr_entry0x17(void);
extern void rt_hw_intr_entry0x18(void);
extern void rt_hw_intr_entry0x19(void);
extern void rt_hw_intr_entry0x1a(void);
extern void rt_hw_intr_entry0x1b(void);
extern void rt_hw_intr_entry0x1c(void);
extern void rt_hw_intr_entry0x1d(void);
extern void rt_hw_intr_entry0x1e(void);
extern void rt_hw_intr_entry0x1f(void);

extern void rt_hw_intr_entry0x20(void);
extern void rt_hw_intr_entry0x21(void);
extern void rt_hw_intr_entry0x22(void);
extern void rt_hw_intr_entry0x23(void);
extern void rt_hw_intr_entry0x24(void);
extern void rt_hw_intr_entry0x25(void);
extern void rt_hw_intr_entry0x26(void);
extern void rt_hw_intr_entry0x27(void);
extern void rt_hw_intr_entry0x28(void);
extern void rt_hw_intr_entry0x29(void);
extern void rt_hw_intr_entry0x2a(void);
extern void rt_hw_intr_entry0x2b(void);
extern void rt_hw_intr_entry0x2c(void);
extern void rt_hw_intr_entry0x2d(void);
extern void rt_hw_intr_entry0x2e(void);
extern void rt_hw_intr_entry0x2f(void);

static void gate_set(rt_hw_gate_t *gate, rt_hw_intr_entry_t handler,
                     rt_uint32_t selector, rt_uint32_t attributes, rt_uint8_t privilege)
{
    rt_ubase_t offset = (rt_ubase_t) handler;
    gate->offset_low  = offset & 0xffff;
    gate->selector    = selector;
    gate->attributes  = attributes | (privilege << 5);
    gate->datacount   = 0;
    gate->offset_high = (offset >> 16) & 0xffff;
}

void rt_hw_gate_init(void)
{
    rt_hw_gate_t *idt = (rt_hw_gate_t *) (IDT_VADDR);
    /*
    将中断描述符表的内容设置成内核下的中断门
    并把汇编部分的中断处理函数传入进去
    */
    int i;
    for (i = 0; i < MAX_IDT_NR; i++) {
        gate_set(IDT_OFF2PTR(idt, i), 0, 0, 0, 0);
    }
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE), rt_hw_intr_entry0x00, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+1), rt_hw_intr_entry0x01, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+2), rt_hw_intr_entry0x02, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+3), rt_hw_intr_entry0x03, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+4), rt_hw_intr_entry0x04, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+5), rt_hw_intr_entry0x05, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+6), rt_hw_intr_entry0x06, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+7), rt_hw_intr_entry0x07, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+8), rt_hw_intr_entry0x08, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+9), rt_hw_intr_entry0x09, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+10), rt_hw_intr_entry0x0a, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+11), rt_hw_intr_entry0x0b, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+12), rt_hw_intr_entry0x0c, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+13), rt_hw_intr_entry0x0d, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+14), rt_hw_intr_entry0x0e, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+15), rt_hw_intr_entry0x0f, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+16), rt_hw_intr_entry0x10, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+17), rt_hw_intr_entry0x11, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+18), rt_hw_intr_entry0x12, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+19), rt_hw_intr_entry0x13, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+20), rt_hw_intr_entry0x14, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+21), rt_hw_intr_entry0x15, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+22), rt_hw_intr_entry0x16, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+23), rt_hw_intr_entry0x17, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+24), rt_hw_intr_entry0x18, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+25), rt_hw_intr_entry0x19, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+26), rt_hw_intr_entry0x1a, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+27), rt_hw_intr_entry0x1b, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+28), rt_hw_intr_entry0x1c, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+29), rt_hw_intr_entry0x1d, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+30), rt_hw_intr_entry0x1e, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, EXCEPTION_INTR_BASE+31), rt_hw_intr_entry0x1f, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);

    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE), rt_hw_intr_entry0x20, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+1), rt_hw_intr_entry0x21, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+2), rt_hw_intr_entry0x22, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+3), rt_hw_intr_entry0x23, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+4), rt_hw_intr_entry0x24, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+5), rt_hw_intr_entry0x25, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+6), rt_hw_intr_entry0x26, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+7), rt_hw_intr_entry0x27, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+8), rt_hw_intr_entry0x28, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+9), rt_hw_intr_entry0x29, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+10), rt_hw_intr_entry0x2a, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+11), rt_hw_intr_entry0x2b, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+12), rt_hw_intr_entry0x2c, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+13), rt_hw_intr_entry0x2d, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+14), rt_hw_intr_entry0x2e, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    gate_set(IDT_OFF2PTR(idt, IRQ_INTR_BASE+15), rt_hw_intr_entry0x2f, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    /* 系统调用处理中断 */
#ifdef RT_USING_USERSPACE
    extern void hw_syscall_entry(void);
    gate_set(IDT_OFF2PTR(idt, SYSCALL_INTR_BASE), hw_syscall_entry, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL3);
#endif /* RT_USING_USERSPACE */

    extern void load_new_idt(rt_ubase_t size, rt_ubase_t idtr);
    load_new_idt(IDT_LIMIT, IDT_VADDR);
}
