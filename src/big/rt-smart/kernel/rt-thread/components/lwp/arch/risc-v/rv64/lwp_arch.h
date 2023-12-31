/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#ifndef  LWP_ARCH_H__
#define  LWP_ARCH_H__

#include <lwp.h>
#include <lwp_arch_comm.h>
#include <riscv_mmu.h>

#ifdef RT_USING_USERSPACE

#ifdef RT_USING_USERSPACE_32BIT_LIMIT
#define USER_HEAP_VADDR     0xF0000000UL
#define USER_HEAP_VEND      0xFE000000UL
#define USER_STACK_VSTART   0xE0000000UL
#define USER_STACK_VEND     USER_HEAP_VADDR
#define USER_VADDR_START    0xC0000000UL
#define USER_VADDR_TOP      0xFF000000UL
#define USER_LOAD_VADDR     0xD0000000UL
#define LDSO_LOAD_VADDR     USER_LOAD_VADDR
#else
#define USER_HEAP_VADDR     0x300000000UL
#define USER_HEAP_VEND      0xffffffffffff0000UL
#define USER_STACK_VSTART   0x270000000UL
#define USER_STACK_VEND     USER_HEAP_VADDR
#define USER_VADDR_START    0x100000000UL
#define USER_VADDR_TOP      0xfffffffffffff000UL
#define USER_LOAD_VADDR     0x200000000
#define LDSO_LOAD_VADDR     0x200000000
#endif

/* this attribution is cpu specified, and it should be defined in riscv_mmu.h */
#ifndef MMU_MAP_U_RWCB
#define MMU_MAP_U_RWCB 0
#endif

#ifndef MMU_MAP_U_RW
#define MMU_MAP_U_RW 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

rt_mmu_info* arch_kernel_get_mmu_info(void);

rt_inline unsigned long rt_hw_ffz(unsigned long x)
{
    return __builtin_ffsl(~x) - 1;
}

rt_inline void icache_invalid_all(void)
{
    //TODO:
}

#ifdef __cplusplus
}
#endif

#endif

#endif  /*LWP_ARCH_H__*/
