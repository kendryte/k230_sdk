/**
 * @file mpi_vicap_api.h
 * @author
 * @brief Defines APIs related to virtual video input device
 * @version 1.0
 * @date 2022-09-22
 *
 * @copyright
 * Copyright (c), Canaan Bright Sight Co., Ltd
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
#ifndef __MPI_VDSS_API_H__
#define __MPI_VDSS_API_H__

#include "k_type.h"
#include "k_vdss_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


// only test
k_s32 mpi_vdss_reg_test(void);

k_s32 mpi_vdss_rst_all(k_u8 val);
k_s32 kd_mpi_vdss_start_pipe(k_u32 dev_num, k_u32 chn_num);
k_s32 kd_mpi_vdss_stop_pipe(k_u32 dev_num, k_u32 chn_num);
k_s32 kd_mpi_vdss_set_chn_attr(k_u32 dev_num, k_u32 chn_num, k_vicap_chn_attr *attr);
k_s32 kd_mpi_vdss_set_dev_attr(k_vicap_dev_attr *attr);
k_s32 kd_mpi_vdss_dump_frame(k_u32 dev_num, k_u32 chn_num, k_video_frame_info *vf_info, k_u32 timeout_ms);
k_s32 kd_mpi_vdss_chn_release_frame(k_u32 dev_num, k_u32 chn_num, const k_video_frame_info *vf_info);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
