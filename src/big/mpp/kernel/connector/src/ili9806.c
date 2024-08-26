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

static void ili9806_480x800_init(k_u8 test_mode_en)
{
    {
        uint8_t param[] = { 0xff, 0xff, 0x98, 0x06, 0x04, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x08, 0x10 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x21, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x30, 0x02 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x31, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x40, 0x16 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x41, 0x33 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x42, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x43, 0x85 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x44, 0x8b };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x45, 0x1b };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x50, 0x78 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x51, 0x78 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x52, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x53, 0x60 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x60, 0x07 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x61, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x62, 0x07 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x63, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xA0, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa1, 0x0b };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa2, 0x12 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa3, 0x0c };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa4, 0x05 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa5, 0x0c };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa6, 0x07 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa7, 0x16 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa8, 0x06 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xa9, 0x0a };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xaa, 0x0f };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xab, 0x06 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xac, 0x0e };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xad, 0x1a };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xae, 0x12 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xaf, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc0, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc1, 0x0b };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc2, 0x12 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc3, 0x0c };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc4, 0x05 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc5, 0x0c };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc6, 0x07 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc7, 0x16 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc8, 0x06 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xc9, 0x0a };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xca, 0x0f };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xcb, 0x06 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xcc, 0x0e };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xcd, 0x1a };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xce, 0x12 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xcf, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xff, 0xff, 0x98, 0x06, 0x04, 0x06 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x00, 0xa0 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x01, 0x05 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x02, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x03, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x04, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x05, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x06, 0x88 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x07, 0x04 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x08, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x09, 0x90 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x0a, 0x04 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x0b, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x0c, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x0d, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x0e, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x0f, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x10, 0x55 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x11, 0x50 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x12, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x13, 0x85 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x14, 0x85 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x15, 0xc0 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x16, 0x0b };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x17, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x18, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x19, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x1a, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x1b, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x1c, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x1d, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x20, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x21, 0x23 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x22, 0x45 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x23, 0x67 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x24, 0x01 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x25, 0x23 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x26, 0x45 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x27, 0x67 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x30, 0x02 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x31, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x32, 0x11 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x33, 0xaa };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x34, 0xbb };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x35, 0x66 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x36, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x37, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x38, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x39, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x3a, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x3b, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x3c, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x3d, 0x20 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x3e, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x3f, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x40, 0x22 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x53, 0x1a };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xff, 0xff, 0x98, 0x06, 0x04, 0x07 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x17, 0x12 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x21 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x02, 0x77 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0xff, 0xff, 0x98, 0x06, 0x04, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x35, 0x00 };
        connecter_dsi_send_pkg(param, sizeof(param));
    }
    {
        uint8_t param[] = { 0x11 };
        connecter_dsi_send_pkg(param, sizeof(param));
        rt_thread_mdelay(100);
    }
    {
        uint8_t param[] = { 0x29 };
        connecter_dsi_send_pkg(param, sizeof(param));
        rt_thread_mdelay(100);
    }
}

static void ili9806_power_reset(k_s32 on)
{
    k_u8 rst_gpio;

    rst_gpio = DISPLAY_LCD_RST_GPIO;

    kd_pin_mode(rst_gpio, GPIO_DM_OUTPUT);

    if (on)
        kd_pin_write(rst_gpio, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    else
        kd_pin_write(rst_gpio, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
}

static void ili9806_set_backlight(k_s32 on)
{
    k_u8 backlight_gpio;

    backlight_gpio = DISPLAY_LCD_BACKLIGHT_EN;

    kd_pin_mode(backlight_gpio, GPIO_DM_OUTPUT);
    if (on)
        kd_pin_write(backlight_gpio, GPIO_PV_HIGH);
    else
        kd_pin_write(backlight_gpio, GPIO_PV_LOW);
}

static k_s32 ili9806_power_on(void* ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;

    if (on) {
        // rst vo;
        k230_display_rst();
        // rst ili9806
        ili9806_power_reset(0);
        rt_thread_mdelay(200);
        ili9806_power_reset(1);
        rt_thread_mdelay(200);
        // enable backlight
        ili9806_set_backlight(1);
    } else {
        ili9806_set_backlight(0);
    }

    return ret;
}

static k_s32 ili9806_set_phy_freq(k_connectori_phy_attr* phy_attr)
{
    k_vo_mipi_phy_attr mipi_phy_attr;

    memset(&mipi_phy_attr, 0, sizeof(k_vo_mipi_phy_attr));

    mipi_phy_attr.m = phy_attr->m;
    mipi_phy_attr.n = phy_attr->n;
    mipi_phy_attr.hs_freq = phy_attr->hs_freq;
    mipi_phy_attr.voc = phy_attr->voc;
    mipi_phy_attr.phy_lan_num = K_DSI_4LAN;
    connector_set_phy_freq(&mipi_phy_attr);

    return 0;
}

static k_s32 ili9806_dsi_resolution_init(k_connector_info* info)
{
    k_vo_dsi_attr attr;
    k_vo_display_resolution resolution;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));
    attr.lan_num = info->lan_num;
    attr.cmd_mode = info->cmd_mode;
    attr.lp_div = 8;
    attr.work_mode = info->work_mode;
    memcpy(&resolution, &info->resolution, sizeof(k_vo_display_resolution));
    memcpy(&attr.resolution, &resolution, sizeof(k_vo_display_resolution));
    connector_set_dsi_attr(&attr);

    if (info->screen_test_mode)
        ili9806_480x800_init(1);
    else
        ili9806_480x800_init(0);

    connector_set_dsi_enable(1);

    if (info->dsi_test_mode == 1)
        connector_set_dsi_test_mode();

    return 0;
}

static k_s32 ili9806_vo_resolution_init(k_vo_display_resolution* resolution, k_u32 bg_color, k_u32 intr_line)
{
    k_vo_display_resolution vo_resolution;
    k_vo_pub_attr attr;

    memset(&attr, 0, sizeof(k_vo_pub_attr));
    attr.bg_color = bg_color;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    connector_set_vo_init();
    connector_set_vtth_intr(1, intr_line);
    connector_set_vo_param(&attr);
    connector_set_vo_enable();

    return 0;
}

k_s32 ili9806_init(void* ctx, k_connector_info* info)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;

    if (info->pixclk_div != 0)
        connector_set_pixclk(info->pixclk_div);

    ret |= ili9806_set_phy_freq(&info->phy_attr);
    ret |= ili9806_dsi_resolution_init(info);
    ret |= ili9806_vo_resolution_init(&info->resolution, info->bg_color, info->intr_line);

    return ret;
}

static k_s32 ili9806_get_chip_id(void* ctx, k_u32* chip_id)
{
    k_s32 ret = 0;

    return ret;
}

static k_s32 ili9806_conn_check(void* ctx, k_s32* conn)
{
    k_s32 ret = 0;

    *conn = 1;

    return ret;
}

static k_s32 ili9806_set_mirror(void* ctx, k_connector_mirror* mirror)
{
    k_connector_mirror ili9806_mirror;

    ili9806_mirror = *mirror;

    switch (ili9806_mirror) {
    case K_CONNECTOR_MIRROR_HOR:
        break;
    case K_CONNECTOR_MIRROR_VER:
        break;
    case K_CONNECTOR_MIRROR_BOTH:
        break;
    default:
        rt_kprintf("ili9806_mirror(%d) is not support \n", ili9806_mirror);
        break;
    }
    return 0;
}

struct connector_driver_dev ili9806_connector_drv = {
    .connector_name = "ili9806",
    .connector_func = {
        .connector_power = ili9806_power_on,
        .connector_init = ili9806_init,
        .connector_get_chip_id = ili9806_get_chip_id,
        .connector_conn_check = ili9806_conn_check,
        .connector_set_mirror = ili9806_set_mirror,
    },
};
