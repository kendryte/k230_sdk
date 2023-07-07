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
#include "mapi_vdss_api.h"
#include "msg_client_dispatch.h"
#include "mapi_vvi_comm.h"
#include "mpi_vdss_api.h"
#include "msg_vdss.h"
#include "k_type.h"
#include "k_vdss_comm.h"

#include <stdio.h>

k_s32 kd_mapi_vdss_rst(k_u8 val)
{
    k_s32 ret;
    k_u8 res = 0;

    res = val;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VI, 0, 0), MSG_CMD_MEDIA_VDSS_RST,
            &res, sizeof(res), NULL);

    if(ret != K_SUCCESS) {
        printf("mapi_send_sync failed\n");
    }
    return ret;
}


k_s32 kd_mapi_vdss_start_pipe(k_u32 dev_num, k_u32 chn_num)
{
    k_s32 ret;
    msg_vdss_pipe_t pipe_attr;

    memset(&pipe_attr, 0 ,sizeof(pipe_attr));
    pipe_attr.vdss_dev = dev_num;
    pipe_attr.vdss_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VI, 0, 0), MSG_CMD_MEDIA_VDSS_PIPE_START,
            &pipe_attr, sizeof(pipe_attr), NULL);

    if(ret != K_SUCCESS) {
        printf("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vdss_stop_pipe(k_u32 dev_num, k_u32 chn_num)
{
    k_s32 ret;
    msg_vdss_pipe_t pipe;

    pipe.vdss_dev = dev_num;
    pipe.vdss_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VI, 0, 0), MSG_CMD_MEDIA_VDSS_PIPE_STOP,
            &pipe, sizeof(pipe), NULL);

    if(ret != K_SUCCESS) {
        printf("mapi_send_sync failed\n");
    }
    return ret;
}


k_s32 kd_mapi_vdss_set_chn_attr(k_u32 dev_num, k_u32 chn_num, k_vicap_chn_attr *attr)
{
    msg_vdss_chn_info info;
    k_s32 ret;

    if(dev_num >= VDSS_MAX_DEV_NUMS)
        return K_MAPI_ERR_VDSS_ILLEGAL_PARAM;
    if(chn_num >= VDSS_MAX_CHN_NUMS)
        return K_MAPI_ERR_VDSS_ILLEGAL_PARAM;
    if(!attr)
        return K_MAPI_ERR_VDSS_NULL_PTR;

    memset(&info, 0, sizeof(info));
    info.chn_num = chn_num;
    info.dev_num = dev_num;
    memcpy(&info.attr, attr, sizeof(k_vicap_chn_attr));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VI, 0, 0), MSG_CMD_MEDIA_VDSS_SET_CHN_ATTR,
            &info, sizeof(info), NULL);

    if(ret != K_SUCCESS) {
        printf("mapi_send_sync failed\n");
    }
    return ret;
}


k_s32 kd_mapi_vdss_set_dev_attr(k_vicap_dev_attr *attr)
{
    k_vicap_dev_attr info;
    k_s32 ret;
    if(!attr)
        return K_MAPI_ERR_VDSS_NULL_PTR;

    memset(&info, 0, sizeof(info));
    memcpy(&info, attr, sizeof(k_vicap_dev_attr));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VI, 0, 0), MSG_CMD_MEDIA_VDSS_SET_DEV_ATTR,
            &info, sizeof(info), NULL);

    if(ret != K_SUCCESS) {
        printf("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vdss_dump_frame(k_u32 dev_num, k_u32 chn_num, k_video_frame_info *vf_info, k_u32 timeout_ms)
{
    msg_vdss_frame_t info;
    k_s32 ret = 0;

    if(dev_num >= VDSS_MAX_DEV_NUMS)
        return K_MAPI_ERR_VDSS_INVALID_DEVID;
    if(chn_num >= VDSS_MAX_CHN_NUMS)
        return K_MAPI_ERR_VDSS_INVALID_CHNID;


    info.vdss_dev = dev_num;
    info.vdss_chn = chn_num;
    info.milli_sec = timeout_ms;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VI, 0, 0), MSG_CMD_MEDIA_VDSS_DUMP_FRAME,
            &info, sizeof(info), NULL);

    printf("@@@@  msg_vdss_frame_t  phy addr  is %x \n", info.vf_info.v_frame.phys_addr[0]);

    if(ret != K_SUCCESS) {
        printf("kd_mapi_vdss_dump_frame failed\n");
    }
    memcpy(vf_info, &info.vf_info, sizeof(k_video_frame_info));
    return ret;
}


k_s32 kd_mapi_vdss_chn_release_frame(k_u32 dev_num, k_u32 chn_num, const k_video_frame_info *vf_info)
{
    msg_vdss_frame_t info;
    k_s32 ret;

    if(dev_num >= VDSS_MAX_DEV_NUMS)
        return K_MAPI_ERR_VDSS_INVALID_DEVID;
    if(chn_num >= VDSS_MAX_CHN_NUMS)
        return K_MAPI_ERR_VDSS_INVALID_CHNID;

    info.vdss_dev = dev_num;
    info.vdss_chn = chn_num;

    memcpy(&info.vf_info, vf_info, sizeof(k_video_frame_info));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VI, 0, 0), MSG_CMD_MEDIA_VDSS_RELEASE_FRAME,
            &info, sizeof(info), NULL);

    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}