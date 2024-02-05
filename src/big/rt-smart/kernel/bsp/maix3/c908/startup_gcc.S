/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/10/01     Bernard      The first version
 * 2018/12/27     Jesven       Add SMP support
 * 2020/6/12      Xim          Port to QEMU and remove SMP support
 */

#define __ASSEMBLY__
#define SSTATUS_FS      0x00006000U /* initial state of FPU, clear to disable */
/*
 * CSRs
 */
#define mxstatus    0x7c0
#define mhcr        0x7c1
#define mcor        0x7c2
#define mhint       0x7c5
#define msmpr       0x7f3
#define mccr2       0x7c3

#define STARTUP_USE_SBI

#include <cpuport.h>

  .global	_start
  .section ".start", "ax"
_start:
  j 1f
  .word 0xdeadbeef
  .align 3
  .global g_wake_up
  g_wake_up:
      .dword 1
      .dword 0
1:
#ifndef STARTUP_USE_SBI
  csrw mie, 0
  csrw mip, 0

  /* plic PLIC_CTRL:S_PER bit */
  fence w,o
  li t0, 0xf001ffffc
  li t1, 1
  sw t1, 0(t0)

#if 0
  /* cfg pmp, value from sbi */
  /* 0x00000000 ~ 0x80000000 DDR, NAPOT rwx
  set $pmpaddr0 = 0x0 >> 2 | ((0x80000000 - 1) >> 3) */
  li t0, 0xFFFFFFF
  csrw pmpaddr0, t0

  /*
  # 0xf 00000000 ~ 0xf 08000000 plic, NAPOT rw
  set $pmpaddr1 = 0xf00000000 >> 2 | ((0x8000000 - 1) >> 3)
  */
  li t0, ((0xf00000000 >> 2) | ((0x8000000 - 1) >> 3))
  csrw pmpaddr1, t0

  /*
  # 0x80000000 ~ 0x100000000 peripherals, NAPOT rw
  set $pmpaddr2 = 0x80000000 >> 2 | ((0x80000000 - 1) >> 3)
  */
  li t0, ((0x80000000 >> 2) | ((0x80000000 - 1) >> 3))
  csrw pmpaddr2, t0

  li t0, 0x9b9b9f
  csrw pmpcfg0, t0
#else
  li t0, 0xfffffff
  csrw pmpaddr0, t0

  li t0, 0x200bffff
  csrw pmpaddr1, t0

  li t0, 0x2fffffff
  csrw pmpaddr2, t0

  li t0, 0x3c1ffffff
  csrw pmpaddr3, t0

  li t0, 0x1b1b1f1f
  csrw pmpcfg0, t0
#endif
  /* enable fp, prev-mode s-mode */
  csrr t0, mstatus
  li t1, (~(3 << 11))
  and t0, t0, t1
  li t1, (~(1 << 13))
  and t0, t0, t1
  li t1, (~(1 << 7))
  and t0, t0, t1
  li t1, ((1 << 11) | (1 << 13) | (1 << 17))
  or t0, t0, t1
  csrw mstatus, t0

  /* value from sbi */
  li t0, 0x638000
  csrw mxstatus, t0

  /* s-mode int&except enable*/
  li t0, 0x222
  csrw mideleg, t0
  li t0, 0xb1ff
  csrw medeleg, t0

  /* cache... */
  li t0, 0x11ff
  csrw mhcr, t0

  li t0, 0x70013
  csrw mcor, t0

  li t0, 0xe0410009
  csrw mccr2, t0

  li t0, 0x16e30c
  csrw mhint, t0
  li t0, 0x1
  csrw msmpr, t0

  /* enable dcache.call */
  csrr t0, 0x7d9
  ori t0, t0, 1
  csrw 0x7d9, t0

  /* mtime s-mode enable */
  li t0, 0xffffffff;
  csrw mcounteren, t0

  la t0, 2f
  csrw mepc, t0
  csrw stvec, t0
  csrw sscratch, 0
  csrw sie, 0
  csrw satp, 0

  mret

2:
#endif
  csrw sie, 0
  csrw sip, 0
  la t0, trap_entry
  csrw stvec, t0

  li x1, 0
  li x2, 0
  li x3, 0
  li x4, 0
  li x5, 0
  li x6, 0
  li x7, 0
  li x8, 0
  li x9, 0
  li x10,0
  li x11,0
  li x12,0
  li x13,0
  li x14,0
  li x15,0
  li x16,0
  li x17,0
  li x18,0
  li x19,0
  li x20,0
  li x21,0
  li x22,0
  li x23,0
  li x24,0
  li x25,0
  li x26,0
  li x27,0
  li x28,0
  li x29,0
  li x30,0
  li x31,0

  /* set to disable FPU */
  li t0, SSTATUS_FS
  csrc sstatus, t0
  li t0, 0x40000 // SUM in sstatus
  csrs sstatus, t0

.option push
.option norelax
  la gp, __global_pointer$
.option pop

  // removed SMP support here
  la   sp, __stack_start__
  li   t0, __STACKSIZE__
  add  sp, sp, t0
  csrw sscratch, sp
  call primary_cpu_entry
