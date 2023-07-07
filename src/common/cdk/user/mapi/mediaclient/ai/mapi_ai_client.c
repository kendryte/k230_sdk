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
#include "mapi_ai_api.h"
#include "msg_client_dispatch.h"
#include "mapi_ai_comm.h"
#include "mpi_ai_api.h"
#include "msg_ai.h"
#include "k_type.h"


#define CHECK_MAPI_AI_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_ai_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AI_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

k_s32 kd_mapi_ai_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ai_hdl)
{
    k_s32 ret;
    CHECK_MAPI_AI_NULL_PTR("dev_attr", dev_attr);
    k_msg_ai_pipe_attr_t ai_attr;
    ai_attr.ai_dev = dev;
    ai_attr.ai_chn = chn;
    ai_attr.attr = *dev_attr;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_INIT,
        &ai_attr, sizeof(ai_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    *ai_hdl = ai_attr.ai_hdl;

    return ret;
}

k_s32 kd_mapi_ai_deinit(k_handle ai_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_DEINIT,
        &ai_hdl, sizeof(ai_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_start(k_handle ai_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_START,
        &ai_hdl, sizeof(ai_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_stop(k_handle ai_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_STOP,
        &ai_hdl, sizeof(ai_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_get_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);

    k_s32 ret;
    k_msg_ai_frame_t ai_frame;
    ai_frame.ai_hdl = ai_hdl;
    ai_frame.audio_frame = *frame;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_GETFRAME,
        &ai_frame, sizeof(ai_frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_release_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);

    k_s32 ret;
    k_msg_ai_frame_t ai_frame;
    ai_frame.ai_hdl = ai_hdl;
    ai_frame.audio_frame = *frame;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_RELEASEFRAME,
        &ai_frame, sizeof(ai_frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}