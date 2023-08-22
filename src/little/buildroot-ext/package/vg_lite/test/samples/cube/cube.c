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
#include "glMatrix.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <vg_lite.h>
#include <unistd.h>

void drm_wait_vsync(void);
void drm_init(void);
void drm_exit(void);
int drm_display(unsigned index);
int drm_get_dmabuf_fd(unsigned index);
void drm_get_resolution(unsigned* width, unsigned* height);
void* drm_get_map(unsigned index);

#define __func__ __FUNCTION__
char *error_type[] = {
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

vg_lite_error_t vg_lite_draw_line(vg_lite_buffer_t* target, vg_lite_color_t color, float x, float y, float tx, float ty, float stroke_width) {
    static int8_t path_data[] = {
        2, 0, 1,
        4, 1, 1,
        4, 1, -1,
        4, 0, -1,
        0
    };
    static vg_lite_path_t path = {
        .bounding_box={0, 1, 1, -1},
        .quality=VG_LITE_HIGH,
        .format=VG_LITE_S8,
        .uploaded={0},
        .path_length=sizeof(path_data),
        .path=path_data,
        .path_changed=1,
        .pdata_internal=0
    };
    const float w = target->width / 2. * (tx - x);
    const float h = target->height / 2. * (ty - y);
    const float length = sqrtf(w * w + h * h);
    const float angle = atan2f(h, w) * 180 / M_PI;

    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    vg_lite_translate((1 + x) * target->width / 2, (1 - y) * target->height / 2., &matrix); // position
    vg_lite_rotate(-angle, &matrix); // angle
    vg_lite_scale(length, stroke_width / 2., &matrix); // length

    return vg_lite_draw(target, &path, VG_LITE_FILL_NON_ZERO, &matrix, VG_LITE_BLEND_NONE, color);
}

vg_lite_error_t vg_lite_draw_line_3d(vg_lite_buffer_t* target, vg_lite_color_t color, mat4 mvp, vec4 p0, vec4 p1, float width) {
    vec4 p0_n;
    vec4 p1_n;
    glMatrix_vec4_transform(p0_n, mvp, p0);
    glMatrix_vec4_transform(p1_n, mvp, p1);
    return vg_lite_draw_line(target, color, p0_n[0] / p0_n[3], p0_n[1] / p0_n[3], p1_n[0] / p1_n[3], p1_n[1] / p1_n[3], width);
}

vg_lite_error_t vg_lite_draw_line_strip_3d(vg_lite_buffer_t* target, vg_lite_color_t color, mat4 mvp, vec4 ps[], unsigned num_p, unsigned strip[], unsigned num_strip, float width) {
    vec4 ps_n[num_p];
    for (unsigned i = 0; i < num_p; i++) {
        glMatrix_vec4_transform(ps_n[i], mvp, ps[i]);
        ps_n[i][0] /= ps_n[i][3];
        ps_n[i][1] /= ps_n[i][3];
    }
    vg_lite_error_t error;
    for (unsigned i = 1; i < num_strip; i++) {
        error = vg_lite_draw_line(target, color,
            ps_n[strip[i - 1]][0], ps_n[strip[i - 1]][1],
            ps_n[strip[i]][0], ps_n[strip[i]][1],
            width
        );
        if (error != VG_LITE_SUCCESS) {
            return error;
        }
        #define DRAW_DIRECTLY 0
        #if DRAW_DIRECTLY
        error = vg_lite_finish();
        if (error != VG_LITE_SUCCESS) {
            return error;
        }
        #endif
    }
    return VG_LITE_SUCCESS;
}

unsigned flag_exit = 1;

void sig_handler(int x) {
    printf("receive signal %d, exit", x);
    flag_exit = 0;
}

long timeval_delta(const struct timeval* start, const struct timeval* end) {
    return 1000000 * (end->tv_sec - start->tv_sec) + end->tv_usec - start->tv_usec;
}

int main(int argc, char* argv[]) {
    float x = -1, y = 1, tx = 0.5, ty = -1;
    x = atof(argv[1]);
    y = atof(argv[2]);
    tx = atof(argv[3]);
    ty = atof(argv[4]);
    vg_lite_error_t error = VG_LITE_SUCCESS;

    drm_init();
    atexit(drm_exit);

    int buf_fd[2];
    buf_fd[0] = drm_get_dmabuf_fd(0);
    if (buf_fd[0] < 0) {
        perror("get fd");
        return buf_fd[0];
    }
    buf_fd[1] = drm_get_dmabuf_fd(1);
    if (buf_fd[1] < 0) {
        perror("get fd");
        return buf_fd[1];
    }

    unsigned width, height;
    drm_get_resolution(&width, &height);
    printf("buffer: %ux%u\n", width, height);

    // Initialize the blitter.
    CHECK_ERROR(vg_lite_init(width, height));

    // build vg_lite_buffer
    vg_lite_buffer_t buffers[2];
    memset(buffers, 0, sizeof(buffers));
    buffers[0].width = width;
    buffers[0].height = height;
    buffers[0].format = VG_LITE_BGRA8888;
    buffers[0].stride = buffers[0].width * 4;
    buffers[0].memory = drm_get_map(0);
    CHECK_ERROR(vg_lite_map(&buffers[0], VG_LITE_MAP_DMABUF, buf_fd[0]));
    buffers[1].width = width;
    buffers[1].height = height;
    buffers[1].format = VG_LITE_BGRA8888;
    buffers[1].stride = buffers[1].width * 4;
    buffers[1].memory = drm_get_map(1);
    CHECK_ERROR(vg_lite_map(&buffers[1], VG_LITE_MAP_DMABUF, buf_fd[1]));

    unsigned buffer_ptr = 0;
    vg_lite_buffer_t* fb = &buffers[buffer_ptr];

    // Clear the buffer with black.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFF000000UL));
    CHECK_ERROR(vg_lite_finish());

    // Draw spining cube
    mat4 projection_matrix;
    glMatrix_mat4_perspective(projection_matrix, 90. * M_PI / 180., (float)width / height, 0.1, 100);
    double rx = 0.f, ry = 0.f, rz = 0.f;
    double rxs = 0.f, rys = 0.f, rzs = 0.f;
    const double k = 5. / 10000000.;

    struct timeval last, last_period = {.tv_sec = 0, .tv_usec = 0};
    gettimeofday(&last, NULL);
    srand(last.tv_sec);

    const uint64_t period = 3000000;

    while (flag_exit) {
        // MVP
        mat4 model_view_matrix, mvp;
#define COUNT_TIME 0
#if COUNT_TIME
        struct timeval tv1;
        gettimeofday(&tv1, NULL);
#endif

        glMatrix_mat4_identity(model_view_matrix);
        glMatrix_mat4_rotateX(model_view_matrix, model_view_matrix, rx);
        glMatrix_mat4_rotateY(model_view_matrix, model_view_matrix, ry);
        glMatrix_mat4_rotateZ(model_view_matrix, model_view_matrix, rz);
        model_view_matrix[2][3] = -20;

        glMatrix_mat4_multiply(mvp, projection_matrix, model_view_matrix);

        // Clear the buffer with black.
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFF000000));
#define GOOD 1
#if GOOD
        CHECK_ERROR(vg_lite_draw_line_strip_3d(
            fb, 0xFFFFFFFFUL, mvp,
            (vec4[8]){
                {1, 1, -1, 1},
                {1, 1, 1, 1},
                {-1, 1, -1, 1},
                {-1, 1, 1, 1},

                {1, -1, -1, 1},
                {1, -1, 1, 1},
                {-1, -1, -1, 1},
                {-1, -1, 1, 1},
            }, 8,
            (unsigned[17]){0, 4, 7, 6, 5, 7, 3, 2, 6, 4, 5, 1, 3, 0, 2, 1, 0},
            17, 5
        ));
#else
        vec4 a = {-1, -1, -1, 1};
        vec4 b = {1, 1, 1, 1};
        CHECK_ERROR(vg_lite_draw_line(fb, 0xFFFFFFFFU, x, y, tx, ty, 3));
        // CHECK_ERROR(vg_lite_draw_line_3d(fb, 0xFFFFFFFFU, mvp, a, b, 5));
#endif
#if COUNT_TIME
        struct timeval tv2;
        gettimeofday(&tv2, NULL);
#endif
        CHECK_ERROR(vg_lite_finish());

        // animation
        struct timeval current;
        gettimeofday(&current, NULL);
#if COUNT_TIME
        printf(
            "total: %ld us, cpu: %ld us, gpu: %ld us\n",
            timeval_delta(&tv1, &current),
            timeval_delta(&tv1, &tv2),
            timeval_delta(&tv2, &current)
        );
#endif
        uint64_t delta = 1000000 * (current.tv_sec - last.tv_sec) + current.tv_usec - last.tv_usec;
        last = current;
        rx += rxs * delta;
        ry += rys * delta;
        rz += rzs * delta;
        if (1000000 * (current.tv_sec - last_period.tv_sec) + current.tv_usec - last_period.tv_usec > period) {
            // change direction
            const double theta = (double)RAND_MAX / rand() * 2. * M_PI;
            const double phi = (double)RAND_MAX / rand() * M_PI;
            rxs = sin(phi) * cos(theta) * k;
            rys = sin(phi) * sin(theta) * k;
            rzs = cos(phi) * k;

            last_period = current;
        }

        // ping-pong
        drm_display(buffer_ptr);
        buffer_ptr ^= 1;
        fb = &buffers[buffer_ptr];
    }

ErrorHandler:
    // Cleanup.
    cleanup();
    return error;
}
