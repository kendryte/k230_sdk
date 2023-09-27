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

#include "sample_vdd_r.h"

extern sample_vdd_cfg_t g_vdd_cfg[];

void sample_vdd_vicap_config(k_u32 ch)
{
#ifdef ENABLE_VDSS
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    sample_venc_conf_t *venc_conf;

    mpi_vdss_rst_all(2);

    venc_conf = &g_venc_conf[ch];

    memset(&dev_attr, 0, sizeof(dev_attr));
    dev_attr.dev_num = ch;
    dev_attr.height = venc_conf->chn_height;
    dev_attr.width = venc_conf->chn_width;
    dev_attr.sensor_type = 1;

    dev_attr.artr.csi = CSI0;
    dev_attr.artr.type = CLOSE_3D_MODE;
    dev_attr.artr.mode = LINERA_MODE;
    dev_attr.artr.dev_format[0] = RAW10;
    dev_attr.artr.phy_attr.lan_num = MIPI_1LAN;
    dev_attr.artr.phy_attr.freq = MIPI_800M;
    dev_attr.artr.bind_dvp = DVP_CSI1_FLASE_TRIGGER0;

    kd_mpi_vdss_set_dev_attr(&dev_attr);

    memset(&chn_attr, 0, sizeof(chn_attr));
    chn_attr.format = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    chn_attr.height = venc_conf->chn_height;
    chn_attr.width = venc_conf->chn_width;
    chn_attr.enable = 1;

    kd_mpi_vdss_set_chn_attr(ch, ch, &chn_attr);
#else
    sample_vdd_cfg_t *vdd_cfg;
    k_s32 ret;
    k_vicap_dev vicap_dev = VICAP_DEV_ID_0;
    k_vicap_chn vicap_chn = ch;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;

    vdd_cfg = &g_vdd_cfg[ch];

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));

    sensor_info.sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info.sensor_type, &sensor_info);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    dev_attr.acq_win.width = sensor_info.width;
    dev_attr.acq_win.height = sensor_info.height;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(vicap_dev, dev_attr);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    chn_attr.out_win.width = vdd_cfg->img_width;
    chn_attr.out_win.height = vdd_cfg->img_height;

    chn_attr.crop_win = chn_attr.out_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;

    //chn_attr.bit_width = ISP_PIXEL_YUV_8_BIT;
    chn_attr.pix_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    chn_attr.buffer_num = VDD_INPUT_BUF_CNT;
    chn_attr.buffer_size = vdd_cfg->img_width * vdd_cfg->img_height * 3 / 2;
    //chn_attr.block_type = ISP_BUFQUE_TIMEOUT_TYPE;
    //chn_attr.wait_time = 500;

    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, vicap_chn, chn_attr);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_vicap_init(vicap_dev);
    VDD_CHECK_RET(ret, __func__, __LINE__);
#endif
}

void sample_vdd_vicap_start(k_u32 ch)
{
#ifdef ENABLE_VDSS
    k_s32 ret;
    ret = kd_mpi_vdss_start_pipe(0, ch);
    VDD_CHECK_RET(ret, __func__, __LINE__);
#else
    k_s32 ret;

    ret = kd_mpi_vicap_start_stream(VICAP_DEV_ID_0);
    VDD_CHECK_RET(ret, __func__, __LINE__);
#endif
}

void sample_vicap_stop(k_u32 ch)
{
#ifdef ENABLE_VDSS
    k_s32 ret;
    ret = kd_mpi_vdss_stop_pipe(0, ch);
    VDD_CHECK_RET(ret, __func__, __LINE__);
#else
    k_s32 ret;

    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
    VDD_CHECK_RET(ret, __func__, __LINE__);
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_0);
    VDD_CHECK_RET(ret, __func__, __LINE__);
#endif
}

void sample_vdd_vicap_stop(k_u32 ch)
{
#ifdef ENABLE_VDSS
    k_s32 ret;
    ret = kd_mpi_vdss_stop_pipe(0, ch);
    CHECK_RET(ret, __func__, __LINE__);
#else
    k_s32 ret;

    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
    VDD_CHECK_RET(ret, __func__, __LINE__);
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_0);
    VDD_CHECK_RET(ret, __func__, __LINE__);
#endif
}