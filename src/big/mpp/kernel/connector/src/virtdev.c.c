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

#include "connector_dev.h"
#include "io.h"
#include "drv_gpio.h"
#include "k_vo_comm.h"
#include "k_connector_comm.h"
#include "k_board_config_comm.h"

static k_s32 virtdev_power_on(void* ctx, k_s32 on)
{
    return 0;
}

static k_s32 virtdev_vo_resolution_init(k_vo_display_resolution* resolution, k_u32 bg_color, k_u32 intr_line)
{
    k_vo_pub_attr attr;

    memset(&attr, 0, sizeof(k_vo_pub_attr));
    attr.bg_color = bg_color;
    attr.sync_info = resolution;

    connector_set_vo_init();
    connector_set_vtth_intr(1, intr_line);
    connector_set_vo_param(&attr);
    connector_set_vo_enable();

    return 0;
}

static k_s32 virtdev_init(void* ctx, k_connector_info* info)
{
    k_vo_display_resolution* resolution = &info->resolution;
    // 使用pclk字段表示帧率，如果设置为非0则自动计算分频和行场信息，否则使用结构体中的配置
    if (resolution->pclk != 0) {
        uint32_t hact, htotal, htotal_min, htotal_max = 4096;
        uint32_t vact, vtotal, vtotal_min, vtotal_max = 4096;
        uint32_t fps, pixtotal, pixclk_div, intr_line;
        hact = resolution->hdisplay;
        vact = resolution->vdisplay;
        fps = resolution->pclk;
        if (hact < 64 || hact > 4096 || vact < 64 || vact > 4096 || fps > 200)
            return -1;
        intr_line = 32 - __builtin_clz(vact);
        vtotal_min = (1UL << intr_line) + 15;
        if (vtotal_min < (vact + 45))
            vtotal_min = vact + 45;
        htotal_min = hact + 96;
        pixclk_div = 594000000 / (vtotal_min * htotal_min * fps);
        if (pixclk_div == 0)
            return -1;
        if (pixclk_div > 128)
            pixclk_div = 128;
        pixtotal = 594000000 / pixclk_div / fps;
        for (vtotal = vtotal_min; vtotal < htotal_max; vtotal++) {
            htotal = pixtotal / vtotal;
            if (htotal <= htotal_max)
                break;
            else if (htotal < htotal_min)
                return -1;
        }
        info->pixclk_div = pixclk_div;
        info->intr_line = intr_line;
        resolution->htotal = htotal;
        resolution->hsync_len = 32;
        resolution->hback_porch = 32;
        resolution->hfront_porch = htotal - hact - 64;
        resolution->vtotal = vtotal;
        resolution->vsync_len = 5;
        resolution->vback_porch = 5;
        resolution->vfront_porch = vtotal - vact - 10;
    }
    connector_set_pixclk(info->pixclk_div);
    virtdev_vo_resolution_init(resolution, info->bg_color, info->intr_line);

    return 0;
}

static k_s32 virtdev_get_chip_id(void* ctx, k_u32* chip_id)
{
    *chip_id = 0xFFFFFFFF;
    return 0;
}

static k_s32 virtdev_conn_check(void* ctx, k_s32* conn)
{
    *conn = 1;
    return 0;
}

struct connector_driver_dev virtdev_connector_drv = {
    .connector_name = "virtdev",
    .connector_func = {
        .connector_power = virtdev_power_on,
        .connector_init = virtdev_init,
        .connector_get_chip_id = virtdev_get_chip_id,
        .connector_conn_check = virtdev_conn_check,
    },
};
