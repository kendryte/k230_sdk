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
#include "mapi_dpu_api.h"
#include "msg_client_dispatch.h"
#include "mapi_dpu_comm.h"
#include "mpi_dpu_api.h"
#include "msg_dpu.h"
#include "k_type.h"
#include "read_dpu_data.h"

#define CHECK_MAPI_DPU_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_dpu_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_DPU_NULL_PTR;                                      \
        }                                                                      \
    } while (0)


k_s32 dpu_internuclear_fifo_create(k_u32 chn_num)
{
    k_s32 ret;
    msg_dpu_fifo_t dpu_fifo;

    memset(&dpu_fifo, 0, sizeof(dpu_fifo));
    dpu_fifo.fifo_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_INIT_DATAFIFO,
                         &dpu_fifo, sizeof(dpu_fifo), NULL);


    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }

    k_u64 datafifo_phyAddr;
    datafifo_phyAddr = dpu_fifo.phyAddr;
    printf("create_datafifo datafifo_phyAddr %lx \n", datafifo_phyAddr);

    read_dpu_data_init(chn_num, datafifo_phyAddr);
    return ret;
}

k_s32 dpu_internuclear_fifo_delete(k_u32 chn_num)
{
    printf("%s\n", __FUNCTION__);
    read_dpu_data_deinit(chn_num);

    k_s32 ret;
    msg_dpu_fifo_t dpu_fifo;

    memset(&dpu_fifo, 0, sizeof(dpu_fifo));
    dpu_fifo.fifo_chn = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_DELETE_DATAFIFO,
                         &dpu_fifo, sizeof(dpu_fifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_dpu_init(k_dpu_info_t *init)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret = 0;

    CHECK_MAPI_DPU_NULL_PTR("k_dpu_info_t", init);
    if(init->dev_cnt > VICAP_DEV_ID_MAX)
        return K_MAPI_ERR_DPU_ILLEGAL_PARAM;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_INIT,
                         init, sizeof(k_dpu_info_t), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }

    for(int i = 0; i < init->dev_cnt; i++)
    {
        dpu_internuclear_fifo_create(i);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_close(k_u32 dev_cnt)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;

    if(dev_cnt > VICAP_DEV_ID_MAX)
        return K_MAPI_ERR_DPU_ILLEGAL_PARAM;
    for(int i = 0; i < dev_cnt; i++)
    {
        dpu_internuclear_fifo_delete(i);
    }

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_CLOSE,
                         NULL, 0, NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_dpu_start_grab()
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;
    msg_dpu_callback_attr dpu_callback_msg;

    memset(&dpu_callback_msg, 0, sizeof(dpu_callback_msg));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_START_GRAB,
                         &dpu_callback_msg, sizeof(dpu_callback_msg), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_stop_grab()
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;
    msg_dpu_callback_attr dpu_callback_msg;

    memset(&dpu_callback_msg, 0, sizeof(dpu_callback_msg));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_STOP_GRAB,
                         &dpu_callback_msg, sizeof(dpu_callback_msg), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_dpu_registercallback(k_u32 dev_num, kd_dpu_callback_s *pst_dpu_cb)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;
    msg_dpu_callback_attr dpu_callback_msg;

    if(dev_num > VICAP_DEV_ID_MAX)
        return K_MAPI_ERR_DPU_ILLEGAL_PARAM;
    CHECK_MAPI_DPU_NULL_PTR("pst_dpu_cb", pst_dpu_cb);

    memset(&dpu_callback_msg, 0, sizeof(dpu_callback_msg));
    dpu_callback_msg.dpu_chn = dev_num;
    memcpy(&dpu_callback_msg.callback_attr, pst_dpu_cb, sizeof(kd_dpu_callback_s));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_REGISTER_CALLBACK,
                         &dpu_callback_msg, sizeof(dpu_callback_msg), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_dpu_unregistercallback(k_u32 dev_num, kd_dpu_callback_s *pst_dpu_cb)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;
    msg_dpu_callback_attr dpu_callback_msg;

    if(dev_num > VICAP_DEV_ID_MAX)
        return K_MAPI_ERR_DPU_ILLEGAL_PARAM;
    CHECK_MAPI_DPU_NULL_PTR("pst_dpu_cb", pst_dpu_cb);

    memset(&dpu_callback_msg, 0, sizeof(dpu_callback_msg));
    dpu_callback_msg.dpu_chn = dev_num;
    memcpy(&dpu_callback_msg.callback_attr, pst_dpu_cb, sizeof(kd_dpu_callback_s));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_UNREGISTER_CALLBACK,
                         &dpu_callback_msg, sizeof(dpu_callback_msg), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_dpu_release_frame(k_u32 dev_num)
{
    k_s32 ret = 0;

    if(dev_num > VICAP_DEV_ID_MAX)
        return K_MAPI_ERR_DPU_ILLEGAL_PARAM;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_DPU, 0, 0), MSG_CMD_MEDIA_DPU_RELEASE_FRAME,
                         &dev_num, sizeof(k_dpu_info_t), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}