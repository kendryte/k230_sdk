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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

void drm_wait_vsync(void);
void drm_init(void);
void drm_exit(void);
int drm_display(unsigned index);
int drm_get_dmabuf_fd(unsigned index);
void drm_get_resolution(unsigned* width, unsigned* height);
void* drm_get_map(unsigned index);
vg_lite_error_t test_triangle(vg_lite_buffer_t* buffer);

#define __func__ __FUNCTION__
char *error_type[] = 
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",      
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

static void cleanup(void)
{
    vg_lite_close();
}

union RGBA32 {
    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    } bits;
    uint32_t value;
} color;

int main(int argc, const char * argv[]) {
    /* Initialize vglite. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    unsigned width, height;
    vg_lite_buffer_t buffer;

    color.value = 0xff1080ff;
    if (argc >= 2) {
        color.bits.red = atoi(argv[1]);
    }
    if (argc >= 3) {
        color.bits.green = atoi(argv[2]);
    }
    if (argc >= 4) {
        color.bits.blue = atoi(argv[3]);
    }
    if (argc >= 5) {
        color.bits.alpha = atoi(argv[4]);
    }
    printf("red(%u) green(%u) blue(%u) alpha(%u)\n",
        color.bits.red, color.bits.green, color.bits.blue, color.bits.alpha
    );

    drm_init();
    atexit(drm_exit);
    int buf_fd = drm_get_dmabuf_fd(0);
    if (buf_fd < 0) {
        perror("get fd");
        return buf_fd;
    }
    drm_get_resolution(&width, &height);
    CHECK_ERROR(vg_lite_init(width, height));

    // build vg_lite_buffer
    memset(&buffer, 0, sizeof(buffer));
    buffer.width = width;
    buffer.height = height;
    buffer.format = VG_LITE_BGRA8888;
    buffer.stride = buffer.width * 4;
    buffer.memory = drm_get_map(0);
    CHECK_ERROR(vg_lite_map(&buffer, VG_LITE_MAP_DMABUF, buf_fd));
    printf("buffer phys: %08x\n", buffer.address);

    CHECK_ERROR(vg_lite_clear(&buffer, NULL, 0xffffffffU));
    CHECK_ERROR(vg_lite_clear(&buffer, &(vg_lite_rectangle_t){100, 200, 300, 200}, color.value));
    CHECK_ERROR(vg_lite_finish());

    printf("display: %d\n", drm_display(0));
    getchar();
    test_triangle(&buffer);
    getchar();

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
