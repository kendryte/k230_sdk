#ifndef _C908_CACHE_H_
#define _C908_CACHE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void csi_dcache_clean_invalid_range(void* addr, uint64_t size) {
    uint64_t op_addr = (uint64_t)addr;
    int64_t op_size = size + op_addr % 64;
    const int64_t linesize = 64;

    __asm volatile("fence iorw, iorw");

    while (op_size > 0) {
        __asm volatile("dcache.civa %0" : : "r"(op_addr));
        op_addr += linesize;
        op_size -= linesize;
    }

    __asm volatile("fence iorw, iorw");
    __asm volatile("fence.i");
    __asm volatile("fence r, r");
}


#ifdef __cplusplus
}
#endif

#endif