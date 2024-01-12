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

#define DELAY_MS_BACKLIGHT_DEFAULT     200
#define DELAY_MS_BACKLIGHT_FIRST       1

static k_s32 g_blacklight_delay_ms = DELAY_MS_BACKLIGHT_FIRST;


static void st7701_480x800_init(k_u8 test_mode_en)
{
    // k_u8 param1[] = {0x11, 0x00};
    k_u8 param2[] = {0xFF, 0x77,0x01,0x00,0x00,0x13};
    k_u8 param3[] = {0xEF, 0x08};
    k_u8 param4[] = {0xFF, 0x77,0x01,0x00,0x00,0x10};
    k_u8 param5[] = {0xC0, 0x63,0x00};      // (99 + 1) * 8  = 800
    k_u8 param6[] = {0xC1, 0x10, 0x02};     // vbp  = 0x10 = 16   vfp = 0x02
    k_u8 param7[] = {0xC2, 0x31, 0x02};     // pclk = 512 + 12 = 614
    k_u8 param8[] = {0xCC, 0x10};
    k_u8 param9[] = {0xB0, 0xC0, 0x0C, 0x92, 0x0C, 0x10, 0x05, 0x02, 0x0D, 0x07, 0x21, 0x04, 0x53, 0x11, 0x6A, 0x32, 0x1F};
    k_u8 param10[] = {0xB1, 0xC0, 0x87, 0xCF, 0x0C, 0x10, 0x06, 0x00, 0x03, 0x08, 0x1D, 0x06, 0x54, 0x12, 0xE6, 0xEC, 0x0F};
    k_u8 param11[] = {0xFF, 0x77,0x01,0x00,0x00,0x11};
    k_u8 param12[] = {0xB0, 0x5D};
    k_u8 param13[] = {0xB1, 0x62};
    k_u8 param14[] = {0xB2, 0x82};
    k_u8 param15[] = {0xB3, 0x80}; // 0x05
    k_u8 param16[] = {0xB5, 0x42};
    k_u8 param17[] = {0xB7, 0x85};
    k_u8 param18[] = {0xB8, 0x20};
    k_u8 param19[] = {0xC0, 0x09};
    k_u8 param20[] = {0xC1, 0x78};
    k_u8 param21[] = {0xC2, 0x78};
    k_u8 param22[] = {0xD0, 0x88};
    k_u8 param23[] = {0xEE, 0x42};

    k_u8 param24[] = {0xE0, 0x00, 0x00, 0x02};
    k_u8 param25[] = {0xE1, 0x04,0xA0,0x06,0xA0,0x05,0xA0,0x07,0xA0,0x00,0x44,0x44};
    k_u8 param26[] = {0xE2, 0x00,0x00,0x33,0x33,0x01,0xA0,0x00,0x00,0x01,0xA0,0x00,0x00};
    k_u8 param27[] = {0xE3, 0x00,0x00,0x33,0x33};
    k_u8 param28[] = {0xE4, 0x44,0x44}; // 0x05
    k_u8 param29[] = {0xE5, 0x0C,0x30,0xA0,0xA0,0x0E,0x32,0xA0,0xA0,0x08,0x2C,0xA0,0xA0,0x0A,0x2E,0xA0,0xA0};
    k_u8 param30[] = {0xE6, 0x00,0x00,0x33,0x33};
    k_u8 param31[] = {0xE7, 0x44,0x44};
    k_u8 param32[] = {0xE8, 0x0D,0x31,0xA0,0xA0,0x0F,0x33,0xA0,0xA0,0x09,0x2D,0xA0,0xA0,0x0B,0x2F,0xA0,0xA0};
    k_u8 param33[] = {0xEB, 0x00,0x01,0xE4,0xE4,0x44,0x88,0x00};
    k_u8 param34[] = {0xED, 0xFF,0xF5,0x47,0x6F,0x0B,0xA1,0xA2,0xBF,0xFB,0x2A,0x1A,0xB0,0xF6,0x74,0x5F,0xFF};
    k_u8 param35[] = {0xEF, 0x08,0x08,0x08,0x40,0x3F, 0x64};
    k_u8 param36[] = {0xFF, 0x77,0x01,0x00,0x00,0x13};
    k_u8 param37[] = {0xE8, 0x00, 0x0E};
    k_u8 param38[] = {0xFF, 0x77,0x01,0x00,0x00,0x00};
    k_u8 param39[] = {0x11};

    k_u8 param40[] = {0xFF, 0x77,0x01,0x00,0x00,0x13};
    k_u8 param41[] = {0xE8, 0x00, 0x0C};

    k_u8 param42[] = {0xE8, 0x00, 0x00};
    k_u8 param43[] = {0xFF, 0x77,0x01,0x00,0x00,0x00};
    k_u8 param44[] = {0x3A, 0x50};
    k_u8 param45[] = {0x29};

    // k_u32 val = connecter_dsi_read_pkg(0xB9);

    // rt_kprintf("0xB9 val is %d \n", val);

    // connecter_dsi_send_pkg(param1, sizeof(param1));
    // usleep(120000);
    connecter_dsi_send_pkg(param2, sizeof(param2));
    connecter_dsi_send_pkg(param3, sizeof(param3));
    connecter_dsi_send_pkg(param4, sizeof(param4));
    connecter_dsi_send_pkg(param5, sizeof(param5));
    connecter_dsi_send_pkg(param6, sizeof(param6));
    connecter_dsi_send_pkg(param7, sizeof(param7));
    connecter_dsi_send_pkg(param8, sizeof(param8));
    connecter_dsi_send_pkg(param9, sizeof(param9));
    connecter_dsi_send_pkg(param10, sizeof(param10));
    connecter_dsi_send_pkg(param11, sizeof(param11));
    connecter_dsi_send_pkg(param12, sizeof(param12));
    connecter_dsi_send_pkg(param13, sizeof(param13));
    connecter_dsi_send_pkg(param14, sizeof(param14));
    connecter_dsi_send_pkg(param15, sizeof(param15));
    connecter_dsi_send_pkg(param16, sizeof(param16));
    connecter_dsi_send_pkg(param17, sizeof(param17));
    connecter_dsi_send_pkg(param18, sizeof(param18));
    connecter_dsi_send_pkg(param19, sizeof(param19));
    connecter_dsi_send_pkg(param20, sizeof(param20));
    connecter_dsi_send_pkg(param21, sizeof(param21));
    connecter_dsi_send_pkg(param22, sizeof(param22));
    connecter_dsi_send_pkg(param23, sizeof(param23));
    connector_delay_us(100000);
    connecter_dsi_send_pkg(param24, sizeof(param24));
    connecter_dsi_send_pkg(param25, sizeof(param25));
    connecter_dsi_send_pkg(param26, sizeof(param26));
    connecter_dsi_send_pkg(param27, sizeof(param27));
    connecter_dsi_send_pkg(param28, sizeof(param28));
    connecter_dsi_send_pkg(param29, sizeof(param29));
    connecter_dsi_send_pkg(param30, sizeof(param30));
    connecter_dsi_send_pkg(param31, sizeof(param31));
    connecter_dsi_send_pkg(param32, sizeof(param32));
    connecter_dsi_send_pkg(param33, sizeof(param33));
    connecter_dsi_send_pkg(param34, sizeof(param34));
    connecter_dsi_send_pkg(param35, sizeof(param35));
    connecter_dsi_send_pkg(param36, sizeof(param36));
    connecter_dsi_send_pkg(param37, sizeof(param37));
    connecter_dsi_send_pkg(param38, sizeof(param38));
    connecter_dsi_send_pkg(param39, sizeof(param39));
    connector_delay_us(200000);
    connecter_dsi_send_pkg(param40, sizeof(param40));
    connecter_dsi_send_pkg(param41, sizeof(param41));
    connector_delay_us(10000);
    connecter_dsi_send_pkg(param42, sizeof(param42));
    connecter_dsi_send_pkg(param43, sizeof(param43));
    connecter_dsi_send_pkg(param44, sizeof(param44));
    connecter_dsi_send_pkg(param45, sizeof(param45));
    connector_delay_us(50000);
}

static void st7701_480x854_init(k_u8 test_mode_en)
{
    k_u8 param1[] = {0x11, 0x00};
    k_u8 param2[] = {0xFF, 0x77,0x01,0x00,0x00,0x10};
    k_u8 param3[] = {0xC0, 0xE9,0x03};                      // (105 + 1) * 8 + 3 x 2 = 854
    k_u8 param4[] = {0xC1, 0x12,0x02};                      // vbp = 0x12  =18   vfp = 0x2
    k_u8 param5[] = {0xC2, 0x31,0x08};                      // pclk = 512 + (0x8 x 16) = 640
    k_u8 param6[] = {0xCC, 0x10};
    k_u8 param7[] = {0xB0, 0x00,0x0A,0x13,0x0E,0x12,0x07,0x05,0x08,0x08,0x1F,0x07,0x15,0x13,0xE3,0x2A,0x11};
    k_u8 param8[] = {0xB1, 0x00,0x0A,0x12,0x0E,0x12,0x07,0x04,0x07,0x07,0x1E,0x04,0x13,0x10,0x23,0x29,0x11};
    k_u8 param9[] = {0xFF, 0x77,0x01,0x00,0x00,0x11};
    k_u8 param10[] = {0xB0, 0x4D};
    k_u8 param11[] = {0xB1, 0x1C};
    k_u8 param12[] = {0xB2, 0x07};
    k_u8 param13[] = {0xB3, 0x80};
    k_u8 param14[] = {0xB5, 0x47};
    k_u8 param15[] = {0xB7, 0x85}; // 0x05
    k_u8 param16[] = {0xB8, 0x21};

    k_u8 param22[] = {0xB9, 0x10};
    k_u8 param23[] = {0xC1, 0x78};
    k_u8 param24[] = {0xC2, 0x78};
    k_u8 param25[] = {0xD0, 0x88};
    k_u8 param26[] = {0xE0, 0x00,0x00,0x02};
    k_u8 param27[] = {0xE1, 0x0B,0x00,0x0D,0x00,0x0C,0x00,0x0E,0x00,0x00,0x44,0x44};
    k_u8 param28[] = {0xE2, 0x33,0x33,0x44,0x44,0x64,0x00,0x66,0x00,0x65,0x00,0x67,0x00,0x00};
    k_u8 param29[] = {0xE3, 0x00,0x00,0x33,0x33};
    k_u8 param30[] = {0xE4, 0x44,0x44}; // 0x05
    k_u8 param31[] = {0xE5, 0x0C,0x78,0xA0,0xA0,0x0E,0x78,0xA0,0xA0,0x10,0x78,0xA0,0xA0,0x12,0x78,0xA0,0xA0};
    k_u8 param32[] = {0xE6, 0x00,0x00,0x33,0x33};
    k_u8 param33[] = {0xE7, 0x44,0x44};
    k_u8 param34[] = {0xE8, 0x0D,0x78,0xA0,0xA0,0x0F,0x78,0xA0,0xA0,0x11,0x78,0xA0,0xA0,0x13,0x78,0xA0,0xA0};
    k_u8 param35[] = {0xEB, 0x02,0x00,0x39,0x39,0xEE,0x44,0x00};
    k_u8 param36[] = {0xEC, 0x00,0x00};
    k_u8 param37[] = {0xED, 0xFF,0xF1,0x04,0x56,0x72,0x3F,0xFF,0xFF,0xFF,0xFF,0xF3,0x27,0x65,0x40,0x1F,0xFF};
    k_u8 param38[] = {0xFF, 0x77,0x01,0x00,0x00,0x00};
    k_u8 param39[] = {0x29, 0x00};

    connecter_dsi_send_pkg(param1, sizeof(param1));
    usleep(100000);
    connecter_dsi_send_pkg(param2, sizeof(param2));
    connecter_dsi_send_pkg(param3, sizeof(param3));
    connecter_dsi_send_pkg(param4, sizeof(param4));
    connecter_dsi_send_pkg(param5, sizeof(param5));
    connecter_dsi_send_pkg(param6, sizeof(param6));
    connecter_dsi_send_pkg(param7, sizeof(param7));
    connecter_dsi_send_pkg(param8, sizeof(param8));
    connecter_dsi_send_pkg(param9, sizeof(param9));
    connecter_dsi_send_pkg(param10, sizeof(param10));
    connecter_dsi_send_pkg(param11, sizeof(param11));
    connecter_dsi_send_pkg(param12, sizeof(param12));
    connecter_dsi_send_pkg(param13, sizeof(param13));
    connecter_dsi_send_pkg(param14, sizeof(param14));
    connecter_dsi_send_pkg(param15, sizeof(param15));
    connecter_dsi_send_pkg(param16, sizeof(param16));

    connecter_dsi_send_pkg(param22, sizeof(param22));
    connecter_dsi_send_pkg(param23, sizeof(param23));
    connecter_dsi_send_pkg(param24, sizeof(param24));
    connecter_dsi_send_pkg(param25, sizeof(param25));
    usleep(100000);
    connecter_dsi_send_pkg(param26, sizeof(param26));
    connecter_dsi_send_pkg(param27, sizeof(param27));
    connecter_dsi_send_pkg(param28, sizeof(param28));
    connecter_dsi_send_pkg(param29, sizeof(param29));

    connecter_dsi_send_pkg(param30, sizeof(param30));
    connecter_dsi_send_pkg(param31, sizeof(param31));
    connecter_dsi_send_pkg(param32, sizeof(param32));
    connecter_dsi_send_pkg(param33, sizeof(param33));
    connecter_dsi_send_pkg(param34, sizeof(param34));
    connecter_dsi_send_pkg(param35, sizeof(param35));
    connecter_dsi_send_pkg(param36, sizeof(param36));
    connecter_dsi_send_pkg(param37, sizeof(param37));
    connecter_dsi_send_pkg(param38, sizeof(param38));
    connecter_dsi_send_pkg(param39, sizeof(param39));


    connecter_dsi_read_pkg(0x5);
}


static void st7701_power_reset(k_s32 on)
{
    k_u8 rst_gpio;

    rst_gpio = DISPLAY_LCD_RST_GPIO;

    // rt_kprintf("rst_gpio is %d \n",rst_gpio);

    kd_pin_mode(rst_gpio, GPIO_DM_OUTPUT);
    
    if (on)
        kd_pin_write(rst_gpio, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    else
        kd_pin_write(rst_gpio, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH

}

static void st7701_set_backlight(k_s32 on)
{
    k_u8 backlight_gpio;

    backlight_gpio = DISPLAY_LCD_BACKLIGHT_EN;

    // rt_kprintf("backlight_gpio is %d \n",backlight_gpio);

    kd_pin_mode(backlight_gpio, GPIO_DM_OUTPUT);
    if (on)
        kd_pin_write(backlight_gpio, GPIO_PV_HIGH);
    else
        kd_pin_write(backlight_gpio, GPIO_PV_LOW);

}


static k_s32 st7701_power_on(void* ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;
    // rst vo;
    k230_display_rst();

    if (on) {
        
        // rst st7701 
        st7701_power_reset(1);
        rt_thread_mdelay(g_blacklight_delay_ms);
        st7701_power_reset(0);
        rt_thread_mdelay(g_blacklight_delay_ms);
        st7701_power_reset(1);

        g_blacklight_delay_ms = DELAY_MS_BACKLIGHT_DEFAULT;
        //enable backlight
        st7701_set_backlight(1);
    } else {
        st7701_set_backlight(0);
    }
    
    return ret;
}


static k_s32 st7701_set_phy_freq(k_connectori_phy_attr *phy_attr)
{
    k_vo_mipi_phy_attr mipi_phy_attr;

    memset(&mipi_phy_attr, 0, sizeof(k_vo_mipi_phy_attr));

    mipi_phy_attr.m = phy_attr->m;
    mipi_phy_attr.n = phy_attr->n;
    mipi_phy_attr.hs_freq = phy_attr->hs_freq;
    mipi_phy_attr.voc = phy_attr->voc;
    mipi_phy_attr.phy_lan_num = K_DSI_4LAN;
    connector_set_phy_freq(&mipi_phy_attr);
    // rt_kprintf("config phy success phy_attr->m is %d  phy_attr->n is %d phy_attr->hs_freq is %d phy_attr->voc is %d \n", phy_attr->m,  phy_attr->n, phy_attr->hs_freq, phy_attr->voc);
    return 0;
}


static k_s32 st7701_dsi_resolution_init(k_connector_info *info)
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

    if(info->type == ST7701_V1_MIPI_2LAN_480X800_30FPS)
    {
        if(info->screen_test_mode)
            st7701_480x800_init(1);
        else
            st7701_480x800_init(0);
    }
    else if(info->type == ST7701_V1_MIPI_2LAN_480X854_30FPS)
    {
        if(info->screen_test_mode)
            st7701_480x854_init(1);
        else
            st7701_480x854_init(0);
    }

    connector_set_dsi_enable(1);

    if(info->dsi_test_mode == 1)
        connector_set_dsi_test_mode();
        
    return 0;
}


static k_s32 st7701_vo_resolution_init(k_vo_display_resolution *resolution, k_u32 bg_color, k_u32 intr_line)
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
   

k_s32 st7701_init(void *ctx, k_connector_info *info)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;

    if(info->pixclk_div != 0)
        connector_set_pixclk(info->pixclk_div);

    ret |= st7701_set_phy_freq(&info->phy_attr);
    ret |= st7701_dsi_resolution_init(info);
    ret |= st7701_vo_resolution_init(&info->resolution, info->bg_color, info->intr_line);

    return ret;
}

static k_s32 st7701_get_chip_id(void* ctx, k_u32* chip_id)
{
    k_s32 ret = 0;

    return ret;
}


static k_s32 st7701_conn_check(void* ctx, k_s32* conn)
{
    k_s32 ret = 0;

    *conn = 1;

    return ret;
}

struct connector_driver_dev st7701_connector_drv = {
    .connector_name = "st7701",
    .connector_func = {
        .connector_power = st7701_power_on,
        .connector_init = st7701_init,
        .connector_get_chip_id = st7701_get_chip_id,
        .connector_conn_check = st7701_conn_check,
    },
};