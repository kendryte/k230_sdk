/*
 * UVC gadget test application
 *
 * Copyright (C) 2010 Ideas on board SPRL <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <linux/usb/ch9.h>

#include "uvc.h"
#include "video.h"
#include "uvc_gadgete.h"
#include "kstream.h"
#include "log.h"
#include "frame_cache.h"

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

#define PU_BRIGHTNESS_MIN_VAL 0
#define PU_BRIGHTNESS_MAX_VAL 255
#define PU_BRIGHTNESS_STEP_SIZE 1
#define PU_BRIGHTNESS_DEFAULT_VAL 127

#define clamp(val, min, max) ({                 \
                                  typeof(val)__val = (val);              \
                                  typeof(min)__min = (min);              \
                                  typeof(max)__max = (max);              \
                                  (void) (&__val == &__min);              \
                                  (void) (&__val == &__max);              \
                                  __val = __val < __min ? __min : __val;   \
                                  __val > __max ? __max : __val; })

#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum
{
    IO_METHOD_MMAP = 0x1,
    IO_METHOD_USERPTR = 0x2,
} io_method;

/* Buffer representing one video frame */
struct buffer {
    struct v4l2_buffer buf;
    void *start;
    size_t length;
};

#define WAITED_NODE_SIZE (4)
static frame_node_t *__waited_node[WAITED_NODE_SIZE];

extern unsigned int g_bulk;
extern unsigned int g_standalone;
extern unsigned int g_imagesize;
extern unsigned int g_bulk_size;

static void clear_waited_node()
{
    int i = 0;
    uvc_cache_t *uvc_cache = uvc_cache_get();

    for (i = 0; i < WAITED_NODE_SIZE; i++)
    {
        if ((__waited_node[i] != 0) && uvc_cache)
        {
            put_node_to_queue(uvc_cache->free_queue, __waited_node[i]);
            __waited_node[i] = 0;
        }
    }
}

static struct uvc_device*uvc_open(const char* devname)
{
    struct uvc_device* dev;
    struct v4l2_capability cap;
    int ret;
    int fd;

    fd = open(devname, O_RDWR | O_NONBLOCK);

    if (fd == -1)
    {
        LOG("v4l2 open failed(%s): %s (%d)\n",devname, strerror(errno), errno);
        return NULL;
    }

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);

    if (ret < 0)
    {
        LOG("unable to query device: %s (%d)\n", strerror(errno),
            errno);
        close(fd);
        return NULL;
    }

    //LOG("open succeeded(%s:caps=0x%04x)\n", devname, cap.capabilities);

    /*
    V4L2_CAP_VIDEO_CAPTURE 0x00000001 support video Capture interface.
    V4L2_CAP_VIDEO_OUTPUT 0x00000002 support video output interface.
    V4L2_CAP_VIDEO_OVERLAY 0x00000004 support video cover interface.
    V4L2_CAP_VBI_CAPTURE 0x00000010 Original VBI Capture interface.
    V4L2_CAP_VBI_OUTPUT 0x00000020 Original VBI Output interface.
    V4L2_CAP_SLICED_VBI_CAPTURE 0x00000040 Sliced VBI Capture interface.
    V4L2_CAP_SLICED_VBI_OUTPUT 0x00000080 Sliced VBI Output interface.
    V4L2_CAP_RDS_CAPTURE 0x00000100 undefined
    */
    if (!(cap.capabilities & 0x00000002)) {
        close(fd);
        return NULL;
    }

    //LOG("device is %s on bus %s\n", cap.card, cap.bus_info);

    dev = (struct uvc_device*)malloc(sizeof * dev);

    if (dev == NULL)
    {
        close(fd);
        return NULL;
    }

    memset(dev, 0, sizeof * dev);
    dev->fd = fd;

    CLEAR(__waited_node);

    return dev;
}

static void uvc_close(struct uvc_device* dev)
{
    close(dev->fd);
    free(dev);
}

/* ---------------------------------------------------------------------------
 * Video streaming
 */
static void uvc_video_fill_buffer_standalone(struct uvc_device *dev, struct v4l2_buffer *buf)
{
    unsigned int bpl;
    unsigned int i;
    int test = 1;
    int c = 0;

    switch (dev->fcc) {
    case V4L2_PIX_FMT_YUYV:
        /* Fill the buffer with video data. */
        bpl = dev->width * 2;
        for (i = 0; i < dev->height; ++i)
            memset(dev->mem[buf->index].start + i * bpl, dev->color++, bpl);

        buf->bytesused = bpl * dev->height;
		// printf("uvc_video_fill_buffer %d \n", buf->bytesused);
        break;
    case V4L2_PIX_FMT_NV12:
        /* Fill the buffer with video data. */
        bpl = dev->width * 1;
        for (i = 0; i < dev->height; ++i)
            memset(dev->mem[buf->index].start + i * bpl, dev->color++, bpl);
        buf->bytesused = bpl * dev->height;
		bpl = dev->width / 2;
        for (i = 0; i < dev->height; ++i)
            memset(dev->mem[buf->index].start + buf->bytesused + i * bpl, dev->color++, bpl);
        buf->bytesused += bpl * dev->height;
		printf("uvc_video_fill_buffer %d \n", buf->bytesused);
        break;
    case V4L2_PIX_FMT_MJPEG:
	case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_HEVC:
        memcpy(dev->mem[buf->index].start, dev->imgdata, dev->imgsize);
        buf->bytesused = dev->imgsize;
		break;
    }
    buf->m.userptr = (unsigned long)dev->dummy_buf[buf->index].start;
    buf->length = dev->dummy_buf[buf->index].length;
}

static void uvc_video_fill_buffer_userptr(struct uvc_device* dev, struct v4l2_buffer* buf)
{
    static uint32_t debug_init = 0xabcd0000;
    struct timeval start, end;

    int retry_count = 0;
    buf->bytesused = 0;

    switch (dev->fcc)
    {
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_HEVC:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_NV12:
    {
        uvc_cache_t *uvc_cache = uvc_cache_get();
        frame_node_t *node = 0;
        frame_queue_t *q = 0, *fq = 0;

retry:
        if (uvc_cache)
        {
            q  = uvc_cache->ok_queue;
            fq = uvc_cache->free_queue;
            get_node_from_queue(q, &node);
        }

        if ((__waited_node[buf->index] != 0) && uvc_cache)
        {
            put_node_to_queue(fq, __waited_node[buf->index]);
            __waited_node[buf->index] = 0;
        }

        if (node != 0)
        {
            buf->bytesused = node->used;
            buf->m.userptr = (unsigned long) node->mem;
            buf->length = node->length;
            __waited_node[buf->index] = node;
        }
        else if (retry_count++ < 60)
        {
            // the perfect solution is using locker and waiting queue's notify.
            // but here just only simply used usleep method and try again.
            // it works fine now.
            usleep(100000);
            goto retry;
        }
        else
        {
            LOG("retry_count out...\n");
        }
    }
        break;
    default:
        LOG("what's up....\n");
        break;
    }
}

static int uvc_video_process_userptr(struct uvc_device* dev)
{
    struct v4l2_buffer buf;
    int ret;

    // INFO("#############uvc_video_process_userptr\n");

    memset(&buf, 0, sizeof buf);
    buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_USERPTR;

    if ((ret = ioctl(dev->fd, VIDIOC_DQBUF, &buf)) < 0)
    {
        return ret;
    }
    if (g_standalone)
    {
        uvc_video_fill_buffer_standalone(dev, &buf);
    }else
    {
        uvc_video_fill_buffer_userptr(dev, &buf);
    }

    if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0)
    {
        printf("buf.length=%d bytesused=%d userptr=0x%lx\n", buf.length, buf.bytesused, buf.m.userptr);
        LOG("Unable to requeue buffer: %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    return 0;
}

static int uvc_video_reqbufs_userptr(struct uvc_device* dev, int nbufs)
{
    // INFO("%s:nbufs = %d.\n", __func__, nbufs);
    struct v4l2_requestbuffers rb;
    int ret = 0;
    unsigned int i, j, bpl, payload_size;

    dev->nbufs = 0;

    memset(&rb, 0, sizeof rb);
    rb.count = nbufs;
    rb.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rb);

    if (ret < 0)
    {
        INFO("Unable to allocate buffers: %s (%d).\n",
             strerror(errno), errno);
        return ret;
    }
    if (!rb.count)
		return 0;
    dev->nbufs = rb.count;

 //   INFO("%u buffers allocated.\n", rb.count);
    if (g_standalone) {
        dev->color = 0x00;
        /* Allocate buffers to hold dummy data pattern. */
        dev->dummy_buf = (struct buffer *)calloc(rb.count, sizeof dev->dummy_buf[0]);
        if (!dev->dummy_buf) {
            printf("UVC: Out of memory\n");
            ret = -ENOMEM;
            goto err;
        }

        switch (dev->fcc) {
        case V4L2_PIX_FMT_YUYV:
            bpl = dev->width * 2;
            payload_size = dev->width * dev->height * 2;
            break;
        case V4L2_PIX_FMT_NV12:
            payload_size = dev->width * dev->height * 3 / 2;
            break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_HEVC:

            payload_size = dev->imgsize;//dev->width * dev->height;
            break;
        }

        for (i = 0; i < rb.count; ++i) {
            printf("[%d] %d width:%d height:%d\n", i, payload_size, dev->width, dev->height);
            dev->dummy_buf[i].length = payload_size;
            dev->dummy_buf[i].start = malloc(payload_size);
            if (!dev->dummy_buf[i].start) {
                printf("UVC: Out of memory\n");
                ret = -ENOMEM;
                goto err;
            }

            if (V4L2_PIX_FMT_YUYV == dev->fcc)
                for (j = 0; j < dev->height; ++j)
                    memset(dev->dummy_buf[i].start + j * bpl, dev->color++, bpl);

            if (V4L2_PIX_FMT_NV12 == dev->fcc)
            {
                bpl = dev->width;
                for (j = 0; j < dev->height; ++j)
                    memset(dev->dummy_buf[i].start + j * bpl, dev->color++, bpl);
                bpl = dev->width / 2;
                for (j = 0; j < dev->height; ++j)
                    memset(dev->dummy_buf[i].start + dev->width*dev->height +  j * bpl, dev->color++, bpl);
            }
            if ((V4L2_PIX_FMT_MJPEG == dev->fcc) || (V4L2_PIX_FMT_H264 == dev->fcc) || (V4L2_PIX_FMT_HEVC == dev->fcc))
				memcpy(dev->dummy_buf[i].start, dev->imgdata,
						dev->imgsize);
        }

        dev->mem = dev->dummy_buf;
    }
    return 0;
err:
    return ret;
}

static int uvc_video_stream_userptr(struct uvc_device* dev, int enable)
{
    struct v4l2_buffer buf;
    unsigned int i;
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    int ret = 0;

    // INFO("%s:Starting video stream.\n", __func__);

    for (i = 0; i < (dev->nbufs); ++i)
    {
        memset(&buf, 0, sizeof buf);

        buf.index = i;
        buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_USERPTR;

        if (g_standalone)
        {
            uvc_video_fill_buffer_standalone(dev, &buf);
        }else
        {
            uvc_video_fill_buffer_userptr(dev, &buf);
        }
        printf("buf.length=%d bytesused=%d userptr=0x%lx\n", buf.length, buf.bytesused, buf.m.userptr);
        if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0)
        {
            INFO("Unable to queue buffer(%d): %s (%d).\n", i,
                 strerror(errno), errno);
            break;
        }
    }

    ioctl(dev->fd, VIDIOC_STREAMON, &type);
    dev->streaming = 1;

    return ret;
}

static int uvc_video_set_format(struct uvc_device* dev)
{
    struct v4l2_format fmt;
    int ret;

    memset(&fmt, 0, sizeof fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width  = dev->width;
    fmt.fmt.pix.height = dev->height;
    fmt.fmt.pix.pixelformat = dev->fcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    printf("=============0x%08x. 0x%08x. 0x%08x.", dev->fcc, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_H264);
    if ((dev->fcc == V4L2_PIX_FMT_MJPEG) || (dev->fcc == V4L2_PIX_FMT_H264) || (dev->fcc == V4L2_PIX_FMT_HEVC))
    {
        fmt.fmt.pix.sizeimage = dev->imgsize;
    }

    if ((ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt)) < 0)
    {
        LOG("Unable to set format: %s (%d).\n",
            strerror(errno), errno);
    }

    return ret;
}

static int uvc_video_init(struct uvc_device* dev __attribute__ ((__unused__)))
{
    return 0;
}

/* ---------------------------------------------------------------------------
 * Request processing
 */

struct uvc_frame_info
{
    unsigned int width;
    unsigned int height;
    unsigned int intervals[8];
};

struct uvc_format_info
{
    unsigned int                 fcc;
    const struct uvc_frame_info* frames;
};

static const struct uvc_frame_info uvc_frames_nv12[] = {
    { 640, 360, {333333, 0}, },
    { 0, 0, {0, }, },
};

static const struct uvc_frame_info uvc_frames_mjpeg[] =
{
    { 1280,  720, {333333,		0  }, },
    {    0,    0, {		0,         }, },
};

static const struct uvc_frame_info uvc_frames_h264[] =
{
    { 1280,  720, {333333,		0  }, },
    {    0,    0, {		0,         }, },
};

static const struct uvc_frame_info uvc_frames_h265[] =
{
    { 1280,  720, {333333,		0  }, },
    {    0,    0, {		0,         }, },
};

static const struct uvc_format_info uvc_formats[] =
{
    {V4L2_PIX_FMT_NV12, uvc_frames_nv12},
    { V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg },
    { V4L2_PIX_FMT_H264,  uvc_frames_h264  },
    // { V4L2_PIX_FMT_HEVC,  uvc_frames_h265  },
};

static void uvc_streamoff(struct uvc_device *dev)
{
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    ioctl(dev->fd, VIDIOC_STREAMOFF, &type);
    if (g_standalone == 0) {
        kstream_shutdown();
    }
    dev->streaming = 0;

 //   INFO("Stopping video stream.\n");

 }

static void disable_uvc_video(struct uvc_device* dev)
{
    unsigned int i;

    uvc_streamoff(dev);
    uvc_video_reqbufs_userptr(dev, 0);
    if(g_standalone)
    {
        for (i = 0; i < dev->nbufs; ++i)
            free(dev->dummy_buf[i].start);

        free(dev->dummy_buf);
    }
    else
    {
        clear_waited_node();
    }
}

void clear_ok_queue()
{
    frame_node_t *node = 0;
    uvc_cache_t *uvc_cache = uvc_cache_get();

    while (0 == get_node_from_queue(uvc_cache->ok_queue, &node))
    {
        node->used = 0;
        put_node_to_queue(uvc_cache->free_queue, node);
    }
}

static void enable_uvc_video(struct uvc_device* dev)
{
    encoder_property p;

    if(g_standalone == 0)
    {
        clear_ok_queue();
        clear_waited_node();
        p.format = dev->fcc;
        p.width    = dev->width;
        p.height   = dev->height;
        p.compsite = 0;

        kstream_set_enc_property(&p);
        kstream_startup();
    }

    uvc_video_reqbufs_userptr(dev, WAITED_NODE_SIZE);
    uvc_video_stream_userptr(dev, 1);
}

static void uvc_fill_streaming_control(struct uvc_device* dev,
                                       struct uvc_streaming_control* ctrl,
                                       int iframe, int iformat)
{
    const struct uvc_format_info* format;
    const struct uvc_frame_info* frame;
    unsigned int nframes;

    if (iformat < 0)
    {
        iformat = ARRAY_SIZE(uvc_formats) + iformat;
    }

    if ((iformat < 0) || (iformat >= (int)ARRAY_SIZE(uvc_formats)))
    {
        return;
    }

   // INFO("iformat = %d\n", iformat);
    format = &uvc_formats[iformat];

    nframes = 0;

    while (format->frames[nframes].width != 0)
    {
        ++nframes;
    }

    if (iframe < 0)
    {
        iframe = nframes + iframe;
    }

    if ((iframe < 0) || (iframe >= (int)nframes))
    {
        return;
    }

    frame = &format->frames[iframe];

    memset(ctrl, 0, sizeof * ctrl);

    ctrl->bmHint = 1;
    ctrl->bFormatIndex = iformat + 1;
    ctrl->bFrameIndex = iframe + 1;
    ctrl->dwFrameInterval = frame->intervals[0];

    switch (format->fcc)
    {
    case V4L2_PIX_FMT_YUYV:
        ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 2;
        break;

    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_NV12:
        ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 3 / 2;
        break;

    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_HEVC:
        dev->width = frame->width;
        dev->height = frame->height;
        ctrl->dwMaxVideoFrameSize = frame->width * frame->height;
        break;
    }

    if (dev->bulk) {
        ctrl->dwMaxPayloadTransferSize = dev->bulk_size;   /* TODO this should be filled by the driver. */
    } else {
        ctrl->dwMaxPayloadTransferSize = 1024;
    }
    ctrl->bmFramingInfo = 3;
    ctrl->bPreferedVersion = 1;
    ctrl->bMaxVersion = 1;

}

static void uvc_events_process_standard(struct uvc_device* dev, struct usb_ctrlrequest* ctrl,
                                        struct uvc_request_data* resp)
{
    INFO("standard request\n");
    (void)dev;
    (void)ctrl;
    (void)resp;
}

static void uvc_event_undefined_control(struct uvc_device *dev,
                                        uint8_t req,
                                        uint8_t cs,
                                        struct uvc_request_data *resp)
{
    switch (cs)
    {
    case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
        resp->length = dev->request_error_code.length;
        resp->data[0] = dev->request_error_code.data[0];
        //printf("dev->request_error_code.data[0] = %d\n",dev->request_error_code.data[0]);
        break;
    default:
        dev->request_error_code.length = 1;
        dev->request_error_code.data[0] = 0x06;
        break;
    }
}

static void uvc_events_process_control(
    struct uvc_device *dev, uint8_t req, uint8_t cs, uint8_t entity_id, uint8_t len, struct uvc_request_data *resp)
{
    switch (entity_id) {
    case 0:
        switch (cs) {
        case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
            /* Send the request error code last prepared. */
            resp->data[0] = dev->request_error_code.data[0];
            resp->length = dev->request_error_code.length;
            break;

        default:
            /*
             * If we were not supposed to handle this
             * 'cs', prepare an error code response.
             */
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;
        }
        break;

    /* Camera terminal unit 'UVC_VC_INPUT_TERMINAL'. */
    case 1:
        switch (cs) {
        /*
         * We support only 'UVC_CT_AE_MODE_CONTROL' for CAMERA
         * terminal, as our bmControls[0] = 2 for CT. Also we
         * support only auto exposure.
         */
        case UVC_CT_AE_MODE_CONTROL:
            switch (req) {
            case UVC_SET_CUR:
                /* Incase of auto exposure, attempts to
                 * programmatically set the auto-adjusted
                 * controls are ignored.
                 */
                resp->data[0] = 0x01;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;

            case UVC_GET_INFO:
                /*
                 * TODO: We support Set and Get requests, but
                 * don't support async updates on an video
                 * status (interrupt) endpoint as of
                 * now.
                 */
                resp->data[0] = 0x03;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;

            case UVC_GET_CUR:
            case UVC_GET_DEF:
            case UVC_GET_RES:
                /* Auto Mode â€“ auto Exposure Time, auto Iris. */
                resp->data[0] = 0x02;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                /*
                 * We don't support this control, so STALL the
                 * control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * value.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;

        default:
            /*
             * We don't support this control, so STALL the control
             * ep.
             */
            resp->length = -EL2HLT;
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;
        }
        break;

    /* processing unit 'UVC_VC_PROCESSING_UNIT' */
    case 2:
        switch (cs) {
        /*
         * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
         * Unit, as our bmControls[0] = 1 for PU.
         */
        case UVC_PU_BRIGHTNESS_CONTROL:
            switch (req) {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_BRIGHTNESS_MIN_VAL;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_BRIGHTNESS_MAX_VAL;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->brightness_val, resp->length);
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                /*
                 * We support Set and Get requests and don't
                 * support async updates on an interrupt endpt
                 */
                resp->data[0] = 0x03;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request, set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_BRIGHTNESS_DEFAULT_VAL;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request, set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_BRIGHTNESS_STEP_SIZE;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request, set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                /*
                 * We don't support this control, so STALL the
                 * default control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * code.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;

        default:
            /*
             * We don't support this control, so STALL the control
             * ep.
             */
            resp->length = -EL2HLT;
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;
        }

        break;

    case 10:
        switch (cs) {
        case 9:
            switch (req)
            {
                case UVC_SET_CUR:
                    resp->length = 4;
                    break;
                case UVC_GET_LEN:
                    resp->data[0] = 0x04;
                    resp->data[1] = 0x00;
                    resp->length = 2;
                    break;
                case UVC_GET_INFO:
                    resp->data[0] = 0x03;
                    resp->length = 1;
                    break;
                default:
                    break;
            }
            break;
        default:
            switch (req)
            {
            case UVC_GET_MIN:
                resp->length = 4;
                break;
            case UVC_GET_LEN:
                resp->data[0] = 0x04;
                resp->data[1] = 0x00;
                resp->length = 2;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 1;
                resp->data[0] = cs;
                break;
            default:
                resp->length = 1;
                resp->data[0] = 0x06;
                break;
            }
            break;
        }

        break;
    default:
        /*
         * If we were not supposed to handle this
         * 'cs', prepare a Request Error Code response.
         */
        dev->request_error_code.data[0] = 0x06;
        dev->request_error_code.length = 1;
        break;
    }

    // printf("control request (req %02x cs %02x)\n", req, cs);
}

static void uvc_events_process_streaming(struct uvc_device* dev, uint8_t req, uint8_t cs,
                                         struct uvc_request_data* resp)
{
    struct uvc_streaming_control* ctrl;

    if ((cs != UVC_VS_PROBE_CONTROL) && (cs != UVC_VS_COMMIT_CONTROL))
    {
        return;
    }

    ctrl = (struct uvc_streaming_control*)&resp->data;
    resp->length = sizeof * ctrl;

    //request
    switch (req)
    {
        //0x01
    case UVC_SET_CUR:
        dev->control = cs;
        resp->length = 34;
        break;

        //0x81
    case UVC_GET_CUR:
        if (cs == UVC_VS_PROBE_CONTROL)
        {
            memcpy(ctrl, &dev->probe, sizeof * ctrl);
        }
        else
        {
            memcpy(ctrl, &dev->commit, sizeof * ctrl);
        }

        break;

        //0x82
    case UVC_GET_MIN:

        //0x83
    case UVC_GET_MAX:
        //uvc_fill_streaming_control(dev, ctrl, 0, 0);
        //break;

        //0x87
    case UVC_GET_DEF:
        uvc_fill_streaming_control(dev, ctrl, req == UVC_GET_MAX ? -1 : 0,
                                   req == UVC_GET_MAX ? -1 : 0);
        break;

        //0x84
    case UVC_GET_RES:
        memset(ctrl, 0, sizeof * ctrl);
        break;

        //0x85
    case UVC_GET_LEN:
        resp->data[0] = 0x00;
        resp->data[1] = 0x22;
        resp->length = 2;
        break;

        //0x86
    case UVC_GET_INFO:
        resp->data[0] = 0x03;
        resp->length = 1;
        break;
    }
}

static void set_probe_status(struct uvc_device* dev, int cs, int req)
{
    if (cs == 0x01)
    {
        switch (req)
        {
        case 0x01:
        {
            dev->probe_status.set = 1;
        }
            break;
        case 0x81:
        {
            dev->probe_status.get = 1;
        }
            break;
        case 0x82:
        {
            dev->probe_status.min = 1;
        }
            break;
        case 0x83:
        {
            dev->probe_status.max = 1;
        }
            break;
        case 0x84:
        {}
            break;

        case 0x85:
        {}
            break;

        case 0x86:
        {}
            break;
        }
    }
}

static int check_probe_status(struct uvc_device* dev)
{
    if ((dev->probe_status.get == 1)
       && (dev->probe_status.set == 1)
       && (dev->probe_status.min == 1)
       && (dev->probe_status.max == 1))
    {
        return 1;
    }

    RLOG("the probe status is not correct...\n");

    return 0;
}

static void uvc_events_process_class(struct uvc_device* dev, struct usb_ctrlrequest* ctrl,
                                     struct uvc_request_data* resp)
{
    unsigned char probe_status = 1;

    if (probe_status)
    {
        unsigned char type = ctrl->bRequestType & USB_RECIP_MASK;
        switch (type)
        {
        case USB_RECIP_INTERFACE:
#if 0
           // LOG("reqeust type :INTERFACE\n");
           // LOG("interface : %d\n", (ctrl->wIndex &0xff));
            LOG("unit id : %d\n", ((ctrl->wIndex & 0xff00)>>8));
            LOG("cs code : 0x%02x(%s)\n", (ctrl->wValue >> 8), (char*)get_interface_cs_s((ctrl->wValue >> 8)));
            LOG("req code: 0x%02x(%s)\n", ctrl->bRequest, (char*)getcode_s(ctrl->bRequest));
            LOG("################################################\n");
#endif
            set_probe_status(dev, (ctrl->wValue >> 8), ctrl->bRequest);
            break;
        case USB_RECIP_DEVICE:
            LOG("request type :DEVICE\n");
            break;
        case USB_RECIP_ENDPOINT:
            LOG("request type :ENDPOINT\n");
            break;
        case USB_RECIP_OTHER:
            LOG("request type :OTHER\n");
            break;
        }
    }

    if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
    {
        return;
    }

    //save unit id, interface id and control selector
    dev->control = (ctrl->wValue >> 8);
    dev->unit_id = ((ctrl->wIndex & 0xff00) >> 8);
    dev->interface_id = (ctrl->wIndex & 0xff);

    switch (ctrl->wIndex & 0xff)
    {
        //0x0   0x100
    case UVC_INTF_CONTROL:
        uvc_events_process_control(dev, ctrl->bRequest, ctrl->wValue >> 8, ctrl->wIndex >> 8, ctrl->wLength, resp);
        break;

        //0x1
    case UVC_INTF_STREAMING:
        uvc_events_process_streaming(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
        break;

    default:
        break;
    }
}

static void uvc_events_process_setup(struct uvc_device* dev, struct usb_ctrlrequest* ctrl,
                                     struct uvc_request_data* resp)
{
    dev->control = 0;
    dev->unit_id = 0;
    dev->interface_id = 0;

    printf(
        "\nbRequestType %02x bRequest %02x wValue %04x wIndex %04x "
        "wLength %04x\n",
        ctrl->bRequestType, ctrl->bRequest, ctrl->wValue, ctrl->wIndex, ctrl->wLength);

    switch (ctrl->bRequestType & USB_TYPE_MASK)
    {
    case USB_TYPE_STANDARD:
        uvc_events_process_standard(dev, ctrl, resp);
        break;

    case USB_TYPE_CLASS:
        uvc_events_process_class(dev, ctrl, resp);
        break;

    default:
        break;
    }
}

static const char* to_string(unsigned int format)
{
    switch (format)
    {
        case V4L2_PIX_FMT_H264:
            return "H264";
            break;

        case V4L2_PIX_FMT_HEVC:
            return "H265";
            break;

        case V4L2_PIX_FMT_MJPEG:
            return "MJPEG";
            break;

        case V4L2_PIX_FMT_YUYV:
            return "YUYV";
            break;

        case V4L2_PIX_FMT_YUV420:
            return "YUV420";
            break;

        case V4L2_PIX_FMT_NV12:
            return "NV12";
            break;

        default:
            return "unknown format";
            break;
    }
}

static void handle_control_interface_data(struct uvc_device *dev, struct uvc_request_data *data)
{
    switch (dev->unit_id)
    {
        case 1:
            break;
        case 2:
            break;
        case 10:
            switch (dev->control)
            {
                case 9:
                    /*todo*/
                    break;
                default:
                    break;
            }
            printf("control:%d length=%d 0x%x 0x%x \n", dev->control, data->length, data->data[0], data->data[1]);
            break;
        default:
            break;
    }
}

static void uvc_events_process_data(struct uvc_device* dev, struct uvc_request_data* data)
{
    struct uvc_streaming_control* target;
    struct uvc_streaming_control* ctrl;
    const struct uvc_format_info* format;
    const struct uvc_frame_info* frame;
    const unsigned int* interval;
    unsigned int iformat, iframe;
    unsigned int nframes;

    if ((dev->unit_id != 0) && (dev->interface_id == UVC_INTF_CONTROL))
    {
        return handle_control_interface_data(dev, data);
    }

    switch (dev->control)
    {
        case UVC_VS_PROBE_CONTROL:
           INFO("setting probe control, length = %d\n", data->length);
            target = &dev->probe;
            break;

        case UVC_VS_COMMIT_CONTROL:
           INFO("setting commit control, length = %d\n", data->length);
            target = &dev->commit;
            break;

        default:
           INFO("setting unknown control, length = %d\n", data->length);
            return;
    }

    ctrl = (struct uvc_streaming_control*)&data->data;

    // INFO("ctrl->bFormatIndex = %d\n", (unsigned int)ctrl->bFormatIndex);

    iformat = clamp((unsigned int)ctrl->bFormatIndex, 1U,
                    (unsigned int)ARRAY_SIZE(uvc_formats));

    //RLOG("set iformat = %d \n", iformat);

    format = &uvc_formats[iformat - 1];

    nframes = 0;

    // INFO("format->frames[nframes].width: %d\n", format->frames[nframes].width);
    // INFO("format->frames[nframes].height: %d\n", format->frames[nframes].height);

    while (format->frames[nframes].width != 0)
    {
        ++nframes;
    }

    iframe = clamp((unsigned int)ctrl->bFrameIndex, 1U, nframes);
    frame = &format->frames[iframe - 1];
    interval = frame->intervals;

    while ((interval[0] < ctrl->dwFrameInterval) && interval[1])
    {
        ++interval;
    }

    target->bFormatIndex = iformat;
    target->bFrameIndex = iframe;

    switch (format->fcc)
    {
        case V4L2_PIX_FMT_YUYV:
            target->dwMaxVideoFrameSize = frame->width * frame->height * 2;
            break;
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_NV12:
            target->dwMaxVideoFrameSize = frame->width * frame->height * 1.5;
            break;

        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_HEVC:

            if (dev->imgsize == 0)
            {
                LOG("WARNING: MJPEG requested and no image loaded.\n");
            }
            dev->width = frame->width;
            dev->height = frame->height;

            target->dwMaxVideoFrameSize = frame->width * frame->height * 2;
            break;
    }

    target->dwFrameInterval = *interval;

    #if 0
    INFO("set interval=%d format=%d frame=%d dwMaxPayloadTransferSize=%d ctrl->dwMaxPayloadTransferSize = %d\n",
         target->dwFrameInterval,
         target->bFormatIndex,
         target->bFrameIndex,
         target->dwMaxPayloadTransferSize,
         ctrl->dwMaxPayloadTransferSize);
    #endif

    if ((dev->control == UVC_VS_COMMIT_CONTROL) && check_probe_status(dev))
    {
        dev->fcc    = format->fcc;
        dev->width  = frame->width;
        dev->height = frame->height;


        uvc_video_set_format(dev);
        INFO("\nset device format=%s width=%d height=%d\n", to_string(dev->fcc), dev->width, dev->height);

        if (dev->bulk != 0)
        {
            disable_uvc_video(dev);
            enable_uvc_video(dev);
        }
    }

    if (dev->control == UVC_VS_COMMIT_CONTROL)
    {
        memset(&dev->probe_status, 0, sizeof (dev->probe_status));
    }
}

static void uvc_events_process(struct uvc_device* dev)
{
    struct v4l2_event v4l2_event;
    struct uvc_event* uvc_event = (struct uvc_event*)(void*)&v4l2_event.u.data;
    struct uvc_request_data resp;
    int ret;

    // INFO("#############uvc_events_process\n");

    ret = ioctl(dev->fd, VIDIOC_DQEVENT, &v4l2_event);

    if (ret < 0)
    {
        LOG("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno),
            errno);
        return;
    }

    memset(&resp, 0, sizeof resp);

    resp.length = -EL2HLT;

    switch (v4l2_event.type)
    {
        //0x08000000
    case UVC_EVENT_CONNECT:
        INFO("handle connect event \n");
        return;
        //0x08000001
    case UVC_EVENT_DISCONNECT:
        INFO("handle disconnect event\n");
        return;

        //0x08000004   UVC class
    case UVC_EVENT_SETUP:
        // INFO("handle setup event\n");
        uvc_events_process_setup(dev, &uvc_event->req, &resp);
        break;

        //0x08000005
    case UVC_EVENT_DATA:
        // INFO("handle data event\n");
        uvc_events_process_data(dev, &uvc_event->data);
        return;

        //0x08000002
    case UVC_EVENT_STREAMON:
        INFO("UVC_EVENT_STREAMON\n");
        if (!dev->bulk)
        {
            enable_uvc_video(dev);
        }
        return;

        //0x08000003
    case UVC_EVENT_STREAMOFF:
        INFO("UVC_EVENT_STREAMOFF\n");
        if (!dev->bulk)
        {
            disable_uvc_video(dev);
        }

        return;
    }

    ret = ioctl(dev->fd, UVCIOC_SEND_RESPONSE, &resp);
    if (ret < 0)
    {
        LOG("UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno),
            errno);
        return;
    }
}

static void uvc_events_init(struct uvc_device* dev)
{
    struct v4l2_event_subscription sub;

    uvc_fill_streaming_control(dev, &dev->probe, 0, 0);
    uvc_fill_streaming_control(dev, &dev->commit, 0, 0);

    if (dev->bulk)
    {
        /* FIXME Crude hack, must be negotiated with the driver. */
        dev->probe.dwMaxPayloadTransferSize  = dev->bulk_size;
        dev->commit.dwMaxPayloadTransferSize = dev->bulk_size;
    }

    memset(&sub, 0, sizeof sub);
    sub.type = UVC_EVENT_SETUP;
    ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DATA;
    ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMON;
    ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMOFF;
    ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}

static void
image_load(struct uvc_device *dev, const char *img)
{
	int fd = -1;

	if (img == NULL)
		return;

	fd = open(img, O_RDONLY);
	if (fd == -1) {
		printf("Unable to open MJPEG image '%s'\n", img);
		return;
	}
    dev->img_used = 0;
	// dev->imgsize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	dev->imgdata = malloc(dev->imgsize);
	if (dev->imgdata == NULL) {
		printf("Unable to allocate memory for MJPEG image\n");
		dev->imgsize = 0;
		return;
	}

	read(fd, dev->imgdata, dev->imgsize);
	close(fd);
}
/* ---------------------------------------------------------------------------
 * main
 */

static struct uvc_device *__uvc_device = 0;

int open_uvc_device(const char *devpath)
{
    struct uvc_device* dev;

    char* device = (char*)devpath;

    dev = uvc_open(device);
    if (dev == 0)
    {
        return -1;
    }

    dev->imgsize  = g_imagesize;
    dev->bulk = g_bulk;
    dev->bulk_size = g_bulk_size;

    dev->height = 720;
    dev->width = 1280;

   LOG("set imagesize = %d\n", dev->imgsize);
   LOG("set bulkmode = %d\n", dev->bulk);
   LOG("set bulksize = %d\n", dev->bulk_size);
   LOG("set g_standalone = %d\n", g_standalone);

    uvc_events_init(dev);
    uvc_video_init(dev);

    if(g_standalone)
        image_load(dev, "red.jpg");

    __uvc_device = dev;

    return 0;
}

int close_uvc_device()
{
    if (__uvc_device != 0)
    {
        disable_uvc_video(__uvc_device);
        uvc_close(__uvc_device);
    }

    __uvc_device = 0;

    return 0;
}


int run_uvc_data()
{
    fd_set efds,wfds;
    struct timeval tv;
    int r;

    if (!__uvc_device)
    {
        return -1;
    }

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&efds);
    FD_ZERO(&wfds);

    if (__uvc_device->streaming == 1)
    {
        FD_SET(__uvc_device->fd, &wfds);
    }

    r = select(__uvc_device->fd + 1, NULL, &wfds, NULL, &tv);
    if (r > 0)
    {
        if (FD_ISSET(__uvc_device->fd, &efds))
        {
        }
        if (FD_ISSET(__uvc_device->fd, &wfds))
        {
            uvc_video_process_userptr(__uvc_device);
        }
    }

    return r;
}

int run_uvc_device()
{
    fd_set efds, wfds;
    struct timeval tv;
    int r;

    if (!__uvc_device)
    {
        return -1;
    }

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&efds);
    FD_ZERO(&wfds);
    FD_SET(__uvc_device->fd, &efds);

    if (__uvc_device->streaming == 1)
    {
        // FD_SET(__uvc_device->fd, &wfds);
    }

    r = select(__uvc_device->fd + 1, NULL, NULL, &efds, &tv);
    if (r > 0)
    {
        if (FD_ISSET(__uvc_device->fd, &efds))
        {
            uvc_events_process(__uvc_device);
        }

        if (FD_ISSET(__uvc_device->fd, &wfds))
        {
            // uvc_video_process_userptr(__uvc_device);
        }
    }

    return r;
}
