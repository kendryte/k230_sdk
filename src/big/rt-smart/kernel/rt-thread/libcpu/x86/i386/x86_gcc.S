/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-13     JasonHu      first version
 */

#define __ASSEMBLY__
#include "segment.h"

.global load_new_gdt
load_new_gdt:
    movl 4(%esp), %eax
    movw %ax, 6(%esp)
    lgdt 6(%esp)
    
    # flush segment registers
    movw $KERNEL_DATA_SEL, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    xor %eax, %eax
    movw %ax, %fs
    movw %ax, %gs
    ljmp $KERNEL_CODE_SEL, $.newpc
.newpc:
    ret

.global load_new_idt
load_new_idt:
    movl 4(%esp), %eax
    movw %ax, 6(%esp)
    lidt 6(%esp)
    ret

.global write_cr3
write_cr3:
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret

.global read_cr0
read_cr0:
    movl %cr0, %eax
    ret

.global read_cr2
read_cr2:
    movl %cr2, %eax
    ret
    
.global write_cr0
write_cr0:
    movl 4(%esp), %eax
    movl %eax, %cr0
    ret