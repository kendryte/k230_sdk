/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2021-07-15     JasonHu,GuEe-GUI  first version
 */

#ifndef __BOOT_MODULE_H__
#define __BOOT_MODULE_H__

#include <rtdef.h>

#define BOOT_MODULE_INFO_ADDR 0x3F1000

#define SIZE_MB (1024*1024)
#define MAX_BOOT_MODULES_NUM 1
#define MAX_BOOT_MODULES_SIZE (1 * SIZE_MB)

enum boot_module_type
{
    // Unknown type
    BOOT_MODULE_UNKNOWN = 0,
};

struct boot_modules_info_block
{
    rt_uint32_t modules_num;
    rt_uint32_t modules_size;
    struct {
        rt_uint32_t type;
        rt_uint32_t size;
        rt_uint32_t start;
        rt_uint32_t end;
    } modules[MAX_BOOT_MODULES_NUM];
} __attribute__((packed));

rt_inline void boot_module_info_init()
{
    struct boot_modules_info_block *modules_info = (struct boot_modules_info_block *)BOOT_MODULE_INFO_ADDR;
    modules_info->modules_num = 0;
    modules_info->modules_size = 0;
}

rt_inline void *boot_module_info_find(unsigned long base_addr, enum boot_module_type type)
{
    int i;
    struct boot_modules_info_block *modules_info;
    modules_info = (struct boot_modules_info_block *)(base_addr + BOOT_MODULE_INFO_ADDR);

    for (i = 0; i < modules_info->modules_num; ++i)
    {
        if (modules_info->modules[i].type == type)
        {
            return (void*)(base_addr + modules_info->modules[i].start);
        }
    }
    return (void*)0;
}

#endif /* __BOOT_MODULE_H__ */
