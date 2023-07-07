/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-15     JasonHu,GUI  first version
 */

#include "boot_module.h"
#include "multiboot2.h"
#include <string.h>

static void boot_module_init(struct multiboot_tag *tag)
{
    struct boot_modules_info_block *modules_info = (struct boot_modules_info_block *)BOOT_MODULE_INFO_ADDR;
    int index = modules_info->modules_num;

    if (index >= MAX_BOOT_MODULES_NUM
        || modules_info->modules_size + ((struct multiboot_tag_module *)tag)->size > MAX_BOOT_MODULES_SIZE)
    {
        return;
    }

    modules_info->modules[index].size = ((struct multiboot_tag_module *)tag)->size;
    modules_info->modules[index].start = ((struct multiboot_tag_module *)tag)->mod_start;
    modules_info->modules[index].end = ((struct multiboot_tag_module *)tag)->mod_end;
    modules_info->modules[index].type = BOOT_MODULE_UNKNOWN;

    modules_info->modules_size += modules_info->modules[index].size;
    ++modules_info->modules_num;
}

static void boot_memory_init(struct multiboot_tag *tag)
{
    unsigned long mem_upper = ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper;
    unsigned long mem_lower = ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower;
    // memory size store in 0x000001000
    *((unsigned int *)0x000001000) = ((mem_upper - mem_lower) << 10) + 0x100000;
}

int rt_boot_setup_entry(unsigned long magic, unsigned long addr)
{
    // whether a multiboot
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC || addr & 7)
        return -1;
    struct multiboot_tag *tag;

    boot_module_info_init();

    for (tag = (struct multiboot_tag*)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END; \
        tag = (struct multiboot_tag*)((rt_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        switch (tag->type)
        {
        case MULTIBOOT_TAG_TYPE_MODULE:
            boot_module_init(tag);
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            boot_memory_init(tag);
            break;
        default: // other type, do nothing
            break;
        }
    }
    return 0;
}
