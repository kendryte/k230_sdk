/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/10/28     Bernard      The unify RISC-V porting implementation
 * 2018/12/27     Jesven       Add SMP support
 * 2021/02/02     lizhirui     Add userspace support
 */

#define __ASSEMBLY__
#include "cpuport.h"
#include "stackframe.h"

    .globl rt_hw_context_switch_to
rt_hw_context_switch_to:
    LOAD sp, (a0)

    la s0, rt_current_thread
    LOAD s1, (s0)

    #ifdef RT_USING_USERSPACE
        mv a0, s1
        call lwp_mmu_switch
    #endif

    RESTORE_ALL
    sret

/*
 * void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to);
 *
 * a0 --> from SP pointer
 * a1 --> to SP pointer
 */
    .globl rt_hw_context_switch
rt_hw_context_switch:
    // backup prev stack top of from thread
    csrw sscratch, sp

    // on sret, reset status to
    // 1. interrupt DISABLE
    // 2. cpu in supervisor mode
    // 3. reserved other status
    li t0, 0x120
    csrs sstatus, t0

    // simulate a exception return to supervisor mode
    // by storing ra in sepc of from-thread stack frame
    csrw sepc, ra

    // NOTE: we do NOT guarantee that any temporary registers
    // will remains their value after SAVE_ALL
    SAVE_ALL

    STORE sp, (a0)

    // restore to thread context
    LOAD sp, (a1)

    la s0, rt_current_thread
    LOAD s1, (s0)

    #ifdef RT_USING_USERSPACE
        mv a0, s1
        call lwp_mmu_switch
    #endif

    LOAD t0, 2 * REGBYTES(sp)
    andi t0, t0, 0x100
    bnez t0, rt_hw_context_switch_exit
    call arch_ret_to_user

rt_hw_context_switch_exit:
    RESTORE_ALL
    sret

#ifdef ENABLE_VECTOR
/**
 * @param a0 pointer to frame bottom
 */
.global rt_hw_vector_ctx_save
rt_hw_vector_ctx_save:
    SAVE_VECTOR a0
    ret

/**
 * @param a0 pointer to frame bottom
 */
.global rt_hw_vector_ctx_restore
rt_hw_vector_ctx_restore:
    RESTORE_VECTOR a0
    ret

.global rt_hw_disable_vector
rt_hw_disable_vector:
    li t0, SSTATUS_VS
    csrc sstatus, t0
    ret

.global rt_hw_enable_vector
rt_hw_enable_vector:
    li t0, SSTATUS_VS
    csrs sstatus, t0
    ret
#endif /* ENABLE_VECTOR */
