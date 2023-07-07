/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2021-07-16     JasonHu,GuEe-GUI  first version
 */

#define __ASSEMBLY__
#include "multiboot2.h"

# the size of stack is 16KB
#define STACK_SIZE 0x4000

#define KSTACK_TOP_PHY 0x9f000

.code32

.extern rt_boot_setup_entry, primary_cpu_entry

.section .init
.globl start, _start

start:
_start:
    jmp multiboot_entry

.align 8
multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC                  # magic number (multiboot 2)
    .long MULTIBOOT_ARCHITECTURE_I386              # architecture 0 (protected mode i386)
    .long multiboot_header_end - multiboot_header  # header length
    # checksum
    .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))
    # insert optional multiboot tags here

    # required end tag
    .align 8
    .short MULTIBOOT_HEADER_TAG_END # type
    .short 0                        # flags
    .long  8                        # size
multiboot_header_end:
multiboot_entry:
    # initialize the stack pointer
    movl $(stack + STACK_SIZE), %esp

    # reset EFLAGS
    pushl $0
    popf

    # push the pointer to the Multiboot information structure
    pushl %ebx
    # push the magic value
    pushl %eax

    # jump to rt_boot_setup_entry
    call rt_boot_setup_entry

    # jump to setup_fail if rt_boot_setup_entry return -1
    popl %eax
    cmpl $-1, %eax
    je setup_fail
    
    # set kernel stack top
    movl $KSTACK_TOP_PHY, %esp

    # jump to kernel_start
    movl $primary_cpu_entry, %eax
    jmp *%eax

setup_fail:
    # print "Error!" in protected mode
    movl $0xcf72cf45, 0xb8000
    movl $0xcf6fcf72, 0xb8004
    movl $0xcf21cf72, 0xb8008

multiboot_hlt:
    hlt
    jmp multiboot_hlt

    .comm stack, STACK_SIZE