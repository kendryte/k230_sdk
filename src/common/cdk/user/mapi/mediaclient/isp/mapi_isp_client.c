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
#include "mapi_isp_api.h"
#include "msg_client_dispatch.h"
#include "mapi_vicap_comm.h"
#include "mpi_isp_api.h"
#include "msg_isp.h"
#include "k_type.h"
#include "k_isp_comm.h"

#include <stdio.h>

k_s32 kd_mapi_isp_ae_get_roi(k_vicap_dev dev_num, k_isp_ae_roi *ae_roi)
{
    k_s32 ret = K_SUCCESS;
    if(dev_num > VICAP_DEV_ID_MAX || dev_num < VICAP_DEV_ID_0)
    {
        mapi_vicap_error_trace("kd_mapi_isp_ae_get_roi, dev_num is over range!");
    }
    if(ae_roi == NULL)
    {
        mapi_vicap_error_trace("kd_mapi_isp_ae_get_roi, ae_roi is null ptr!");
        return K_MAPI_ERR_ISP_NULL_PTR;
    }
    msg_isp_ae_roi_t roi;
    memset(&roi, 0, sizeof(msg_isp_ae_roi_t));
    roi.dev_num = dev_num;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ISP, 0, 0), MSG_CMD_MEDIA_ISP_AE_ROI_GET,
            &roi, sizeof(msg_isp_ae_roi_t), NULL);
    if(ret != K_SUCCESS)
    {
        mapi_vicap_error_trace("kd_mapi_isp_ae_get_roi falied");
    }
    memcpy(ae_roi, &roi.ae_roi, sizeof(k_isp_ae_roi));
    return ret;
}

k_s32 kd_mapi_isp_ae_set_roi(k_vicap_dev dev_num, k_isp_ae_roi ae_roi)
{
    if(dev_num > VICAP_DEV_ID_MAX || dev_num < VICAP_DEV_ID_0)
    {
        mapi_vicap_error_trace("kd_mapi_isp_ae_set_roi, dev_num is over range!");
    }
    k_s32 ret = K_SUCCESS;
    if(ae_roi.roiNum > MSG_ISP_AE_ROI_WINDOWS_MAX)
    {
        mapi_vicap_error_trace("kd_mapi_isp_ae_set_roi, roi num is over range!");
    }
    msg_isp_ae_roi_t roi;
    roi.dev_num = dev_num;
    memcpy(&roi.ae_roi, &ae_roi, sizeof(k_isp_ae_roi));
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ISP, 0, 0), MSG_CMD_MEDIA_ISP_AE_ROI_SET,
            &roi, sizeof(msg_isp_ae_roi_t), NULL);
    if(ret != K_SUCCESS)
    {
        mapi_vicap_error_trace("kd_mapi_isp_ae_set_roi falied");
        return K_FAILED;
    }
    return K_SUCCESS;
}