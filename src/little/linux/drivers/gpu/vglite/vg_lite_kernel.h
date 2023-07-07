/****************************************************************************
 *
 *    The MIT License (MIT)
 *
 *    Copyright (c) 2014 - 2020 Vivante Corporation
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
 *    Copyright (C) 2014 - 2020 Vivante Corporation
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

#ifndef _vg_lite_kernel_h_
#define _vg_lite_kernel_h_

/* Interrupt IDs from GPU. */
#define EVENT_UNEXPECTED_MESH  0x80000000
#define EVENT_CMD_BAD_WRITE    0x40000000
#define EVENT_ERROR_RECOVER    0x20000000
#define EVENT_CMD_SWITCH       0x10000000
#define EVENT_MCU_BAD_WRITE    0x08000000
#define EVENT_END              0

#define MAX_CONTIGUOUS_SIZE 0x01000000

#define VG_LITE_INFINITE    0xFFFFFFFF
#define CMDBUF_COUNT        2

#define VG_LITE_ALIGN(number, align_bytes)    \
        (((number) + ((align_bytes) - 1)) & ~((align_bytes) - 1))
#define VG_LITE_PAD(number, align_bytes) \
        ((number) + (((align_bytes) - (number) % (align_bytes)) % (align_bytes)))

#ifndef BIT
#define BIT(x)                 (1 << (x))
#endif

#define VG_LITE_KERNEL_IS_GPU_IDLE() \
((vg_lite_hal_peek(VG_LITE_HW_IDLE) & VG_LITE_HW_IDLE_STATE) == VG_LITE_HW_IDLE_STATE)

/* Hardware chip Ids */
#define GPU_CHIP_ID_GCNANOLITEV         0x255
#define GPU_CHIP_ID_GC355               0x355
#define GPU_CHIP_ID_GCNANOULTRAV        0x265

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VG_LITE_ERROR
#define VG_LITE_ERROR  1
/*!
 *    @abstract Error codes that the vg_lite functions can return.
 *
 *    @discussion
 *    All API functions return a status code. On success, <code>VG_LITE_SUCCESS</code> will be returned when a function is
 *    successful. This value is set to zero, so if any function returns a non-zero value, an error has occurred.
 */
typedef enum vg_lite_error {
    VG_LITE_SUCCESS = 0,        /*! Success. */
    VG_LITE_INVALID_ARGUMENT,   /*! An invalid argument was specified. */
    VG_LITE_OUT_OF_MEMORY,      /*! Out of memory. */
    VG_LITE_NO_CONTEXT,         /*! No context or an uninitialized context specified. */
    VG_LITE_TIMEOUT,            /*! A timeout has occurred during a wait. */
    VG_LITE_OUT_OF_RESOURCES,   /*! Out of system resources. */
    VG_LITE_GENERIC_IO,         /*! Cannot communicate with the kernel driver. */
    VG_LITE_NOT_SUPPORT,        /*! Function call not supported. */
    VG_LITE_ALREADY_EXISTS,     /*! Object already exists */
    VG_LITE_NOT_ALIGNED,        /*! Data alignment error */
    VG_LITE_FLEXA_TIME_OUT,     /*! VG timeout requesting for segment buffer */
    VG_LITE_FLEXA_HANDSHAKE_FAIL,/*! VG and SBI synchronizer handshake failed */
} vg_lite_error_t;

#endif

typedef enum vg_lite_kernel_counter {
    /* Dont't touch the counter. */
    VG_LITE_NONE,

    /* Turn the counter on. */
    VG_LITE_ON,

    /* Turn the counter off. */
    VG_LITE_OFF,

    /* Query the counter and reset its values. */
    VG_LITE_QUERY,
} vg_lite_kernel_counter_t;

typedef enum vg_lite_kernel_command {
    /* Initialize the GPU. */
    VG_LITE_INITIALIZE,

    /* Terminate the GPU. */
    VG_LITE_TERMINATE,

    /* Allocate memory. */
    VG_LITE_ALLOCATE,

    /* Free memory. */
    VG_LITE_FREE,

    /* Submit a command buffer to the GPU. */
    VG_LITE_SUBMIT,

    /* Wait for the GPU to be completed. */
    VG_LITE_WAIT,

    /* Reset the GPU. */
    VG_LITE_RESET,

    /* Debug commands. */
    VG_LITE_DEBUG,

    /* Map memory. */
    VG_LITE_MAP,

    /* Unmap memory. */
    VG_LITE_UNMAP,

    /* Check info. */
    VG_LITE_CHECK,

    /* Query mem. */
    VG_LITE_QUERY_MEM,

    /* Flexa disable */
    VG_LITE_FLEXA_DISABLE,

    /* Flexa enable */
    VG_LITE_FLEXA_ENABLE,

    /* Flexa stop frame */
    VG_LITE_FLEXA_STOP_FRAME,

    /* Set background address */
    VG_LITE_FLEXA_SET_BACKGROUND_ADDRESS,

    /* Map memory to user */
    VG_LITE_MAP_MEMORY,

    /* Unmap memory to user */
    VG_LITE_UNMAP_MEMORY,

    /* DMA-buf handler */
    VG_LITE_BUFFER_FROM_DMA_BUF
} vg_lite_kernel_command_t;

struct vg_lite_kernel_context {
    /* Command buffer. */
    void      *command_buffer[CMDBUF_COUNT];
    void      *command_buffer_logical[CMDBUF_COUNT];
    unsigned int    command_buffer_physical[CMDBUF_COUNT];

    /* Tessellation buffer. */
    void      *tessellation_buffer;
    void      *tessellation_buffer_logical;
    unsigned int    tessellation_buffer_physical;
};

/* Context structure. */
typedef struct vg_lite_kernel_context vg_lite_kernel_context_t;

typedef struct capabilities {
    unsigned int tiled : 2;
    unsigned int l2_cache : 1;
} capabilities_t;

typedef union vg_lite_capabilities {
    capabilities_t cap;
    unsigned int       data;
} vg_lite_capabilities_t;

typedef struct vg_lite_kernel_initialize {
    /* Command buffer size. */
    unsigned int command_buffer_size;

    /* Tessellation buffer width. */
    int tessellation_width;

    /* Tessellation buffer height. */
    int tessellation_height;

    /* OUTPUT */

    /* Context pointer. */
    vg_lite_kernel_context_t *context;

    /* Capabilities. */
    vg_lite_capabilities_t capabilities;

    /* Allocated command buffer. */
    void *command_buffer[CMDBUF_COUNT];

    /* GPU address for command buffer. */
    unsigned int command_buffer_gpu[CMDBUF_COUNT];

    /* GPU addresses for tesselation buffers. */
    unsigned int tessellation_buffer_gpu;

    /* Logic addresses for tessellation buffers: used by SW Tessellator. */
    unsigned char *tessellation_buffer_logic;

    /* Size of each level of the tesselation buffer. */
    unsigned int tessellation_buffer_size;

    /* Size of each level of the vg count buffer. */
    unsigned int vg_count_buffer_size;

    /* Width and height of tessellation buffer. */
    unsigned int tessellation_width_height;
} vg_lite_kernel_initialize_t;

typedef struct vg_lite_kernel_terminate {
    /* Context to terminate. */
    vg_lite_kernel_context_t *context;
} vg_lite_kernel_terminate_t;

typedef struct vg_lite_kernel_allocate {
    /* Number of bytes to allocate. */
    unsigned int bytes;

    /* Flag to indicate whether the allocated memory is contiguous or not. */
    int contiguous;

    /* OUTPUT */

    /* Memory handle. */
    void *memory_handle;

    /* Allocated memory. */
    void *memory;

    /* GPU address of allocated memory. */
    unsigned int memory_gpu;
} vg_lite_kernel_allocate_t;

typedef struct vg_lite_kernel_free {
    /* Memory handle to free. */
    void *memory_handle;
} vg_lite_kernel_free_t;

typedef struct vg_lite_kernel_submit {
    /* Context to submit to. */
    vg_lite_kernel_context_t *context;

    /* Pointer to command buffer. */
    void *commands;

    /* Number of bytes in command buffer. */
    unsigned int command_size;

    /* Command Buffer ID. */
    unsigned int command_id;
} vg_lite_kernel_submit_t;

typedef struct vg_lite_kernel_wait {
    /* Context to wait for. */
    vg_lite_kernel_context_t *context;

    /* Timeout in milliseconds. */
    unsigned int timeout_ms;

    /* The event to wait. */
    unsigned int event_mask;

    /* The event(s) got after waiting. */
    unsigned int event_got;
} vg_lite_kernel_wait_t;

typedef struct vg_lite_kernel_reset {
    /* Context to reset. */
    vg_lite_kernel_context_t *context;
} vg_lite_kernel_reset_t;

typedef struct vg_lite_kernel_debug {
    /* Context to debug. */
    vg_lite_kernel_context_t *context;

    /* Bandwidth counter enabler. */
    vg_lite_kernel_counter_t bandwidth_counter;

    /* Pixel counter enabler. */
    vg_lite_kernel_counter_t pixel_counters;

    /* OUTPUT */

    /* Bandwidth counters:
     *  [0] - burst of 8.
     *  [1] - burst of 16.
     *  [2] - burst of 32.
     *  [3] - burst of 64.
     */
    unsigned int bandwidth[4];

    /* Pixel counters:.
     *  [0] - Number of tessellated pixels.
     *  [1] - Number of imaged pixels.
     *  [2] - Number of rendered pixels.
     */
    unsigned int pixels[3];
} vg_lite_kernel_debug_t;

typedef struct vg_lite_kernel_map {
    /* Number of bytes to map. */
    unsigned int bytes;

    /* Logical memory address or NULL. */
    void *logical;

    /* Physical memory address or 0. */
    unsigned int physical;

    /* OUTPUT */

    /* Memory handle for mapped memory. */
    void *memory_handle;

    /* GPU address of mapped memory. */
    unsigned int memory_gpu;
} vg_lite_kernel_map_t;

typedef struct vg_lite_kernel_unmap {
    /* Memory handle to unmap. */
    void *memory_handle;
} vg_lite_kernel_unmap_t;

typedef struct vg_lite_kernel_info {
    /* Register's address. */
    unsigned int addr;

    /* Check register info. */
    unsigned int reg;
} vg_lite_kernel_info_t;

typedef struct vg_lite_kernel_flexa_info {
    unsigned int                    sbi_mode;
    unsigned int                    sync_mode;
    unsigned int                    flexa_mode;
    unsigned int                    stream_id;
    unsigned int                    segment_buffer_address;
    unsigned int                    segment_count;
    unsigned int                    segment_size;
    unsigned int                    stop_flag;
    unsigned int                    start_flag;
    unsigned int                    reset_flag;
} vg_lite_kernel_flexa_info_t;

typedef struct vg_lite_kernel_mem {
    unsigned int bytes;
} vg_lite_kernel_mem_t;

typedef struct vg_lite_kernel_map_memory {
    /* Number of bytes to map. */
    unsigned int bytes;

    /* Physical memory address. */
    unsigned int physical;

    /* Logical memory address. */
    void *logical;
} vg_lite_kernel_map_memory_t;

typedef struct vg_lite_kernel_unmap_memory {
    /* Number of bytes to map. */
    unsigned int bytes;

    /* Logical memory address. */
    void *logical;
} vg_lite_kernel_unmap_memory_t;

vg_lite_error_t vg_lite_kernel(vg_lite_kernel_command_t command, void *data);

#ifdef __cplusplus
}
#endif
#endif /* _vg_lite_kernel_h_ */
