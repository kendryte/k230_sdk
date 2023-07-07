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
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "msg_vicap.h"
#include "mapi_vicap_api.h"
#include "mapi_vicap_comm.h"
#include "mapi_sys_api.h"
#include "mpi_vicap_api.h"
#include "k_vicap_comm.h"

#define CHECK_MAPI_VICAP_NULL_PTR(paraname, ptr)                      \
    do {                                                              \
        if ((ptr) == NULL) {                                          \
            mapi_vicap_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_VICAP_NULL_PTR;                         \
        }                                                             \
    } while (0)

#define MAPI_VICAP_UNUSED(x)    ((x)=(x))

static k_vicap_sensor_info sensor_info[VICAP_DEV_ID_MAX];

k_s32 kd_mapi_vicap_get_sensor_fd(k_vicap_sensor_attr *sensor_attr)
{
    k_s32 ret = 0;
    CHECK_MAPI_VICAP_NULL_PTR("sensor_attr", sensor_attr);
    ret = kd_mpi_vicap_get_sensor_fd(sensor_attr);
    if(ret != K_SUCCESS)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_get_sensor_fd failed:0x%x\n", ret);
        return VICAP_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_vicap_dump_frame(k_vicap_dev dev_num, k_vicap_chn chn_num, k_vicap_dump_format foramt, k_video_frame_info *vf_info, k_u32 milli_sec)
{
    k_s32 ret = 0;
    CHECK_MAPI_VICAP_NULL_PTR("vf_info", vf_info);
    ret = kd_mpi_vicap_dump_frame(dev_num, chn_num, foramt, vf_info, milli_sec);
    if(ret != K_SUCCESS)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_dump_frame failed:0x%x\n", ret);
        return VICAP_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_vicap_release_frame(k_vicap_dev dev_num, k_vicap_chn chn_num, const k_video_frame_info *vf_info)
{
    k_s32 ret;
    CHECK_MAPI_VICAP_NULL_PTR("vf_info", vf_info);
    ret = kd_mpi_vicap_dump_release(dev_num, chn_num, vf_info);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("kd_mapi_vicap_release_frame failed:0x:%x\n", ret);
        return VICAP_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_vicap_get_sensor_info(k_vicap_sensor_info *sensor_info)
{
    k_s32 ret;
    CHECK_MAPI_VICAP_NULL_PTR("sensor_info", sensor_info);
    ret = kd_mpi_vicap_get_sensor_info(sensor_info->sensor_type, sensor_info);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("kd_mapi_vicap_get_sensor_info failed:0x:%x\n", ret);
        return VICAP_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_vicap_set_dev_attr(k_vicap_dev_set_info dev_info)
{
    k_s32 ret = 0;

    /* dev attr */
    if(dev_info.vicap_dev >= VICAP_DEV_ID_MAX || dev_info.vicap_dev < VICAP_DEV_ID_0)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_set_dev_attr failed, dev_num %d out of range\n", dev_info.vicap_dev);
        return K_FAILED;
    }

    if(dev_info.sensor_type > SENSOR_TYPE_MAX || dev_info.sensor_type < OV_OV9732_MIPI_1920X1080_30FPS_10BIT_LINEAR)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_set_dev_attr failed, sensor_type %d out of range\n", dev_info.sensor_type);
        return K_FAILED;
    }

    k_vicap_dev_attr dev_attr;

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    memset(&sensor_info[dev_info.vicap_dev], 0, sizeof(k_vicap_sensor_info));

    sensor_info[dev_info.vicap_dev].sensor_type = dev_info.sensor_type;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info[dev_info.vicap_dev].sensor_type, &sensor_info[dev_info.vicap_dev]);
    if(ret)
    {
        printf("kd_mpi_vicap_get_sensor_info failed:0x%x\n", ret);
        return K_FAILED;
    }
    dev_attr.dw_enable = dev_info.dw_en;
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = sensor_info[dev_info.vicap_dev].width;
    dev_attr.acq_win.height = sensor_info[dev_info.vicap_dev].height;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;
    dev_attr.pipe_ctrl.data = dev_info.pipe_ctrl.data;
    // af need disable
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info[dev_info.vicap_dev], sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_set_dev_attr(dev_info.vicap_dev, dev_attr);
    if(ret)
    {
        mapi_vicap_error_trace("kd_mpi_vicap_set_dev_attr failed:0x%x\n", ret);
        return K_FAILED;
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_vicap_set_chn_attr(k_vicap_chn_set_info chn_info)
{
    k_s32 ret = 0;
    if(chn_info.vicap_dev >= VICAP_DEV_ID_MAX || chn_info.vicap_dev < VICAP_DEV_ID_0)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_set_dev_attr failed, dev_num %d out of range\n", chn_info.vicap_dev);
        return K_FAILED;
    }

    if(chn_info.vicap_chn >= VICAP_CHN_ID_MAX || chn_info.vicap_chn < VICAP_CHN_ID_0)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_set_attr failed, chn_num %d out of range\n", chn_info.vicap_chn);
        return K_FAILED;
    }

    k_vicap_chn_attr chn_attr;
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));

    /* chn attr */
    if(!chn_info.out_height && chn_info.out_width)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_set_attr, failed, out_width: %d, out_height: %d error\n", chn_info.out_width, chn_info.out_height);
        return K_FAILED;
    }

    if(chn_info.pixel_format > PIXEL_FORMAT_BUTT || chn_info.pixel_format < PIXEL_FORMAT_RGB_444)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_set_attr, failed, pixel_formatr: %d out of range\n", chn_info.pixel_format);
        return K_FAILED;
    }

    if(chn_info.pixel_format == PIXEL_FORMAT_RGB_BAYER_10BPP || chn_info.pixel_format == PIXEL_FORMAT_RGB_BAYER_12BPP)
    {
        chn_attr.out_win.h_start = 0;
        chn_attr.out_win.v_start = 0;
        chn_attr.out_win.width = sensor_info[chn_info.vicap_dev].width;
        chn_attr.out_win.height = sensor_info[chn_info.vicap_dev].height;
    }
    else
    {
        chn_attr.out_win.width = chn_info.out_width;
        chn_attr.out_win.height = chn_info.out_height;
    }

    if(chn_info.crop_en)
    {
        chn_attr.crop_win.h_start = chn_info.crop_h_start;
        chn_attr.crop_win.v_start = chn_info.crop_v_start;
        chn_attr.crop_win.width = chn_info.out_width;
        chn_attr.crop_win.height = chn_info.out_height;
    }
    else
    {
        chn_attr.crop_win.h_start = 0;
        chn_attr.crop_win.v_start = 0;
        chn_attr.crop_win.width = sensor_info[chn_info.vicap_dev].width;
        chn_attr.crop_win.height = sensor_info[chn_info.vicap_dev].height;
    }

    chn_attr.scale_win.h_start = 0;
    chn_attr.scale_win.v_start = 0;
    chn_attr.scale_win.width = sensor_info[chn_info.vicap_dev].width;
    chn_attr.scale_win.height = sensor_info[chn_info.vicap_dev].height;

    chn_attr.crop_enable = chn_info.crop_en;
    chn_attr.scale_enable = chn_info.scale_en;
    chn_attr.chn_enable = chn_info.chn_en;
    chn_attr.pix_format = chn_info.pixel_format;
    chn_attr.buffer_num = 6;
    chn_attr.buffer_size = chn_info.buf_size;
    ret = kd_mpi_vicap_set_chn_attr(chn_info.vicap_dev, chn_info.vicap_chn, chn_attr);
    if(ret)
    {
        mapi_vicap_error_trace("kd_mpi_vicap_set_chn_attr failed:0x%x\n", ret);
        return K_FAILED;
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_vicap_start(k_vicap_dev vicap_dev)
{
    k_s32 ret = 0;
    if(vicap_dev > VICAP_DEV_ID_MAX || vicap_dev < VICAP_DEV_ID_0)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_start failed, dev_num %d out of range\n", vicap_dev);
        return K_FAILED;
    }
    ret = kd_mpi_vicap_init(vicap_dev);
    if(ret)
    {
        mapi_vicap_error_trace("kd_mpi_vicap_init, vicap dev(%d) init failed.\n", vicap_dev);
        kd_mpi_vicap_deinit(vicap_dev);
        return K_FAILED;
    }

    ret = kd_mpi_vicap_start_stream(vicap_dev);
    if(ret)
    {
        mapi_vicap_error_trace("kd_mpi_vicap_start_stream, vicap dev(%d) start stream failed.\n", vicap_dev);
        kd_mpi_vicap_deinit(vicap_dev);
        kd_mpi_vicap_stop_stream(vicap_dev);
        return K_FAILED;
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_vicap_stop(k_vicap_dev vicap_dev)
{
    k_s32 ret = 0;
    if(vicap_dev > VICAP_DEV_ID_MAX || vicap_dev < VICAP_DEV_ID_0)
    {
        mapi_vicap_error_trace("kd_mapi_vicap_stop failed, dev_num %d out of range\n", vicap_dev);
        return K_FAILED;
    }
    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if(ret)
    {
        mapi_vicap_error_trace("kd_mpi_vicap_stop_stream failed\n");
    }
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if(ret)
    {
        mapi_vicap_error_trace("kd_mpi_vicap_deinit failed\n");
        return K_FAILED;
    }
    return K_SUCCESS;
}

/* vicap set attr */