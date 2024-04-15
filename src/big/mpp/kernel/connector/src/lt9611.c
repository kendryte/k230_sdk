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

#include <drivers/i2c.h>

#include "connector_dev.h"
#include "io.h"
#include "drv_gpio.h"
#include "k_vo_comm.h"
#include "k_connector_comm.h"
#include "k_board_config_comm.h"

#define DBG_TAG          "lt9611"
#ifdef RT_DEBUG
#define DBG_LVL          DBG_LOG
#else
#define DBG_LVL          DBG_WARNING
#endif
#define DBG_COLOR
#include <rtdbg.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

typedef struct {
    k_u8 addr;
    k_u8 val;
} k_i2c_reg;

typedef struct {
    k_u16 slave_addr;
    const char *i2c_name;
    struct rt_i2c_bus_device *i2c_bus;
} k_i2c_info;

struct lt9611_dev {
#define LT9611_PORTA    100600
#define LT9611_PORTB    100601
    k_u32 input_port;
    k_u32 pcr_m;
    k_i2c_info i2c_info;
};

struct lt9611_resolution {
    k_u8    std_timing_high;
    k_u8    std_timing_low;
    k_u8    cea861_vic;
    k_u32   connector_type;
};

#define LT9611_RESOLUTION_NUM   5
struct lt9611_resolution lt9611_resolution[LT9611_RESOLUTION_NUM] = {
    {0xd1, 0xc0, 16,    LT9611_MIPI_4LAN_1920X1080_60FPS},
    {0x00, 0x00, 34,    LT9611_MIPI_4LAN_1920X1080_30FPS},
    {0x81, 0xc0, 4,     LT9611_MIPI_4LAN_1280X720_60FPS},
    {0x00, 0x00, 19,    LT9611_MIPI_4LAN_1280X720_50FPS},
    {0x00, 0x00, 1,    LT9611_MIPI_4LAN_640X480_60FPS},
};

static void connector_set_drvdata(struct connector_driver_dev *dev, void *data)
{
    dev->driver_data = data;
}

static void *connector_get_drvdata(struct connector_driver_dev *dev)
{
    return dev->driver_data;
}

k_s32 lt9611_read_reg(k_i2c_info *i2c_info, k_u8 reg_addr, k_u8 *reg_val)
{
    struct rt_i2c_msg msg[2];

    RT_ASSERT(i2c_info != RT_NULL);
    msg[0].addr  = i2c_info->slave_addr;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = &reg_addr;

    msg[1].addr  = i2c_info->slave_addr;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = 1;
    msg[1].buf   = reg_val;

    if (rt_i2c_transfer(i2c_info->i2c_bus, msg, 2) != 2)
        return RT_ERROR;

    return RT_EOK;
}

k_s32 lt9611_write_reg(k_i2c_info *i2c_info, k_u8 reg_addr, k_u8 reg_val)
{
    struct rt_i2c_msg msg;
    k_u8 buf[2];

    RT_ASSERT(i2c_info != RT_NULL);
    buf[0] = reg_addr;
    buf[1] = reg_val;
    msg.addr  = i2c_info->slave_addr;
    msg.flags = RT_I2C_WR;
    msg.len   = 2;
    msg.buf   = buf;

    if (rt_i2c_transfer(i2c_info->i2c_bus, &msg, 1) != 1)
        return RT_ERROR;

    return RT_EOK;
}

k_s32 lt9611_write_multi_reg(k_i2c_info *i2c_info, const k_i2c_reg *reg_list, k_u32 reg_num)
{
    k_s32 ret = 0;
    k_u32 i;

    for (i = 0; i < reg_num; i++) {
        ret = lt9611_write_reg(i2c_info, reg_list[i].addr, reg_list[i].val);
        if (ret)
            return RT_ERROR;
    }

    return ret;
}

static void lt9611_reset(k_u8 lt9611_reset_pin)
{    
    kd_pin_mode(lt9611_reset_pin, GPIO_DM_OUTPUT);
    kd_pin_write(lt9611_reset_pin, GPIO_PV_LOW);
    rt_thread_mdelay(100);

    kd_pin_write(lt9611_reset_pin, GPIO_PV_HIGH);
    rt_thread_mdelay(100);
}

static k_s32 lt9611_set_interface(struct lt9611_dev *lt9611_dev)
{
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x80);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xee, 0x01);

    return 0;
}

static k_s32 lt9611_init_system(struct lt9611_dev *lt9611_dev)
{
    const k_i2c_reg system_regs[] = {
        { 0xff, 0x81 },
        { 0x01, 0x18 }, /* sel xtal clock */
        { 0xff, 0x82 },
        { 0x51, 0x11 },

        /* timer for frequency meter */
        { 0xff, 0x82 },
        { 0x1b, 0x69 }, /* timer 2 */
        { 0x1c, 0x78 },
        { 0xcb, 0x69 }, /* timer 1 */
        { 0xcc, 0x78 },

        /* power consumption for work */
        { 0xff, 0x80 },
        { 0x04, 0xf0 },
        { 0x06, 0xf0 },
        { 0x0a, 0x80 },
        { 0x0b, 0x46 },
        { 0x0d, 0xef },
        { 0x11, 0xfa },
    };

    return lt9611_write_multi_reg(&lt9611_dev->i2c_info, system_regs, ARRAY_SIZE(system_regs));
}

static k_s32 lt9611_mipi_input_analog(struct lt9611_dev *lt9611_dev)
{
    const k_i2c_reg mipi_input_analog_regs[] = {
        { 0xff, 0x81 },
        { 0x06, 0x60 }, /* port A rx current */
        { 0x07, 0x3f }, /* port A lane0 and lane1 eq set */
        { 0x08, 0x3f }, /* port A lane2 and lane3 eq set */
        { 0x0a, 0xfe }, /* port A ldo voltage set */
        { 0x0b, 0xbf }, /* enable port A lprx */

        { 0x11, 0x60 }, /* port B rx current */
        { 0x12, 0x3f }, /* port B lane0 and lane1 eq set */
        { 0x13, 0x3f }, /* port B lane2 and lane3 eq set */
        { 0x15, 0xfe }, /* port B ldo voltage set */
        { 0x16, 0xbf }, /* enable port B lprx */

        { 0x1c, 0x03 }, /* PortA clk lane no-LP mode */
        { 0x20, 0x03 }, /* PortB clk lane no-LP mode */
    };

    return lt9611_write_multi_reg(&lt9611_dev->i2c_info, mipi_input_analog_regs, ARRAY_SIZE(mipi_input_analog_regs));
}

static k_s32 lt9611_mipi_input_digital(struct lt9611_dev *lt9611_dev)
{
    if (lt9611_dev->input_port == LT9611_PORTA) {
        lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x50, 0x10);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x83);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x03, 0x00);
    } else if (lt9611_dev->input_port == LT9611_PORTB) {
        lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x50, 0x14);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x83);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x00, 0x60);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x03, 0x4f);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x04, 0x00);
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0x40);
    }

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x4f, 0x80);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x83);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x02, 0x08);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x06, 0x08);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x0a, 0x00);

    return 0;
}

static k_s32 lt9611_read_video(struct lt9611_dev *lt9611_dev)
{
    k_s32 ret = 0;
    k_u8 low, high;
    k_u16 vtotal, vactive, htotal_sys;

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    lt9611_read_reg(&lt9611_dev->i2c_info, 0x6c, &high);
    lt9611_read_reg(&lt9611_dev->i2c_info, 0x6d, &low);
    vtotal = ((high << 8) | low);

    lt9611_read_reg(&lt9611_dev->i2c_info, 0x82, &high);
    lt9611_read_reg(&lt9611_dev->i2c_info, 0x83, &low);
    vactive = ((high << 8) | low);

    lt9611_read_reg(&lt9611_dev->i2c_info, 0x86, &high);
    lt9611_read_reg(&lt9611_dev->i2c_info, 0x87, &low);
    htotal_sys = ((high << 8) | low);

    LOG_D("vtotal %d vactive %d htotal_sys %d\n", vtotal, vactive, htotal_sys);

    return 0;
}

static k_s32 lt9611_setup_pll(struct lt9611_dev *lt9611_dev, k_u32 pclk)
{
    k_u32 postdiv = 0;

    const k_i2c_reg pll_regs[] = {
        { 0xff, 0x81 },
        { 0x23, 0x40 },
        { 0x24, 0x62 },
        { 0x25, 0x80 },
        { 0x26, 0x55 },
        { 0x2c, 0x37 },
        { 0x2f, 0x01 },
        { 0x27, 0x66 },
        { 0x28, 0x88 },
        { 0x2a, 0x20 },
    };

    lt9611_write_multi_reg(&lt9611_dev->i2c_info, pll_regs, ARRAY_SIZE(pll_regs));

    if (pclk > 150000) {
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x2d, 0x88);
        postdiv = 1;
    } else if (pclk > 80000) {
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x2d, 0x99);
        postdiv = 2;
    } else {
        lt9611_write_reg(&lt9611_dev->i2c_info, 0x2d, 0xaa);
        postdiv = 4;
    }
    lt9611_dev->pcr_m = ((pclk * 5 * postdiv) / 27000) - 1;
 
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x83);
	lt9611_write_reg(&lt9611_dev->i2c_info, 0x2d, 0x40);
	lt9611_write_reg(&lt9611_dev->i2c_info, 0x31, 0x08);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x26, 0x80 | lt9611_dev->pcr_m);

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    pclk = pclk / 2;
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xe3, pclk / 65536);
    pclk = pclk % 65536;
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xe4, pclk / 256);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xe5, pclk % 256);

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xde, 0x20);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xde, 0xe0);

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x80);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x11, 0x5a);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x11, 0xfa);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x16, 0xf2);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x18, 0xdc);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x18, 0xfc);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x16, 0xf3);

    return 0;
}

static k_s32 lt9611_setup_pcr(struct lt9611_dev *lt9611_dev)
{
    const k_i2c_reg pcr_regs[] = {
        { 0xff, 0x83 },
        { 0x0b, 0x01 },
        { 0x0c, 0x10 },
        { 0x48, 0x00 },
        { 0x49, 0x81 },

        /* stage 1 */
        { 0x21, 0x4a },
        { 0x24, 0x71 },
        { 0x25, 0x30 },
        { 0x2a, 0x01 },

        /* stage 2 */
        { 0x4a, 0x40 },

		/* MK limit */
		{ 0x2d, 0x40 },
		{ 0x31, 0x08 },
    };
    lt9611_write_multi_reg(&lt9611_dev->i2c_info, pcr_regs, ARRAY_SIZE(pcr_regs));

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x83);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x1d, 0x10);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x26, lt9611_dev->pcr_m);

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x80);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x11, 0x5a);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x11, 0xfa);

    return 0;
}

static k_s32 lt9611_setup_timing(struct lt9611_dev *lt9611_dev, k_vo_display_resolution *resolution)
{
    k_u32 htotal = resolution->htotal;
    k_u32 hdisplay = resolution->hdisplay;
    k_u32 hsync_len = resolution->hsync_len;
    k_u32 hback_porch = resolution->hback_porch;
    k_u32 hfront_porch = resolution->hfront_porch;

    k_u32 vtotal = resolution->vtotal;
    k_u32 vdisplay = resolution->vdisplay;
    k_u32 vsync_len = resolution->vsync_len;
    k_u32 vback_porch = resolution->vback_porch;
    k_u32 vfront_porch = resolution->vfront_porch;

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x83);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x0d, (k_u8)(vtotal / 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x0e, (k_u8)(vtotal % 256));

    lt9611_write_reg(&lt9611_dev->i2c_info, 0x0f, (k_u8)(vdisplay / 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x10, (k_u8)(vdisplay % 256));

    lt9611_write_reg(&lt9611_dev->i2c_info, 0x11, (k_u8)(htotal / 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x12, (k_u8)(htotal % 256));

    lt9611_write_reg(&lt9611_dev->i2c_info, 0x13, (k_u8)(hdisplay / 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x14, (k_u8)(hdisplay % 256));

    lt9611_write_reg(&lt9611_dev->i2c_info, 0x15, (k_u8)(vsync_len % 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x16, (k_u8)(hsync_len % 256));

    lt9611_write_reg(&lt9611_dev->i2c_info, 0x17, (k_u8)(vfront_porch % 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x18, (k_u8)((vsync_len + vback_porch) % 256));

    lt9611_write_reg(&lt9611_dev->i2c_info, 0x19, (k_u8)(hfront_porch % 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x1a, (k_u8)(((hfront_porch / 256) << 4) + (hsync_len + hback_porch) / 256));
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x1b, (k_u8)((hsync_len + hback_porch) % 256));

    return 0;
}

static k_s32 lt9611_hdmi_tx_digital(struct lt9611_dev *lt9611_dev)
{
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x84);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x43, 0x21);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x45, 0x40);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x47, 0x34);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x3d, 0x0a); /* UD1 infoframe */

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xd6, 0x8e);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xd7, 0x04);

    return 0;
}

static k_s32 lt9611_hdmi_tx_phy(struct lt9611_dev *lt9611_dev)
{
    const k_i2c_reg hdmi_tx_phy_regs[] = {
        { 0xff, 0x81 },
        { 0x30, 0x6a },
        { 0x31, 0x44 }, /* HDMI DC mode */
        { 0x32, 0x4a },
        { 0x33, 0x0b },
        { 0x34, 0x00 },
        { 0x35, 0x00 },
        { 0x36, 0x00 },
        { 0x37, 0x44 },
        { 0x3f, 0x0f },
        { 0x40, 0x98 },
        { 0x41, 0x98 },
        { 0x42, 0x98 },
        { 0x43, 0x98 },
        { 0x44, 0x0a },
    };

    return lt9611_write_multi_reg(&lt9611_dev->i2c_info, hdmi_tx_phy_regs, ARRAY_SIZE(hdmi_tx_phy_regs));
}

static k_s32 lt9611_irq_init(struct lt9611_dev *lt9611_dev)
{
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x58, 0x0a);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x59, 0x00);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x9e, 0xf7);

    return 0;
}

static k_s32 lt9611_enable_hpd_interrupts(struct lt9611_dev *lt9611_dev)
{
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0xff);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0x3f);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x03, 0x3f);

    return 0;
}

static k_s32 lt9611_enable_hdmi_out(struct lt9611_dev *lt9611_dev)
{
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x81);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x23, 0x40);

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xde, 0x20);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0xde, 0xe0);

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x80);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x18, 0xdc);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x18, 0xfc);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x16, 0xf1);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x16, 0xf3);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x11, 0x5a);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x11, 0xfa);

    lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x81);
    lt9611_write_reg(&lt9611_dev->i2c_info, 0x30, 0xea);

    return 0;
}

static struct lt9611_dev *lt9611_dev_create(k_u32 input_port, k_u16 slave_addr, const char *i2c_name)
{
    struct rt_i2c_bus_device *i2c_bus;
    struct lt9611_dev *lt9611_dev;

    lt9611_dev = rt_malloc(sizeof(struct lt9611_dev));
    if (lt9611_dev == RT_NULL)
        return RT_NULL;

    i2c_bus = rt_i2c_bus_device_find(i2c_name);
    if (i2c_bus == RT_NULL) {
        LOG_E("can't find %s deivce \n", i2c_name);
        return RT_NULL;
    }

    lt9611_dev->input_port          = input_port;
    lt9611_dev->i2c_info.i2c_bus    = i2c_bus;
    lt9611_dev->i2c_info.i2c_name   = i2c_name;
    lt9611_dev->i2c_info.slave_addr = slave_addr;

    return lt9611_dev;
}

static k_s32 lt9611_power_on(void* ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;
    struct lt9611_dev *lt9611_dev;

    k230_display_rst();
    lt9611_reset(LT9611_RESET_GPIO);
    lt9611_dev = lt9611_dev_create(LT9611_PORTB, LT9611_SLAVE_ADDR, LT9611_I2C_BUS);
    if (lt9611_dev == RT_NULL) {
        LOG_E("lt9611_dev_create failed \n");
        return K_FAILED;
    }
    lt9611_set_interface(lt9611_dev);

    connector_set_drvdata(dev, lt9611_dev);

    return ret;
}

static k_s32 k230_set_phy_freq(k_connectori_phy_attr *phy_attr)
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

static k_s32 k230_dsi_resolution_init(k_connector_info *info)
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
    connector_set_dsi_enable(1);

    if(info->dsi_test_mode == 1)
        connector_set_dsi_test_mode();

    return 0;
}

static k_s32 k230_vo_resolution_init(k_connector_type type, k_vo_display_resolution *resolution, k_u32 bg_color, k_u32 intr_line)
{
    k_vo_display_resolution vo_resolution;
    k_vo_pub_attr pub_attr;
    k_vo_sync_attr sync_attr;

    memset(&pub_attr, 0, sizeof(k_vo_pub_attr));
    pub_attr.bg_color = bg_color;
    pub_attr.intf_sync = K_VO_OUT_1080P30;
    pub_attr.intf_type = K_VO_INTF_MIPI;
    pub_attr.sync_info = resolution;

    memset(&sync_attr, 0, sizeof(k_vo_sync_attr));
    sync_attr.hsync_start = 2;
    sync_attr.hsync_stop = 5;

    sync_attr.hsync1_start = 2;
    sync_attr.hsync1_stop = 5;
    sync_attr.hsync2_start = 2;
    sync_attr.hsync2_stop = 5;

    sync_attr.vsync1_start = 0;
    sync_attr.vsync1_stop = 0;
    sync_attr.vsync2_start = 0;
    sync_attr.vsync2_stop = 0;

    connector_set_vo_init();
    connector_set_vtth_intr(1, intr_line);
    connector_set_vo_attr(&pub_attr, &sync_attr);
    connector_set_vo_enable();

    return 0;
}

k_s32 lt9611_init(void *ctx, k_connector_info *info)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;
    struct lt9611_dev *lt9611_dev = connector_get_drvdata(dev);
    k_u32 pclk = info->resolution.pclk;

    ret |= lt9611_init_system(lt9611_dev);
    ret |= lt9611_mipi_input_analog(lt9611_dev);
    ret |= lt9611_mipi_input_digital(lt9611_dev);
    ret |= lt9611_setup_pll(lt9611_dev, pclk);
    ret |= lt9611_setup_pcr(lt9611_dev);
    ret |= lt9611_setup_timing(lt9611_dev, &info->resolution);
    ret |= lt9611_hdmi_tx_digital(lt9611_dev);
    ret |= lt9611_hdmi_tx_phy(lt9611_dev);
    ret |= lt9611_irq_init(lt9611_dev);
    ret |= lt9611_enable_hpd_interrupts(lt9611_dev);
    ret |= lt9611_enable_hdmi_out(lt9611_dev);
    if(info->pixclk_div != 0)
        connector_set_pixclk(info->pixclk_div);
    ret |= k230_vo_resolution_init(info->type, &info->resolution, info->bg_color, info->intr_line);
    ret |= k230_set_phy_freq(&info->phy_attr);
    ret |= k230_dsi_resolution_init(info);
    rt_thread_mdelay(50);
    ret |= lt9611_read_video(lt9611_dev);

    return ret;
}

static k_s32 lt9611_get_chip_id(void* ctx, k_u32* chip_id)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;
    struct lt9611_dev *lt9611_dev = connector_get_drvdata(dev);
    k_u8 id_high;
    k_u8 id_mid;

    ret |= lt9611_read_reg(&lt9611_dev->i2c_info, 0x00, &id_high);
    ret |= lt9611_read_reg(&lt9611_dev->i2c_info, 0x01, &id_mid);

    chip_id = (id_high << 8) | id_mid;
    LOG_D("lt9611 chip_id %02x\n", chip_id);

    return ret;
}

#define EDID_BLOCK_COUNT    8
#define EDID_BLOCK_SIZE     32
static k_s32 lt9611_read_edid(struct lt9611_dev *lt9611_dev, k_u8 *edid_data, k_s32 len)
{
    k_s32 ret = 0;
    k_s32 conn = 0;
    k_s32 i, j;
    k_u8 hpd_state = 0x00;
    k_u8 ddc_state = 0x00;
    k_u8 reg_val = 0x00;

    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    ret |= lt9611_read_reg(&lt9611_dev->i2c_info, 0x5e, &hpd_state);
    conn = !!(hpd_state & 0x04);
    if (conn == 0) {
        LOG_W("HDMI monitor disconnected \n");
        return 0;
    }

    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x85);
    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x03, 0xc9);
    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x04, 0xa0);
    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x05, 0x00);
    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x06, EDID_BLOCK_SIZE);
    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x14, 0x7f);

    for (i = 0; i < EDID_BLOCK_COUNT; i++) {
        ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x05, i * EDID_BLOCK_SIZE);
        ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0x36);
        ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0x31);
        ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0x37);
        rt_thread_mdelay(8);

        ret |= lt9611_read_reg(&lt9611_dev->i2c_info, 0x40, &ddc_state);
        if (ddc_state & 0x02) {
            for (j = 0; j < EDID_BLOCK_SIZE; j++) {
                ret |= lt9611_read_reg(&lt9611_dev->i2c_info, 0x83, &reg_val);
                edid_data[i * EDID_BLOCK_SIZE + j] = reg_val;
            }
        } else if (ddc_state & 0x50) {
            LOG_W("HDMI read EDID failed: DDC no ack \n");
            lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0x1f);
            return -1;
        } else {
            LOG_W("HDMI read EDID failed \n");
            lt9611_write_reg(&lt9611_dev->i2c_info, 0x07, 0x1f);
            return -1;
        }
    }

    return 256;
}

#define MATCH_OK        0
#define MATCH_FAILED    1
static k_s32 parse_stardard_timing(k_u8 *stardard_timing, k_s32 len, struct lt9611_resolution *lt9611_resolution)
{
    k_s32 i = 0;

    for (i = 0; i < len; i = i + 2) {
        if ((stardard_timing[i] == lt9611_resolution->std_timing_high) &&
                (stardard_timing[i + 1] == lt9611_resolution->std_timing_low))
            return MATCH_OK;
    }

    return MATCH_FAILED;
}

static k_s32 parse_cea861_timing(k_u8 *cea861_timing, k_s32 len, struct lt9611_resolution *lt9611_resolution)
{
    k_s32 i = 0;

    for (i = 0; i < len; i ++) {
        if (cea861_timing[i] == lt9611_resolution->cea861_vic)
            return MATCH_OK;
    }

    return MATCH_FAILED;
}

static k_s32 lt9611_parse_edid(k_u8 *edid_data, k_s32 len, k_connector_negotiated_data *negotiated_data)
{
    k_u8 stardard_timing[16];
    k_u8 cea861_timing[256];
    k_u8 cea861_len = 0;
    k_s32 i = 0;
    k_s32 match = 0;
    k_s32 negotiated_count = 0;

    memcpy(stardard_timing, edid_data + 0x26, 16);
    if (edid_data[0x7E] == 0x01 && edid_data[0x80] == 0x02) {
        cea861_len = edid_data[0x84] & 0x1F;
        memcpy(cea861_timing, edid_data + 0x85, cea861_len);
    }

    for (i = 0; i < LT9611_RESOLUTION_NUM; i ++) {
        match = parse_stardard_timing(stardard_timing, 16, &lt9611_resolution[i]);
        if (match == MATCH_OK) {
            negotiated_data->negotiated_types[negotiated_count] = lt9611_resolution[i].connector_type;
            negotiated_count ++;
            continue;
        }

        match = parse_cea861_timing(cea861_timing, cea861_len, &lt9611_resolution[i]);
        if (match == MATCH_OK) {
            negotiated_data->negotiated_types[negotiated_count] = lt9611_resolution[i].connector_type;
            negotiated_count ++;
            continue;
        }
    }
    negotiated_data->negotiated_count = negotiated_count;

    return negotiated_count;
}

static k_s32 lt9611_get_negotiated_data(void* ctx, k_connector_negotiated_data *negotiated_data)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;
    struct lt9611_dev *lt9611_dev = connector_get_drvdata(dev);
    k_u8 edid_data[256];

    memset(edid_data, 0x00, 256);
    ret = lt9611_read_edid(lt9611_dev, edid_data, 256);
    if (ret == 0) {
        negotiated_data->connection_status = 0;
        negotiated_data->negotiated_count = 1;
        negotiated_data->negotiated_types[0] = LT9611_MIPI_4LAN_1920X1080_60FPS;
        return 1;
    }

    ret = lt9611_parse_edid(edid_data, 256, negotiated_data);
    if (ret == 0) {
        negotiated_data->connection_status = 1;
        negotiated_data->negotiated_count = 1;
        negotiated_data->negotiated_types[0] = LT9611_MIPI_4LAN_1920X1080_60FPS;
        return 1;
    }

    return negotiated_data->negotiated_count;
}

static k_s32 lt9611_conn_check(void* ctx, k_s32* conn)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;
    struct lt9611_dev *lt9611_dev = connector_get_drvdata(dev);
    k_u8 hpd_state = 0;

    ret |= lt9611_write_reg(&lt9611_dev->i2c_info, 0xff, 0x82);
    ret |= lt9611_read_reg(&lt9611_dev->i2c_info, 0x5e, &hpd_state);

    *conn = !!(hpd_state & 0x04);

    return ret;
}

struct connector_driver_dev lt9611_connector_drv = {
    .connector_name = "lt9611",
    .connector_func = {
        .connector_power = lt9611_power_on,
        .connector_init = lt9611_init,
        .connector_get_chip_id = lt9611_get_chip_id,
        .connector_get_negotiated_data = lt9611_get_negotiated_data,
        .connector_conn_check = lt9611_conn_check,
    },
};
