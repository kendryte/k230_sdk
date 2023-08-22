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

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "vg_lite_kernel.h"
#include "vg_lite_ioctl.h"

static int device = 0;
static uint32_t length = 0;
static void * mapped = NULL;

vg_lite_error_t vg_lite_kernel(vg_lite_kernel_command_t command, void * data)
{
    struct ioctl_data to_kernel;
    static const uint32_t bytes[] = {
        sizeof(vg_lite_kernel_initialize_t),
        sizeof(vg_lite_kernel_terminate_t),
        sizeof(vg_lite_kernel_allocate_t),
        sizeof(vg_lite_kernel_free_t),
        sizeof(vg_lite_kernel_submit_t),
        sizeof(vg_lite_kernel_wait_t),
        sizeof(vg_lite_kernel_reset_t),
        sizeof(vg_lite_kernel_debug_t),
        sizeof(vg_lite_kernel_map_t),
        sizeof(vg_lite_kernel_unmap_t),
        sizeof(vg_lite_kernel_info_t),
        sizeof(vg_lite_kernel_mem_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_map_memory_t),
        sizeof(vg_lite_kernel_unmap_memory_t),
        sizeof(vg_lite_kernel_close_t),
        sizeof(vg_lite_kernel_cache_t),
        sizeof(vg_lite_kernel_export_memory_t),
    };

    if (device == 0) {
        device = open("/dev/vg_lite", O_RDWR);
        if (device == -1)
            return VG_LITE_GENERIC_IO;

        /* Default contiguous mapping size. */
        length = MAX_CONTIGUOUS_SIZE;

        mapped = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, device, 0);
        if ((mapped == NULL) ||
            (mapped == (char *)-1)) {
            close(device);
            device = 0;
            return VG_LITE_GENERIC_IO;
        }
    }

    to_kernel.command = command;
    to_kernel.buffer = data;
    to_kernel.bytes = bytes[command];

    if (ioctl(device, VG_LITE_IOCTL, &to_kernel) < 0)
        return VG_LITE_GENERIC_IO;

    return to_kernel.error;
}
