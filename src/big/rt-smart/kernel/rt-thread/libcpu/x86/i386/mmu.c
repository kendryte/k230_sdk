/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-14     JasonHu      first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <stdlib.h>
#include <string.h>
#include <rtdbg.h>

#include "mmu.h"
#include "cache.h"
#include "i386.h"

#ifdef RT_USING_USERSPACE
#include "page.h"
#endif /* RT_USING_USERSPACE */

// #define RT_DEBUG_MMU_X86

static void __rt_hw_mmu_unmap(rt_mmu_info *mmu_info,void *v_addr,rt_size_t npages);

#ifdef RT_USING_USERSPACE
void *_rt_hw_mmu_map(rt_mmu_info *mmu_info,void *v_addr,void *p_addr,rt_size_t size,rt_size_t attr);
void *_rt_hw_mmu_map_auto(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size,rt_size_t attr);
#else
void *rt_hw_mmu_map(rt_mmu_info *mmu_info, void* p_addr, size_t size, size_t attr);
#endif

void _rt_hw_mmu_unmap(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size);
void *_rt_hw_mmu_v2p(rt_mmu_info *mmu_info,void *v_addr);

void *current_mmu_table = RT_NULL;

static void rt_hw_cpu_tlb_invalidate()
{
    mmu_flush_tlb();
}

void *rt_hw_mmu_tbl_get()
{
    return current_mmu_table;
}

void rt_hw_mmu_switch(void *mmu_table)
{
    current_mmu_table = mmu_table;
    if (mmu_table == RT_NULL)
    {
        dbg_log(DBG_ERROR, "rt_hw_mmu_switch: NULL mmu table!\n");
    }
    else
    {
        RT_ASSERT(__CHECKALIGN(mmu_table,PAGE_OFFSET_BIT));
        mmu_set_pagetable((rt_ubase_t)mmu_table);
    }
}

/**
 * init page table, check vaddr whether used.
 */
int rt_hw_mmu_map_init(rt_mmu_info *mmu_info,void *v_address,rt_size_t size,rt_size_t *vtable,rt_size_t pv_off)
{
    size_t l1_off,va_s,va_e;
    rt_base_t level;

    if((!mmu_info) || (!vtable))
    {
        return -1;
    }

    va_s = (rt_size_t)v_address;
    va_e = ((rt_size_t)v_address) + size - 1;

    if(va_e < va_s)
    {
        dbg_log(DBG_ERROR, "end=%p lower than start=%p\n", va_e, va_s);
        return -1;
    }

    //convert address to level 1 page frame id
    va_s = GET_L1(va_s);
    va_e = GET_L1(va_e);

    if(va_s == 0)
    {
        return -1;
    }

    level = rt_hw_interrupt_disable();

    //vtable initialization check
    for(l1_off = va_s;l1_off <= va_e;l1_off++)
    {
        size_t v = vtable[l1_off];

        if(PTE_USED(v))
        {
            rt_hw_interrupt_enable(level);
            return -1;
        }
    }

    va_s = (rt_size_t)v_address;
    va_e = ((rt_size_t)v_address) + size;

    mmu_info -> vtable = vtable;
    mmu_info -> vstart = va_s;
    mmu_info -> vend = va_e;
    mmu_info -> pv_off = pv_off;

    rt_hw_interrupt_enable(level);
    return 0;
}

void rt_hw_mmu_kernel_map_init(rt_mmu_info *mmu_info,rt_size_t vaddr_start,rt_size_t size)
{
    vaddr_start = vaddr_start & PAGE_ADDR_MASK;
    rt_size_t paddr_start = vaddr_start;
    rt_size_t vaddr_end = vaddr_start + __ALIGNUP(size, PAGE_OFFSET_BIT);

    rt_kprintf("kernel: map on [%p~%p]\n", vaddr_start, vaddr_end);
    pde_t *pdt = (pde_t *)mmu_info->vtable;

    rt_size_t pde_nr = (vaddr_end - vaddr_start) / (PTE_PER_PAGE * PAGE_SIZE);
    rt_size_t pte_nr = ((vaddr_end - vaddr_start) / PAGE_SIZE) % PTE_PER_PAGE;
    rt_size_t *pte_addr = (rt_size_t *) PAGE_TABLE_VADDR;
    rt_size_t pde_off = GET_L1(vaddr_start);
    int i, j;
    for (i = 0; i < pde_nr; i++)
    {
        pdt[pde_off + i] = MAKE_PTE(pte_addr, KERNEL_PAGE_ATTR);
        for (j = 0; j < PTE_PER_PAGE; j++)
        {
            pte_addr[j] = MAKE_PTE(paddr_start, KERNEL_PAGE_ATTR);
            paddr_start += PAGE_SIZE;
        }
        pte_addr += PAGE_SIZE;
    }
    if (pte_nr > 0)
    {
        pdt[pde_off + i] = MAKE_PTE(pte_addr, KERNEL_PAGE_ATTR);
        for (j = 0; j < pte_nr; j++)
        {
            pte_addr[j] = MAKE_PTE(paddr_start, KERNEL_PAGE_ATTR);
            paddr_start += PAGE_SIZE;
        }
    }
}

static int __rt_hw_mmu_map(rt_mmu_info *mmu_info,void *v_addr,void *p_addr,rt_size_t npages,rt_size_t attr)
{
    size_t loop_va = (size_t)v_addr & ~ARCH_PAGE_MASK;
    size_t loop_pa = (size_t)p_addr & ~ARCH_PAGE_MASK;
    size_t l1_off, l2_off;
    size_t *mmu_l1, *mmu_l2;

    if (!mmu_info)
    {
        return -1;
    }
    while (npages--)
    {
        l1_off = GET_L1(loop_va);
        l2_off = GET_L2(loop_va);
        mmu_l1 =  (size_t*)mmu_info->vtable + l1_off;
        if(PTE_USED(*mmu_l1))
        {
            mmu_l2 = ((size_t *)GET_PADDR(*mmu_l1));
            rt_page_ref_inc(mmu_l2, 0); /* mmu l2 ref inc when map */
            mmu_l2 += l2_off;
        }
        else
        {
            mmu_l2 = (size_t*)rt_pages_alloc(0);
            if (mmu_l2)
            {
                rt_memset(mmu_l2, 0, ARCH_PAGE_SIZE);
                /* cache maintain */
                rt_hw_cpu_dcache_clean(mmu_l2, ARCH_PAGE_SIZE);

                *mmu_l1 = MAKE_PTE((size_t)mmu_l2, attr | PTE_P);
                /* cache maintain */
                rt_hw_cpu_dcache_clean(mmu_l1, sizeof(*mmu_l1));

                mmu_l2 += l2_off;
            }
            else
            {
                /* error, unmap and quit */
                __rt_hw_mmu_unmap(mmu_info, v_addr, npages);
                return -1;
            }
        }
        *mmu_l2 = MAKE_PTE(loop_pa, attr | PTE_P);
        /* cache maintain */
        rt_hw_cpu_dcache_clean(mmu_l2, sizeof(*mmu_l2));

        loop_va += ARCH_PAGE_SIZE;
        loop_pa += ARCH_PAGE_SIZE;
    }
    return 0;
}

#ifdef RT_USING_USERSPACE
//check whether the range of virtual address are free
static int check_vaddr(rt_mmu_info *mmu_info,void *va,rt_size_t pages)
{
    rt_size_t loop_va = __UMASKVALUE((rt_size_t)va,PAGE_OFFSET_MASK);
    rt_size_t l1_off, l2_off;
    rt_size_t *mmu_l1,*mmu_l2;

    if(!pages)
    {
        dbg_log(DBG_ERROR, "%s: check vaddr=%p pages=zero!\n", __func__, va);
        return -1;
    }

    if(!mmu_info)
    {
        dbg_log(DBG_ERROR, "%s: check vaddr=%p pages=%d mmu NULL!\n", __func__, va, pages);
        return -1;
    }

    while(pages--)
    {
        l1_off = GET_L1(loop_va);
        l2_off = GET_L2(loop_va);
        mmu_l1 = ((rt_size_t *)mmu_info -> vtable) + l1_off;

        if(PTE_USED(*mmu_l1))
        {
            mmu_l2 = ((rt_size_t *)GET_PADDR(*mmu_l1)) + l2_off;

            if(PTE_USED(*mmu_l2))
            {
                dbg_log(DBG_ERROR, "%s: check vaddr=%p pages=%d mmu l2 used %p->%x!\n", __func__, va, pages, mmu_l2, *mmu_l2);
                return -1;
            }
        }

        loop_va += PAGE_SIZE;
    }

    return 0;
}
#endif  /* RT_USING_USERSPACE */

//find a range of free virtual address specified by pages
static size_t find_vaddr(rt_mmu_info *mmu_info, int pages)
{
    size_t va;
    size_t find_va = 0;
    int n = 0;
    size_t start, end;

    if (!pages)
    {
        return 0;
    }

    if (!mmu_info)
    {
        return 0;
    }

    start = mmu_info->vstart;
    end = mmu_info->vend;
    va = mmu_info->vstart;
    for (; start < end; start += ARCH_PAGE_SIZE, va += ARCH_PAGE_SIZE)
    {
        if (_rt_hw_mmu_v2p(mmu_info, (void *)va))
        {
            n = 0;
            find_va = 0;
            continue;
        }
        if (!find_va)
        {
            find_va = va;
        }
        n++;
        if (n >= pages)
        {
            return find_va;
        }
    }
    return 0;
}

#ifdef RT_USING_USERSPACE
void *_rt_hw_mmu_map(rt_mmu_info *mmu_info,void *v_addr,void *p_addr,rt_size_t size,rt_size_t attr)
{
    rt_size_t pa_s,pa_e;
    rt_size_t vaddr;
    rt_size_t pages;
    int ret;

    if(!size)
    {
        return 0;
    }

    pa_s = (rt_size_t)p_addr;
    pa_e = ((rt_size_t)p_addr) + size - 1;
    pa_s = GET_PF_ID(pa_s);
    pa_e = GET_PF_ID(pa_e);
    pages = pa_e - pa_s + 1;
    if(v_addr)
    {
        vaddr = (rt_size_t)v_addr;
        pa_s = (rt_size_t)p_addr;
        if(GET_PF_OFFSET(vaddr) != GET_PF_OFFSET(pa_s))
        {
            return 0;
        }

        vaddr = __UMASKVALUE(vaddr,PAGE_OFFSET_MASK);

        if(check_vaddr(mmu_info,(void *)vaddr,pages) != 0)
        {
            dbg_log(DBG_ERROR, "%s: check vaddr=%p pages=%d failed!\n", __func__, vaddr, pages);
            return 0;
        }
    }
    else
    {
        vaddr = find_vaddr(mmu_info,pages);
    }

    if(vaddr)
    {
        ret = __rt_hw_mmu_map(mmu_info,(void *)vaddr,p_addr,pages,attr);

        if(ret == 0)
        {
            rt_hw_cpu_tlb_invalidate();
            return (void *)(vaddr | GET_PF_OFFSET((rt_size_t)p_addr));
        }
    }

    return 0;
}

#else
void *_rt_hw_mmu_map(rt_mmu_info *mmu_info, void* p_addr, size_t size, size_t attr)
{
    size_t pa_s, pa_e;
    size_t vaddr;
    int pages;
    int ret;

    pa_s = (size_t)p_addr;
    pa_e = (size_t)p_addr + size - 1;
    pa_s >>= ARCH_PAGE_SHIFT;
    pa_e >>= ARCH_PAGE_SHIFT;
    pages = pa_e - pa_s + 1;
    vaddr = find_vaddr(mmu_info, pages);
    if (vaddr) {
        ret = __rt_hw_mmu_map(mmu_info, (void*)vaddr, p_addr, pages, attr);
        if (ret == 0)
        {
            rt_hw_cpu_tlb_invalidate();
            return (void*)(vaddr + ((size_t)p_addr & ARCH_PAGE_MASK));
        }
    }
    return 0;
}
#endif  /* RT_USING_USERSPACE */

#ifdef RT_USING_USERSPACE
static int __rt_hw_mmu_map_auto(rt_mmu_info *mmu_info,void *v_addr,rt_size_t npages,rt_size_t attr)
{
    rt_size_t loop_va = __UMASKVALUE((rt_size_t)v_addr, PAGE_OFFSET_MASK);
    rt_size_t loop_pa;
    rt_size_t i;
    rt_size_t left_npages = npages;
    rt_size_t used_npages;
    void *va,*pa;

    if (!mmu_info)
    {
        return -1;
    }

    while (left_npages)
    {
        loop_pa = (rt_size_t)rt_pages_alloc(0);
        if (!loop_pa)
        {
            goto err;
        }
        rt_memset((void *)loop_pa, 0, ARCH_PAGE_SIZE);
        if (__rt_hw_mmu_map(mmu_info, (void *)loop_va, (void *)loop_pa, 1, attr) < 0)
        {
            rt_pages_free((void *)loop_pa, 0);  /* free unmaped phy page first */
            goto err;
        }
        --left_npages;
        loop_va += PAGE_SIZE;
    }
    return 0;
err:
    va = (void *)__UMASKVALUE((rt_size_t)v_addr, PAGE_OFFSET_MASK);
    used_npages = npages - left_npages;

    for (i = 0; i < used_npages; i++)
    {
        pa = rt_hw_mmu_v2p(mmu_info, va);
        if (pa)
        {
            rt_pages_free(pa, 0);
        }
        va = (void *)((rt_uint8_t *)va + PAGE_SIZE);
    }
    __rt_hw_mmu_unmap(mmu_info,v_addr, used_npages);
    return -1;
}

void *_rt_hw_mmu_map_auto(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size,rt_size_t attr)
{
    rt_size_t vaddr;
    rt_size_t offset;
    rt_size_t pages;
    int ret;

    if(!size)
    {
        return 0;
    }

    offset = GET_PF_OFFSET((rt_size_t)v_addr);
    size += (offset + ARCH_PAGE_SIZE - 1);
    pages = size >> PAGE_OFFSET_BIT;

    if(v_addr)
    {
        vaddr = __UMASKVALUE((rt_size_t)v_addr, PAGE_OFFSET_MASK);

        if(check_vaddr(mmu_info,(void *)vaddr, pages) != 0)
        {
            dbg_log(DBG_ERROR, "_rt_hw_mmu_map_auto: check vaddr %p on pages %d failed!\n", vaddr, pages);
            return 0;
        }
    }
    else
    {
        vaddr = find_vaddr(mmu_info,pages);
    }

    if(vaddr)
    {
        ret = __rt_hw_mmu_map_auto(mmu_info, (void *)vaddr, pages, attr);

        if(ret == 0)
        {
            rt_hw_cpu_tlb_invalidate();
            return (void *)(vaddr | offset);
        }
        dbg_log(DBG_ERROR, "_rt_hw_mmu_map_auto: do __rt_hw_mmu_map_auto failed!\n");
    }
    else
    {
        dbg_log(DBG_ERROR, "_rt_hw_mmu_map_auto: get vaddr failed!\n");
    }
    return 0;
}
#endif  /* RT_USING_USERSPACE */

/**
 * unmap page on v_addr, free page if unmapped, further more, if page table empty, need free it.
 */
static void __rt_hw_mmu_unmap(rt_mmu_info *mmu_info,void *v_addr,rt_size_t npages)
{
    rt_size_t loop_va = __UMASKVALUE((rt_size_t)v_addr, PAGE_OFFSET_MASK);
    rt_size_t l1_off, l2_off;
    rt_size_t *mmu_l1, *mmu_l2;

    RT_ASSERT(mmu_info);

    if ((rt_size_t)v_addr < mmu_info->vstart || (rt_size_t)v_addr >= mmu_info -> vend)
    {
        dbg_log(DBG_ERROR, "unmap vaddr %p out of range [%p~%p)\n", v_addr, mmu_info->vstart, mmu_info->vend);
        return;
    }

    while(npages--)
    {
        l1_off = (rt_size_t)GET_L1(loop_va);
        l2_off = (rt_size_t)GET_L2(loop_va);
        mmu_l1 = ((rt_size_t *)mmu_info -> vtable) + l1_off;
        if (!PTE_USED(*mmu_l1))
        {
            dbg_log(DBG_ERROR, "unmap vaddr %p mmu l1 unused %p->%x\n", v_addr, mmu_l1, *mmu_l1);
        }
        RT_ASSERT(PTE_USED(*mmu_l1))
        mmu_l2 = (rt_size_t *)(GET_PADDR(*mmu_l1)) + l2_off;
        if (!PTE_USED(*mmu_l2))
        {
            dbg_log(DBG_ERROR, "unmap vaddr %p mmu l2 unused %p->%x\n", v_addr, mmu_l2, *mmu_l2);
        }
        RT_ASSERT(PTE_USED(*mmu_l2));
        *mmu_l2 = 0;    /* clear page table entry */
        rt_hw_cpu_dcache_clean(mmu_l2, sizeof(*mmu_l2));
        mmu_l2 -= l2_off;   /* get base addr on page aligned */

        if(rt_pages_free(mmu_l2, 0)) /* page table no phy page, empty */
        {
            *mmu_l1 = 0;    /* clear page dir table entry */
            rt_hw_cpu_dcache_clean(mmu_l1, sizeof(*mmu_l1));
        }
        
        loop_va += PAGE_SIZE;
    }
}

void _rt_hw_mmu_unmap(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size)
{
    rt_size_t va_s,va_e;
    rt_size_t pages;

    va_s = ((rt_size_t)v_addr) >> PAGE_OFFSET_BIT;
    va_e = (((rt_size_t)v_addr) + size - 1) >> PAGE_OFFSET_BIT;
    pages = va_e - va_s + 1;
    __rt_hw_mmu_unmap(mmu_info,v_addr,pages);
    rt_hw_cpu_tlb_invalidate();
}

#ifdef RT_USING_USERSPACE
/**
 * map vaddr in vtable with size and attr, this need a phy addr
 *
 * if v_addr == RT_NULL, get a valid vaddr to map.
 *
 * success return start vaddr, failed return RT_NULL
 */
void *rt_hw_mmu_map(rt_mmu_info *mmu_info,void *v_addr,void *p_addr,rt_size_t size,rt_size_t attr)
{
    void *ret;
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    ret = _rt_hw_mmu_map(mmu_info,v_addr,p_addr,size,attr);
    rt_hw_interrupt_enable(level);
    return ret;
}

/**
 * map vaddr in vtable with size and attr, this will auto alloc phy addr
 *
 * if v_addr == RT_NULL, get a valid vaddr to map.
 *
 * success return start vaddr, failed return RT_NULL
 */
void *rt_hw_mmu_map_auto(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size,rt_size_t attr)
{
    void *ret;
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    ret = _rt_hw_mmu_map_auto(mmu_info,v_addr,size,attr);
    rt_hw_interrupt_enable(level);
    return ret;
}
#else
/**
 * map vaddr in vtable with size and attr, this need a phy addr
 *
 * success return start vaddr, failed return RT_NULL
 */
void *rt_hw_mmu_map(rt_mmu_info *mmu_info, void* p_addr, size_t size, size_t attr)
{
    void *ret;
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    ret = _rt_hw_mmu_map(mmu_info, p_addr, size, attr);
    rt_hw_interrupt_enable(level);
    return ret;
}
#endif

/**
 * unmap vaddr in vtable, free phyaddr and page table
 */
void rt_hw_mmu_unmap(rt_mmu_info *mmu_info,void *v_addr,rt_size_t size)
{
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    _rt_hw_mmu_unmap(mmu_info,v_addr,size);
    rt_hw_interrupt_enable(level);
}

void *_rt_hw_mmu_v2p(rt_mmu_info *mmu_info, void *v_addr)
{
    size_t l1 = GET_L1((size_t)v_addr);
    pde_t *pde = &mmu_info->vtable[l1];
    if (*pde & PTE_P)
    {
        size_t *pte_addr = (size_t *)GET_PADDR(*pde);
        size_t l2 = GET_L2((size_t)v_addr);
        pte_t *pte = (pte_t *)&pte_addr[l2];
        if (*pte & PTE_P)
        {
            return (void *)(GET_PADDR(*pte) | GET_PF_OFFSET((rt_size_t)v_addr));
        }
    }
    return RT_NULL;
}

#ifdef RT_DEBUG_MMU_X86
void *_rt_hw_mmu_v2p_with_dbg(rt_mmu_info *mmu_info, void *v_addr)
{
    rt_kprintf("v2p: mmu vtable=%p, vaddr=%p\n", mmu_info->vtable, v_addr);
    size_t l1 = GET_L1((size_t)v_addr);
    rt_kprintf("=>L1=%d ", l1);
    pde_t *pde = &mmu_info->vtable[l1];
    rt_kprintf("pde=>%p:%x (%x|%x)\n", pde, *pde, GET_PADDR(*pde), GET_PATTR(*pde));
    if (*pde & PTE_P)
    {
        size_t *pte_addr = (size_t *)GET_PADDR(*pde);
        size_t l2 = GET_L2((size_t)v_addr);
        rt_kprintf("  =>L2=%d ", l2);
        pte_t *pte = (pte_t *)&pte_addr[l2];
        rt_kprintf("pte=>%p:%x (%x|%x)\n", pte, *pte, GET_PADDR(*pte), GET_PATTR(*pte));
        if (*pte & PTE_P)
        {
            rt_kprintf("    =>paddr:%p\n", GET_PADDR(*pte));
            return (void *)GET_PADDR(*pte);
        }
    }
    rt_kprintf("v2p: mmu v2p %p failed!\n", v_addr);
    return RT_NULL;
}
#endif

/**
 * virtual addr to physical addr
 *
 * success return phyaddr, failed return RT_NULL
 */
void *rt_hw_mmu_v2p(rt_mmu_info *mmu_info,void *v_addr)
{
    void *ret;
    rt_base_t level;

    level = rt_hw_interrupt_disable();
#ifdef RT_DEBUG_MMU_X86
    ret = _rt_hw_mmu_v2p_with_dbg(mmu_info,v_addr);
#else
    ret = _rt_hw_mmu_v2p(mmu_info,v_addr);
#endif
    rt_hw_interrupt_enable(level);
    return ret;
}

void mmu_set_pagetable(rt_ubase_t addr)
{
    /* set new pgdir will flush tlb */
    write_cr3(addr);
}

void mmu_enable_user_page_access()
{
}

void mmu_disable_user_page_access()
{
}

void mmu_enable()
{
    write_cr0(read_cr0() | CR0_PG);
}
