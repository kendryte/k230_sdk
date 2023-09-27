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

/**
 * @file lv_port_disp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "buf_mgt.hpp"
#include "disp.h"
#include "lv_port.h"
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/
#define DISP_HOR_RES 1080
#define DISP_VER_RES 1920/2
#define display_width DISP_HOR_RES
#define display_height DISP_VER_RES

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                       lv_color_t *color_p);
static void *thread_drm_vsync(void *arg);
extern void input_map_set(int index);

/**********************
 *  STATIC VARIABLES
 **********************/
static struct drm_dev drm_dev;
static uint32_t screen_width, screen_height;

#define DRM_UI_BUF_USE_PLANE 2

#define DRM_UI_BUF_COUNT 2

#define DRM_BUF_COUNT                                                          \
    (DRM_UI_BUF_COUNT)

#define DRM_UI_BUF_SRART_IDX 0
#define DRM_UI_BUF_END_IDX (DRM_UI_BUF_SRART_IDX + DRM_UI_BUF_COUNT)

static struct drm_buffer drm_bufs[DRM_BUF_COUNT];
static lv_color_t lvgl_buf[DISP_HOR_RES * DISP_VER_RES];
static buf_mgt_t ui_buf_mgt;

/**********************
 *      MACROS
 **********************/
#define DRM_DEV_NAME_DEFAULT "/dev/dri/card0"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    if (disp_init())
        return;
    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/
    static lv_disp_draw_buf_t draw_buf_dsc;
    lv_color_t *draw_buf = (lv_color_t *)malloc(display_width * display_height /
                           2 * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&draw_buf_dsc, draw_buf, NULL,
                          display_width * display_height /
                          2); /*Initialize the display buffer*/
    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/
    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = display_width;
    disp_drv.ver_res = display_height;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc;
    disp_drv.screen_transp = 1;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void drm_wait_vsync(struct drm_dev *dev)
{
    static drmEventContext drm_event_ctx;
    int ret;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(dev->fd, &fds);

    do
    {
        ret = select(dev->fd + 1, &fds, NULL, NULL, NULL);
    }
    while (ret == -1 && errno == EINTR);

    if (ret < 0)
    {
        fprintf(stderr, "select failed: %s\n", strerror(errno));
        return;
    }

    if (FD_ISSET(dev->fd, &fds))
    {
        drmHandleEvent(dev->fd, &drm_event_ctx);
        dev->pflip_pending = false;
    }
}

static int plane_config(struct drm_dev *dev, struct drm_buffer *pbuf_ui)
{
    drmModeAtomicReq *req;
    struct drm_object *obj;
    struct drm_buffer *buf;
    uint32_t flags;
    int ret;

    if ((ret = drmModeCreatePropertyBlob(dev->fd, &dev->mode, sizeof(dev->mode),
                                         &dev->mode_blob_id)) != 0)
    {
        fprintf(stderr, "couldn't create a blob property\n");
        return ret;
    }

    req = drmModeAtomicAlloc();
    /* set id of the CRTC id that the connector is using */
    obj = &dev->conn;
    if ((ret = drm_set_object_property(req, obj, "CRTC_ID", dev->crtc_id)) < 0)
        goto err;

    /* set the mode id of the CRTC; this property receives the id of a blob
     * property that holds the struct that actually contains the mode info */
    obj = &dev->crtc;
    if ((ret = drm_set_object_property(req, obj, "MODE_ID",
                                       dev->mode_blob_id)) < 0)
        goto err;

    /* set the CRTC object as active */
    if ((ret = drm_set_object_property(req, obj, "ACTIVE", 1)) < 0)
        goto err;

    /* set properties of the plane related to the CRTC and the framebuffer */
    obj = &dev->planes[DRM_UI_BUF_USE_PLANE];
    buf = pbuf_ui;
    if ((ret = drm_set_object_property(req, obj, "FB_ID", buf->fb)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "CRTC_ID", dev->crtc_id)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "SRC_X", 0)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "SRC_Y", 0)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "SRC_W", buf->width << 16)) <
            0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "SRC_H", buf->height << 16)) <
            0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "CRTC_X", buf->offset_x)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "CRTC_Y", buf->offset_y)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "CRTC_W", buf->width)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "CRTC_H", buf->height)) < 0)
        goto err;

    flags = DRM_MODE_ATOMIC_TEST_ONLY | DRM_MODE_ATOMIC_ALLOW_MODESET;
    if ((ret = drmModeAtomicCommit(dev->fd, req, flags, NULL)) < 0)
    {
        fprintf(stderr, "test-only atomic commit failed, %d\n", errno);
        goto err;
    }

    flags = DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_PAGE_FLIP_EVENT;
    if ((ret = drmModeAtomicCommit(dev->fd, req, flags, NULL)) < 0)
    {
        fprintf(stderr, "atomic commit failed, %d\n", errno);
        goto err;
    }

    dev->pflip_pending = true;
err:
    drmModeAtomicFree(req);

    return ret;
}

static int plane_update(struct drm_dev *dev, struct drm_buffer *pbuf_ui)
{
    drmModeAtomicReq *req;
    struct drm_object *obj;
    uint32_t fb;
    uint32_t flags;
    int ret;

    req = drmModeAtomicAlloc();
    /* set properties of the plane related to the CRTC and the framebuffer */
    obj = &dev->planes[DRM_UI_BUF_USE_PLANE];
    fb = pbuf_ui ? pbuf_ui->fb : 0;
    if ((ret = drm_set_object_property(req, obj, "FB_ID", fb)) < 0)
        goto err;
    if ((ret = drm_set_object_property(req, obj, "CRTC_ID",
                                       fb ? dev->crtc_id : 0)) < 0)
        goto err;

    flags = DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_PAGE_FLIP_EVENT;
    if ((ret = drmModeAtomicCommit(dev->fd, req, flags, NULL)) < 0)
    {
        fprintf(stderr, "atomic commit failed, %d\n", errno);
        goto err;
    }

    dev->pflip_pending = true;
err:
    drmModeAtomicFree(req);

    return ret;
}

static int alloc_drm_buff(void)
{
    int fd = drm_dev.fd;

    for (int i = 0; i < DRM_BUF_COUNT; i++)
    {
        if (drm_create_fb(fd, &drm_bufs[i]))
        {
            fprintf(stderr, "couldn't create buffer %u\n", i);
            for (int ii = 0; ii < i; ii++)
                drm_destroy_fb(fd, &drm_bufs[ii]);
            return -1;
        }
    }

    return 0;
}

static void free_drm_buff(void)
{
    int fd = drm_dev.fd;

    for (int i = 0; i < DRM_BUF_COUNT; i++)
        drm_destroy_fb(fd, &drm_bufs[i]);
}

static int disp_init(void)
{
    if (drm_dev_setup(&drm_dev, DRM_DEV_NAME_DEFAULT))
        return -1;

    drm_get_resolution(&drm_dev, &screen_width, &screen_height);
    input_map_set(0);

    for (int i = DRM_UI_BUF_SRART_IDX; i < DRM_UI_BUF_END_IDX; i++)
    {
        drm_bufs[i].width = ALIGNED_UP_POWER_OF_TWO(display_width, 3);
        drm_bufs[i].height = ALIGNED_DOWN_POWER_OF_TWO(display_height, 0);
        drm_bufs[i].offset_x = (screen_width - display_width) / 2;
        //drm_bufs[i].offset_y = (screen_height - display_height) / 2;
        drm_bufs[i].offset_y = (screen_height - display_height);
        drm_bufs[i].fourcc = DRM_FORMAT_ARGB8888;
        drm_bufs[i].bpp = 32;
        buf_mgt_reader_put(&ui_buf_mgt, (void *)i);
    }

    if (alloc_drm_buff())
    {
        drm_dev_cleanup(&drm_dev);
        return -1;
    }

    for (int i = DRM_UI_BUF_SRART_IDX; i < DRM_UI_BUF_END_IDX; i++)
        memset(drm_bufs[i].map, 0, drm_bufs[i].size);

    pthread_t tid_drm_vsync;
    pthread_attr_t tattr_drm_vsync;
    pthread_attr_init(&tattr_drm_vsync);
    int max_prio = sched_get_priority_max(SCHED_RR);
    struct sched_param sp = {max_prio};
    pthread_attr_setschedpolicy(&tattr_drm_vsync, SCHED_RR);
    pthread_attr_setschedparam(&tattr_drm_vsync, &sp);
    pthread_attr_setdetachstate(&tattr_drm_vsync, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid_drm_vsync, &tattr_drm_vsync, thread_drm_vsync,
                   &drm_dev);
    pthread_attr_destroy(&tattr_drm_vsync);

    return 0;
}

static void disp_deinit(void)
{
    free_drm_buff();
    drm_dev_cleanup(&drm_dev);
}

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                       lv_color_t *color_p)
{
    lv_color_t *dst, *src = color_p;
    for (int y = area->y1; y <= area->y2; y++)
    {
        dst = lvgl_buf + display_width * y + area->x1;
        for (int x = area->x1; x <= area->x2; x++)
            *dst++ = *src++;
    }
    lv_disp_flush_ready(disp_drv);
    if (disp_drv->draw_buf->last_area != 1 ||
            disp_drv->draw_buf->last_part != 1)
        return;

    while (1)
    {
        uint64_t index;
        if (buf_mgt_writer_get(&ui_buf_mgt, (void **)&index, 1) < 0)
        {
            usleep(5000);
            continue;
        }
        memcpy(drm_bufs[index].map, lvgl_buf,
               display_width * display_height * sizeof(lv_color_t));
        buf_mgt_writer_put(&ui_buf_mgt, (void *)index);
        static bool isfirst = true;
        if (isfirst)
        {
            isfirst = false;
            plane_config(&drm_dev, &drm_bufs[index]);
        }
        break;
    }
}

uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

static void *thread_drm_vsync(void *arg)
{
    struct drm_dev *pdev = (struct drm_dev *)arg;
    uint64_t ui_idx;
    int ui_stat;

    while (1)
    {
        if (pdev->pflip_pending)
        {
            drm_wait_vsync(pdev);
        }
        else
        {
            usleep(30000);
            continue;
        }
        if (pdev->cleanup)
        {
            disp_deinit();
            break;
        }
        // get ui buf index
        ui_stat = buf_mgt_reader_get(&ui_buf_mgt, (void **)&ui_idx, 1);

        if (ui_idx < DRM_UI_BUF_SRART_IDX || ui_idx >= DRM_UI_BUF_END_IDX)
            ui_idx = DRM_UI_BUF_SRART_IDX;

        plane_update(pdev, &drm_bufs[ui_idx]);
    }

    return 0;
}

#ifdef __cplusplus
} /*extern "C"*/
#endif
