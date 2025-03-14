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
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <vg_lite.h>
#include "../sample_vo/vo_test_case.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "mpi_connector_api.h"
#include "mpi_vo_api.h"
#include "mpi_vb_api.h"

#define BUFFER_NUM 4

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

static void sig_handler(int x) {
    printf("receive signal %d, exit\n", x);
    flag_exit = 0;
}

long timeval_delta(const struct timeval* start, const struct timeval* end) {
    return 1000000 * (end->tv_sec - start->tv_sec) + end->tv_usec - start->tv_usec;
}

static int vo_display(vg_lite_buffer_t* buffer) {
    k_vo_osd osd = K_VO_OSD1;
    k_vb_blk_handle handle = kd_mpi_vb_phyaddr_to_handle(buffer->address);
    k_video_frame_info vf_info = {
        .mod_id = K_ID_VO,
        .pool_id = kd_mpi_vb_handle_to_pool_id(handle),
        .v_frame = {
            .phys_addr[0] = buffer->address,
            .phys_addr[1] = buffer->address,
            .phys_addr[2] = buffer->address,
            .width = buffer->width,
            .height = buffer->height,
            .pixel_format = PIXEL_FORMAT_ARGB_8888
        }
    };
    return kd_mpi_vo_chn_insert_frame(osd + 3, &vf_info);
}

static vg_lite_buffer_t buffers[BUFFER_NUM];

static void vo_exit(void) {
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        kd_mpi_vb_release_block(kd_mpi_vb_phyaddr_to_handle(buffers[i].address));
    }
    kd_mpi_vo_osd_disable(K_VO_OSD1);
    kd_mpi_vo_disable();
    usleep(50000);
    kd_mpi_vb_exit();
}

static int vo_init(k_connector_type connector_type) {
    k_s32 ret = 0;
    k_s32 connector_fd;
    k_connector_info connector_info;
    k_vo_osd osd = K_VO_OSD1;

    memset(&connector_info, 0, sizeof(k_connector_info));

    //connector get sensor info
    ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    connector_fd = kd_mpi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return K_ERR_VO_NOTREADY;
    }

    // set connect power
    kd_mpi_connector_power_set(connector_fd, 1);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);
    unsigned width = connector_info.resolution.hdisplay, height = connector_info.resolution.vdisplay;
    uint32_t size = width * height * 4;

    // layer
    osd_info info = {
        .act_size = {
            .width = width,
            .height = height,
        },
        .format = PIXEL_FORMAT_ARGB_8888,
        .offset = {
            (connector_info.resolution.hdisplay - width) / 2,
            (connector_info.resolution.vdisplay - height) / 2,
        },
        .global_alptha = 0xff
    };
    vo_creat_osd_test(osd, &info);

    // vb
    k_vb_config vb_config;
    memset(&vb_config, 0, sizeof(k_vb_config));
    vb_config.max_pool_cnt = 64;
    vb_config.comm_pool[0].blk_cnt = BUFFER_NUM + 2;
    vb_config.comm_pool[0].blk_size = size;
    vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    ret = kd_mpi_vb_set_config(&vb_config);
    if (ret) {
        return -1;
    }
    k_vb_supplement_config supplement_config;
    memset(&supplement_config, 0, sizeof(k_vb_supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    kd_mpi_vb_set_supplement_config(&supplement_config);
    if (ret) {
        return -1;
    }
    ret = kd_mpi_vb_init();
    if (ret) {
        return -1;
    }

    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        k_vb_blk_handle handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, size, NULL);
        buffers[i].address = kd_mpi_vb_handle_to_phyaddr(handle);
        buffers[i].width = width;
        buffers[i].height = height;
        buffers[i].format = VG_LITE_ARGB8888;
        buffers[i].stride = buffers[i].width * 4;

    }

    return kd_mpi_vo_enable();
}

int main(int argc, char* argv[]) {
    vg_lite_error_t error = VG_LITE_SUCCESS;
    k_connector_type connector_type = LT9611_MIPI_4LAN_1920X1080_60FPS;

    if (argc >= 2) {
        connector_type = atoi(argv[1]);
    }
    memset(buffers, 0, sizeof(buffers));
    if (vo_init(connector_type) != 0) {
        printf("vo init failed\n");
        return -1;
    }
    unsigned width = buffers[0].width, height = buffers[0].height;
    printf("display %ux%u\n", width, height);

    // Initialize the blitter.
    CHECK_ERROR(vg_lite_init(width, height));

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    unsigned buffer_ptr = 0;
    vg_lite_buffer_t* fb = &buffers[buffer_ptr];
    vg_lite_color_t background_color = 0xFFFFFFFFUL, line_color = 0xFF349bebUL;

    // Clear the buffer with black.
    CHECK_ERROR(vg_lite_clear(fb, NULL, background_color));
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
        model_view_matrix[2][3] = -15;

        glMatrix_mat4_multiply(mvp, projection_matrix, model_view_matrix);

        // Clear the buffer with black.
        CHECK_ERROR(vg_lite_clear(fb, NULL, background_color));
#define GOOD 1
#if GOOD
        CHECK_ERROR(vg_lite_draw_line_strip_3d(
            fb, line_color, mvp,
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
        static uint64_t cnt = 0;
        cnt += 1;
        printf("submit: %lu\n", cnt);
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
        vo_display(fb);
        buffer_ptr += 1;
        buffer_ptr = buffer_ptr % BUFFER_NUM;
        fb = &buffers[buffer_ptr];
        usleep(17000);
    }

ErrorHandler:
    // Cleanup.
    vo_exit();
    cleanup();
    return error;
}
