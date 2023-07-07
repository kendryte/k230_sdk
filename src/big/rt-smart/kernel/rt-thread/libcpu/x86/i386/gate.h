/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-16     JasonHu      first version
 */

#ifndef __X86_GATE_H__
#define __X86_GATE_H__

#include <rtconfig.h>

#define IDT_LIMIT       0x000007ff
#define IDT_PADDR       0x003F0800

#define IDT_VADDR       (KERNEL_VADDR_START + IDT_PADDR)

#define MAX_IDT_NR (IDT_LIMIT/8)

#define IDT_OFF2PTR(idt, off)    (idt + off)

/* DA: Descriptor Attribute */
#define DA_TASK_GATE        0x85
#define DA_386_CALL_GATE    0x8C
#define DA_386_INTR_GATE    0x8E
#define DA_386_TRAP_GATE    0x8F

#define DA_GATE_DPL0 0
#define DA_GATE_DPL1 1
#define DA_GATE_DPL2 2
#define DA_GATE_DPL3 3

#ifndef __ASSEMBLY__
void rt_hw_gate_init(void);
#endif

#endif  /* __X86_GATE_H__ */
