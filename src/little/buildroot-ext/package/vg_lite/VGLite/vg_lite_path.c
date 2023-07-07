/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vg_lite.h"
#include "c908_cache.h"

vg_lite_error_t vg_lite_upload_path(vg_lite_path_t * path)
{
    uint32_t bytes;
    vg_lite_buffer_t Buf, *buffer;
    buffer = &Buf;
    
    /* Compute the number of bytes required for path + command buffer prefix/postfix. */
    bytes = (8 + path->path_length + 7 + 8) & ~7;
    
    /* Allocate GPU memory. */
    buffer->width  = bytes;
    buffer->height = 1;
    buffer->stride = 0;
    buffer->format = VG_LITE_A8;
    if (vg_lite_allocate(buffer) != VG_LITE_SUCCESS) {
        /* Free the vg_lite_buffer structure. */
#if defined(DEBUG) || defined(_DEBUG)
        printf("%s out of memory!\n",__FUNCTION__);
#endif
        return VG_LITE_OUT_OF_MEMORY;
    }
    
    /* Initialize command buffer prefix. */
    ((uint32_t *) buffer->memory)[0] = 0x40000000 | ((path->path_length + 7) / 8);
    ((uint32_t *) buffer->memory)[1] = 0;
    
    /* Copy the path data. */
    memcpy((uint32_t *) buffer->memory + 2, path->path, path->path_length);
    
    /* Initialize command buffer postfix. */
    ((uint32_t *) buffer->memory)[bytes / 4 - 2] = 0x70000000;
    ((uint32_t *) buffer->memory)[bytes / 4 - 1] = 0;

    /* Flush cache. */
    csi_dcache_clean_invalid_range(buffer->memory, bytes);
    
    /* Mark path as uploaded. */
    path->path = buffer->memory;
    path->uploaded.handle = buffer->handle;
    path->uploaded.address = buffer->address;
    path->uploaded.memory = buffer->memory;
    path->uploaded.bytes = bytes;
    path->path_changed = 0;
    VLM_PATH_ENABLE_UPLOAD(*path);      /* Implicitly enable path uploading. */
    
    /* Return pointer to vg_lite_buffer structure. */
    return VG_LITE_SUCCESS;
}

/* Path data operations. */
#define CDALIGN(value, by) (((value) + (by) - 1) & ~((by) - 1))
#define CDMIN(x, y) ((x) > (y) ? (y) : (x))
#define CDMAX(x, y) ((x) > (y) ? (x) : (y))

static int32_t get_data_count(uint8_t cmd)
{
    static int32_t count[] = {
        0,
        0,
        2,
        2,
        2,
        2,
        4,
        4,
        6,
        6
    };
    
    if (cmd > VLC_OP_CUBIC_REL) {
        return -1;
    }
    else {
        return count[cmd];
    }
}

static int32_t get_data_size(vg_lite_format_t format)
{
    int32_t data_size = 0;
    
    switch (format) {
        case VG_LITE_S8:
            data_size = sizeof(int8_t);
            break;
            
        case VG_LITE_S16:
            data_size = sizeof(int16_t);
            break;
            
        case VG_LITE_S32:
            data_size = sizeof(int32_t);
            break;
            
        default:
            data_size = sizeof(vg_lite_float_t);
            break;
    }

    return data_size;
}

int32_t vg_lite_path_calc_length(uint8_t *cmd, uint32_t count, vg_lite_format_t format)
{
    int32_t size = 0;
    int32_t dCount = 0;
    uint32_t i = 0;
    int32_t data_size = 0;
    
    data_size = get_data_size(format);
    
    for (i = 0; i < count; i++) {
        size++;     /* OP CODE. */
        
        dCount = get_data_count(cmd[i]);
        if (dCount > 0) {
            size = CDALIGN(size, data_size);
            size += dCount * data_size;
        }
    }
    
    return size;
}

void vg_lite_path_append(vg_lite_path_t *path,
                            uint8_t        *cmd,
                            void           *data,
                            uint32_t        seg_count)
{
    uint32_t i;
    int32_t j;
    int32_t offset = 0;
    int32_t dataCount = 0;
    float *dataf = (float*) data;
    float *pathf = NULL;
    uint8_t *pathc = NULL;
    int32_t data_size;
    float px = 0.0f, py = 0.0f, cx = 0.0f, cy = 0.0f;
    int rel = 0;
    
    data_size = get_data_size(path->format);
    path->path_changed= 1;
    
    pathf = (float *)path->path;
    pathc = (uint8_t *)path->path;
    
    /* Loop to fill path data. */
    for (i = 0; i < seg_count; i++) {
        *(pathc + offset) = cmd[i];
        offset++;
        
        dataCount = get_data_count(cmd[i]);
        if (dataCount > 0) {
            offset = CDALIGN(offset, data_size);
            pathf = (float *) (pathc + offset);
            
            if ((cmd[i] > VLC_OP_CLOSE) &&
                ((cmd[i] & 0x01) == 1)){
                rel = 1;
            }
            else {
                rel = 0;
            }

            for (j = 0; j < dataCount / 2; j++) {
                pathf[j * 2] = *dataf++;
                pathf[j * 2 + 1] = *dataf++;
                
                if (rel) {
                    cx = px + pathf[j * 2];
                    cy = py + pathf[j * 2 + 1];
                }
                else {
                    cx = pathf[j * 2];
                    cy = pathf[j * 2 + 1];
                }

                /* Update path bounds. */
                path->bounding_box[0] = CDMIN(path->bounding_box[0], cx);
                path->bounding_box[2] = CDMAX(path->bounding_box[2], cx);
                path->bounding_box[1] = CDMIN(path->bounding_box[1], cy);
                path->bounding_box[3] = CDMAX(path->bounding_box[3], cy);
            }
            px = cx;
            py = cy;
            
            offset += dataCount * data_size;
        }
    }

    path->path_length = offset;
}
