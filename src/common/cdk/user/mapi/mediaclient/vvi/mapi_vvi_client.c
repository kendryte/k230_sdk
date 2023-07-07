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
#include "mapi_vvi_api.h"
#include "msg_client_dispatch.h"
#include "mapi_vvi_comm.h"
#include "mpi_vvi_api.h"
#include "msg_vvi.h"
#include "k_type.h"


#define CHECK_MAPI_VVI_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_vvi_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_VVI_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

k_s32 kd_mapi_vvi_start_pipe(k_u32 dev, k_u32 chn, k_vvi_dev_attr *dev_attr, k_vvi_chn_attr *chn_attr)
{
    k_s32 ret;
    msg_vvi_pipe_attr_t pipe_attr;

    CHECK_MAPI_VVI_NULL_PTR("dev_attr", dev_attr);
    CHECK_MAPI_VVI_NULL_PTR("chn_attr", chn_attr);

    memset(&pipe_attr, 0 ,sizeof(pipe_attr));
    pipe_attr.vvi_dev = dev;
    pipe_attr.vvi_chn = chn;
    memcpy(&pipe_attr.dev_attr, dev_attr, sizeof(k_vvi_dev_attr));
    memcpy(&pipe_attr.chn_attr, chn_attr, sizeof(k_vvi_chn_attr));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_PIPE_START,
            &pipe_attr, sizeof(pipe_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vvi_stop_pipe(k_u32 dev, k_u32 chn)
{
    k_s32 ret;
    msg_vvi_pipe_t pipe;

    pipe.vvi_dev = dev;
    pipe.vvi_chn = chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_PIPE_STOP,
            &pipe, sizeof(pipe), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vvi_insert_pic(k_u32 dev, k_u32 chn, k_video_frame_info *vf_info)
{
    k_s32 ret;
    msg_vvi_frame_t frame;

    CHECK_MAPI_VVI_NULL_PTR("vf_info", vf_info);

    memset(&frame, 0, sizeof(frame));
    frame.vvi_dev = dev;
    frame.vvi_chn = chn;
    memcpy(&frame.vf_info, vf_info, sizeof(k_video_frame_info));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_INSERT_PIC,
            &frame, sizeof(frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vvi_remove_pic(k_u32 dev, k_u32 chn)
{
    k_s32 ret;
    msg_vvi_pipe_t pipe;

    pipe.vvi_dev = dev;
    pipe.vvi_chn = chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_REMOVE_PIC,
            &pipe, sizeof(pipe), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vvi_dump_frame(k_u32 dev, k_u32 chn, k_video_frame_info *vf_info, k_s32 milli_sec)
{
    k_s32 ret;
    msg_vvi_frame_t frame;

    CHECK_MAPI_VVI_NULL_PTR("vf_info", vf_info);

    memset(&frame, 0, sizeof(frame));
    frame.vvi_dev = dev;
    frame.vvi_chn = chn;
    frame.milli_sec = milli_sec;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_DUMP_FRAME,
            &frame, sizeof(frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }
    memcpy(vf_info, &frame.vf_info, sizeof(k_video_frame_info));

    return ret;
}

k_s32 kd_mapi_vvi_release_frame(k_u32 dev, k_u32 chn, const k_video_frame_info *vf_info)
{
    k_s32 ret;
    msg_vvi_frame_t frame;

    CHECK_MAPI_VVI_NULL_PTR("vf_info", vf_info);

    memset(&frame, 0, sizeof(frame));
    frame.vvi_dev = dev;
    frame.vvi_chn = chn;
    memcpy(&frame.vf_info, vf_info, sizeof(k_video_frame_info));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_RELEASE_FRAME,
            &frame, sizeof(frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_vvi_bind_vvo(k_u32 vvi_dev, k_u32 vvi_chn, k_u32 vvo_dev, k_u32 vvo_chn)
{
    k_s32 ret;
    msg_vvi_bind_vvo_t bind_info;

    bind_info.vvi_dev = vvi_dev;
    bind_info.vvo_dev = vvo_dev;
    bind_info.vvi_chn = vvi_chn;
    bind_info.vvo_chn = vvo_chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_BIND_VVO,
            &bind_info, sizeof(bind_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vvi_unbind_vvo(k_u32 vvi_dev, k_u32 vvi_chn, k_u32 vvo_dev, k_u32 vvo_chn)
{
    k_s32 ret;
    msg_vvi_bind_vvo_t unbind_info;

    unbind_info.vvi_dev = vvi_dev;
    unbind_info.vvo_dev = vvo_dev;
    unbind_info.vvi_chn = vvi_chn;
    unbind_info.vvo_chn = vvo_chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VVI, 0, 0), MSG_CMD_MEDIA_VVI_UNBIND_VVO,
            &unbind_info, sizeof(unbind_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}