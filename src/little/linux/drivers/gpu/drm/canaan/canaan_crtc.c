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

static int canaan_crtc_enable_vblank(struct drm_crtc *crtc)
{
    struct canaan_crtc *canaan_crtc = to_canaan_crtc(crtc);
    struct canaan_vo *vo = canaan_crtc->vo;

    DRM_DEBUG_DRIVER("Enable vblank on CRTC:%d\n", crtc->base.id);
    return canaan_vo_enable_vblank(vo);
}

static void canaan_crtc_disable_vblank(struct drm_crtc *crtc)
{
    struct canaan_crtc *canaan_crtc = to_canaan_crtc(crtc);
    struct canaan_vo *vo = canaan_crtc->vo;

    DRM_DEBUG_DRIVER("Disable vblank on CRTC:%d\n", crtc->base.id);
    canaan_vo_disable_vblank(vo);
}

static void canaan_crtc_atomic_enable(struct drm_crtc *crtc,
                                struct drm_crtc_state *old_crtc_state)
{
    struct canaan_crtc *canaan_crtc = to_canaan_crtc(crtc);
    struct canaan_vo *vo = canaan_crtc->vo;

    DRM_DEBUG_DRIVER("Enable the CRTC:%d\n", crtc->base.id);
    canaan_vo_enable_crtc(vo, canaan_crtc);
    drm_crtc_vblank_on(crtc);
}

static void canaan_crtc_atomic_disable(struct drm_crtc *crtc,
                                struct drm_crtc_state *old_crtc_state)
{
    struct canaan_crtc *canaan_crtc = to_canaan_crtc(crtc);
    struct canaan_vo *vo = canaan_crtc->vo;

    DRM_DEBUG_DRIVER("Disable the CRTC:%d\n", crtc->base.id);
    drm_crtc_vblank_off(crtc);
    canaan_vo_disable_crtc(vo, canaan_crtc);

    if (crtc->state->event && !crtc->state->active) {
        spin_lock_irq(&crtc->dev->event_lock);
        drm_crtc_send_vblank_event(crtc, crtc->state->event);
        spin_unlock_irq(&crtc->dev->event_lock);

        crtc->state->event = NULL;
    }
}

static void canaan_crtc_atomic_flush(struct drm_crtc *crtc,
                                struct drm_crtc_state *old_crtc_state)
{
    struct canaan_crtc *canaan_crtc = to_canaan_crtc(crtc);
    struct canaan_vo *vo = canaan_crtc->vo;
	struct drm_pending_vblank_event *event = crtc->state->event;

    DRM_DEBUG_DRIVER("Flush the configuration \n");
    canaan_vo_flush_config(vo);

	if (event) {
		WARN_ON(drm_crtc_vblank_get(crtc) != 0);

		spin_lock_irq(&crtc->dev->event_lock);
		drm_crtc_arm_vblank_event(crtc, event);
		spin_unlock_irq(&crtc->dev->event_lock);
		crtc->state->event = NULL;
	}
}

static const struct drm_crtc_funcs canaan_crtc_funcs = {
    .reset = drm_atomic_helper_crtc_reset,
    .destroy = drm_crtc_cleanup,
    .set_config = drm_atomic_helper_set_config,
    .page_flip = drm_atomic_helper_page_flip,
    .gamma_set = drm_atomic_helper_legacy_gamma_set,
    .atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
    .atomic_destroy_state = drm_atomic_helper_crtc_destroy_state,
    .enable_vblank = canaan_crtc_enable_vblank,
    .disable_vblank = canaan_crtc_disable_vblank,
};

static const struct drm_crtc_helper_funcs canaan_crtc_helper_funcs = {
    .atomic_enable = canaan_crtc_atomic_enable,
    .atomic_disable = canaan_crtc_atomic_disable,
    .atomic_flush = canaan_crtc_atomic_flush,
};

struct canaan_crtc *canaan_crtc_create(struct drm_device *drm_dev,
                                struct drm_plane *primary, struct drm_plane *cursor,
                                struct canaan_vo *vo)
{
    int ret = 0;
    struct canaan_crtc *canaan_crtc = NULL;
    struct drm_crtc *crtc = NULL;
    struct device *dev = vo->dev;

    canaan_crtc = devm_kzalloc(dev, sizeof(struct canaan_crtc), GFP_KERNEL);
    if (!canaan_crtc)
        return ERR_PTR(-ENOMEM);
    canaan_crtc->vo = vo;
    crtc = &canaan_crtc->base;

    ret = drm_crtc_init_with_planes(drm_dev, crtc,
                                    primary, cursor,
                                    &canaan_crtc_funcs, "canaan_crtc");
    if (ret) {
        return ERR_PTR(ret);
        DRM_DEV_ERROR(dev, "Failed to init CRTC \n");
    }

    drm_crtc_helper_add(crtc, &canaan_crtc_helper_funcs);

    drm_mode_crtc_set_gamma_size(crtc, 256);
    drm_crtc_enable_color_mgmt(crtc, 0, false, 256);
    DRM_DEBUG_DRIVER("Create the CRTC:%d\n", crtc->base.id);

    return canaan_crtc;
}

