/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <asm/cache.h>
#include <dm/uclass-internal.h>
#include <cache.h>
#include <asm/csr.h>

#ifndef __ASM
#define __ASM                   __asm     /*!< asm keyword for GNU Compiler */
#endif

#ifndef __INLINE
#define __INLINE                inline    /*!< inline keyword for GNU Compiler */
#endif

#ifndef __ALWAYS_STATIC_INLINE
#define __ALWAYS_STATIC_INLINE  __attribute__((always_inline)) static inline
#endif

#ifndef __STATIC_INLINE
#define __STATIC_INLINE         static inline
#endif

#ifndef __ICACHE_PRESENT
#define __ICACHE_PRESENT          1U
#endif

#ifndef __DCACHE_PRESENT
#define __DCACHE_PRESENT          1U
#endif

#ifndef __L2CACHE_PRESENT
#define __L2CACHE_PRESENT         1U
#endif

/* CACHE Register Definitions */
#define CACHE_MHCR_WBR_Pos                     8U                                            /*!< CACHE MHCR: WBR Position */
#define CACHE_MHCR_WBR_Msk                     (0x1UL << CACHE_MHCR_WBR_Pos)                 /*!< CACHE MHCR: WBR Mask */

#define CACHE_MHCR_IBPE_Pos                    7U                                            /*!< CACHE MHCR: IBPE Position */
#define CACHE_MHCR_IBPE_Msk                    (0x1UL << CACHE_MHCR_IBPE_Pos)                /*!< CACHE MHCR: IBPE Mask */

#define CACHE_MHCR_L0BTB_Pos                   6U                                            /*!< CACHE MHCR: L0BTB Position */
#define CACHE_MHCR_L0BTB_Msk                   (0x1UL << CACHE_MHCR_L0BTB_Pos)               /*!< CACHE MHCR: BTB Mask */

#define CACHE_MHCR_BPE_Pos                     5U                                            /*!< CACHE MHCR: BPE Position */
#define CACHE_MHCR_BPE_Msk                     (0x1UL << CACHE_MHCR_BPE_Pos)                 /*!< CACHE MHCR: BPE Mask */

#define CACHE_MHCR_RS_Pos                      4U                                            /*!< CACHE MHCR: RS Position */
#define CACHE_MHCR_RS_Msk                      (0x1UL << CACHE_MHCR_RS_Pos)                  /*!< CACHE MHCR: RS Mask */

#define CACHE_MHCR_WB_Pos                      3U                                            /*!< CACHE MHCR: WB Position */
#define CACHE_MHCR_WB_Msk                      (0x1UL << CACHE_MHCR_WB_Pos)                  /*!< CACHE MHCR: WB Mask */

#define CACHE_MHCR_WA_Pos                      2U                                            /*!< CACHE MHCR: WA Position */
#define CACHE_MHCR_WA_Msk                      (0x1UL << CACHE_MHCR_WA_Pos)                  /*!< CACHE MHCR: WA Mask */

#define CACHE_MHCR_DE_Pos                      1U                                            /*!< CACHE MHCR: DE Position */
#define CACHE_MHCR_DE_Msk                      (0x1UL << CACHE_MHCR_DE_Pos)                  /*!< CACHE MHCR: DE Mask */

#define CACHE_MHCR_IE_Pos                      0U                                            /*!< CACHE MHCR: IE Position */
#define CACHE_MHCR_IE_Msk                      (0x1UL << CACHE_MHCR_IE_Pos)                  /*!< CACHE MHCR: IE Mask */

#define CACHE_INV_ADDR_Pos                     5U
#define CACHE_INV_ADDR_Msk                     (0xFFFFFFFFUL << CACHE_INV_ADDR_Pos)

__ALWAYS_STATIC_INLINE uint64_t __get_MHCR(void)
{
    uint64_t result;

    __ASM volatile("csrr %0, mhcr" : "=r"(result));
    return (result);
}

__ALWAYS_STATIC_INLINE void __set_MHCR(uint64_t mhcr)
{
    __ASM volatile("csrw mhcr, %0" : : "r"(mhcr));
}
/**
  \brief   Instruction Synchronization Barrier
  \details Instruction Synchronization Barrier flushes the pipeline in the processor,
           so that all instructions following the ISB are fetched from cache or memory,
           after the instruction has been completed.
 */
__ALWAYS_STATIC_INLINE void __ISB(void)
{
    // __ASM volatile("fence.i");
    // __ASM volatile("fence r, r");
	asm volatile(".long 0x0000100f\n":::"memory");
	asm volatile(".long 0x0220000f\n":::"memory");
}


/**
  \brief   Data Synchronization Barrier
  \details Acts as a special kind of Data Memory Barrier.
           It completes when all explicit memory accesses before this instruction complete.
 */
__ALWAYS_STATIC_INLINE void __DSB(void)
{
    // __ASM volatile("fence iorw, iorw");
	asm volatile(".long 0x0ff0000f\n":::"memory");
}

/**
  \brief   Invalid all icache
  \details invalid all icache.
 */
__ALWAYS_STATIC_INLINE void __ICACHE_IALL(void)
{
    // __ASM volatile("icache.iall");
	asm volatile(".long 0x0100000b\n":::"memory");
}

/**
  \brief   Clear & invalid all dcache
  \details clear & invalid all dcache.
 */
__ALWAYS_STATIC_INLINE void __DCACHE_CIALL(void)
{
    // __ASM volatile("dcache.ciall");
	asm volatile(".long 0x0030000b\n":::"memory");
}

__ALWAYS_STATIC_INLINE void __DCACHE_CPA(uint64_t addr)
{
    // __ASM volatile("dcache.cpa %0" : : "r"(addr));
	asm volatile(".long 0x0220000f\n":::"memory");
}

__ALWAYS_STATIC_INLINE void __L2CACHE_CIALL(void)
{
    // __ASM volatile("l2cache.ciall");
	asm volatile(".long 0x0170000b\n":::"memory");
}

/* ##########################  Cache functions  #################################### */
/**
  \ingroup  CSI_Core_FunctionInterface
  \defgroup CSI_Core_CacheFunctions Cache Functions
  \brief    Functions that configure Instruction and Data cache.
  @{
 */

/**
  \brief   Enable I-Cache
  \details Turns on I-Cache
  */
__STATIC_INLINE void csi_icache_enable (void)
{
#if (__ICACHE_PRESENT == 1U)
    uint32_t cache;
    __DSB();
    __ISB();
    __ICACHE_IALL();
    cache = __get_MHCR();
    cache |= CACHE_MHCR_IE_Msk;
    __set_MHCR(cache);
    __DSB();
    __ISB();
#endif
}

/**
  \brief   Disable I-Cache
  \details Turns off I-Cache
  */
__STATIC_INLINE void csi_icache_disable (void)
{
#if (__ICACHE_PRESENT == 1U)
    uint32_t cache;
    __DSB();
    __ISB();
    cache = __get_MHCR();
    cache &= ~CACHE_MHCR_IE_Msk;            /* disable icache */
    __set_MHCR(cache);
    __ICACHE_IALL();                        /* invalidate all icache */
    __DSB();
    __ISB();
#endif
}

/**
  \brief   Enable D-Cache
  \details Turns on D-Cache
  \note    I-Cache also turns on.
  */
__STATIC_INLINE void csi_dcache_enable (void)
{
#if (__DCACHE_PRESENT == 1U)
    uint32_t cache;
    __DSB();
    __ISB();
    __DCACHE_CIALL();                        /* invalidate all dcache */
    cache = __get_MHCR();
//    cache |= (CACHE_MHCR_DE_Msk | CACHE_MHCR_WB_Msk | CACHE_MHCR_WA_Msk | CACHE_MHCR_BPE_Msk | CACHE_MHCR_L0BTB_Msk | CACHE_MHCR_IBPE_Msk | CACHE_MHCR_WBR_Msk);      /* enable all Cache */
    cache |= (CACHE_MHCR_DE_Msk | CACHE_MHCR_WB_Msk | CACHE_MHCR_WA_Msk | CACHE_MHCR_RS_Msk | CACHE_MHCR_BPE_Msk | CACHE_MHCR_L0BTB_Msk | CACHE_MHCR_IBPE_Msk | CACHE_MHCR_WBR_Msk);      /* enable all Cache */
    __set_MHCR(cache);

    __DSB();
    __ISB();
#endif
}

/**
  \brief   Disable D-Cache
  \details Turns off D-Cache
  \note    I-Cache also turns off.
  */
__STATIC_INLINE void csi_dcache_disable (void)
{
#if (__DCACHE_PRESENT == 1U)
    register uint32_t cache;
    __DSB();
    __ISB();
    cache = __get_MHCR();
    cache &= ~(uint32_t)CACHE_MHCR_DE_Msk; /* disable all Cache */
	__set_MHCR(cache);
    __DCACHE_CIALL();                             /* invalidate all Cache */
    __DSB();
    __ISB();
#endif
}

/**
  \brief   Clean & Invalidate D-Cache
  \details Cleans and Invalidates D-Cache
  \note    I-Cache also flush.
  */
__STATIC_INLINE void csi_dcache_clean_invalid (void)
{
#if (__DCACHE_PRESENT == 1U)
    __DSB();
    __ISB();
    __DCACHE_CIALL();                                   /* clean and inv all Cache */
    __DSB();
    __ISB();
#endif
}

__STATIC_INLINE void csi_l2cache_flush_invalid (void)
{
#if (__L2CACHE_PRESENT == 1U)
    __DSB();
    __ASM volatile("sync.i");
    __L2CACHE_CIALL();                                     /* flush & Invalidate l2 Cache */
    __DSB();
    __ASM volatile("sync.i");
#endif
}

void flush_dcache_all(void)
{
	csi_dcache_clean_invalid();
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	for (; i < end; i += CONFIG_SYS_CACHELINE_SIZE)
		asm volatile(".long 0x0295000b");  /* dcache.cpa a0 */

	sync_is();
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	for (; i < end; i += CONFIG_SYS_CACHELINE_SIZE)
		asm volatile(".long 0x02a5000b");  /* dcache.ipa a0 */

	sync_is();
}

void icache_enable(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	csi_icache_enable();
#endif
}

void icache_disable(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	csi_icache_disable();
#endif
}

void dcache_enable(void)
{
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	csi_dcache_enable();
	// L2 cache is is always enabled
#endif
}

void dcache_disable(void)
{
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	csi_dcache_disable();
	// L2 cache is is always enabled
#endif
}

