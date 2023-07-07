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
#include "msg_vvi.h"
#include "mapi_vvi_api.h"
#include "mapi_vvi_comm.h"
#include "mapi_sys_api.h"

#define CHECK_MAPI_VVI_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_vvi_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_VVI_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

#define MAPI_VVI_UNUSED(x)    ((x)=(x))

k_s32 kd_mapi_vvi_start_pipe(k_u32 dev, k_u32 chn, k_vvi_dev_attr *dev_attr, k_vvi_chn_attr *chn_attr)
{
    k_s32 ret;

    CHECK_MAPI_VVI_NULL_PTR("dev_attr", dev_attr);
    CHECK_MAPI_VVI_NULL_PTR("chn_attr", chn_attr);

    ret = kd_mpi_vvi_set_dev_attr(dev, dev_attr);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_set_dev_attr failed:0x%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    ret = kd_mpi_vvi_set_chn_attr(chn, chn_attr);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_set_chn_attr failed:0x%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    ret = kd_mpi_vvi_start_pipe(dev, chn);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_start_pipe failed:0x%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_vvi_stop_pipe(k_u32 dev, k_u32 chn)
{
    return kd_mpi_vvi_stop_pipe(dev, chn);
}

k_s32 kd_mapi_vvi_insert_pic(k_u32 dev, k_u32 chn, k_video_frame_info *vf_info)
{
    k_s32 ret;

    CHECK_MAPI_VVI_NULL_PTR("vf_info", vf_info);
    MAPI_VVI_UNUSED(dev);

    ret = kd_mpi_vvi_chn_insert_pic(chn, vf_info);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_chn_insert_pic faield:0x%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_vvi_remove_pic(k_u32 dev, k_u32 chn)
{
    k_s32 ret;

    MAPI_VVI_UNUSED(dev);
    ret = kd_mpi_vvi_chn_remove_pic(chn);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_chn_remove_pic faield:0x%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_vvi_dump_frame(k_u32 dev, k_u32 chn, k_video_frame_info *vf_info, k_s32 milli_sec)
{
    k_s32 ret;
    CHECK_MAPI_VVI_NULL_PTR("vf_info", vf_info);
    MAPI_VVI_UNUSED(dev);

    ret = kd_mpi_vvi_chn_dump_request(chn);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_chn_dump_request failed:0x%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    ret = kd_mpi_vvi_chn_dump_frame(chn, vf_info, milli_sec);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_chn_dump_frame failed:0x%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_vvi_release_frame(k_u32 dev, k_u32 chn, const k_video_frame_info *vf_info)
{
    k_s32 ret;

    CHECK_MAPI_VVI_NULL_PTR("vf_info", vf_info);
    MAPI_VVI_UNUSED(dev);

    ret = kd_mpi_vvi_chn_dump_release(chn, vf_info);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mpi_vvi_chn_dump_release failed:0x:%x\n", ret);
        return VVI_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_vvi_bind_vvo(k_u32 vvi_dev, k_u32 vvi_chn, k_u32 vvo_dev, k_u32 vvo_chn)
{
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn vvo_mpp_chn;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = vvi_dev;
    vvi_mpp_chn.chn_id = vvi_chn;
    vvo_mpp_chn.mod_id = K_ID_V_VO;
    vvo_mpp_chn.dev_id = vvo_dev;
    vvo_mpp_chn.chn_id = vvo_chn;
    return kd_mpi_sys_bind(&vvi_mpp_chn, &vvo_mpp_chn);
}


k_s32 kd_mapi_vvi_unbind_vvo(k_u32 vvi_dev, k_u32 vvi_chn, k_u32 vvo_dev, k_u32 vvo_chn)
{
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn vvo_mpp_chn;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = vvi_dev;
    vvi_mpp_chn.chn_id = vvi_chn;
    vvo_mpp_chn.mod_id = K_ID_V_VO;
    vvo_mpp_chn.dev_id = vvo_dev;
    vvo_mpp_chn.chn_id = vvo_chn;
    return kd_mpi_sys_unbind(&vvi_mpp_chn, &vvo_mpp_chn);
}