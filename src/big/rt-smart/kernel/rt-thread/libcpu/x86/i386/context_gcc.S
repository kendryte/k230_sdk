/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021/07/14     JasonHu      First version
 */

#define __ASSEMBLY__
#include <rtconfig.h>

.code32
.text

/*
 * void rt_hw_context_switch_to_real(rt_ubase_t to);
 */
.globl rt_hw_context_switch_to_real
rt_hw_context_switch_to_real:
    movl 0x4(%esp), %eax    // get thread "to"
    movl (%eax), %esp       // restore sp
    
    popl %ebp
    popl %ebx
    popl %edi
    popl %esi
    ret

/*
 * void rt_hw_context_switch_real(rt_ubase_t from, rt_ubase_t to);
 */
.globl rt_hw_context_switch_real
rt_hw_context_switch_real:
    pushl %esi
    pushl %edi
    pushl %ebx
    pushl %ebp

    movl 0x14(%esp), %eax   // get "from"
    movl %esp, (%eax)       // save sp

    movl 0x18(%esp), %eax   // get "to"
    movl (%eax), %esp       // restore sp

    popl %ebp
    popl %ebx
    popl %edi
    popl %esi
    ret
