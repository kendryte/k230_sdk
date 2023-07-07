/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-06     JasonHu      first version
 */

#ifndef __X86_SEGMENT_H__
#define __X86_SEGMENT_H__

#include <rtconfig.h>

/* DA: Descriptor Attribute */
#define DA_32           0x4000  /* 32 bits segment */
#define DA_G            0x8000  /* segment limit is 4KB */
#define DA_DPL0         0x00    /* DPL = 0 */
#define DA_DPL1         0x20    /* DPL = 1 */
#define DA_DPL2         0x40    /* DPL = 2 */
#define DA_DPL3         0x60    /* DPL = 3 */
#define DA_DR           0x90    /* readonly data */
#define DA_DRW          0x92    /* read/write data */
#define DA_DRWA         0x93    /* accessed read/write data  */
#define DA_C            0x98    /* code */
#define DA_CR           0x9A    /* readable code */
#define DA_CCO          0x9C    /* only execute consistent code segment */
#define DA_CCOR         0x9E    /* executable readable and consistent code segment */
#define DA_LDT          0x82    /* local descriptor table */
#define DA_386TSS       0x89    /* 386 TSS */

/* SA : Selector Attribute */
#define SA_RPL0     0
#define SA_RPL1     1
#define SA_RPL2     2
#define SA_RPL3     3

#define SA_TIG      0   /* selector in GDT */
#define SA_TIL      1   /* selector in IDT */

/* index of descriptor */
#define INDEX_DUMMY 0
#define INDEX_KERNEL_CODE 1
#define INDEX_KERNEL_DATA 2
#define INDEX_TSS 3
#define INDEX_USER_CODE 4
#define INDEX_USER_DATA 5
#define INDEX_USER_TLS 6

#define KERNEL_CODE_SEL ((INDEX_KERNEL_CODE << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_DATA_SEL ((INDEX_KERNEL_DATA << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_STACK_SEL KERNEL_DATA_SEL

#define KERNEL_TSS_SEL ((INDEX_TSS << 3) + (SA_TIG << 2) + SA_RPL0)

#define USER_CODE_SEL ((INDEX_USER_CODE << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_DATA_SEL ((INDEX_USER_DATA << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_STACK_SEL USER_DATA_SEL

#define USER_TLS_SEL ((INDEX_USER_TLS << 3) + (SA_TIG << 2) + SA_RPL3)

#define GDT_LIMIT           0x000007ff
#define GDT_PADDR           0x003F0000

#define GDT_VADDR           (KERNEL_VADDR_START + GDT_PADDR)

#define GDT_OFF2PTR(gdt, off)    (gdt + off)

#define GDT_BOUND_BOTTOM   0
#define GDT_BOUND_TOP      0xffffffff

#define GDT_KERNEL_CODE_ATTR        (DA_CR | DA_DPL0 | DA_32 | DA_G)
#define GDT_KERNEL_DATA_ATTR        (DA_DRW | DA_DPL0 | DA_32 | DA_G)
#define GDT_USER_CODE_ATTR          (DA_CR | DA_DPL3 | DA_32 | DA_G)
#define GDT_USER_DATA_ATTR          (DA_DRW | DA_DPL3 | DA_32 | DA_G)
#define GDT_TSS_ATTR                (DA_386TSS)
#define GDT_USER_TLS_ATTR           (DA_DR | DA_DPL3 | DA_32 | DA_G)  /* read only data seg */

#ifndef __ASSEMBLY__

#include <rtdef.h>

void rt_hw_segment_init(void);

void rt_hw_seg_tls_set(rt_ubase_t base);
rt_ubase_t rt_hw_seg_tls_get();

#endif

#endif  /*__X86_SEGMENT_H__*/
