/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021/07/15     JasonHu      The first version
 */
#define __ASSEMBLY__
#include <rtconfig.h>

.code32
.text

.extern rt_interrupt_enter
.extern rt_interrupt_leave
.extern rt_thread_switch_interrupt_flag
.extern rt_interrupt_from_thread
.extern rt_interrupt_to_thread
.extern rt_hw_interrupt_dispatch
.extern rt_hw_context_switch

// cpu no error code, need push by us
.macro rt_hw_intr_entry_push_errcode p1
.global rt_hw_intr_entry\p1
rt_hw_intr_entry\p1:
    pushl $0x00
    
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    
    pushal

    movl %ss, %edx
    movl %edx, %ds
    movl %edx, %es

    pushl $\p1

    call rt_interrupt_enter
    pushl %esp
    call rt_hw_interrupt_dispatch
    addl $4, %esp
    call rt_interrupt_leave

#ifdef RT_USING_SIGNALS  
    // check signal and do signal
    pushl %esp
    call lwp_try_do_signal
    addl $4, %esp
#endif /* RT_USING_SIGNALS */

    /**
     * if rt_thread_switch_interrupt_flag == 1 then 
     *     jmp rt_hw_intr_thread_switch
     * end
     */
    movl $rt_thread_switch_interrupt_flag, %eax
    movl (%eax), %ebx
    cmp $0x1, %ebx
    jz rt_hw_intr_thread_switch

    // jmp to exit
    movl $rt_hw_intr_exit, %eax
    jmp *%eax
.endm

// cpu with error code
.macro rt_hw_intr_entry p1
.global rt_hw_intr_entry\p1
rt_hw_intr_entry\p1:
    nop;
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    pushal

    movl %ss, %edx
    movl %edx, %ds
    movl %edx, %es

    pushl $\p1

    call rt_interrupt_enter
    pushl %esp;
    call rt_hw_interrupt_dispatch
    addl $4, %esp;
    call rt_interrupt_leave
    
#ifdef RT_USING_SIGNALS
    // check signal and do signal
    pushl %esp
    call lwp_try_do_signal
    addl $4, %esp
#endif /* RT_USING_SIGNALS */

    /**
     * if rt_thread_switch_interrupt_flag == 1 then 
     *     jmp rt_hw_intr_thread_switch
     * end
     */
    movl $rt_thread_switch_interrupt_flag, %eax
    movl (%eax), %ebx
    cmp $0x1, %ebx
    jz rt_hw_intr_thread_switch

    // jmp to exit
    movl $rt_hw_intr_exit, %eax
    jmp *%eax
.endm

rt_hw_intr_entry_push_errcode 0x00
rt_hw_intr_entry_push_errcode 0x01
rt_hw_intr_entry_push_errcode 0x02
rt_hw_intr_entry_push_errcode 0x03 
rt_hw_intr_entry_push_errcode 0x04
rt_hw_intr_entry_push_errcode 0x05
rt_hw_intr_entry_push_errcode 0x06
rt_hw_intr_entry_push_errcode 0x07 
rt_hw_intr_entry 0x08
rt_hw_intr_entry_push_errcode 0x09
rt_hw_intr_entry 0x0a
rt_hw_intr_entry 0x0b 
rt_hw_intr_entry_push_errcode 0x0c
rt_hw_intr_entry 0x0d
rt_hw_intr_entry 0x0e
rt_hw_intr_entry_push_errcode 0x0f 
rt_hw_intr_entry_push_errcode 0x10
rt_hw_intr_entry 0x11
rt_hw_intr_entry_push_errcode 0x12
rt_hw_intr_entry_push_errcode 0x13 
rt_hw_intr_entry_push_errcode 0x14
rt_hw_intr_entry_push_errcode 0x15
rt_hw_intr_entry_push_errcode 0x16
rt_hw_intr_entry_push_errcode 0x17 
rt_hw_intr_entry 0x18
rt_hw_intr_entry_push_errcode 0x19
rt_hw_intr_entry 0x1a
rt_hw_intr_entry 0x1b 
rt_hw_intr_entry_push_errcode 0x1c
rt_hw_intr_entry 0x1d
rt_hw_intr_entry 0x1e
rt_hw_intr_entry_push_errcode 0x1f 
rt_hw_intr_entry_push_errcode 0x20
rt_hw_intr_entry_push_errcode 0x21
rt_hw_intr_entry_push_errcode 0x22
rt_hw_intr_entry_push_errcode 0x23
rt_hw_intr_entry_push_errcode 0x24
rt_hw_intr_entry_push_errcode 0x25
rt_hw_intr_entry_push_errcode 0x26
rt_hw_intr_entry_push_errcode 0x27
rt_hw_intr_entry_push_errcode 0x28
rt_hw_intr_entry_push_errcode 0x29
rt_hw_intr_entry_push_errcode 0x2a
rt_hw_intr_entry_push_errcode 0x2b
rt_hw_intr_entry_push_errcode 0x2c
rt_hw_intr_entry_push_errcode 0x2d
rt_hw_intr_entry_push_errcode 0x2e
rt_hw_intr_entry_push_errcode 0x2f
rt_hw_intr_entry_push_errcode 0x80    // syscall

rt_hw_intr_thread_switch:
    // set rt_thread_switch_interrupt_flag as 0
    movl $0x0, %ebx
    movl %ebx, (%eax)

    // push to into stack
    movl $rt_interrupt_to_thread, %eax   // get "to"
    movl (%eax), %ebx
    
    // push from into stack
    movl $rt_interrupt_from_thread, %ecx   // get "from"
    movl (%ecx), %edx

    pushl %ebx
    pushl %edx
    call rt_hw_context_switch
    addl $8, %esp   // restore stack

    // jmp to exit
    movl $rt_hw_intr_exit, %eax
    jmp *%eax

#ifdef RT_USING_USERSPACE

.extern rt_hw_syscall_dispath

#ifdef RT_USING_SIGNALS
.extern lwp_try_do_signal
#endif /* RT_USING_SIGNALS */

.global hw_syscall_entry
hw_syscall_entry:
    pushl $0x00
    
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    
    pushal

    movl %ss, %edx
    movl %edx, %ds
    movl %edx, %es

    pushl $0x80

    sti // enable interrupt

    pushl %esp
    call rt_hw_syscall_dispath
    addl $4, %esp

#ifdef RT_USING_SIGNALS
    // check signal and do signal
    pushl %esp
    call lwp_try_do_signal
    addl $4, %esp
#endif /* RT_USING_SIGNALS */

    cli // disable interrupt

    // jmp to exit
    movl $rt_hw_intr_exit, %eax
    jmp *%eax

.global arch_syscall_exit
arch_syscall_exit:
#endif /* RT_USING_USERSPACE */
.global rt_hw_intr_exit
rt_hw_intr_exit:
    addl $4, %esp               // skip intr no

    popal
    
    popl %gs
    popl %fs
    popl %es
    popl %ds

    addl $4, %esp               // skip error_code

    iret