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
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include "msg_ai.h"
#include "mapi_ai_api.h"
#include "mpi_ai_api.h"
#include "mapi_ai_comm.h"
#include "mapi_sys_api.h"
#include "k_acodec_comm.h"


#define CHECK_MAPI_AI_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_ai_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AI_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

static k_s32 g_acodec_fd = -1;
static k_s32 acodec_check_open(void)
{
    if (g_acodec_fd < 0)
    {
        g_acodec_fd = open("/dev/acodec_device", O_RDWR);
        if (g_acodec_fd < 0)
        {
            perror("open err\n");
            return -1;
        }
    }
    return 0;
}

static k_s32 acodec_check_close(void)
{
    if (g_acodec_fd >= 0)
    {
        close(g_acodec_fd);
        g_acodec_fd = -1;
    }
    return 0;
}

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
    acodec_check_close();
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
        for (int i = chn; i >= 0; i --)
        {
            ret = kd_mpi_ai_disable_chn(dev, i);
        }
    }
    else
    {
        ret = kd_mpi_ai_disable_chn(dev,chn);
    }

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_disable_chn failed:0x%x,dev:%d,channel:%d\n", ret,dev,chn);
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

k_s32 kd_mapi_ai_set_pitch_shift_attr(k_handle ai_hdl, const k_ai_chn_pitch_shift_param *param)
{
    CHECK_MAPI_AI_NULL_PTR("param", param);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_set_pitch_shift_attr(dev,chn, param);
}

k_s32 kd_mapi_ai_get_pitch_shift_attr(k_handle ai_hdl, k_ai_chn_pitch_shift_param *param)
{
    CHECK_MAPI_AI_NULL_PTR("param", param);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_get_pitch_shift_attr(dev,chn, param);
}

k_s32 kd_mapi_ai_bind_ao(k_handle ai_hdl,k_handle ao_hdl)
{
    k_mpp_chn ai_mpp_chn;
    k_mpp_chn ao_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ((ai_hdl) >> 16) & 0xFFFF;
    ai_mpp_chn.chn_id = (ai_hdl) & 0xFFFF;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ((ao_hdl) >> 16) & 0xFFFF;
    ao_mpp_chn.chn_id = (ao_hdl) & 0xFFFF;

    return kd_mpi_sys_bind(&ai_mpp_chn, &ao_mpp_chn);
}

k_s32 kd_mapi_ai_unbind_ao(k_handle ai_hdl,k_handle ao_hdl)
{
    k_mpp_chn ai_mpp_chn;
    k_mpp_chn ao_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ((ai_hdl) >> 16) & 0xFFFF;
    ai_mpp_chn.chn_id = (ai_hdl) & 0xFFFF;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ((ao_hdl) >> 16) & 0xFFFF;
    ao_mpp_chn.chn_id = (ao_hdl) & 0xFFFF;

    return kd_mpi_sys_unbind(&ai_mpp_chn, &ao_mpp_chn);
}

k_s32 kd_mapi_ai_set_volume(k_handle ai_hdl,float volume)
{
    k_s32 ret;
    float gain_value = volume;
    if (acodec_check_open())
        return K_FAILED;

    ioctl(g_acodec_fd, k_acodec_set_alc_gain_micl, &gain_value);
    ioctl(g_acodec_fd, k_acodec_set_alc_gain_micr, &gain_value);

    return 0;
}

k_s32 kd_mapi_acodec_reset()
{
    if (acodec_check_open())
        return K_FAILED;

    ioctl(g_acodec_fd, k_acodec_reset, NULL);
    return 0;
}