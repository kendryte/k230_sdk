/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __CANAAN_VO_H__
#define __CANAAN_VO_H__

#define CANAAN_PLANE_NUMBER     5

struct canaan_vo {
    struct device       *dev;
    struct drm_device   *drm_dev;

    void __iomem        *reg_base;
    int                 irq;
    atomic_t            vsync_enabled;

    struct canaan_crtc  *canaan_crtc;
    struct canaan_plane *canaan_plane[CANAAN_PLANE_NUMBER];
};

int canaan_vo_check_plane(struct canaan_vo *vo, struct canaan_plane *canaan_plane,
                    struct drm_plane_state *plane_state);
void canaan_vo_update_plane(struct canaan_vo *vo, struct canaan_plane *canaan_plane,
                    struct drm_plane_state *plane_state);
void canaan_vo_disable_plane(struct canaan_vo *vo, struct canaan_plane *canaan_plane);

int canaan_vo_enable_vblank(struct canaan_vo *vo);
void canaan_vo_disable_vblank(struct canaan_vo *vo);
void canaan_vo_enable_crtc(struct canaan_vo *vo, struct canaan_crtc *canaan_crtc);
void canaan_vo_disable_crtc(struct canaan_vo *vo, struct canaan_crtc *canaan_crtc);
void canaan_vo_flush_config(struct canaan_vo *vo);

#endif /* __CANAAN_VO_H__ */
