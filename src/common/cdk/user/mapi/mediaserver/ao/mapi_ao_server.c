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
#include "msg_ao.h"
#include "mapi_ao_api.h"
#include "mapi_ao_comm.h"
#include "mapi_sys_api.h"
#include "k_acodec_comm.h"


#define CHECK_MAPI_AO_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_ao_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AO_NULL_PTR;                                      \
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

k_s32 kd_mapi_ao_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ao_hdl)
{
    k_s32 ret;
    CHECK_MAPI_AO_NULL_PTR("dev_attr", dev_attr);

    ret = kd_mpi_ao_set_pub_attr(dev, dev_attr);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("kd_mpi_ao_set_pub_attr failed:0x%x\n", ret);
        return K_FAILED;
    }

    *ao_hdl = AO_CREATE_HANDLE(dev,chn);

    return ret;
}

k_s32 kd_mapi_ao_deinit(k_handle ao_hdl)
{
    acodec_check_close();
    return K_SUCCESS;
}

k_s32 kd_mapi_ao_start(k_handle ao_hdl)
{
    k_s32 ret;
    k_u32 dev;
    k_u32 chn;
    AO_GET_DEVICE_AND_CHANNEL(ao_hdl,dev,chn);

    if (dev != 0)
    {
        mapi_ao_error_trace("dev value not supported\n");
        return K_FAILED;
    }

    if (chn <0 || chn > 2)
    {
        mapi_ao_error_trace("chn value not supported\n");
        return K_FAILED;
    }

    ret = kd_mpi_ao_enable(dev);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("kd_mpi_ao_enable failed:0x%x\n", ret);
        return K_FAILED;
    }

    if (2 == chn)
    {
        for (int i =0;i < 2;i ++)
        {
            ret = kd_mpi_ao_enable_chn(dev,i);
            if(ret != K_SUCCESS) {
                mapi_ao_error_trace("kd_mpi_ao_enable_chn(%d) failed:0x%x\n",i, ret);
                return K_FAILED;
            }
        }
    }
    else
    {
        ret = kd_mpi_ao_enable_chn(dev,chn);
        if(ret != K_SUCCESS) {
        mapi_ao_error_trace("kd_mpi_ao_enable_chn failed:0x%x\n", ret);
        return K_FAILED;
        }
    }

    return ret;
}

k_s32 kd_mapi_ao_stop(k_handle ao_hdl)
{
    k_s32 ret;
    k_u32 dev;
    k_u32 chn;
    AO_GET_DEVICE_AND_CHANNEL(ao_hdl,dev,chn);

    if (dev != 0)
    {
        mapi_ao_error_trace("dev value not supported\n");
        return K_FAILED;
    }

    if (chn <0 || chn > 2)
    {
        mapi_ao_error_trace("chn value not supported\n");
        return K_FAILED;
    }

    if (2 == chn)
    {
        for (int i =0;i < 2;i ++)
        {
            ret = kd_mpi_ao_disable_chn(dev,i);
            if(ret != K_SUCCESS) {
            mapi_ao_error_trace("kd_mpi_ao_disable_chn(%d) failed:0x%x\n",i, ret);
            return K_FAILED;
            }
        }
    }
    else
    {
        ret = kd_mpi_ao_disable_chn(dev,chn);
        if(ret != K_SUCCESS) {
        mapi_ao_error_trace("kd_mpi_ao_disable_chn failed:0x%x\n", ret);
        return K_FAILED;
        }
    }

    ret = kd_mpi_ao_disable(dev);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("kd_mpi_ai_disable failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

k_s32 kd_mapi_ao_send_frame(k_handle ao_hdl, const k_audio_frame *frame)
{
    CHECK_MAPI_AO_NULL_PTR("frame", frame);

    k_s32 ret;
    k_u32 dev;
    k_u32 chn;
    AO_GET_DEVICE_AND_CHANNEL(ao_hdl,dev,chn);

    if (dev != 0)
    {
        mapi_ao_error_trace("dev value not supported\n");
        return K_FAILED;
    }

    if (chn <0 || chn > 1)
    {
        mapi_ao_error_trace("chn value not supported\n");
        return K_FAILED;
    }

    ret = kd_mpi_ao_send_frame(dev,chn,frame,1000);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("kd_mpi_ao_send_frame failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}


k_s32 kd_mapi_ao_set_volume(k_handle ao_hdl,float volume)
{
    float gain_value = volume;
    if (acodec_check_open())
        return K_FAILED;
    ioctl(g_acodec_fd, k_acodec_set_gain_hpoutl, &gain_value);
    ioctl(g_acodec_fd, k_acodec_set_gain_hpoutr, &gain_value);

    ioctl(g_acodec_fd, k_acodec_set_dacl_volume, &gain_value);
    ioctl(g_acodec_fd, k_acodec_set_dacr_volume, &gain_value);
    return K_SUCCESS;
}