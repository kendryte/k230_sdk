/**
 * @file rt_mem.c
 * @author
 * @brief
 * @version 1.0
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022 Canaan Inc.
 *
 */
#include <time.h>
#include <string.h>
#include <rtthread.h>
#include <dfs_file.h>
#include "lwp_user_mm.h"

#ifndef PAGE_OFFSET_BIT
#define PAGE_OFFSET_BIT     12
#endif

static struct rt_device mem_dev;

static int mem_open(struct dfs_fd *file)
{
    return 0;
}

static int mem_close(struct dfs_fd *file)
{
    return 0;
}

static int mem_ioctl(struct dfs_fd *file, int cmd, void *args)
{
    struct dfs_mmap2_args *mmap_args;
    unsigned long offset;
    int cached = 1;

    if(cmd != RT_FIOMMAP2)
        return RT_EOK;

    if(file->flags & O_SYNC)
        cached = 0;

    mmap_args = (struct dfs_mmap2_args *)(args);
    offset = mmap_args->pgoffset << PAGE_OFFSET_BIT;
    void *mmap_addr;

    mmap_addr = lwp_map_user_phy(lwp_self(), RT_NULL,
                                (void *)(size_t)(mmap_args->pgoffset << PAGE_OFFSET_BIT),
                                mmap_args->length, cached);
    mmap_args->ret = mmap_addr;

    return 0;
}

static const struct dfs_file_ops mem_fops = {
    .open = mem_open,
    .close = mem_close,
    .ioctl = mem_ioctl,
};


int mem_device_init(void)
{
    RT_ASSERT(!rt_device_find("mem"));
    mem_dev.type    = RT_Device_Class_Miscellaneous;

    /* no private */
    mem_dev.user_data = RT_NULL;

    rt_device_register(&mem_dev, "mem", RT_DEVICE_FLAG_RDWR);

    mem_dev.fops = &mem_fops;

    return 0;
}
INIT_DEVICE_EXPORT(mem_device_init);

