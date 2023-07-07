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
#include <unistd.h>
#include "msg_ai.h"
#include "mapi_ai_api.h"
#include "mpi_ai_api.h"
#include "mapi_ai_comm.h"
#include "mapi_sys_api.h"


#define CHECK_MAPI_AI_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_ai_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AI_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

k_s32 kd_mapi_ai_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ai_hdl)
{
    CHECK_MAPI_AI_NULL_PTR("dev_attr", dev_attr);
    k_s32 ret;
    ret = kd_mpi_ai_set_pub_attr(dev, dev_attr);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_set_pub_attr failed:0x%x\n", ret);
        return K_FAILED;
    }

    *ai_hdl = AI_CREATE_HANDLE(dev,chn);

    return ret;
}

k_s32 kd_mapi_ai_deinit(k_handle ai_hdl)
{

    return K_SUCCESS;
}

k_s32 kd_mapi_ai_start(k_handle ai_hdl)
{
    k_s32 ret;
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    //i2s
    if (0 == dev)
    {
        if (chn <0 || chn >1)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    //pdm
    else if (1 == dev)
    {
        if (chn <0 || chn > 3)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    else
    {
        mapi_ai_error_trace("dev value not supported\n");
        return K_FAILED;
    }


    ret = kd_mpi_ai_enable(dev);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_enable failed:0x%x\n", ret);
        return K_FAILED;
    }

    if (1 == dev)
    {
        //pdm enable must from small to large
        for (int i =0;i <= chn;i++)
        {
           ret = kd_mpi_ai_enable_chn(dev,i);
        }
    }
    else
    {
        ret = kd_mpi_ai_enable_chn(dev,chn);
    }


    return ret;
}

k_s32 kd_mapi_ai_stop(k_handle ai_hdl)
{
    k_s32 ret;
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    //i2s
    if (0 == dev)
    {
        if (chn <0 || chn >1)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    //pdm
    else if (1 == dev)
    {
        if (chn <0 || chn > 3)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    else
    {
        mapi_ai_error_trace("dev value not supported\n");
        return K_FAILED;
    }

    if (1 == dev)
    {
        //pdm disable must from large to small
        for (int i = chn - 1; i >= 0; i --)
        {
            ret = kd_mpi_ai_disable_chn(dev, i);
        }
    }
    else
    {
        ret = kd_mpi_ai_disable_chn(dev,chn);
    }

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_disable_chn failed:0x%x\n", ret);
        return K_FAILED;
    }

    ret = kd_mpi_ai_disable(dev);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_disable failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

k_s32 kd_mapi_ai_get_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_get_frame(dev,chn,frame,1000);
}

k_s32 kd_mapi_ai_release_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_release_frame(dev,chn,frame);
}