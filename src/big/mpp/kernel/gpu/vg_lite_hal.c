/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2022 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (C) 2014 - 2022 Vivante Corporation
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/
#include "k_type.h"
#include <rtthread.h>
#include <rtdbg.h>
#include "ioremap.h"
#include "lwp_pid.h"
#include "lwp_user_mm.h"
#include "mmz_ext.h"
#include "rtdef.h"
#include "rthw.h"
#include "rtthread.h"
#include "sysctl_media_clk.h"
#include "sysctl_pwr.h"
#include "sysctl_rst.h"
#include "vg_lite_platform.h"
#include "vg_lite_ioctl.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hal.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hw.h"

static void sleep(uint32_t msec)
{
    rt_thread_mdelay(msec);
}

#define rt_kprintf(...) (void*)0

static    uint32_t    registerMemBase    = 0x90800000;
static    uint32_t    irq_num            = 135;

/* If bit31 is activated this indicates a bus error */
#define IS_AXI_BUS_ERR(x) ((x)&(1U << 31))

/* Default heap size is 16MB. */
static int heap_size = MAX_CONTIGUOUS_SIZE;

void __attribute__((weak)) vg_lite_bus_error_handler();

struct mapped_memory {
    void * klogical;
    vg_lite_kernel_map_memory_t map;
};

struct vg_lite_device {
    /* void * gpu; */
    uint32_t register_base;    /* Always use physical for register access in RTOS. */
    /* struct page * pages; */
    void * virtual;
    uint32_t physical;
    uint32_t size;
    int irq_enabled;
    volatile uint32_t int_flags;
    struct rt_semaphore int_queue;
    void * device;
    int registered;
    int major;
    struct class * class;
    int created;
};

struct client_data {
    struct vg_lite_device * device;
    struct vm_area_struct * vm;
    void * contiguous_mapped;
};

static struct vg_lite_device Device, * device;

void * vg_lite_hal_alloc(unsigned long size)
{
    return rt_malloc(size);
}

void vg_lite_hal_free(void * memory)
{
    rt_free(memory);
}

void vg_lite_hal_delay(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

void vg_lite_hal_barrier(void)
{
    // TODO: flush all alloc memory
}

void vg_lite_hal_initialize(void) {}

void vg_lite_hal_deinitialize(void) {}

vg_lite_error_t vg_lite_hal_allocate_contiguous(unsigned long size, void ** logical, void ** klogical, uint32_t * physical,void ** node)
{
    unsigned long aligned_size;
    k_u64 phy_addr;
    struct mapped_memory* heap;
    vg_lite_kernel_map_memory_t map_node;

    /* Align the size to 64 bytes. */
    aligned_size = (size + 63) & ~63;
    heap = rt_malloc(sizeof(struct mapped_memory));
    if (mmz_ext_malloc_nocache("anonymous", "gpu", &phy_addr, &heap->klogical, aligned_size)) {
        return VG_LITE_OUT_OF_MEMORY;
    }
    heap->map.physical = *physical = (uint32_t)phy_addr;
    heap->map.bytes = aligned_size;
    vg_lite_hal_map_memory(&heap->map);
    *logical = heap->map.logical;
    *node = heap;
    rt_kprintf("vg_lite alloc: %08x %p %p %d\n", heap->map.physical, heap->klogical, heap->map.logical, aligned_size);

    return VG_LITE_SUCCESS;
}

void vg_lite_hal_free_contiguous(void * memory_handle)
{
    struct mapped_memory* heap = memory_handle;
    rt_kprintf("vg_lite free %08x %p %p\n", heap->map.physical, heap->klogical, heap->map.logical);
    vg_lite_kernel_unmap_memory_t node = {
        .bytes = heap->map.bytes,
        .logical = heap->map.logical,
    };
    vg_lite_hal_unmap_memory(&node);
    mmz_ext_free(heap->map.physical, heap->klogical);
    rt_free(heap);
}

void vg_lite_hal_free_os_heap(void)
{
    // TODO: free all alloc memory
}

/* Portable: read register value. */
uint32_t vg_lite_hal_peek(uint32_t address)
{
    k_u32 val;

	asm volatile("lw %0, 0(%1)" : "=r" (val) : "r" (device->virtual + address));
	return val;
}

/* Portable: write register. */
void vg_lite_hal_poke(uint32_t address, uint32_t data)
{
    asm volatile("sw %0, 0(%1)" : : "r" (data), "r" (device->virtual + address));
}

vg_lite_error_t vg_lite_hal_query_mem(vg_lite_kernel_mem_t *mem)
{
    // TODO: get memory information
    mem->bytes  = 0;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_hal_map_memory(vg_lite_kernel_map_memory_t *node)
{
    // return VG_LITE_SUCCESS;
    node->logical = lwp_map_user_phy(
        lwp_from_pid(lwp_getpid()),
        RT_NULL,
        (void*)(size_t)node->physical,
        node->bytes,
        0
    );
    if (node->logical == NULL) {
        return VG_LITE_OUT_OF_RESOURCES;
    }
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_hal_unmap_memory(vg_lite_kernel_unmap_memory_t *node)
{
    return VG_LITE_SUCCESS;
    if (lwp_unmap_user_phy(lwp_from_pid(lwp_getpid()), node->logical)) {
        return VG_LITE_INVALID_ARGUMENT;
    }
    return VG_LITE_SUCCESS;
}

void __attribute__((weak)) vg_lite_bus_error_handler()
{
    /*
     * Default implementation of the bus error handler does nothing. Application
     * should override this handler if it requires to be notified when a bus
     * error event occurs.
     */
     return;
}

static void vg_lite_IRQHandler(int irq, void *param)
{
    uint32_t flags = vg_lite_hal_peek(VG_LITE_INTR_STATUS);

    if (flags) {
        /* Combine with current interrupt flags. */
        device->int_flags |= flags;

        /* Wake up any waiters. */
        rt_sem_release(&device->int_queue);
    }
}

int32_t vg_lite_hal_wait_interrupt(uint32_t timeout, uint32_t mask, uint32_t * value)
{
    rt_err_t err = rt_sem_take(&device->int_queue, 1000);
    if (err == RT_EOK) {
        if (device->int_flags & mask) {
            *value = device->int_flags & mask;
        }
        device->int_flags = 0;
        if (IS_AXI_BUS_ERR(*value))
        {
            vg_lite_bus_error_handler();
        }

        unsigned cnt = 0;
        while ((vg_lite_hal_peek(VG_LITE_HW_IDLE) != 0x7fffffff) && (cnt < 100)) {
            vg_lite_hal_delay(1);
            cnt += 1;
        }

        return 1;
    } else {
        return 0;
    }
}

vg_lite_error_t vg_lite_hal_memory_export(int32_t *fd)
{
    return VG_LITE_NOT_SUPPORT;
}


void * vg_lite_hal_map(uint32_t flags, uint32_t bytes, void *logical, uint32_t physical, int32_t dma_buf_fd, uint32_t *gpu)
{
    // not support
    return NULL;
}

void vg_lite_hal_unmap(void * handle)
{
    return;
}

vg_lite_error_t vg_lite_hal_operation_cache(void *handle, vg_lite_cache_op_t cache_op)
{
    (void) handle;
    (void) cache_op;

    return VG_LITE_SUCCESS;
}

static struct rt_device g_gpu_device;

static int gpu_open(struct dfs_fd *file) {
    rt_kprintf("vg_lite open\n");
    /* Turn on the power. */
    sysctl_pwr_up(SYSCTL_PD_DISP);
    /* Turn on the clock. */
    sysctl_media_clk_set_leaf_en(SYSCTL_CLK_DISP_GPU, true);
    sysctl_reset(SYSCTL_RESET_GPU);
    rt_hw_interrupt_umask(irq_num);
    return 0;
}

static int gpu_close(struct dfs_fd *file) {
    /* TODO: Remove clock. */
    /* TODO: Remove power. */
    rt_kprintf("vg_lite close\n");
    rt_hw_interrupt_umask(irq_num);
    return 0;
}

static const char * const vg_lite_command_string[] = {
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

static int gpu_ioctl(struct dfs_fd *file, int cmd, void *args) {
    struct ioctl_data arguments;
    void *data;

    if (cmd != VG_LITE_IOCTL)
        return -1;

    // FIXME: if ((void *)!arg)
    if (!args)
        return -1;

    lwp_get_from_user(&arguments, (void *)args, sizeof(arguments));
    data = rt_malloc(arguments.bytes);
    lwp_get_from_user(data, arguments.buffer, arguments.bytes);

    if (arguments.command < sizeof(vg_lite_command_string) / 8) {
        rt_kprintf("vg_lite: ioctl %s\n", vg_lite_command_string[arguments.command]);
    } else {
        rt_kprintf("vg_lite: ioctl unknown command\n");
    }
    arguments.error = vg_lite_kernel(arguments.command, data);

    lwp_put_to_user(arguments.buffer, data, arguments.bytes);
    rt_free(data);
    lwp_put_to_user((void *)args, &arguments, sizeof(arguments));

    return 0;
}

static struct dfs_file_ops gpu_ops = {
    .open = gpu_open,
    .close = gpu_close,
    .ioctl = gpu_ioctl,
};

int vg_lite_init(void)
{
    /* Initialize memory and objects ***************************************/
    /* Create device structure. */
    rt_kprintf("VGLite built %s %s\n", __DATE__, __TIME__);
    device = &Device;

    /* Zero out the enture structure. */
    memset(device, 0, sizeof(struct vg_lite_device));

    /* Setup register memory. **********************************************/
    device->register_base = registerMemBase;
    device->virtual = rt_ioremap_nocache((void*)(rt_size_t)device->register_base, 0x1000);
    if (device->virtual == NULL) {
        return -1;
    }
    rt_hw_interrupt_mask(irq_num);
    rt_hw_interrupt_install(irq_num, vg_lite_IRQHandler, NULL, "gpu");

    rt_err_t err = rt_device_register(&g_gpu_device, "vg_lite", RT_DEVICE_OFLAG_RDWR);
    if (err != RT_EOK) {
        rt_iounmap(device->virtual);
        return -1;
    }
    g_gpu_device.fops = &gpu_ops;

    rt_sem_init(&device->int_queue, "gpu", 0, RT_IPC_FLAG_FIFO);
    device->int_flags = 0;

    /* Success. */
    return 0;
}
