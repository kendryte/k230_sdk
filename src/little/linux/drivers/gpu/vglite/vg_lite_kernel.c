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

#include "vg_lite_platform.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hal.h"
#include "vg_lite_hw.h"
#include "vg_lite_options.h"
#if defined(__linux__) && !defined(EMULATOR)
#include <linux/sched.h>
/*#include <asm/uaccess.h>*/
#include <linux/uaccess.h>
#include <linux/version.h>
#endif

#define FLEXA_TIMEOUT_STATE                 BIT(21)
#define FLEXA_HANDSHEKE_FAIL_STATE          BIT(22)
#define MIN_TS_SIZE                         (8 << 10)

#ifdef BACKUP_COMMAND
typedef struct power_context_command {
    uint32_t command;
    uint32_t data;
}
power_context_command_t;

static vg_lite_kernel_context_t global_power_context = {0};
static power_context_command_t *power_context_klogical = NULL;
#endif

static int s_reference = 0;

static vg_lite_error_t do_terminate(vg_lite_kernel_terminate_t * data);

static void soft_reset(void);

#ifdef BACKUP_COMMAND
static void execute_recovery_command(uint32_t address, uint32_t size)
{

    vg_lite_hal_poke(VG_LITE_HW_CMDBUF_ADDRESS, address);
    vg_lite_hal_poke(VG_LITE_HW_CMDBUF_SIZE, size);
}

static int check_power_context(int context_index)
{
    int ret = -1;

    if (power_context_klogical[context_index].command != 0x80000000) 
        ret = 1;

    return ret;
}

static vg_lite_error_t backup_power_context_buffer(uint32_t *command_buffer_klogical, uint32_t size)
{
    int index              = 0;
    int ret                = -1;
    uint32_t address       = 0;
    uint32_t context_index = 0; 
    uint32_t data          = 0;

    if (NULL == command_buffer_klogical) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    for (index = 0; index < size; index++) {
        address = command_buffer_klogical[index];

        if (address == 0x30010A1B) {
            power_context_klogical[0].data = command_buffer_klogical[index+1];
            continue;
        }

        if ((address & 0xFFFF0000) == 0x30010000) {
            data = command_buffer_klogical[index+1];
            context_index = address & 0x0000FFFF;
            ret = check_power_context(context_index);
            if (-1 != ret) {
                power_context_klogical[context_index].data    = data;
            } else {
                power_context_klogical[context_index].command = address;
                power_context_klogical[context_index].data    = data;
            }
        }
    }    

    return VG_LITE_SUCCESS;
}
#endif

static void gpu(int enable)
{
    vg_lite_hw_clock_control_t value;
    uint32_t          reset_timer = 2;
    const uint32_t    reset_timer_limit = 1000;

    if (enable) {
        /* Disable clock gating. */
        value.data = vg_lite_hal_peek(VG_LITE_HW_CLOCK_CONTROL);
        value.control.clock_gate = 0;
        vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
        vg_lite_hal_delay(1);

        /* Set clock speed. */
        value.control.scale = 64;
        value.control.scale_load = 1;
        vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
        vg_lite_hal_delay(1);
        value.control.scale_load = 0;
        vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
        vg_lite_hal_delay(5);

        do {
            /* Perform a soft reset. */
            soft_reset();
            vg_lite_hal_delay(reset_timer);
            reset_timer *= 2;   // If reset failed, try again with a longer wait. Need to check why if dead lopp happens here.
        } while (!VG_LITE_KERNEL_IS_GPU_IDLE());
    }
    else
    {
        while (!VG_LITE_KERNEL_IS_GPU_IDLE() && 
            (reset_timer < reset_timer_limit)   // Force shutdown if timeout.
            ) {
            vg_lite_hal_delay(reset_timer);
            reset_timer *= 2;
        }

        /* Set idle speed. */
        value.data = vg_lite_hal_peek(VG_LITE_HW_CLOCK_CONTROL);
        value.control.scale = 1;
        value.control.scale_load = 1;
        vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
        vg_lite_hal_delay(1);
        value.control.scale_load = 0;
        vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
        vg_lite_hal_delay(5);

        /* Enable clock gating. */
        value.control.clock_gate = 1;
        vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
        vg_lite_hal_delay(1);
    }
}

/* Initialize some customized modeuls [DDRLess]. */
static vg_lite_error_t init_3rd(vg_lite_kernel_initialize_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    /* TODO: Init the YUV<->RGB converters. Reserved for SOC. */
    /* vg_lite_hal_poke(0x00514, data->yuv_pre);
       vg_lite_hal_poke(0x00518, data->yuv_post);
     */
    return error;
}

static vg_lite_error_t init_vglite(vg_lite_kernel_initialize_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_context_t * context;
    int      i;

#if defined(__linux__) && !defined(EMULATOR)
    vg_lite_kernel_context_t __user * context_usr;
    vg_lite_kernel_context_t mycontext = { 0 };

    // Construct the context.
    context_usr = (vg_lite_kernel_context_t  __user *) data->context;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)    
     if (!access_ok(VERIFY_READ, context_usr, sizeof(*context_usr)) ||
       !access_ok(VERIFY_WRITE, context_usr, sizeof(*context_usr))) {
#else
     if (!access_ok(context_usr, sizeof(*context_usr)) ||
       !access_ok(context_usr, sizeof(*context_usr))) {
#endif
        /* Out of memory. */
        return VG_LITE_OUT_OF_MEMORY;
    }
    context = &mycontext;
#else
    // Construct the context.
    context = data->context;
    if (context == NULL)
    {
        /* Out of memory. */
        return VG_LITE_OUT_OF_MEMORY;
    }
#endif

    /* Zero out all pointers. */
    for (i = 0; i < CMDBUF_COUNT; i++) {
        context->command_buffer[i]          = NULL;
        context->command_buffer_logical[i]  = NULL;
        context->command_buffer_physical[i] = 0;
    }
    context->tess_buffer            = NULL;
    context->tessbuf_logical    = NULL;
    context->tessbuf_physical   = 0;
#ifdef BACKUP_COMMAND
    global_power_context.power_context_logical = NULL;
    global_power_context.power_context_klogical = NULL;
    global_power_context.power_context_physical = 0;
    global_power_context.power_context = NULL;
    global_power_context.power_context_capacity = 32 << 10;
    global_power_context.power_context_size = (VG_LITE_ALIGN(global_power_context.power_context_capacity, VGLITE_MEM_ALIGNMENT) + 7) / 8;
    printk("global_power_context.power_context_capacity = %d\n", global_power_context.power_context_capacity);
    printk("global_power_context.power_context_size     = %d\n", global_power_context.power_context_size);
#endif
    /* Increment reference counter. */
    if (s_reference++ == 0) {
        /* Initialize the SOC. */
        vg_lite_hal_initialize();

        /* Enable the GPU. */
        gpu(1);
    }

    /* Fill in hardware capabilities. */
    data->capabilities.data = 0;

    /* Allocate the command buffer. */
    if (data->command_buffer_size) {
        for (i = 0; i < 2; i ++)
        {
            /* Allocate the memory. */
            error = vg_lite_hal_allocate_contiguous(data->command_buffer_size,
                                                    &context->command_buffer_logical[i],
                                                    &context->command_buffer_klogical[i],
                                                    &context->command_buffer_physical[i],
                                                    &context->command_buffer[i]);
            if (error != VG_LITE_SUCCESS) {
                /* Free any allocated memory. */
                vg_lite_kernel_terminate_t terminate = { context };
                do_terminate(&terminate);
                
                /* Out of memory. */
                return error;
            }
            
            /* Return command buffer logical pointer and GPU address. */
            data->command_buffer[i] = context->command_buffer_logical[i];
            data->command_buffer_gpu[i] = context->command_buffer_physical[i];
        }
    }

#ifdef BACKUP_COMMAND
    if (global_power_context.power_context_capacity) {
        /*  Allocate the backup buffer. */
        error = vg_lite_hal_allocate_contiguous(global_power_context.power_context_capacity,
                                                &global_power_context.power_context_logical,
                                                &global_power_context.power_context_klogical,
                                                &global_power_context.power_context_physical,
                                                &global_power_context.power_context);
        if (error != VG_LITE_SUCCESS) {
            /* Free any allocated memory. */
            vg_lite_kernel_terminate_t terminate = { &global_power_context };
            do_terminate(&terminate);

            /* Out of memory. */
            return error;
        }

        /* init power context buffer to 0x8000000 */
        printk("init global power context to NOP command\n");
        power_context_klogical = (power_context_command_t *)global_power_context.power_context_klogical;
        for (i = 0; i < global_power_context.power_context_size; i++) { 
            power_context_klogical[i].command = 0x80000000;
            power_context_klogical[i].data    = 0x00000000;
        }
        power_context_klogical[0].command = 0x30010A1B;
        power_context_klogical[0].data    = 0x00000001;
        power_context_klogical[1].command = 0x10000007;
        power_context_klogical[1].data    = 0x00000000;
        power_context_klogical[2].command = 0x20000007;
        power_context_klogical[2].data    = 0x00000000;
    }
#endif
    /* Allocate the tessellation buffer. */
    if ((data->tess_width > 0) && (data->tess_height > 0)) 
    {
        int width = data->tess_width;
        int height = 0;
        int vg_countbuffer_size = 0, total_size = 0, ts_buffer_size = 0;

        height = VG_LITE_ALIGN(data->tess_height, 16);

#if (CHIPID==0x355 || CHIPID==0x255)
        {
            unsigned long stride, buffer_size, l1_size, l2_size;
#if (CHIPID==0x355)
            data->capabilities.cap.l2_cache = 1;
            width = VG_LITE_ALIGN(width, 128);
#endif
            /* Check if we can used tiled tessellation (128x16). */
            if (((width & 127) == 0) && ((height & 15) == 0)) {
                data->capabilities.cap.tiled = 0x3;
            } else {
                data->capabilities.cap.tiled = 0x2;
            }

            /* Compute tessellation buffer size. */
            stride = VG_LITE_ALIGN(width * 8, 64);
            buffer_size = VG_LITE_ALIGN(stride * height, 64);
            /* Each bit in the L1 cache represents 64 bytes of tessellation data. */
            l1_size = VG_LITE_ALIGN(VG_LITE_ALIGN(buffer_size / 64, 64) / 8, 64);
#if (CHIPID==0x355)
            /* Each bit in the L2 cache represents 32 bytes of L1 data. */
            l2_size = VG_LITE_ALIGN(VG_LITE_ALIGN(l1_size / 32, 64) / 8, 64);
#else
            l2_size = 0;
#endif
            total_size = buffer_size + l1_size + l2_size;
            ts_buffer_size = buffer_size;
        }
#else /* (CHIPID==0x355 || CHIPID==0x255) */
        {   
            /* Check if we can used tiled tessellation (128x16). */
            if (((width & 127) == 0) && ((height & 15) == 0)) {
                data->capabilities.cap.tiled = 0x3;
            }
            else {
                data->capabilities.cap.tiled = 0x2;
            }

            vg_countbuffer_size = height * 3;
            vg_countbuffer_size = VG_LITE_ALIGN(vg_countbuffer_size, 64);
            total_size = height * 128;
            if (total_size < MIN_TS_SIZE)
                total_size = MIN_TS_SIZE;
            ts_buffer_size = total_size - vg_countbuffer_size;
        }
#endif /* (CHIPID==0x355 || CHIPID==0x255) */

        /* Allocate the memory. */
        error = vg_lite_hal_allocate_contiguous(total_size,
                                                &context->tessbuf_logical,
                                                &context->tessbuf_klogical,
                                                &context->tessbuf_physical,
                                                &context->tess_buffer);
        if (error != VG_LITE_SUCCESS) {
            /* Free any allocated memory. */
            vg_lite_kernel_terminate_t terminate = { context };
            do_terminate(&terminate);
            return error;
        }

        /* Return the tessellation buffer pointers and GPU addresses. */
        data->physical_addr = context->tessbuf_physical;
        data->logical_addr = (uint8_t *)context->tessbuf_logical;
        data->tessbuf_size = ts_buffer_size;
        data->countbuf_size = vg_countbuffer_size;
        data->tess_w_h = width | (height << 16);
    }

    /* Enable all interrupts. */
    vg_lite_hal_poke(VG_LITE_INTR_ENABLE, 0xFFFFFFFF);

#if defined(__linux__) && !defined(EMULATOR)
    if (copy_to_user(context_usr, context, sizeof(vg_lite_kernel_context_t)) != 0) {
      // Free any allocated memory.
      vg_lite_kernel_terminate_t terminate = { context };
      do_terminate(&terminate);

      return VG_LITE_NO_CONTEXT;
    }
#endif
    return error;
}

static vg_lite_error_t do_initialize(vg_lite_kernel_initialize_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    /* Free any allocated memory for the context. */
    do {
        error = init_vglite(data);
        if (error != VG_LITE_SUCCESS)
            break;

        error = init_3rd(data);
        if (error != VG_LITE_SUCCESS)
            break;
    } while (0);

    return error;
}

static vg_lite_error_t terminate_vglite(vg_lite_kernel_terminate_t * data)
{
    vg_lite_kernel_context_t *context = NULL;
#if defined(__linux__) && !defined(EMULATOR)
    vg_lite_kernel_context_t mycontext = {0};
    if (copy_from_user(&mycontext, data->context, sizeof(vg_lite_kernel_context_t)) != 0) {
      return VG_LITE_NO_CONTEXT;
    }
    context = &mycontext;
#else
    context = data->context;
#endif

    /* Free any allocated memory for the context. */
    if (context->command_buffer[0]) {
        /* Free the command buffer. */
        vg_lite_hal_free_contiguous(context->command_buffer[0]);
        context->command_buffer[0] = NULL;
    }

    if (context->command_buffer[1]) {
        /* Free the command buffer. */
        vg_lite_hal_free_contiguous(context->command_buffer[1]);
        context->command_buffer[1] = NULL;
    }

#ifdef BACKUP_COMMAND
    if (global_power_context.power_context) {
        /* Free the power context. */
        vg_lite_hal_free_contiguous(global_power_context.power_context);
        global_power_context.power_context = NULL;
    }
#endif
    if (context->tess_buffer) {
        /* Free the tessellation buffer. */
        vg_lite_hal_free_contiguous(context->tess_buffer);
        context->tess_buffer = NULL;
    }
    vg_lite_hal_free_os_heap();
    /* Decrement reference counter. */
    if (--s_reference == 0) {
        /* Disable the GPU. */
        gpu(0);

        /* De-initialize the SOC. */
        vg_lite_hal_deinitialize();
    }

#if defined(__linux__) && !defined(EMULATOR)
    if (copy_to_user((vg_lite_kernel_context_t  __user *) data->context,
        &mycontext, sizeof(vg_lite_kernel_context_t)) != 0) {
            return VG_LITE_NO_CONTEXT;
    }
#endif
    return VG_LITE_SUCCESS;
}

static vg_lite_error_t terminate_3rd(vg_lite_kernel_terminate_t * data)
{
    /* TODO: Terminate the converters. */

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_terminate(vg_lite_kernel_terminate_t * data)
{
    terminate_vglite(data);
    terminate_3rd(data);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_allocate(vg_lite_kernel_allocate_t * data)
{
    vg_lite_error_t error;
    error = vg_lite_hal_allocate_contiguous(data->bytes, &data->memory, &data->kmemory, &data->memory_gpu, &data->memory_handle);
    return error;
}

static vg_lite_error_t do_free(vg_lite_kernel_free_t * data)
{
    vg_lite_hal_free_contiguous(data->memory_handle);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_submit(vg_lite_kernel_submit_t * data)
{
    uint32_t offset;
    vg_lite_kernel_context_t *context = NULL;
    uint32_t physical = data->context->command_buffer_physical[data->command_id];

#if defined(__linux__) && !defined(EMULATOR)
    vg_lite_kernel_context_t mycontext = { 0 };

    if (copy_from_user(&mycontext, data->context, sizeof(vg_lite_kernel_context_t)) != 0) {
      return VG_LITE_NO_CONTEXT;
    }
    context = &mycontext;
    physical = context->command_buffer_physical[data->command_id];
#else
    context = data->context;
    if (context == NULL)
    {
        return VG_LITE_NO_CONTEXT;
    }
#endif
    /* Perform a memory barrier. */
    vg_lite_hal_barrier();

    offset = (uint8_t *) data->commands - (uint8_t *)context->command_buffer_logical[data->command_id];

#ifdef BACKUP_COMMAND
    backup_power_context_buffer((uint32_t *)((uint8_t *)context->command_buffer_klogical[data->command_id] + offset), (data->command_size + 3) / 4);
#endif
    /* Write the registers to kick off the command execution (CMDBUF_SIZE). */
    vg_lite_hal_poke(VG_LITE_HW_CMDBUF_ADDRESS, physical + offset);
    vg_lite_hal_poke(VG_LITE_HW_CMDBUF_SIZE, (data->command_size + 7) / 8);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_wait(vg_lite_kernel_wait_t * data)
{
    /* Wait for interrupt. */
    if (!vg_lite_hal_wait_interrupt(data->timeout_ms, data->event_mask, &data->event_got)) {
        /* Timeout. */
        return VG_LITE_TIMEOUT;
    }

#if gcFEATURE_VG_FLEXA
    if (data->event_got & FLEXA_TIMEOUT_STATE)
        return VG_LITE_FLEXA_TIME_OUT;

    if (data->event_got & FLEXA_HANDSHEKE_FAIL_STATE)
        return VG_LITE_FLEXA_HANDSHAKE_FAIL;
#endif

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_reset(void)
{
#ifdef BACKUP_COMMAND
    int i = 0;
    vg_lite_kernel_wait_t wait;
    vg_lite_error_t error;

    wait.timeout_ms = 2000;
    wait.event_mask = (uint32_t)~0;
#endif
    /* Disable and enable the GPU. */
    gpu(1);
    vg_lite_hal_poke(VG_LITE_INTR_ENABLE, 0xFFFFFFFF);
    
#ifdef BACKUP_COMMAND
    power_context_klogical[0x00000AD2].command = 0x00000000;
    power_context_klogical[0x00000AD2].data    = 0x00000000;

    printk("after resume and the global_power_context command buffer is : \n");

    for (i = 0; i < 0x00000AD3; i++) {
        if (power_context_klogical[i].command != 0x80000000)
            printk("global_power_context command = %08x, global_power_context data = %08x\n", 
                    power_context_klogical[i].command, power_context_klogical[i].data);
    }

    execute_recovery_command(global_power_context.power_context_physical, 0x00000AD3);

    error = do_wait(&wait);

    printk("execute_recovery_command success!\n");
#endif
    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_gpu_close(void)
{
    gpu(0);

#ifdef BACKUP_COMMAND
    printk("gpu is shutdown!\n");
#endif

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_debug(void)
{
    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_map(vg_lite_kernel_map_t * data)
{
    data->memory_handle = vg_lite_hal_map(data->flags, data->bytes, data->logical, data->physical, data->dma_buf_fd, &data->memory_gpu);
    if (data->memory_handle == NULL)
    {
        return VG_LITE_OUT_OF_RESOURCES;
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_unmap(vg_lite_kernel_unmap_t * data)
{
    vg_lite_hal_unmap(data->memory_handle);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_peek(vg_lite_kernel_info_t * data)
{
    data->reg = vg_lite_hal_peek(data->addr);

    return VG_LITE_SUCCESS;
}

#if gcFEATURE_VG_FLEXA
static vg_lite_error_t do_flexa_enable(vg_lite_kernel_flexa_info_t * data)
{
    /* reset all flexa states */
    vg_lite_hal_poke(0x03600, 0x0);
    /* set sync mode */
    vg_lite_hal_poke(0x03604, data->segment_address);

    vg_lite_hal_poke(0x03608, data->segment_count);

    vg_lite_hal_poke(0x0360C, data->segment_size);

    vg_lite_hal_poke(0x0520, data->sync_mode);

    vg_lite_hal_poke(0x03610, data->stream_id | data->sbi_mode | data->start_flag | data->stop_flag | data->reset_flag);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_flexa_set_background_address(vg_lite_kernel_flexa_info_t * data)
{
    vg_lite_hal_poke(0x03604, data->segment_address);

    vg_lite_hal_poke(0x03608, data->segment_count);

    vg_lite_hal_poke(0x0360C, data->segment_size);

    vg_lite_hal_poke(0x03610, data->stream_id | data->sbi_mode | data->start_flag | data->stop_flag | data->reset_flag);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_flexa_disable(vg_lite_kernel_flexa_info_t * data)
{

    vg_lite_hal_poke(0x0520, data->sync_mode);

    vg_lite_hal_poke(0x03610, data->stream_id | data->sbi_mode);

    /* reset all flexa states */
    vg_lite_hal_poke(0x03600, 0x0);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t do_flexa_stop_frame(vg_lite_kernel_flexa_info_t * data)
{
    vg_lite_hal_poke(0x03610, data->stream_id | data->sbi_mode | data->start_flag | data->stop_flag | data->reset_flag);

    return VG_LITE_SUCCESS;
}
#endif

static vg_lite_error_t do_query_mem(vg_lite_kernel_mem_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    error = vg_lite_hal_query_mem(data);

    return error;
}

static vg_lite_error_t do_map_memory(vg_lite_kernel_map_memory_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    error = vg_lite_hal_map_memory(data);

    return error;
}

static vg_lite_error_t do_unmap_memory(vg_lite_kernel_unmap_memory_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    error = vg_lite_hal_unmap_memory(data);

    return error;
}

static vg_lite_error_t do_cache(vg_lite_kernel_cache_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    error = vg_lite_hal_operation_cache(data->memory_handle, data->cache_op);

    return error;
}

static vg_lite_error_t do_export_memory(vg_lite_kernel_export_memory_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    error = vg_lite_hal_memory_export(&data->fd); 

    return error;
}

static void soft_reset(void)
{
    vg_lite_hw_clock_control_t value;
    value.data = vg_lite_hal_peek(VG_LITE_HW_CLOCK_CONTROL);

    /* Perform a soft reset. */
    value.control.isolate = 1;
    vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
    value.control.soft_reset = 1;
    vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
    vg_lite_hal_delay(5);
    value.control.soft_reset = 0;
    vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
    value.control.isolate = 0;
    vg_lite_hal_poke(VG_LITE_HW_CLOCK_CONTROL, value.data);
}

vg_lite_error_t vg_lite_kernel(vg_lite_kernel_command_t command, void * data)
{
    /* Dispatch on command. */
    switch (command) {
        case VG_LITE_INITIALIZE:
            /* Initialize the context. */
            return do_initialize(data);

        case VG_LITE_TERMINATE:
            /* Terminate the context. */
            return do_terminate(data);

        case VG_LITE_ALLOCATE:
            /* Allocate contiguous memory. */
            return do_allocate(data);

        case VG_LITE_FREE:
            /* Free contiguous memory. */
            return do_free(data);

        case VG_LITE_SUBMIT:
            /* Submit a command buffer. */
            return do_submit(data);

        case VG_LITE_WAIT:
            /* Wait for the GPU. */
            return do_wait(data);

        case VG_LITE_RESET:
            /* Reset the GPU. */
            return do_reset();
            
        case VG_LITE_DEBUG:
            /* Perform debugging features. */
            return do_debug();

        case VG_LITE_MAP:
            /* Map some memory. */
            return do_map(data);

        case VG_LITE_UNMAP:
            /* Unmap some memory. */
            return do_unmap(data);

            /* Get register info. */
        case VG_LITE_CHECK:
            /* Get register value. */
            return do_peek(data);

#if gcFEATURE_VG_FLEXA
        case VG_LITE_FLEXA_DISABLE:
            /* Write register value. */
            return do_flexa_disable(data);

        case VG_LITE_FLEXA_ENABLE:
            /* Write register value. */
            return do_flexa_enable(data);

        case VG_LITE_FLEXA_STOP_FRAME:
            /* Write register value. */
            return do_flexa_stop_frame(data);

        case VG_LITE_FLEXA_SET_BACKGROUND_ADDRESS:
            /* Write register value. */
            return do_flexa_set_background_address(data);
#endif

        case VG_LITE_QUERY_MEM:
            return do_query_mem(data);

        case VG_LITE_MAP_MEMORY:
            /* Map memory to user */
            return do_map_memory(data);

        case VG_LITE_UNMAP_MEMORY:
            /* Unmap memory to user */
            return do_unmap_memory(data);

        case VG_LITE_CLOSE:
            return do_gpu_close();

        case VG_LITE_CACHE:
            return do_cache(data);

        case VG_LITE_EXPORT_MEMORY:
            return do_export_memory(data);

        default:
            break;
    }

    /* Invalid command. */
    return VG_LITE_INVALID_ARGUMENT;
}
