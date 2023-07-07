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

#ifndef __UVC_GADGETE_H__
#define __UVC_GADGETE_H__
#include "uvc.h"
#include "video.h"
#include <stdint.h>

typedef struct uvc_probe_t
{
    unsigned char set;
    unsigned char get;
    unsigned char max;
    unsigned char min;
} uvc_probe_t;

struct uvc_device
{
    int fd;
    struct uvc_streaming_control probe;
    struct uvc_streaming_control commit;
    int control;
    int unit_id;
    int interface_id;
    unsigned int fcc;
    unsigned int width;
    unsigned int height;
    unsigned int nbufs;
    unsigned int bulk;
    uint8_t      color;
    unsigned int imgsize;
    unsigned int img_used;
    unsigned int bulk_size;

	unsigned int mjpg_sz[4];
	void *mjpg_buff[4];
	void *imgdata;

    uvc_probe_t  probe_status;
    int streaming;

    struct buffer *dummy_buf;
    struct buffer *mem;
    unsigned int brightness_val;
    /* USB speed specific */
    int mult;
    int burst;
    int maxpkt;
    enum usb_device_speed speed;

    struct uvc_request_data request_error_code;
};

int open_uvc_device(const char *devpath);
int close_uvc_device();
int run_uvc_device();
int run_uvc_data();

#endif //__UVC_GADGETE_H__
