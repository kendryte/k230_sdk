/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "vg_lite.h"
#include <stdio.h>
#include <stdint.h>

extern char *error_type[];
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

vg_lite_error_t test_triangle(vg_lite_buffer_t* buffer) {
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    uint8_t path_data[] = {
        2, 0, 0,
        4, 0, 1,
        6, 1, 1, 1, 0,
        4, 0, 0,
    0};
    vg_lite_path_t path = {
        .bounding_box = {0., 1., 1., 0.},
        .quality = VG_LITE_HIGH,
        .format = VG_LITE_S8,
        .uploaded = 0,
        .path_length = sizeof(path_data),
        .path = path_data,
        .path_changed = 1,
        .pdata_internal = 0
    };
    CHECK_ERROR(vg_lite_clear(buffer, NULL, 0xffff0000));
    vg_lite_identity(&matrix);
    vg_lite_translate(buffer->width / 3., buffer->height / 3., &matrix);
    vg_lite_scale(500., 500., &matrix);
    CHECK_ERROR(vg_lite_draw(
        buffer, &path,
        VG_LITE_FILL_NON_ZERO,
        &matrix,
        VG_LITE_BLEND_NONE,
        0xff0000ff
    ));
    CHECK_ERROR(vg_lite_finish());
ErrorHandler:
    return error;
}
