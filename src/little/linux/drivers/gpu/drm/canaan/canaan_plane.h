/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __CANAAN_PLANE_H__
#define __CANAAN_PLANE_H__

struct canaan_plane_config {
    char        *name;
    uint32_t    id;
    uint32_t    possible_crtcs;
    uint32_t    num_formats;
    const uint32_t      *formats;
    enum drm_plane_type plane_type;

    uint32_t    plane_offset;
    uint32_t    plane_enable_bit;
    uint32_t    xctl_reg_offset;
    uint32_t    yctl_reg_offset;
};

struct canaan_plane {
    struct drm_plane    base;
    struct canaan_vo    *vo;
    struct canaan_plane_config  *config;
    uint32_t    id;
};

static inline struct canaan_plane *to_canaan_plane(struct drm_plane *plane)
{
    return container_of(plane, struct canaan_plane, base);
}

struct canaan_plane *canaan_plane_create(struct drm_device *drm_dev,
                                    struct canaan_plane_config *config,
                                    struct canaan_vo *vo);
#endif /* __CANAAN_PLANE_H__ */
