/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-14     JasonHu      first version
 */

#ifndef __MMU_H__
#define __MMU_H__

#include <stddef.h>
#include <rtdef.h>
#include <rtconfig.h>

#undef PAGE_SIZE

#define ADDRESS_WIDTH_BITS 32
#define PHYSICAL_ADDRESS_WIDTH_BITS ADDRESS_WIDTH_BITS
#define ARCH_ADDRESS_WIDTH_BITS ADDRESS_WIDTH_BITS

#define __SIZE(bit) (1U << (bit))
#define __MASK(bit) (__SIZE(bit) - 1UL)
#define __UMASK(bit) (~(__MASK(bit)))
#define __MASKVALUE(value,maskvalue) ((value) & (maskvalue))
#define __UMASKVALUE(value,maskvalue) ((value) & (~(maskvalue)))
#define __CHECKUPBOUND(value,bit_count) (!(((rt_size_t)(value)) & (~__MASK(bit_count))))
#define __CHECKALIGN(value,start_bit) (!(((rt_size_t)(value)) & (__MASK(start_bit))))

#define __PARTBIT(value,start_bit,length) (((value) >> (start_bit)) & __MASK(length))

#define __ALIGNUP(value,bit) (((value) + __MASK(bit)) & __UMASK(bit))
#define __ALIGNDOWN(value,bit) ((value) & __UMASK(bit))

#define PAGE_OFFSET_SHIFT 0
#define PAGE_OFFSET_BIT 12
#define PAGE_SIZE __SIZE(PAGE_OFFSET_BIT)
#define PAGE_OFFSET_MASK __MASK(PAGE_OFFSET_BIT)
#define PAGE_ADDR_MASK __UMASK(PAGE_OFFSET_BIT)

#define PTE_SHIFT (PAGE_OFFSET_SHIFT + PAGE_OFFSET_BIT)
#define PTE_BIT 10
#define PDE_SHIFT (PTE_SHIFT + PTE_BIT)
#define PDE_BIT 10

#define mmu_flush_tlb() \
    do \
    { \
        unsigned long tmpreg; \
        __asm__ __volatile__ ( \
                    "movl   %%cr3,  %0  \n\t" \
                    "movl   %0, %%cr3   \n\t" \
                    :"=r"(tmpreg) \
                    : \
                    :"memory" \
                    ); \
    } \
    while(0)

#define ARCH_PAGE_SIZE PAGE_SIZE
#define ARCH_PAGE_MASK (ARCH_PAGE_SIZE - 1)
#define ARCH_PAGE_SHIFT PAGE_OFFSET_BIT

typedef struct
{
    rt_size_t *vtable;
    rt_size_t vstart;
    rt_size_t vend;
    rt_size_t pv_off;
}rt_mmu_info;

typedef rt_size_t pde_t; /* page dir entry */
typedef rt_size_t pte_t; /* page table entry */

/* page offset */
#define GET_PF_ID(addr) ((addr) >> PAGE_OFFSET_BIT)
#define GET_PF_OFFSET(addr) __MASKVALUE(addr,PAGE_OFFSET_MASK)
#define GET_L1(addr) __PARTBIT(addr,PDE_SHIFT,PDE_BIT)
#define GET_L2(addr) __PARTBIT(addr,PTE_SHIFT,PTE_BIT)
#define GET_PADDR(pte) ((pte) & PAGE_ADDR_MASK)
#define GET_PATTR(pte) ((pte) & PAGE_OFFSET_MASK)

#define PTE_PER_PAGE 1024

#define PAGE_TABLE_PADDR     0X3F3000
#define PAGE_TABLE_VADDR     (KERNEL_VADDR_START + PAGE_TABLE_PADDR)

#define MAKE_PTE(paddr, attr) (rt_size_t) (((rt_size_t)(paddr) & PAGE_ADDR_MASK) | ((attr) & PAGE_OFFSET_MASK))

// page table entry (PTE) fields
#define PTE_P     0x001 // Present
#define PTE_R     0x000 // Read
#define PTE_W     0x002 // Write
#define PTE_X     0x000 // Execute
#define PTE_U     0x004 // User
#define PTE_PWT   0x008 // Write-through
#define PTE_S     0x000 // System
#define PTE_A     0x020 // Accessed
#define PTE_D     0x040 // Dirty

#define PAGE_ATTR_RWX (PTE_X | PTE_W | PTE_R)
#define PAGE_ATTR_READONLY (PTE_R)
#define PAGE_ATTR_READEXECUTE (PTE_X | PTE_R)

#define PAGE_ATTR_USER (PTE_U)
#define PAGE_ATTR_SYSTEM (PTE_S)

#define KERNEL_PAGE_ATTR  (PTE_P | PAGE_ATTR_RWX | PAGE_ATTR_SYSTEM)

#define PTE_USED(pte) __MASKVALUE(pte,PTE_P)

#define MMU_MAP_K_RO          (PTE_S | PTE_R)
#define MMU_MAP_K_RWCB        (PTE_S | PTE_R | PTE_W)
#define MMU_MAP_K_RW          (PTE_S | PTE_R | PTE_W)
#define MMU_MAP_K_DEVICE      (PTE_S | PTE_R | PTE_W)
#define MMU_MAP_U_RO          (PTE_U | PTE_R)
#define MMU_MAP_U_RWCB        (PTE_U | PTE_R | PTE_W)
#define MMU_MAP_U_RW          (PTE_U | PTE_R | PTE_W)
#define MMU_MAP_U_DEVICE      (PTE_U | PTE_R | PTE_W)

#define PAGE_ATTR_MASK   PAGE_OFFSET_MASK

void mmu_set_pagetable(rt_ubase_t addr);
void mmu_enable_user_page_access();
void mmu_disable_user_page_access();
void mmu_enable();

void *rt_hw_mmu_tbl_get();
void rt_hw_mmu_switch(void *mmu_table);
int rt_hw_mmu_map_init(rt_mmu_info *mmu_info,void *v_address,rt_size_t size,rt_size_t *vtable,rt_size_t pv_off);
void rt_hw_mmu_kernel_map_init(rt_mmu_info *mmu_info,rt_size_t vaddr_start,rt_size_t size);

#ifdef RT_USING_USERSPACE
void *rt_hw_mmu_map(rt_mmu_info *mmu_info,void *v_addr,void *p_addr,rt_size_t size,rt_size_t attr);
void *rt_hw_mmu_map_auto(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size,rt_size_t attr);
#else
void *rt_hw_mmu_map(rt_mmu_info *mmu_info, void* p_addr, size_t size, size_t attr);
#endif  /* RT_USING_USERSPACE */

void rt_hw_mmu_unmap(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size);
void *rt_hw_mmu_v2p(rt_mmu_info *mmu_info,void *v_addr);

/* used in kernel mmaped area */
#define rt_hw_phy2vir(p) ((rt_ubase_t)(p) + KERNEL_VADDR_START)
#define rt_hw_vir2phy(v) ((rt_ubase_t)(v) - KERNEL_VADDR_START)

#endif
