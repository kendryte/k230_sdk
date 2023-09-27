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
#include <pthread.h>
#include "log.h"
#include "kstream.h"
#include "sample_venc.h"
#include "frame_cache.h"
#include "video.h"
#include "uvc_gadgete.h"

static struct kstream __k_stream = {
    .mpi_sc_ops = NULL,
    .streaming = 0,
};

static kstream *get_kstream()
{
    return &__k_stream;
}

static void _kstream_event_default_control(uint8_t req, struct uvc_request_data *resp)
{
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
        resp->data[0] = 0x01;
        break;
    default:
        resp->length = 1;
        resp->data[0] = 0x06;
        break;
    }
}

int kstream_init(void)
{
    if (get_kstream()->mpi_sc_ops &&
        get_kstream()->mpi_sc_ops->init)
        return get_kstream()->mpi_sc_ops->init();

    return 0;
}

int kstream_deinit(void)
{
    if (get_kstream()->mpi_sc_ops &&
        get_kstream()->mpi_sc_ops->deinit)
        return get_kstream()->mpi_sc_ops->deinit();

    return 0;
}

int kstream_startup(void)
{
    int ret = 0;

    if (get_kstream()->mpi_sc_ops &&
        get_kstream()->mpi_sc_ops->startup)
        ret = get_kstream()->mpi_sc_ops->startup();

    if (ret == 0)
        get_kstream()->streaming = 1;
    return ret;
}

int kstream_shutdown(void)
{
    int ret = 0;

    if (get_kstream()->mpi_sc_ops &&
        get_kstream()->mpi_sc_ops->shutdown)
        ret = get_kstream()->mpi_sc_ops->shutdown();

    if (ret == 0)
        get_kstream()->streaming = 0;
    return ret;
}

int kstream_set_enc_property(struct encoder_property *p)
{
    if (get_kstream()->mpi_sc_ops &&
        get_kstream()->mpi_sc_ops->set_property)
        return get_kstream()->mpi_sc_ops->set_property(p);

    return 0;
}

int kstream_set_enc_idr(void)
{
    if (get_kstream()->mpi_sc_ops &&
        get_kstream()->mpi_sc_ops->set_idr)
        return get_kstream()->mpi_sc_ops->set_idr();

    return 0;
}

int kstream_register_mpi_ops(struct stream_control_ops *sc_ops)
{
    if (sc_ops){
        get_kstream()->mpi_sc_ops = sc_ops;
    }

    return 0;
}

void release_kstream(kstream *stream)
{
}


