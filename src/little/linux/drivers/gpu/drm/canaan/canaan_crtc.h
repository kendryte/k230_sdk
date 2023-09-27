/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __CANAAN_CRTC_H__
#define __CANAAN_CRTC_H__

struct canaan_crtc {
    struct drm_crtc     base;
    struct canaan_vo    *vo;
    struct drm_pending_vblank_event *event;
};

static inline struct canaan_crtc *to_canaan_crtc(struct drm_crtc *crtc)
{
    return container_of(crtc, struct canaan_crtc, base);
}

struct canaan_crtc *canaan_crtc_create(struct drm_device *drm_dev,
                                struct drm_plane *primary, struct drm_plane *cursor,
                                struct canaan_vo *vo);

#endif /* __CANAAN_CRTC_H__ */
