#ifndef __MMZ_EXT_H__
#define __MMZ_EXT_H__

#include "k_type.h"

int mmz_ext_malloc_nocache(const char *mmb_name,
                           const char *mmz_name,
                           k_u64 *phys_addr,
                           void **virt_addr,
                           unsigned long size);

void mmz_ext_free(k_u64 phys_addr, void *virt_addr);

#endif
