/****************************************************************************
 *
 *   The MIT License (MIT)
 *
 *   Copyright (c) 2014 - 2020 Vivante Corporation
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *   DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 *   The GPL License (GPL)
 *
 *   Copyright (C) 2014 - 2020 Vivante Corporation
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *****************************************************************************
 *
 *   Note: This software is released under dual MIT and GPL licenses. A
 *   recipient may use this file under the terms of either the MIT license or
 *   GPL License. If you wish to use only one license not the other, you can
 *   indicate your decision by deleting one of the above license notices in your
 *   version of this file.
 *
 *****************************************************************************/

#include "linux/device/bus.h"
#include "linux/dma-direction.h"
#include "linux/err.h"
#include "linux/kern_levels.h"
#include "linux/power_supply.h"
#include "linux/printk.h"
#include "linux/scatterlist.h"
#include "linux/slab.h"
#include "linux/types.h"
#include "vg_lite_platform.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hal.h"
#include "vg_lite_ioctl.h"
#include "vg_lite_hw.h"
#include "vg_lite_type.h"
#include "vg_lite_debug.h"
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/pagemap.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/mm_types.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/pm_runtime.h>

MODULE_LICENSE("Dual MIT/GPL");

static int vg_lite_init(struct platform_device *pdev);
static int vg_lite_exit(struct platform_device *pdev);

#if KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE
#define VM_FLAGS (VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_DONTDUMP)
#else
#define VM_FLAGS (VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_RESERVED)
#endif

#if KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE
#define current_mm_mmap_sem current->mm->mmap_lock
#else
#define current_mm_mmap_sem current->mm->mmap_sem
#endif

#define GET_PAGE_COUNT(size, p_size) \
( \
((size) + (p_size) - 1) / p_size \
)

/* Struct definitions. */
struct dma_node {
    struct list_head list;
    dma_addr_t dma_addr;
    void* virt_addr;
    unsigned long size;
    vg_lite_kernel_map_memory_t map;
};

enum um_desc_type {
    UM_PAGE_MAP,
    UM_PFN_MAP,
};

struct mapped_memory {
    struct list_head list;
    vg_lite_uint32_t  flags; 
    
    union {
        struct {
            /* parse user dma_buf fd */
            vg_lite_pointer usr_dmabuf;
    
            /* Descriptor of a dma_buf imported. */
            struct dma_buf *dmabuf;
            struct sg_table *sgt;
            struct dma_buf_attachment *attachment;
            vg_lite_uintptr_t *dma_address_array;
    
            vg_lite_int32_t npages;
            vg_lite_int32_t pid;
            struct list_head list;
        } dmabuf_desc;
    
        struct {
            enum um_desc_type type;

            vg_lite_pointer   logical;
            vg_lite_uintptr_t physical;
            vg_lite_int32_t   page_count;

            union {
                /* UM_PAGE_MAP. */
                struct {
                    struct page  **pages;
                };
    
                /* UM_PFN_MAP. */
                struct {
                    vg_lite_long_t  *pfns;
                    vg_lite_int32_t *refs;
                    vg_lite_int32_t  pfns_valid;
               };
            };
    
            /* TODO: Map pages to sg table. */
            struct sg_table   sgt;
            vg_lite_uint32_t  alloc_from_res;
    
            /* record user data */
            vg_lite_uintptr_t user_vaddr;
            vg_lite_uint32_t  size;
            vg_lite_flag_t    vm_flags;
        } um_desc;
    };
};

struct vg_lite_device {
    void *gpu;             /* Register memory base */
    struct page *pages;
    unsigned int order;
    void *virtual;
    u32 physical;
    u32 size;
    int irq_enabled;

    volatile u32 int_flags;

    wait_queue_head_t int_queue;
    void *device;
    int registered;
    int major;
    struct class *class;
    struct device *dev;
    struct clk* clk;
    struct reset_control *reset;
    struct list_head dma_list_head;
    struct list_head mapped_list_head;
    int created;
};

struct client_data {
    struct vg_lite_device *device;
    struct vm_area_struct *vm;
    void *contiguous_mapped;
};

/* Data and objects declarations. */
static int verbose = 1;
static int cached = 1;

static struct vg_lite_device *device;
static struct client_data *private_data;

void vg_lite_hal_delay(u32 milliseconds) {
    /* Delay the requested amount. */
    msleep(milliseconds);
}

void vg_lite_hal_barrier(void) {
    smp_mb();
    if (cached) {
        flush_cache_all();
    }
}

void vg_lite_hal_initialize(void) {};
void vg_lite_hal_deinitialize(void) {};

void vg_lite_hal_open(void) {
    // Power-on
    pm_runtime_get_sync(device->dev);
    // Reset
    reset_control_reset(device->reset);
}

void vg_lite_hal_close(void) {
    // Power-off
    pm_runtime_put_sync(device->dev);
}

#define VG_LITE_PAD(number, align_bytes) \
        ((number) + (((align_bytes) - (number) % (align_bytes)) % (align_bytes)))

vg_lite_error_t vg_lite_hal_allocate_contiguous(unsigned long size, void **logical, void ** klogical, u32 *physical, void **node) {
    struct dma_node* n = kmalloc(sizeof(struct dma_node), GFP_KERNEL);
    if (!n) {
        return VG_LITE_OUT_OF_MEMORY;
    }
#define MANUAL_ALIGN 1
    /* FIXME: Align phy address to 64 bytes, manualy align trig tainted. */
#if MANUAL_ALIGN
    size = VG_LITE_ALIGN(size, 64);
#endif
    n->size = size;
    n->virt_addr = dma_alloc_coherent(device->dev, size, &n->dma_addr, GFP_KERNEL | GFP_DMA);
    if (!n->virt_addr) {
        kfree(n);
        return VG_LITE_OUT_OF_MEMORY;
    }
    // map
    n->map.bytes = n->size;
    n->map.physical = n->dma_addr;
    if (vg_lite_hal_map_memory(&n->map) != VG_LITE_SUCCESS) {
        dma_free_coherent(device->dev, n->size, n->virt_addr, n->dma_addr);
        kfree(n);
        return VG_LITE_OUT_OF_RESOURCES;
    }
    list_add(&n->list, &device->dma_list_head);
#if MANUAL_ALIGN
    *klogical = (u8*)VG_LITE_PAD((size_t)n->virt_addr, 64);
    *logical = (u8*)VG_LITE_PAD((size_t)n->map.logical, 64);
    *physical = VG_LITE_PAD(n->map.physical, 64);
#else
    *logical = n->map.logical;
    *physical = n->map.physical;
#endif
    *node = n;
    printk(KERN_INFO "vg_lite: alloc %lu bytes, v: %016lX, p: %08X\n", size, (size_t)n->map.logical, (u32)n->map.physical);
    return VG_LITE_SUCCESS;
}

void vg_lite_hal_free_contiguous(void *memory_handle)
{
    struct dma_node* n = memory_handle;
    vg_lite_kernel_unmap_memory_t unmap = {.bytes = n->map.bytes, .logical = n->map.logical};
    vg_lite_hal_unmap_memory(&unmap);
    dma_free_coherent(device->dev, n->size, n->virt_addr, n->dma_addr);
    list_del(&n->list);
    kfree(n);
    return;
}

void vg_lite_hal_free_os_heap(void)
{
    struct dma_node* dn;
    struct dma_node* dn2;
    struct mapped_memory* mapped, *_mapped;
    list_for_each_entry_safe(dn, dn2, &device->dma_list_head, list) {
        vg_lite_hal_free_contiguous(dn);
    }
    list_for_each_entry_safe(mapped, _mapped, &device->mapped_list_head, list) {
        vg_lite_hal_unmap(mapped);
    }
}

u32 vg_lite_hal_peek(u32 address)
{
    /* Read data from the GPU register. */
    return *(volatile u32 *)((uint8_t *)device->gpu + address);
    //return readl(device->gpu + address);
}

void vg_lite_hal_poke(u32 address, u32 data)
{
    /* Write data to the GPU register. */
    *(volatile u32 *)((uint8_t *)device->gpu + address) = data;
    //writel(device->gpu + address, data);
}

vg_lite_error_t vg_lite_hal_query_mem(vg_lite_kernel_mem_t *mem)
{
    // FIXME: not impliment
    if (device) {
        mem->bytes = 0;
        return VG_LITE_SUCCESS;
    }
    mem->bytes = 0;
    return VG_LITE_NO_CONTEXT;
}

vg_lite_error_t vg_lite_hal_map_memory(vg_lite_kernel_map_memory_t *node)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    void *_logical = NULL;
    u64 physical = node->physical;
    u32 offset = physical & (PAGE_SIZE - 1);
    u64 bytes = node->bytes + offset;
    u32 num_pages, pfn = 0;
    vg_lite_kernel_unmap_memory_t unmap_node;
    struct vm_area_struct *vma;

#if KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE
    _logical = (void *)vm_mmap(NULL, 0L, bytes, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, 0);
#else
    down_write(&current_mm_mmap_sem);
    _logical = (void *)do_mmap_pgoff(NULL, 0L, bytes,
                PROT_READ | PROT_WRITE, MAP_SHARED, 0);
    up_write(&current_mm_mmap_sem);
#endif

    if (!_logical) {
        node->logical = NULL;
        return VG_LITE_OUT_OF_MEMORY;
    }

    down_write(&current_mm_mmap_sem);

    vma = find_vma(current->mm, (unsigned long)_logical);

    if (!vma)
        return VG_LITE_OUT_OF_RESOURCES;

    pfn = (physical >> PAGE_SHIFT);
    num_pages = GET_PAGE_COUNT(bytes, PAGE_SIZE);

    /* Make this mapping cached / non-cached. */
    if (!cached) {
#if defined(__arm64__) || defined(__aarch64__)
        vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
#else
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif
    }

#if KERNEL_VERSION(3, 7, 0) > LINUX_VERSION_CODE
    vm->vm_flags |= (VM_DONTCOPY | VM_RESERVED);
#else
    vma->vm_flags |= (VM_DONTCOPY | VM_DONTEXPAND | VM_DONTDUMP);
#endif

    if (remap_pfn_range(vma, vma->vm_start, pfn, num_pages << PAGE_SHIFT, vma->vm_page_prot) < 0)
        error = VG_LITE_OUT_OF_MEMORY;

    node->logical = (void *)((uint8_t *)_logical + offset);

    up_write(&current_mm_mmap_sem);

    if (error) {
        unmap_node.bytes = node->bytes;
        unmap_node.logical = node->logical;
        vg_lite_hal_unmap_memory(&unmap_node);
    }

    return error;
}

vg_lite_error_t vg_lite_hal_unmap_memory(vg_lite_kernel_unmap_memory_t *node)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    void *_logical;
    u32 bytes;
    u32 offset = (u64)node->logical & (PAGE_SIZE - 1);

    if (unlikely(!current->mm))
        return error;

    _logical = (void *)((uint8_t *)node->logical - offset);
    bytes = GET_PAGE_COUNT(node->bytes + offset, PAGE_SIZE) * PAGE_SIZE;

#if KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE
    if (vm_munmap((unsigned long)_logical, bytes) < 0) {
        error = VG_LITE_INVALID_ARGUMENT;
        printk(KERN_WARNING "%s: vm_munmap failed\n", __func__);
    }
#else
    down_write(&current_mm_mmap_sem);
    if (do_munmap(current->mm, (unsigned long)_logical, bytes) < 0) {
       error = VG_LITE_INVALID_ARGUMENT;
       printk(KERN_WARNING "%s: do_munmap failed\n", __func__);
    }
    up_write(&current_mm_mmap_sem);
#endif

    return error;
}

int vg_lite_hal_wait_interrupt(u32 timeout, u32 mask, u32 *value)
{
    // FIXME: struct timeval tv;
    unsigned long jiffies;
    unsigned long result;
    #define IGNORE_INTERRUPT 0
    #if IGNORE_INTERRUPT
    unsigned int_flag;
    #endif

    if (timeout == VG_LITE_INFINITE) {
        /* Set 1 second timeout. */
        // FIXME: tv.tv_sec = 1;
        // tv.tv_usec = 0;
        jiffies = msecs_to_jiffies(1000);
    } else {
        /* Convert timeout in ms to timeval. */
        // tv.tv_sec = timeout / 1000;
        // tv.tv_usec = (timeout % 1000) * 1000;
        jiffies = msecs_to_jiffies(timeout);
    }

    /* Convert timeval to jiffies. */
    // jiffies = timeval_to_jiffies(&tv);

    /* Wait for interrupt, ignoring timeout. */
    do {
        result = wait_event_interruptible_timeout(device->int_queue, device->int_flags & mask, jiffies);
        #if IGNORE_INTERRUPT
        int_flag = vg_lite_hal_peek(0x10);
        if (int_flag) {
            result = int_flag;
        }
        printk(
            "vg_lite: waiting... idle: 0x%08X, int: 0x%08X, FE: 0x%08X 0x%08X 0x%08X\n",
            vg_lite_hal_peek(0x4), int_flag,
            vg_lite_hal_peek(0x500), vg_lite_hal_peek(0x504), vg_lite_hal_peek(0x508)
        );
        #endif
    } while (timeout == VG_LITE_INFINITE && result == 0);

    /* Report the event(s) got. */
    if (value)
        *value = device->int_flags & mask;

    device->int_flags = 0;
    return (result != 0);
}

vg_lite_error_t vg_lite_hal_operation_cache(void *handle, vg_lite_cache_op_t cache_op) {
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_hal_memory_export(int32_t *fd)
{
    // TODO
    return VG_LITE_NOT_SUPPORT;
}

void * vg_lite_hal_map(uint32_t flags, uint32_t bytes, void *logical, uint32_t physical, int32_t dma_buf_fd, uint32_t *gpu)
{
    struct mapped_memory * mapped;
   
    mapped = kmalloc(sizeof(struct mapped_memory), GFP_KERNEL);
    if (mapped == NULL) {
        return NULL;
    }
    memset(mapped, 0, sizeof(struct mapped_memory));
    mapped->flags = flags;

    if (flags == VG_LITE_HAL_MAP_DMABUF) {
        struct scatterlist *sg;
        unsigned i;

        mapped->dmabuf_desc.dmabuf = dma_buf_get(dma_buf_fd);
        if (IS_ERR(mapped->dmabuf_desc.dmabuf)) {
            goto error;
        }
        mapped->dmabuf_desc.attachment = dma_buf_attach(mapped->dmabuf_desc.dmabuf, device->dev);
        if (IS_ERR(mapped->dmabuf_desc.attachment)) {
            dma_buf_put(mapped->dmabuf_desc.dmabuf);
            goto error;
        }
        mapped->dmabuf_desc.sgt = dma_buf_map_attachment(mapped->dmabuf_desc.attachment, DMA_BIDIRECTIONAL);
        if (IS_ERR(mapped->dmabuf_desc.sgt)) {
            dma_buf_detach(mapped->dmabuf_desc.dmabuf, mapped->dmabuf_desc.attachment);
            dma_buf_put(mapped->dmabuf_desc.dmabuf);
            goto error;
        }
        for_each_sg(mapped->dmabuf_desc.sgt->sgl, sg, mapped->dmabuf_desc.sgt->orig_nents, i) {
            *gpu = sg_dma_address(sg);
        }
    } else {
        vg_lite_kernel_hintmsg("vg_lite_hal_map: this map type not support!\n");
        return NULL;
    }

    list_add(&mapped->list, &device->mapped_list_head);
    return mapped;

error:
    kfree(mapped);
    return NULL;
}

void vg_lite_hal_unmap(void * handle)
{
    struct mapped_memory * mapped = handle;

    if (mapped->flags == VG_LITE_HAL_MAP_DMABUF) {
        dma_buf_unmap_attachment(mapped->dmabuf_desc.attachment, mapped->dmabuf_desc.sgt, DMA_BIDIRECTIONAL);

        dma_buf_detach(mapped->dmabuf_desc.dmabuf, mapped->dmabuf_desc.attachment);

        dma_buf_put(mapped->dmabuf_desc.dmabuf);
    } else {
        vg_lite_kernel_hintmsg("vg_lite_hal_map: this map type not support!\n");
    }

    list_del(&mapped->list);
    kfree(mapped);
}

int drv_open(struct inode *inode, struct file *file)
{
    struct client_data *data;
    printk(KERN_INFO "vg_lite: open device\n");
    vg_lite_hal_open();
    data = kmalloc(sizeof(*data), GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "vg_lite: kmalloc() return -1\n");
        return -1;
    }
    data->device = device;
    data->contiguous_mapped = NULL;
    file->private_data = data;
    return 0;
}

// FIXME
int drv_release(struct inode *inode, struct file *file)
{
    struct client_data *data = (struct client_data *)file->private_data;
    printk(KERN_INFO "vg_lite: close device\n");
    vg_lite_hal_free_os_heap();

    kfree(data);
    file->private_data = NULL;
    vg_lite_hal_close();

    return 0;
}

#ifdef HAVE_UNLOCKED_IOCTL
long drv_ioctl(struct file *file, unsigned int ioctl_code, unsigned long arg)
#else
static const char *vg_lite_command_string[] = {
    "VG_LITE_INITIALIZE",
    "VG_LITE_TERMINATE",
    "VG_LITE_ALLOCATE",
    "VG_LITE_FREE",
    "VG_LITE_SUBMIT",
    "VG_LITE_WAIT",
    "VG_LITE_RESET",
    "VG_LITE_DEBUG",
    "VG_LITE_MAP",
    "VG_LITE_UNMAP",
    "VG_LITE_CHECK",
    "VG_LITE_QUERY_MEM",
    "VG_LITE_FLEXA_DISABLE",
    "VG_LITE_FLEXA_ENABLE",
    "VG_LITE_FLEXA_STOP_FRAME",
    "VG_LITE_FLEXA_SET_BACKGROUND_ADDRESS",
    "VG_LITE_MAP_MEMORY",
    "VG_LITE_UNMAP_MEMORY",
    "VG_LITE_BUFFER_FROM_DMA_BUF"

};

long drv_ioctl(/*struct inode *inode, */struct file *file, unsigned int ioctl_code, unsigned long arg)
#endif
{
    struct ioctl_data arguments;
    void *data;

#ifndef HAVE_UNLOCKED_IOCTL
    /* inode will be not used */
    //(void)inode;
#endif
    private_data = (struct client_data *)file->private_data;
    if (!private_data)
        return -1;

    if (ioctl_code != VG_LITE_IOCTL)
        return -1;

    // FIXME: if ((void *)!arg)
    if (!arg)
        return -1;

    if (copy_from_user(&arguments, (void *)arg, sizeof(arguments)) != 0)
        return -1;

    data = kmalloc(arguments.bytes, GFP_KERNEL);
    if (!data)
        return -1;

    if (copy_from_user(data, arguments.buffer, arguments.bytes) != 0)
        goto error;

    if (arguments.command < sizeof(vg_lite_command_string) / 8) {
        if (verbose)
            printk(KERN_INFO "vg_lite: ioctl %s\n", vg_lite_command_string[arguments.command]);
    } else {
        printk(KERN_ERR "vg_lite: ioctl unknown command\n");
    }
    arguments.error = vg_lite_kernel(arguments.command, data);

    if (copy_to_user(arguments.buffer, data, arguments.bytes) != 0)
        goto error;

    kfree(data);

    if (copy_to_user((void *)arg, &arguments, sizeof(arguments)) != 0)
        return -1;

    return 0;

error:
    kfree(data);
    return -1;
}

ssize_t drv_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
    struct client_data *private = (struct client_data *)file->private_data;

    if (length != 4)
        return 0;

    if (copy_to_user((void __user *)buffer, (const void *)&private->device->size, sizeof(private->device->size)) != 0)
        return 0;

    memcpy(buffer, &private->device->size, 4);
    return 4;
}

int drv_mmap(struct file *file, struct vm_area_struct *vm)
{
    unsigned long size;
    struct client_data *private = (struct client_data *)file->private_data;

    return 0;

    if (!cached)
#if defined(__arm64__) || defined(__aarch64__)
        vm->vm_page_prot = pgprot_writecombine(vm->vm_page_prot);
#else
        vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);
#endif

#if KERNEL_VERSION(3, 7, 0) > LINUX_VERSION_CODE
    vm->vm_flags |= (VM_DONTCOPY | VM_RESERVED);
#else
    vm->vm_flags |= (VM_DONTCOPY | VM_DONTEXPAND | VM_DONTDUMP);
#endif
    vm->vm_pgoff = 0;

    size = vm->vm_end - vm->vm_start;
    if (size > private->device->size)
        size = private->device->size;

    if (remap_pfn_range(vm, vm->vm_start, private->device->physical >> PAGE_SHIFT, size, vm->vm_page_prot) < 0) {
        printk(KERN_ERR "vg_lite: remap_pfn_range failed\n");
        return -1;
    }

    private->vm = vm;
    private->contiguous_mapped = (void *)vm->vm_start;

    printk(KERN_INFO "vg_lite: mapped %scached contiguous memory to %p\n", cached ? "" : "non-", private->contiguous_mapped);

    return 0;
}

static const struct file_operations file_operations = {
    .owner          = THIS_MODULE,
    .open           = drv_open,
    .release        = drv_release,
    .read           = drv_read,
//#ifdef HAVE_UNLOCKED_IOCTL
    .unlocked_ioctl = drv_ioctl,
//#endif
//#ifdef HAVE_COMPAT_IOCTL
    .compat_ioctl   = drv_ioctl,
//#endif
    .mmap           = drv_mmap,
};

static int vg_lite_exit(struct platform_device *pdev)
{
    /* Check for valid device. */
    if (device) {
        if (device->gpu) {
            /* Unmap the GPU registers. */
            iounmap(device->gpu);
            device->gpu = NULL;
        }

        if (device->pages)
            /* Free the contiguous memory. */
            __free_pages(device->pages, device->order);

        if (device->irq_enabled)
            /* Free the IRQ. */
            free_irq(platform_get_irq(pdev, 0)/*GPU_IRQ*/, device);

        vg_lite_hal_free_os_heap();

        if (device->created)
            /* Destroy the device. */
            device_destroy(device->class, MKDEV(device->major, 0));

        if (device->class)
            /* Destroy the class. */
            class_destroy(device->class);

        if (device->registered)
            /* Unregister the device. */
            unregister_chrdev(device->major, "vg_lite");

        /* Free up the device structure. */
        kfree(device);
    }
    put_device(&pdev->dev);
    return 0;
}

static irqreturn_t irq_handler(int irq, void *context)
{
    struct vg_lite_device *device = context;

    /* Read interrupt status. */
    u32 flags = *(u32 *)((uint8_t *)device->gpu + VG_LITE_INTR_STATUS);

    if (flags) {
        /* Combine with current interrupt flags. */
        device->int_flags |= flags;

        /* Wake up any waiters. */
        wake_up_interruptible(&device->int_queue);

        /* We handled the IRQ. */
        return IRQ_HANDLED;
    }

    /* Not our IRQ. */
    return IRQ_NONE;
}

static int vg_lite_init(struct platform_device *pdev)
{
    struct resource *mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    int irq_line = platform_get_irq(pdev, 0);
    int error = 0;
    struct device* dev;

    /* Create device structure. */
    device = kmalloc(sizeof(*device), GFP_KERNEL);
    if (!device) {
        printk(KERN_ERR "vg_lite: kmalloc failed\n");
        return -1;
    }
    memset(device, 0, sizeof(struct vg_lite_device));
    get_device(&pdev->dev);
    device->dev = &pdev->dev;
    device->clk = devm_clk_get(&pdev->dev, "vglite");
    device->reset = devm_reset_control_get(&pdev->dev, NULL);
    pm_runtime_enable(device->dev);

    /* Map the GPU registers. */
    device->gpu = ioremap(mem->start, resource_size(mem));
    if (!device->gpu) {
        printk(KERN_ERR "vg_lite: ioremap failed %s:%d\n", __func__, __LINE__);
        kfree(device);
        return -1;
    }

    /* Initialize the wait queue. */
    init_waitqueue_head(&device->int_queue);

    /* Install IRQ. */
    if (irq_line < 0) {
        printk(KERN_ERR "vg_lite: platform_get_irq failed, %d\n", irq_line);
        vg_lite_exit(pdev);
        return -1;
    }
    error = request_irq(irq_line/*GPU_IRQ*/, irq_handler, 0, "vg_lite_irq", device);
    if (error) {
        printk(KERN_ERR "vg_lite: request_irq failed, %d\n", error);
        vg_lite_exit(pdev);
        return -1;
    }
    device->irq_enabled = 1;
    printk(KERN_DEBUG "vg_lite: enabled ISR for interrupt %d\n", irq_line/*GPU_IRQ*/);

    /* Register device. */
    device->major = register_chrdev(0, "vg_lite", &file_operations);
    if (device->major < 0) {
        printk(KERN_ERR "vg_lite: register_chrdev failed\n");
        vg_lite_exit(pdev);
        return -1;
    }
    printk(KERN_DEBUG "vg_lite: registered device\n");
    device->registered = 1;

    /* Create the graphics class. */
    device->class = class_create(THIS_MODULE, "vg_lite_class");
    if (!device->class) {
        printk(KERN_ERR "vg_lite: class_create failed\n");
        vg_lite_exit(pdev);
        return -1;
    }
    printk(KERN_DEBUG "vg_lite: created vg_lite_class\n");

    /* Create the device. */
    dev = device_create(device->class, NULL, MKDEV(device->major, 0), NULL, "vg_lite");
    if (!dev) {
        printk(KERN_ERR "vg_lite: device_create failed\n");
        vg_lite_exit(pdev);
        return -1;
    }
    device->created = 1;
    printk(KERN_DEBUG "vg_lite: created /dev/vg_lite device\n");

    INIT_LIST_HEAD(&device->dma_list_head);
    INIT_LIST_HEAD(&device->mapped_list_head);
    if (dma_set_mask_and_coherent(device->dev, DMA_BIT_MASK(32))) {
        printk(KERN_ERR "vg_lite: dma_set_coherent_mask failed\n");
        vg_lite_exit(pdev);
        return -1;
    }

    /* Success. */
    return 0;
}

static const struct of_device_id gc8000ul_of_match[] = {
    { .compatible = "verisilicon,gc8000ul" },
    { /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, gc8000ul_of_match);

module_param(verbose, int, 0600);
module_param(cached, int, 0600);

static struct platform_driver gc8000ul_platform_driver = {
    .driver = {
        .name           = "gc8000ul",
        .of_match_table = gc8000ul_of_match
    },
    .probe              = vg_lite_init,
    .remove             = vg_lite_exit,
};
module_platform_driver(gc8000ul_platform_driver);
