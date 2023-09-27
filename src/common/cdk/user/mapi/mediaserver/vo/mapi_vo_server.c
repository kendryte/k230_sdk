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
#include <unistd.h>
#include "mpi_vo_api.h"
#include "k_vo_comm.h"
#include "mapi_vo_api.h"
#include "mapi_vo_comm.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#define CHECK_MAPI_VICAP_NULL_PTR(paraname, ptr)                      \
    do {                                                              \
        if ((ptr) == NULL) {                                          \
            mapi_vo_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_VO_NULL_PTR;                         \
        }                                                             \
    } while (0)


// rst display 
k_s32 kd_mapi_set_backlight(void)
{
    k_s32 ret = 0;

    ret = kd_display_set_backlight();
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_reset failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


// rst display 
k_s32 kd_mapi_vo_reset(void)
{
    k_s32 ret = 0;

    ret = kd_display_reset();
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_reset failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_dsi_set_test_pattern(void)
{
    k_s32 ret = 0;

    ret = kd_mpi_dsi_set_test_pattern();
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_dsi_set_test_pattern failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_dsi_set_attr(k_vo_dsi_attr *attr)
{
    k_s32 ret = 0;
    
    CHECK_MAPI_VICAP_NULL_PTR("dsi_attr", attr);
    ret = kd_mpi_dsi_set_attr(attr);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_dsi_set_attr failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_dsi_enable(k_u32 enable)
{
    k_s32 ret = 0;

    ret = kd_mpi_dsi_enable(enable);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_dsi_enable failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_dsi_send_cmd(k_u8 *data, k_s32 cmd_len)
{
    k_s32 ret = 0;
    
    CHECK_MAPI_VICAP_NULL_PTR("dsi_attr", data);
    ret = kd_mpi_dsi_send_cmd(data, cmd_len);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_dsi_send_cmd failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_set_mipi_phy_attr(k_vo_mipi_phy_attr *attr)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("mipi_phy_attr", attr);
    ret = kd_mpi_set_mipi_phy_attr(attr);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_set_mipi_phy_attr failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_dsi_read_pkg(k_u8 addr, k_u16 cmd_len, k_u32 *rv_data)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("dsi_read_pkg", rv_data);
    ret = kd_mpi_dsi_read_pkg(addr, cmd_len, rv_data);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_dsi_read_pkg failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_enable(void)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_enable();

    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_enable failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_disable(void)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_disable();
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_disable failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_init(void)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_init();

    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_init failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_set_dev_param(k_vo_pub_attr *attr)
{
    k_s32 ret = 0;
    // k_vo_pub_attr pub_attr;
    // k_vo_display_resolution sync_info;

    CHECK_MAPI_VICAP_NULL_PTR("vo_set_dev_param", attr);

    // memcpy(&pub_attr, attr, sizeof(k_vo_pub_attr));
    // memcpy(&sync_info, attr->sync_info, sizeof(k_vo_display_resolution));
    // pub_attr.sync_info = &sync_info;

    ret = kd_mpi_vo_set_dev_param(attr);

    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_set_dev_param failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_set_user_sync_info(k_u32 pre_div, k_u32 clk_en)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_set_user_sync_info(pre_div, clk_en);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_set_user_sync_info failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_enable_video_layer(k_vo_layer layer)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_enable_video_layer(layer);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_enable_video_layer failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_disable_video_layer(k_vo_layer layer)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_disable_video_layer(layer);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_disable_video_layer failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_set_video_layer_attr(k_vo_layer layer, k_vo_video_layer_attr *attr)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("set_video_layer_attr", attr);
    ret = kd_mpi_vo_set_video_layer_attr(layer, attr);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_set_video_layer_attr failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_set_layer_priority(k_vo_layer layer, k_s32 priority)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_set_layer_priority(layer, priority);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_set_layer_priority failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_vo_set_video_osd_attr(k_vo_osd layer, k_vo_video_osd_attr *attr)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("vo_set_video_osd_attr", attr);
    ret = kd_mpi_vo_set_video_osd_attr(layer, attr);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_set_video_osd_attr failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_osd_enable(k_vo_osd layer)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_osd_enable(layer);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_osd_enable failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_osd_disable(k_vo_osd layer)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_osd_disable(layer);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_osd_disable failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_draw_frame(k_vo_draw_frame *frame)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("vo_draw_frame", frame);
    ret = kd_mpi_vo_draw_frame(frame);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_draw_frame failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_enable_wbc(void)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_enable_wbc();
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_enable_wbc failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_disable_wbc(void)
{
    k_s32 ret = 0;

    ret = kd_mpi_vo_disable_wbc();
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_disable_wbc failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_set_wbc_attr(k_vo_wbc_attr *attr)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("vo_set_wbc_attr", attr);
    ret = kd_mpi_vo_set_wbc_attr(attr);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_set_wbc_attr failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_chn_insert_frame(k_u32 chn_num, k_video_frame_info *vf_info)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("vo_chn_insert_frame", vf_info);
    ret = kd_mpi_vo_chn_insert_frame(chn_num, vf_info);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_chn_insert_frame failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_chn_dump_frame(k_u32 chn_num, k_video_frame_info *vf_info, k_u32 timeout_ms)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("vo_chn_dump_frame", vf_info);
    ret = kd_mpi_vo_chn_dump_frame(chn_num, vf_info, timeout_ms);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_chn_dump_frame failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_vo_chn_dump_release(k_u32 chn_num, const k_video_frame_info *vf_info)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("vo_chn_dump_release", vf_info);
    ret = kd_mpi_vo_chn_dump_release(chn_num, vf_info);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_vo_chn_dump_release failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_get_connector_info(k_connector_type connector_type, k_connector_info *connector_info)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("get_connector_info", connector_info);
    ret = kd_mpi_get_connector_info(connector_type, connector_info);

    printf("connector_type is %d connector_info.name is %s connector_info.bg_clolor is %x \n", connector_type, connector_info->connector_name, connector_info->bg_color);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_get_connector_info failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}



k_s32 kd_mapi_connector_open(const char *connector_name)
{
    k_s32 ret = 0;

    CHECK_MAPI_VICAP_NULL_PTR("get_connector_info", connector_name);

    ret = kd_mpi_connector_open(connector_name);
    if(ret < 0)
    {
        mapi_vo_error_trace("kd_mpi_connector_open failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    return ret;
}


k_s32 kd_mapi_connector_power_set(k_s32 fd, k_bool on)
{
    k_s32 ret = 0;

    ret = kd_mpi_connector_power_set(fd, on);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_connector_open failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }
    
    return K_SUCCESS;
}


k_s32 kd_mapi_connector_init(k_s32 fd, k_connector_info *info)
{
    k_s32 ret = 0;
    k_connector_info con_info;

    memcpy(&con_info, info, sizeof(k_connector_info));

    // printf("kd_mapi_connector_init fd is %d \n", fd);
    // printf("kd_mapi_connector_init con_info.bg_color is %x \n", con_info.bg_color);
    // printf("kd_mapi_connector_init con_info.intr_line is %d \n", con_info.intr_line);
    // printf("kd_mapi_connector_init con_info.cmd_mode is %d \n", con_info.cmd_mode);
    // printf("dsi attr lan_num is %d screen_test_mode is %d dsi_test_mode is %d \n", con_info.lan_num, con_info.screen_test_mode, con_info.dsi_test_mode);
    // printf("phy_attr m is %d con_info.phy_attr.n is %d voc is %x \n", con_info.phy_attr.m, con_info.phy_attr.n, con_info.phy_attr.voc);
    // printf("vo k_vo_display_resolution pclk is %d vfront_porch is %d hdisplay is %d \n", con_info.resolution.pclk,  con_info.resolution.vfront_porch, con_info.resolution.hdisplay );
    

    ret = kd_mpi_connector_init(fd, con_info);
    if(ret != K_SUCCESS)
    {
        mapi_vo_error_trace("kd_mapi_connector_open failed:0x%x\n", ret);
        return VO_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}