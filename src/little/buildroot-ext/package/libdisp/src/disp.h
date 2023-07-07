/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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

#ifndef _DISP_H_
#define _DISP_H_

#include <stdbool.h>
#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define ALIGNED_UP_POWER_OF_TWO(value, n) (((value) + (1 << (n)) - 1) & ~((1 << (n)) - 1))
#define ALIGNED_DOWN_POWER_OF_TWO(value, n) (((value) & ~((1 << (n)) - 1)))

struct drm_buffer {
	int32_t offset_x;
	int32_t offset_y;
	uint32_t width;
	uint32_t height;
	uint32_t fourcc;
	uint32_t bpp;
	uint32_t pitch;
	uint32_t size;
	uint32_t handle;
	uint32_t fb;
	int dma_buf_fd;
	void *map;
};

struct drm_object {
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
	uint32_t id;
};

struct drm_dev {
	int fd;
	uint32_t conn_id, enc_id, crtc_id, crtc_idx;
	drmModeModeInfo mode;
	uint32_t mode_blob_id;
	struct drm_object conn;
	struct drm_object crtc;
	struct drm_object *planes;
	uint32_t *planes_id;
	uint32_t plane_count;
	bool pflip_pending;
	bool cleanup;
};

int drm_dev_setup(struct drm_dev *dev, const char *path);
void drm_dev_cleanup(struct drm_dev *dev);
int drm_create_fb(int fd, struct drm_buffer *buf);
void drm_destroy_fb(int fd, struct drm_buffer *buf);
int drm_set_object_property(drmModeAtomicReq *req, struct drm_object *obj,
			    const char *name, uint64_t value);
int drm_get_resolution(struct drm_dev *dev, uint32_t *width, uint32_t *height);

#if defined(__cplusplus)
}
#endif

#endif