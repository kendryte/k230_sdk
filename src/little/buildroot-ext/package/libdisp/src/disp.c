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

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "disp.h"

static int drm_open(const char *path)
{
	int fd, ret;
	uint64_t cap;

	fd = open(path, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		ret = -errno;
		fprintf(stderr, "cannot open '%s': %m\n", path);
		return ret;
	}

	/* Set that we want to receive all the types of planes in the list. This
	 * have to be done since, for legacy reasons, the default behavior is to
	 * expose only the overlay planes to the users. The atomic API only
	 * works if this is set.
	 */
	ret = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	if (ret) {
		fprintf(stderr, "failed to set universal planes cap, %d\n",
			ret);
		return ret;
	}

	/* Here we set that we're going to use the KMS atomic API. It's supposed
	 * to set the DRM_CLIENT_CAP_UNIVERSAL_PLANES automatically, but it's a
	 * safe behavior to set it explicitly as we did in the previous
	 * commands. This is also good for learning purposes.
	 */
	ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
	if (ret) {
		fprintf(stderr, "failed to set atomic cap, %d", ret);
		return ret;
	}

	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &cap) < 0 || !cap) {
		fprintf(stderr,
			"drm device '%s' does not support dumb buffers\n",
			path);
		close(fd);
		return -EOPNOTSUPP;
	}

	if (drmGetCap(fd, DRM_CAP_CRTC_IN_VBLANK_EVENT, &cap) < 0 || !cap) {
		fprintf(stderr, "drm device '%s' does not support atomic KMS\n",
			path);
		close(fd);
		return -EOPNOTSUPP;
	}

	return fd;
}

static void drm_get_object_properties(int fd, struct drm_object *obj,
				      uint32_t type)
{
	const char *type_str;
	unsigned int i;

	obj->props = drmModeObjectGetProperties(fd, obj->id, type);
	if (!obj->props) {
		switch (type) {
		case DRM_MODE_OBJECT_CONNECTOR:
			type_str = "connector";
			break;
		case DRM_MODE_OBJECT_PLANE:
			type_str = "plane";
			break;
		case DRM_MODE_OBJECT_CRTC:
			type_str = "CRTC";
			break;
		default:
			type_str = "unknown type";
			break;
		}
		fprintf(stderr, "cannot get %s %d properties: %s\n", type_str,
			obj->id, strerror(errno));
		return;
	}

	obj->props_info = (drmModePropertyRes **)calloc(
		obj->props->count_props, sizeof(obj->props_info));
	for (i = 0; i < obj->props->count_props; i++)
		obj->props_info[i] =
			drmModeGetProperty(fd, obj->props->props[i]);
}

int drm_set_object_property(drmModeAtomicReq *req, struct drm_object *obj,
			    const char *name, uint64_t value)
{
	uint32_t prop_id = 0;

	for (int i = 0; i < obj->props->count_props; i++) {
		if (!strcmp(obj->props_info[i]->name, name)) {
			prop_id = obj->props_info[i]->prop_id;
			break;
		}
	}

	if (prop_id == 0) {
		fprintf(stderr, "no object property: %s\n", name);
		return -EINVAL;
	}

	return drmModeAtomicAddProperty(req, obj->id, prop_id, value);
}

static void drm_object_fini(struct drm_object *obj)
{
	if (!obj->props)
		return;
	for (int i = 0; i < obj->props->count_props; i++)
		drmModeFreeProperty(obj->props_info[i]);
	free(obj->props_info);
	drmModeFreeObjectProperties(obj->props);
	obj->props = NULL;
}

static int drm_get_objects(struct drm_dev *dev)
{
	int fd = dev->fd;
	struct drm_object *conn = &dev->conn;
	struct drm_object *crtc = &dev->crtc;
	struct drm_object *plane;

	/* retrieve connector properties from the device */
	conn->id = dev->conn_id;
	drm_get_object_properties(fd, conn, DRM_MODE_OBJECT_CONNECTOR);
	if (!conn->props)
		goto out_conn;

	/* retrieve CRTC properties from the device */
	crtc->id = dev->crtc_id;
	drm_get_object_properties(fd, crtc, DRM_MODE_OBJECT_CRTC);
	if (!crtc->props)
		goto out_crtc;

	/* retrieve plane properties from the device */
	dev->planes = (struct drm_object *)calloc(dev->plane_count,
						  sizeof(struct drm_object));
	for (int i = 0; i < dev->plane_count; i++) {
		plane = &dev->planes[i];
		plane->id = dev->planes_id[i];
		drm_get_object_properties(fd, plane, DRM_MODE_OBJECT_PLANE);
		if (!plane->props)
			goto out_plane;
	}

	return 0;

out_plane:
	for (int i = 0; i < dev->plane_count; i++) {
		if (dev->planes[i].props)
			drm_object_fini(&dev->planes[i]);
	}
	free(dev->planes);
	dev->planes = NULL;
	drm_object_fini(crtc);
out_crtc:
	drm_object_fini(conn);
out_conn:
	fprintf(stderr, "cannot get objects properties\n");
	return -ENOMEM;
}

static void drm_free_objects(struct drm_dev *dev)
{
	drm_object_fini(&dev->conn);
	drm_object_fini(&dev->crtc);
	if (dev->planes) {
		for (int i = 0; i < dev->plane_count; i++) {
			if (dev->planes[i].props)
				drm_object_fini(&dev->planes[i]);
		}
		free(dev->planes);
		dev->planes = NULL;
	}
}

int drm_create_fb(int fd, struct drm_buffer *buf)
{
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	struct drm_mode_map_dumb mreq;
	struct drm_prime_handle prime;
	int ret;
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };

	/* create dumb buffer */
	memset(&creq, 0, sizeof(creq));
	if (buf->fourcc == DRM_FORMAT_NV12) {
		buf->width = (buf->width + 0xf) & (~0xful);
		creq.height = buf->height * 3 / 2;
	} else {
		creq.height = buf->height;
	}
	creq.width = buf->width;
	creq.bpp = buf->bpp;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	if (ret < 0) {
		ret = -errno;
		fprintf(stderr, "cannot create dumb buffer (%d): %m\n", errno);
		return ret;
	}
	buf->size = creq.size;
	buf->handle = creq.handle;
	buf->pitch = creq.pitch;
	/* create framebuffer object for the dumb-buffer */
	if (buf->fourcc == DRM_FORMAT_NV12) {
		handles[1] = buf->handle;
		pitches[1] = buf->pitch;
		offsets[1] = buf->pitch * buf->height;
	}
	handles[0] = buf->handle;
	pitches[0] = buf->pitch;
	offsets[0] = 0;
	ret = drmModeAddFB2(fd, buf->width, buf->height, buf->fourcc, handles,
			    pitches, offsets, &buf->fb, 0);
	if (ret) {
		ret = -errno;
		fprintf(stderr, "cannot create framebuffer (%d): %m\n", errno);
		goto err_destroy;
	}

	/* prepare buffer for memory mapping */
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = buf->handle;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (ret) {
		ret = -errno;
		fprintf(stderr, "cannot map dumb buffer (%d): %m\n", errno);
		goto err_fb;
	}

	/* perform actual memory mapping */
	buf->map = mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
			mreq.offset);
	if (buf->map == MAP_FAILED) {
		ret = -errno;
		fprintf(stderr, "cannot mmap dumb buffer (%d): %m\n", errno);
		goto err_fb;
	}

	memset(&prime, 0, sizeof(prime));
	prime.handle = buf->handle;
	ret = ioctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
	if (ret) {
		ret = -errno;
		fprintf(stderr, "PRIME_HANDLE_TO_FD fail (%d): %m\n", errno);
		goto err_fb;
	}
	buf->dma_buf_fd = prime.fd;

	return 0;

err_fb:
	drmModeRmFB(fd, buf->fb);
err_destroy:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = buf->handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	return ret;
}

void drm_destroy_fb(int fd, struct drm_buffer *buf)
{
	struct drm_mode_destroy_dumb dreq;

	/* unmap buffer */
	munmap(buf->map, buf->size);

	/* delete framebuffer */
	drmModeRmFB(fd, buf->fb);

	/* delete dumb buffer */
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = buf->handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
}

static int drm_find_crtc(struct drm_dev *dev, drmModeRes *res,
			 drmModeConnector *conn)
{
	drmModeEncoder *enc;

	/* first try the currently conected encoder+crtc */
	if (conn->encoder_id)
		enc = drmModeGetEncoder(dev->fd, conn->encoder_id);
	else
		enc = NULL;

	if (enc) {
		if (enc->crtc_id > 0) {
			dev->conn_id = conn->connector_id;
			dev->enc_id = enc->encoder_id;
			dev->crtc_id = enc->crtc_id;
			for (int i = 0; i < res->count_crtcs; ++i) {
				if (dev->crtc_id == res->crtcs[i]) {
					dev->crtc_idx = i;
					break;
				}
			}
			drmModeFreeEncoder(enc);
			return 0;
		}
		drmModeFreeEncoder(enc);
	}
	/* If the connector is not currently bound to an encoder or if the
	 * encoder+crtc is already used by another connector (actually unlikely
	 * but lets be safe), iterate all other available encoders to find a
	 * matching CRTC.
	 */
	for (int i = 0; i < conn->count_encoders; ++i) {
		enc = drmModeGetEncoder(dev->fd, conn->encoders[i]);
		if (!enc) {
			fprintf(stderr,
				"cannot retrieve encoder %u:%u (%d): %m\n", i,
				conn->encoders[i], errno);
			continue;
		}
		/* iterate all global CRTCs */
		for (int j = 0; j < res->count_crtcs; ++j) {
			/* check whether this CRTC works with the encoder */
			if (!(enc->possible_crtcs & (1 << j)))
				continue;
			/* We have found a CRTC, so save it and return. Note
			 * that we have to save its index as well. The CRTC
			 * index (not its ID) will be used when searching for a
			 * suitable plane.
			 */
			if (res->crtcs[j] > 0) {
				dev->conn_id = conn->connector_id;
				dev->enc_id = enc->encoder_id;
				dev->crtc_id = res->crtcs[j];
				dev->crtc_idx = j;
				drmModeFreeEncoder(enc);
				return 0;
			}
		}
		drmModeFreeEncoder(enc);
	}
	fprintf(stderr, "cannot find suitable crtc for connector %u\n",
		conn->connector_id);
	return -ENOENT;
}

static int drm_find_plane(struct drm_dev *dev)
{
	drmModePlaneResPtr plane_res;
	uint32_t plane_count = 0;

	plane_res = drmModeGetPlaneResources(dev->fd);
	if (!plane_res) {
		fprintf(stderr, "drmModeGetPlaneResources failed: %s\n",
			strerror(errno));
		return -ENOENT;
	}

	dev->planes_id =
		(uint32_t *)malloc(plane_res->count_planes * sizeof(uint32_t));
	/* iterates through all planes of a certain device */
	for (int i = 0; i < plane_res->count_planes; i++) {
		int plane_id = plane_res->planes[i];
		drmModePlanePtr plane = drmModeGetPlane(dev->fd, plane_id);
		if (!plane) {
			fprintf(stderr, "drmModeGetPlane(%u) failed: %s\n",
				plane_id, strerror(errno));
			continue;
		}
		/* check if the plane can be used by our CRTC */
		if (plane->possible_crtcs & (1 << dev->crtc_idx)) {
			dev->planes_id[plane_count] = plane_id;
			plane_count++;
		}
		drmModeFreePlane(plane);
	}

	drmModeFreePlaneResources(plane_res);

	dev->plane_count = plane_count;
	if (plane_count)
		return 0;

	free(dev->planes_id);
	dev->planes_id = NULL;
	fprintf(stderr, "couldn't find a plane\n");

	return -EINVAL;
}

static int drm_find_connector(struct drm_dev *dev)
{
	drmModeRes *res;
	drmModeConnector *conn;
	bool find = false;
	int ret;

	/* retrieve resources */
	res = drmModeGetResources(dev->fd);
	if (!res) {
		ret = -errno;
		fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n",
			errno);
		return ret;
	}

	/* iterate all connectors */
	for (int i = 0; i < res->count_connectors; ++i) {
		/* get information for each connector */
		conn = drmModeGetConnector(dev->fd, res->connectors[i]);
		if (!conn) {
			fprintf(stderr,
				"cannot retrieve DRM connector %u:%u (%d): %m\n",
				i, res->connectors[i], errno);
			continue;
		}

		/* check if a monitor is connected */
		if (conn->connection != DRM_MODE_CONNECTED) {
			fprintf(stderr, "ignoring unused connector %u\n",
				conn->connector_id);
			goto next;
		}

		/* check if there is at least one valid mode */
		if (conn->count_modes == 0) {
			fprintf(stderr, "no valid mode for connector %u\n",
				conn->connector_id);
			goto next;
		}

		memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));

		if (drm_find_crtc(dev, res, conn))
			goto next;

		if (drm_find_plane(dev))
			goto next;

		if (drm_get_objects(dev)) {
			free(dev->planes_id);
			dev->planes_id = NULL;
			goto next;
		}

		find = true;
	next:
		drmModeFreeConnector(conn);
		if (find)
			break;
	}
	if (!find)
		fprintf(stderr, "couldn't create any outputs\n");
	/* free resources again */
	drmModeFreeResources(res);
	return 0;
}

int drm_dev_setup(struct drm_dev *dev, const char *path)
{
	if ((dev->fd = drm_open(path)) < 0)
		return dev->fd;

	if (drm_find_connector(dev))
		return -1;

	return 0;
}

void drm_dev_cleanup(struct drm_dev *dev)
{
	drm_free_objects(dev);
	drmModeDestroyPropertyBlob(dev->fd, dev->mode_blob_id);
	free(dev->planes_id);
	close(dev->fd);
}

int drm_get_resolution(struct drm_dev *dev, uint32_t *width, uint32_t *height)
{
	*width = dev->mode.hdisplay;
	*height = dev->mode.vdisplay;

	return 0;
}
