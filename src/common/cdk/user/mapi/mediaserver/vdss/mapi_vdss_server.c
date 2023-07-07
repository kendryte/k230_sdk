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
#include "mapi_vdss_api.h"
#include "mapi_vvi_comm.h"
#include "mapi_sys_api.h"
#include "mpi_vdss_api.h"
#include "k_vdss_comm.h"


k_s32 kd_mapi_vdss_rst(k_u8 val)
{
    return mpi_vdss_rst_all(val);
}


k_s32 kd_mapi_vdss_start_pipe(k_u32 dev_num, k_u32 chn_num)
{
    return kd_mpi_vdss_start_pipe(dev_num, chn_num);
}

k_s32 kd_mapi_vdss_stop_pipe(k_u32 dev_num, k_u32 chn_num)
{
    return kd_mpi_vdss_stop_pipe(dev_num, chn_num);
}


k_s32 kd_mapi_vdss_set_chn_attr(k_u32 dev_num, k_u32 chn_num, k_vicap_chn_attr *attr)
{
    return kd_mpi_vdss_set_chn_attr(dev_num, chn_num, attr);
}


k_s32 kd_mapi_vdss_set_dev_attr(k_vicap_dev_attr *attr)
{
    k_s32 ret = 0;
    ret = kd_mpi_vdss_set_dev_attr(attr);

    printf("###############ret is %d attr sensor_type is %d dev_num is %d \n", ret, attr->sensor_type, attr->dev_num);
    return 0;
}

k_s32 kd_mapi_vdss_dump_frame(k_u32 dev_num, k_u32 chn_num, k_video_frame_info *vf_info, k_u32 timeout_ms)
{
    k_s32 ret = 0;
    ret = kd_mpi_vdss_dump_frame(dev_num, chn_num, vf_info, timeout_ms);
    printf("### server kd_mapi_vdss_dump_frame---------------------- vf_info phy addr is %x \n", vf_info->v_frame.phys_addr[0]);
    return ret;
}


k_s32 kd_mapi_vdss_chn_release_frame(k_u32 dev_num, k_u32 chn_num, const k_video_frame_info *vf_info)
{
    printf("### server kd_mapi_vdss_chn_release_frame---------------------- vf_info phy addr is %x \n", vf_info->v_frame.phys_addr[0]);
    return kd_mpi_vdss_chn_release_frame(dev_num, chn_num, vf_info);
}
