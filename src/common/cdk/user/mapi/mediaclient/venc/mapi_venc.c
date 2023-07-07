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
#include "mapi_venc_api.h"
#include "msg_client_dispatch.h"
#include "mapi_venc_comm.h"
#include "mpi_venc_api.h"
#include "msg_venc.h"
#include "k_type.h"
#include "read_venc_data.h"

#define CHECK_MAPI_VENC_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_venc_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_VENC_NULL_PTR;                                      \
        }                                                                      \
    } while (0)


k_s32 venc_internuclear_fifo_create(k_u32 chn_num)
{
    mapi_venc_error_trace("fifo create###%d \n", chn_num);
    k_s32 ret;
    msg_venc_fifo_t venc_fifo;

    memset(&venc_fifo, 0, sizeof(venc_fifo));
    venc_fifo.fifo_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_INIT_DATAFIFO,
                         &venc_fifo, sizeof(venc_fifo), NULL);


    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }

    k_u64 datafifo_phyAddr;
    datafifo_phyAddr = venc_fifo.phyAddr;
    printf("create_datafifo datafifo_phyAddr %lx \n", datafifo_phyAddr);

    read_venc_data_init(chn_num, datafifo_phyAddr);
    return ret;
}



k_s32 venc_internuclear_fifo_delete(k_u32 chn_num)
{
    mapi_venc_error_trace("fifo delete###%d \n", chn_num);
    read_venc_data_deinit(chn_num);

    k_s32 ret;
    msg_venc_fifo_t venc_fifo;

    memset(&venc_fifo, 0, sizeof(venc_fifo));
    venc_fifo.fifo_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_DELETE_DATAFIFO,
                         &venc_fifo, sizeof(venc_fifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_venc_init(k_u32 chn_num, k_venc_chn_attr *pst_venc_attr)
{
    mapi_venc_error_trace("#####%d \n", chn_num);
    k_s32 ret;
    msg_venc_chn_attr_t venc_chn_attr;

    CHECK_MAPI_VENC_NULL_PTR("k_venc_chn_attr", pst_venc_attr);

    memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
    venc_chn_attr.venc_chn = chn_num;
    memcpy(&venc_chn_attr.chn_attr, pst_venc_attr, sizeof(k_venc_chn_attr));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_INIT,
                         &venc_chn_attr, sizeof(venc_chn_attr), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }


    venc_internuclear_fifo_create(chn_num);

    return ret;
}

k_s32 kd_mapi_venc_deinit(k_u32 chn_num)
{
    venc_internuclear_fifo_delete(chn_num);

    k_s32 ret;
    msg_venc_chn_t venc_attr;

    memset(&venc_attr, 0, sizeof(venc_attr));
    venc_attr.venc_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_DEINIT,
                         &venc_attr, sizeof(venc_attr), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_venc_start(k_s32 chn_num, k_s32 s32_frame_cnt)
{
    mapi_venc_error_trace("%d ", chn_num);
    k_s32 ret;
    msg_venc_start_t venc_start;

    memset(&venc_start, 0, sizeof(venc_start));
    venc_start.venc_chn = chn_num;
    venc_start.s32FrameCnt = s32_frame_cnt;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_START,
                         &venc_start, sizeof(venc_start), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_venc_stop(k_s32 chn_num)
{
    k_s32 ret;
    msg_venc_stop_t venc_stop;

    memset(&venc_stop, 0, sizeof(venc_stop));
    venc_stop.venc_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_STOP,
                         &venc_stop, sizeof(venc_stop), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_venc_bind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num)
{
    k_s32 ret;
    msg_venc_bind_vi_t bind_info;

    bind_info.src_dev = src_dev;
    bind_info.src_chn = src_chn;
    bind_info.venc_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_BIND_VPROC,
                         &bind_info, sizeof(bind_info), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_venc_unbind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num)
{
    k_s32 ret;
    msg_venc_bind_vi_t unbind_info;

    unbind_info.src_dev = src_dev;
    unbind_info.src_chn = src_chn;
    unbind_info.venc_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_UBIND_VPROC,
                         &unbind_info, sizeof(unbind_info), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}




k_s32 kd_mapi_venc_registercallback(k_u32 chn_num, kd_venc_callback_s *pst_venc_cb)
{
    k_s32 ret;
    msg_venc_callback_attr venc_callback_msg;

    CHECK_MAPI_VENC_NULL_PTR("pst_venc_cb", pst_venc_cb);

    memset(&venc_callback_msg, 0, sizeof(venc_callback_msg));
    venc_callback_msg.venc_chn = chn_num;
    memcpy(&venc_callback_msg.callback_attr, pst_venc_cb, sizeof(kd_venc_callback_s));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_REGISTER_CALLBACK,
                         &venc_callback_msg, sizeof(venc_callback_msg), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_venc_unregistercallback(k_u32 chn_num, kd_venc_callback_s *pst_venc_cb)
{
    k_s32 ret;
    msg_venc_callback_attr venc_callback_msg;

    CHECK_MAPI_VENC_NULL_PTR("pst_venc_cb", pst_venc_cb);

    memset(&venc_callback_msg, 0, sizeof(venc_callback_msg));
    venc_callback_msg.venc_chn = chn_num;
    memcpy(&venc_callback_msg.callback_attr, pst_venc_cb, sizeof(kd_venc_callback_s));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_UNREGISTER_CALLBACK,
                         &venc_callback_msg, sizeof(venc_callback_msg), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}


k_s32 kd_mapi_venc_enable_idr(k_s32 chn_num, k_bool idr_enable)
{
    k_s32 ret;
    msg_venc_chn_idr_t venc_idr_chn;

    memset(&venc_idr_chn, 0, sizeof(venc_idr_chn));
    venc_idr_chn.venc_chn = chn_num;
    venc_idr_chn.idr_enable = idr_enable;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_ENABLE_IDR,
                         &venc_idr_chn, sizeof(venc_idr_chn), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_venc_request_idr(k_s32 chn_num)
{
    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_REQUEST_IDR,
                         &chn_num, sizeof(chn_num), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}