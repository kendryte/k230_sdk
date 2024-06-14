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

#if 0
static void ili9806_480x800_init(k_u8 test_mode_en)
{
    k_u8 param1[] =  {0xFF, 0xFF,0x98,0x06};
    k_u8 param2[] =  {0xBA, 0x60};
    k_u8 param3[] =  {0xBC, 0x01,0x12,0x61,0xFF,0x10,0x10,0x0B,0x13,0x32,0x73,0xFF,0xFF,0x0E,0x0E,0x00,0x03,0x66,0x63,0x01,0x00,0x00};
    k_u8 param4[] =  {0xBD, 0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67};
    k_u8 param5[] =  {0xBE, 0x00,0x21,0xAB,0x60,0x22,0x22,0x22,0x22,0x22};
    k_u8 param6[] =  {0xC7, 0x5E,0x80};
    k_u8 param7[] =  {0xED, 0x7F,0x0F,0x00};
    k_u8 param8[] =  {0xB6, 0x02};
    k_u8 param9[] =  {0x3A, 0x55};
    k_u8 param10[] = {0xB5, 0x3E,0x18};
    k_u8 param12[] = {0xC0, 0xAB,0x0B,0x0A};
    k_u8 param13[] = {0xFC, 0x09};
    k_u8 param14[] = {0xDF, 0x00,0x00,0x00,0x00,0x00,0x20};
    k_u8 param15[] = {0xF3, 0x74}; // 0x05
    k_u8 param16[] = {0xB4, 0x00,0x00,0x00};
    k_u8 param17[] = {0xF7, 0x82};
    k_u8 param18[] = {0xB1, 0x00,0x12,0x13};
    k_u8 param19[] = {0xF2, 0x00,0x59,0x40,0x28};
    k_u8 param20[] = {0xC1, 0x07,0x80,0x80,0x20};
    k_u8 param21[] = {0xE0, 0x00,0x17,0x1A,0x0D,0x0E,0x0B,0x07,0x05,0x05,0x09,0x0E,0x0F,0x0D,0x1D,0x1A,0x00};
    k_u8 param22[] = {0xE1, 0x00,0x06,0x0E,0x0D,0x0E,0x0D,0x06,0x06,0x05,0x09,0x0D,0x0E,0x0D,0x1F,0x1D,0x00};
    k_u8 param23[] = {0x36, 0x00};
    k_u8 param24[] = {0x35, 0x00};
    k_u8 param25[] = {0x11};
    k_u8 param26[] = {0x29};

    connecter_dsi_send_pkg(param1, sizeof(param1));
    connecter_dsi_send_pkg(param2, sizeof(param2));
    connecter_dsi_send_pkg(param3, sizeof(param3));
    connecter_dsi_send_pkg(param4, sizeof(param4));
    connecter_dsi_send_pkg(param5, sizeof(param5));
    connecter_dsi_send_pkg(param6, sizeof(param6));
    connecter_dsi_send_pkg(param7, sizeof(param7));
    connecter_dsi_send_pkg(param8, sizeof(param8));
    connecter_dsi_send_pkg(param9, sizeof(param9));
    connecter_dsi_send_pkg(param10, sizeof(param10));

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
    connecter_dsi_send_pkg(param23, 2);
    connecter_dsi_send_pkg(param24, 2);
    connecter_dsi_send_pkg(param25, 1);
    rt_thread_mdelay(120);
    connecter_dsi_send_pkg(param26, 1);
    rt_thread_mdelay(20);
}
#else
static void ili9806_480x800_init(k_u8 test_mode_en)
{
    k_u8 param1[] =  {0xFF,0xff,0x98,0x06,0x04,0x01};
    k_u8 param2[] =  {0x08,0x10};
    k_u8 param3[] =  {0x20,0x00};
    k_u8 param4[] =  {0x21,0x01};
    k_u8 param5[] =  {0x30,0x02};
    k_u8 param6[] =  {0x31,0x02};
    k_u8 param7[] =  {0x60,0x07};
    k_u8 param8[] =  {0x61,0x06};
    k_u8 param9[] =  {0x62,0x06};
    k_u8 param10[] = {0x63,0x04};
    k_u8 param12[] = {0x40,0x18};
    k_u8 param13[] = {0x41,0x33};
    k_u8 param14[] = {0x42,0x11};
    k_u8 param15[] = {0x43,0x09}; // 0x05
    k_u8 param16[] = {0x44,0x0c};
    k_u8 param17[] = {0x46,0x55};
    k_u8 param18[] = {0x47,0x55};
    k_u8 param19[] = {0x45,0x14};
    k_u8 param20[] = {0x50,0x50};
    k_u8 param21[] = {0x51,0x50};
    k_u8 param22[] = {0x52,0x00};
    k_u8 param23[] = {0x53,0x38};
    
    k_u8 param24[] = {0xA0,0x00};
    k_u8 param25[] = {0xa1,0x09};
    k_u8 param26[] = {0xa2,0x0c};
    k_u8 param27[] = {0xa3,0x0f};
    k_u8 param28[] = {0xa4,0x06};
    k_u8 param29[] = {0xa5,0x09};
    k_u8 param30[] = {0xa6,0x07};
    k_u8 param31[] = {0xa7,0x16};
    k_u8 param32[] = {0xa8,0x06};
    k_u8 param33[] = {0xa9,0x09};
    k_u8 param34[] = {0xaa,0x11};
    k_u8 param35[] = {0xab,0x06};
    k_u8 param36[] = {0xac,0x0e};
    k_u8 param37[] = {0xad,0x19};
    k_u8 param38[] = {0xae,0x0e};
    k_u8 param39[] = {0xaf,0x00};

    k_u8 param90[] = {0xc0,0x00};
    k_u8 param91[] = {0xc1,0x09};
    k_u8 param92[] = {0xc2,0x0c};
    k_u8 param93[] = {0xc3,0x0f};
    k_u8 param94[] = {0xc4,0x06};
    k_u8 param95[] = {0xc5,0x09};
    k_u8 param96[] = {0xc6,0x07};
    k_u8 param97[] = {0xc7,0x16};
    k_u8 param98[] = {0xc8,0x06};
    k_u8 param99[] = {0xc9,0x09};
    k_u8 parama1[] = {0xca,0x11};
    k_u8 parama2[] = {0xcb,0x06};
    k_u8 parama3[] = {0xcc,0x0e};
    k_u8 parama4[] = {0xcd,0x19};
    k_u8 parama5[] = {0xce,0x0e};
    k_u8 parama6[] = {0xcf,0x00};


    k_u8 parama7[] = {0xff,0xff,0x98,0x06,0x04,0x06};
    k_u8 parama8[] = {0x00,0xa0};
    k_u8 parama9[] = {0x01,0x05};
    k_u8 paramb0[] = {0x02,0x00};
    k_u8 paramb1[] = {0x03,0x00};
    k_u8 paramb2[] = {0x04,0x01};
    k_u8 paramb3[] = {0x05,0x01};
    k_u8 paramb4[] = {0x06,0x88};
    k_u8 paramb5[] = {0x07,0x04};
    k_u8 paramb6[] = {0x08,0x01};
    k_u8 paramb7[] = {0x09,0x90};
    k_u8 paramb8[] = {0x0a,0x04};
    k_u8 paramb9[] = {0x0b,0x01};
    k_u8 paramc0[] = {0x0c,0x01};
    k_u8 paramc1[] = {0x0d,0x01};
    k_u8 paramc2[] = {0x0e,0x00};
    k_u8 paramc3[] = {0x0f,0x00};


    k_u8 param40[] = {0x10,0x55};
    k_u8 param41[] = {0x11,0x50};
    k_u8 param42[] = {0x12,0x01};
    k_u8 param43[] = {0x13,0x85};
    k_u8 param44[] = {0x14,0x85};
    k_u8 param45[] = {0x15,0xc0};
    k_u8 param46[] = {0x16,0x0b};
    k_u8 param47[] = {0x17,0x00};
    k_u8 param48[] = {0x18,0x00};
    k_u8 param49[] = {0x19,0x00};
    k_u8 param50[] = {0x1a,0x00};
    k_u8 param51[] = {0x1b,0x00};
    k_u8 param52[] = {0x1c,0x00};
    k_u8 param53[] = {0x1d,0x00};

    k_u8 param54[] = {0x20,0x01};
    k_u8 param55[] = {0x21,0x23};
    k_u8 param56[] = {0x22,0x45};
    k_u8 param57[] = {0x23,0x67};
    k_u8 param58[] = {0x24,0x01};
    k_u8 param59[] = {0x25,0x23};
    k_u8 param60[] = {0x26,0x45};
    k_u8 param61[] = {0x27,0x67};

    k_u8 param62[] = {0x30,0x02};
    k_u8 param63[] = {0x31,0x22};
    k_u8 param64[] = {0x32,0x11};
    k_u8 param65[] = {0x33,0xaa};
    k_u8 param66[] = {0x34,0xbb};
    k_u8 param67[] = {0x35,0x66};
    k_u8 param68[] = {0x36,0x00};
    k_u8 param69[] = {0x37,0x22};
    k_u8 param70[] = {0x38,0x22};
    k_u8 param71[] = {0x39,0x22};
    k_u8 param72[] = {0x3a,0x22};
    k_u8 param73[] = {0x3b,0x22};
    k_u8 param74[] = {0x3c,0x22};
    k_u8 param75[] = {0x3d,0x20};
    k_u8 param76[] = {0x3e,0x22};
    k_u8 param77[] = {0x3f,0x22};

    k_u8 param78[] = {0x40,0x22};
    k_u8 param79[] = {0x52,0x12};
    k_u8 param80[] = {0x53,0x12};

    k_u8 param81[] = {0xff,0xff,0x98,0x06,0x04,0x07};
    k_u8 param82[] = {0x17,0x32};
    k_u8 param83[] = {0x02,0x17};
    k_u8 param84[] = {0x18,0x1d};
    k_u8 param85[] = {0xe1,0x79};

    k_u8 param86[] = {0xff,0xff,0x98,0x06,0x04,0x00};
    k_u8 param87[] = {0x3a,0x70};
    k_u8 param88[] = {0x11};
    k_u8 param89[] = {0x29};

    connecter_dsi_send_pkg(param1, sizeof(param1));
    connecter_dsi_send_pkg(param2, sizeof(param2));
    connecter_dsi_send_pkg(param3, 2);
    connecter_dsi_send_pkg(param4, sizeof(param4));
    connecter_dsi_send_pkg(param5, sizeof(param5));
    connecter_dsi_send_pkg(param6, sizeof(param6));
    connecter_dsi_send_pkg(param7, sizeof(param7));
    connecter_dsi_send_pkg(param8, sizeof(param8));
    connecter_dsi_send_pkg(param9, sizeof(param9));
    connecter_dsi_send_pkg(param10, sizeof(param10));

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
    connecter_dsi_send_pkg(param22, 2);
    connecter_dsi_send_pkg(param23, sizeof(param23));


    connecter_dsi_send_pkg(param24, 2);
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
    connecter_dsi_send_pkg(param39, 2);

    connecter_dsi_send_pkg(param90, 2);
    connecter_dsi_send_pkg(param91, sizeof(param91));
    connecter_dsi_send_pkg(param92, sizeof(param92));
    connecter_dsi_send_pkg(param93, sizeof(param93));
    connecter_dsi_send_pkg(param94, sizeof(param94));
    connecter_dsi_send_pkg(param95, sizeof(param95));
    connecter_dsi_send_pkg(param96, sizeof(param96));
    connecter_dsi_send_pkg(param97, sizeof(param97));
    connecter_dsi_send_pkg(param98, sizeof(param98));
    connecter_dsi_send_pkg(param99, sizeof(param99));
    connecter_dsi_send_pkg(parama1, sizeof(parama1));
    connecter_dsi_send_pkg(parama2, sizeof(parama2));
    connecter_dsi_send_pkg(parama3, sizeof(parama3));
    connecter_dsi_send_pkg(parama4, sizeof(parama4));
    connecter_dsi_send_pkg(parama5, sizeof(parama5));
    connecter_dsi_send_pkg(parama6, 2);

    connecter_dsi_send_pkg(parama7, sizeof(parama7));
    connecter_dsi_send_pkg(parama8, sizeof(parama8));
    connecter_dsi_send_pkg(parama9, sizeof(parama9));
    connecter_dsi_send_pkg(paramb0, 2);
    connecter_dsi_send_pkg(paramb1, 2);
    connecter_dsi_send_pkg(paramb2, sizeof(paramb2));
    connecter_dsi_send_pkg(paramb3, sizeof(paramb3));
    connecter_dsi_send_pkg(paramb4, sizeof(paramb4));
    connecter_dsi_send_pkg(paramb5, sizeof(paramb5));
    connecter_dsi_send_pkg(paramb6, sizeof(paramb6));
    connecter_dsi_send_pkg(paramb7, sizeof(paramb7));
    connecter_dsi_send_pkg(paramb8, sizeof(paramb8));
    connecter_dsi_send_pkg(paramb9, sizeof(paramb9));
    connecter_dsi_send_pkg(paramc0, sizeof(paramc0));
    connecter_dsi_send_pkg(paramc1, sizeof(paramc1));
    connecter_dsi_send_pkg(paramc2, 2);
    connecter_dsi_send_pkg(paramc3, 2);
    

    connecter_dsi_send_pkg(param40, sizeof(param40));
    connecter_dsi_send_pkg(param41, sizeof(param41));
    connecter_dsi_send_pkg(param42, sizeof(param42));
    connecter_dsi_send_pkg(param43, sizeof(param43));
    connecter_dsi_send_pkg(param44, sizeof(param44));
    connecter_dsi_send_pkg(param45, sizeof(param45));
    connecter_dsi_send_pkg(param46, sizeof(param46));
    connecter_dsi_send_pkg(param47, 2);
    connecter_dsi_send_pkg(param48, 2);
    connecter_dsi_send_pkg(param49, 2);
    connecter_dsi_send_pkg(param50, 2);
    connecter_dsi_send_pkg(param51, 2);
    connecter_dsi_send_pkg(param52, 2);
    connecter_dsi_send_pkg(param53, 2);

    connecter_dsi_send_pkg(param54, sizeof(param54));
    connecter_dsi_send_pkg(param55, sizeof(param55));
    connecter_dsi_send_pkg(param56, sizeof(param56));
    connecter_dsi_send_pkg(param57, sizeof(param57));
    connecter_dsi_send_pkg(param58, sizeof(param58));
    connecter_dsi_send_pkg(param59, sizeof(param59));
    connecter_dsi_send_pkg(param60, sizeof(param60));
    connecter_dsi_send_pkg(param61, sizeof(param61));

    connecter_dsi_send_pkg(param62, sizeof(param62));
    connecter_dsi_send_pkg(param63, sizeof(param63));
    connecter_dsi_send_pkg(param64, sizeof(param64));
    connecter_dsi_send_pkg(param65, sizeof(param65));
    connecter_dsi_send_pkg(param66, sizeof(param66));
    connecter_dsi_send_pkg(param67, sizeof(param67));
    connecter_dsi_send_pkg(param68, 2);
    connecter_dsi_send_pkg(param69, sizeof(param69));
    connecter_dsi_send_pkg(param70, sizeof(param70));
    connecter_dsi_send_pkg(param71, sizeof(param71));
    connecter_dsi_send_pkg(param72, sizeof(param72));
    connecter_dsi_send_pkg(param73, sizeof(param73));
    connecter_dsi_send_pkg(param74, sizeof(param74));
    connecter_dsi_send_pkg(param75, sizeof(param75));
    connecter_dsi_send_pkg(param76, sizeof(param76));
    connecter_dsi_send_pkg(param77, sizeof(param77));

    connecter_dsi_send_pkg(param78, sizeof(param78));
    connecter_dsi_send_pkg(param79, sizeof(param79));
    connecter_dsi_send_pkg(param80, sizeof(param80));

    connecter_dsi_send_pkg(param81, sizeof(param81));
    connecter_dsi_send_pkg(param82, sizeof(param82));
    connecter_dsi_send_pkg(param83, sizeof(param83));
    connecter_dsi_send_pkg(param84, sizeof(param84));
    connecter_dsi_send_pkg(param85, sizeof(param85));

    connecter_dsi_send_pkg(param86, 6);
    connecter_dsi_send_pkg(param87, sizeof(param87));
    connecter_dsi_send_pkg(param88, sizeof(param88));
    rt_thread_mdelay(150);
    connecter_dsi_send_pkg(param89, sizeof(param89));
    rt_thread_mdelay(150);

}

#endif

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
        ili9806_power_reset(1);
        rt_thread_mdelay(g_blacklight_delay_ms);
        ili9806_power_reset(0);
        rt_thread_mdelay(g_blacklight_delay_ms);
        ili9806_power_reset(1);

        g_blacklight_delay_ms = DELAY_MS_BACKLIGHT_DEFAULT;
        //enable backlight
        ili9806_set_backlight(1);
    } else {
        ili9806_set_backlight(0);
    }
    
    return ret;
}


static k_s32 ili9806_set_phy_freq(k_connectori_phy_attr *phy_attr)
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


static k_s32 ili9806_dsi_resolution_init(k_connector_info *info)
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

    if(info->screen_test_mode)
        ili9806_480x800_init(1);
    else
        ili9806_480x800_init(0);

    connector_set_dsi_enable(1);

    if(info->dsi_test_mode == 1)
        connector_set_dsi_test_mode();

    return 0;
}


static k_s32 ili9806_vo_resolution_init(k_vo_display_resolution *resolution, k_u32 bg_color, k_u32 intr_line)
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
   

k_s32 ili9806_init(void *ctx, k_connector_info *info)
{
    k_s32 ret = 0;
    struct connector_driver_dev* dev = ctx;

    if(info->pixclk_div != 0)
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


static k_s32 ili9806_set_mirror(void* ctx, k_connector_mirror *mirror)
{
    k_connector_mirror ili9806_mirror;

    ili9806_mirror = *mirror;

    switch(ili9806_mirror)
    {
        case K_CONNECTOR_MIRROR_HOR:
            break;
        case K_CONNECTOR_MIRROR_VER: 
            break;
        case K_CONNECTOR_MIRROR_BOTH: 
            break;
        default :
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