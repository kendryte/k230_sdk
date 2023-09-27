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
#include <stdlib.h>
#include <pthread.h>
#include "mapi_ao_api.h"
#include "msg_client_dispatch.h"
#include "mapi_ao_comm.h"
#include "mpi_ao_api.h"
#include "msg_ao.h"
#include "k_type.h"


#define CHECK_MAPI_AO_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_ao_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AO_NULL_PTR;                                      \
        }                                                                      \
    } while (0)


k_s32 kd_mapi_ao_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ao_hdl)
{
    k_s32 ret;
    CHECK_MAPI_AO_NULL_PTR("dev_attr", dev_attr);
    k_msg_ao_pipe_attr_t ao_attr;
    ao_attr.ao_dev = dev;
    ao_attr.ao_chn = chn;
    ao_attr.attr = *dev_attr;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AO, 0, 0), MSG_CMD_MEDIA_AO_INIT,
        &ao_attr, sizeof(ao_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("mapi_send_sync failed\n");
    }

    *ao_hdl = ao_attr.ao_hdl;

    return ret;
}

k_s32 kd_mapi_ao_deinit(k_handle ao_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AO, 0, 0), MSG_CMD_MEDIA_AO_DEINIT,
        &ao_hdl, sizeof(ao_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ao_start(k_handle ao_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AO, 0, 0), MSG_CMD_MEDIA_AO_START,
        &ao_hdl, sizeof(ao_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ao_stop(k_handle ao_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AO, 0, 0), MSG_CMD_MEDIA_AO_STOP,
        &ao_hdl, sizeof(ao_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ao_send_frame(k_handle ao_hdl, const k_audio_frame *frame)
{
    CHECK_MAPI_AO_NULL_PTR("frame", frame);

    k_s32 ret;
    k_msg_ao_frame_t ao_frame;
    ao_frame.audio_frame = *frame;
    ao_frame.ao_hdl = ao_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AO, 0, 0), MSG_CMD_MEDIA_AO_SENDFRAME,
        &ao_frame, sizeof(ao_frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ao_set_volume(k_handle ao_hdl,float volume)
{
    k_s32 ret;
    k_msg_ao_gain_info  gain_info;
    gain_info.ao_hdl = ao_hdl;
    gain_info.gain = volume;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AO, 0, 0), MSG_CMD_MEDIA_AO_SETVOLUME,
        &gain_info, sizeof(gain_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}