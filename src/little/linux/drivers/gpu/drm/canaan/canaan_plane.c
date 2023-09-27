/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/component.h>
#include <linux/of_graph.h>
#include <linux/of_platform.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_of.h>
#include <drm/drm_irq.h>
#include <drm/drm_vblank.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_print.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_plane.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_probe_helper.h>

#include "canaan_vo.h"
#include "canaan_crtc.h"
#include "canaan_plane.h"

static int canaan_plane_atomic_check(struct drm_plane *plane,
                                struct drm_plane_state *plane_state)
{
    struct canaan_plane *canaan_plane = to_canaan_plane(plane);
    struct canaan_vo *vo = canaan_plane->vo;

    if (!plane_state->crtc || !plane_state->fb) {
        DRM_DEBUG_DRIVER("crtc or fb NULL \n");
        return 0;
    }

    DRM_DEBUG_DRIVER("Check plane:%d\n", plane->base.id);
    DRM_DEBUG_DRIVER("(%d,%d)@(%d,%d) -> (%d,%d)@(%d,%d)\n",
                plane_state->src_w >> 16, plane_state->src_h >> 16,
                plane_state->src_x >> 16, plane_state->src_y >> 16,
                plane_state->crtc_w, plane_state->crtc_h,
                plane_state->crtc_x, plane_state->crtc_y);

    return canaan_vo_check_plane(vo, canaan_plane, plane_state);
}

static void canaan_plane_atomic_update(struct drm_plane *plane,
                                struct drm_plane_state *old_plane_state)
{
    struct drm_plane_state *plane_state = plane->state;
    struct canaan_plane *canaan_plane = to_canaan_plane(plane);
    struct canaan_vo *vo = canaan_plane->vo;

    if (!plane_state->crtc || !plane_state->fb) {
        DRM_DEBUG_DRIVER("crtc or fb NULL \n");
        return;
    }

    DRM_DEBUG_DRIVER("Update plane:%d\n", plane->base.id);
    canaan_vo_update_plane(vo, canaan_plane, plane_state);
}

static void canaan_plane_atomic_disable(struct drm_plane *plane,
                                struct drm_plane_state *old_plane_state)
{
    struct canaan_plane *canaan_plane = to_canaan_plane(plane);
    struct canaan_vo *vo = canaan_plane->vo;

    DRM_DEBUG_DRIVER("Disable plane:%d\n", plane->base.id);
    canaan_vo_disable_plane(vo, canaan_plane);
}

static const struct drm_plane_funcs canaan_plane_funcs = {
    .reset = drm_atomic_helper_plane_reset,
    .destroy = drm_plane_cleanup,
    .update_plane = drm_atomic_helper_update_plane,
    .disable_plane = drm_atomic_helper_disable_plane,
    .atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
    .atomic_destroy_state = drm_atomic_helper_plane_destroy_state,
};

static const struct drm_plane_helper_funcs canaan_plane_helper_funcs = {
    .atomic_check = canaan_plane_atomic_check,
    .atomic_update = canaan_plane_atomic_update,
    .atomic_disable = canaan_plane_atomic_disable,
};

struct canaan_plane *canaan_plane_create(struct drm_device *drm_dev,
                                    struct canaan_plane_config *config,
                                    struct canaan_vo *vo)
{
    int ret = 0;
    struct canaan_plane *canaan_plane = NULL;
    struct drm_plane *plane = NULL;
    struct device *dev = vo->dev;

    canaan_plane = devm_kzalloc(dev, sizeof(*canaan_plane), GFP_KERNEL);
    if (!canaan_plane)
        return ERR_PTR(-ENOMEM);
    plane = &canaan_plane->base;

    ret = drm_universal_plane_init(drm_dev, plane, config->possible_crtcs,
                                &canaan_plane_funcs, config->formats, config->num_formats,
                                NULL, config->plane_type, NULL);
    if (ret) {
        DRM_DEV_ERROR(dev, "Failed to init Plane \n");
        return ERR_PTR(ret);
    }

    drm_plane_helper_add(plane, &canaan_plane_helper_funcs);
    canaan_plane->id        = config->id;
    canaan_plane->config    = config;
    canaan_plane->vo        = vo;
    DRM_DEBUG_DRIVER("Create plane:%d\n", plane->base.id);

    return canaan_plane;
}

