/******************************************************************************
 * Copyright (c) 2013-2016 Realtek Semiconductor Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include "usb_io_realtek.h"
//#include "usbh_wifi.h"
#include "rtwlan_bsp.h"

#ifndef NULL
#define	NULL	0
#endif
enum RTW_USB_SPEED {
	RTW_USB_SPEED_UNKNOWN	= 0,
	RTW_USB_SPEED_1_1	= 1,
	RTW_USB_SPEED_2		= 2,
	RTW_USB_SPEED_3		= 3,
};

extern void *g_rtk_wifi_usb;
extern unsigned char g_rtk_wifi_connect;
static void rtw_usb_probe(struct usb_driver *driver)
{
	printf("\n\rrtw_usb_probe start------->\n");

	while (1) {
            if (g_rtk_wifi_connect)
                break;	
            else
                WLAN_BSP_UsLoop(10000);
	}

	printf("\n\rrtw_usb_probe end<-------\n");
}

static int rtw_usb_ctrl_req(void *priv, unsigned char bdir_in, unsigned int wvalue, unsigned char *buf, unsigned int len)
{
	int ret = 0;
	if (USBH_WIFI_Crtlreq(priv, bdir_in, wvalue, buf, len) == USB_OK) {
		ret = 1;
	}
	return ret;
}

static int rtw_usb_get_speed_info(void *priv)
{
	unsigned char usb_speed = 0;
	unsigned char ret = RTW_USB_SPEED_UNKNOWN;

	usb_speed = USBH_WIFI_GetSpeed(priv);

	switch (usb_speed) {
	case 0:
		printf("high Speed Case \n");
		ret = RTW_USB_SPEED_3;
		break;
	case 1:
		printf("full speed Case \n");
		ret = RTW_USB_SPEED_2;
		break;
	case 2:
		printf("low speed Case \n");
		ret = RTW_USB_SPEED_1_1;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

static int rtw_usb_get_in_ep_info(void *priv, unsigned char *ep_addr_array)
{
	return USBH_WIFI_GetInEpInfo(priv, ep_addr_array);
}

static int rtw_usb_get_out_ep_info(void *priv, unsigned char *ep_addr_array)
{
	return USBH_WIFI_GetOutEpInfo(priv, ep_addr_array);
}

static unsigned char rtw_usb_get_bulk_in_pipe(void *priv, unsigned char ep_addr)
{
	return USBH_WIFI_GetBulkInPipe(priv, ep_addr);
}

static unsigned char rtw_usb_get_bulk_out_pipe(void *priv, unsigned char ep_addr)
{
	return USBH_WIFI_GetBulkOutPipe(priv, ep_addr);
}

static int rtw_usb_bulk_in(void *priv, unsigned char pipe, unsigned char *buf, unsigned int len, usb_complete callback, void *arg)
{
	return USBH_WIFI_BulkIn(priv, pipe, buf, len, callback, arg);
}

static int rtw_usb_bulk_out(void *priv, unsigned char pipe, unsigned char *buf, unsigned int len, usb_complete callback, void *arg)
{

	return USBH_WIFI_BulkOut(priv, pipe, buf, len, callback, arg);
}

static int rtw_usb_cancel_bulk_in(void *priv)
{
	return USBH_WIFI_CancelBulkIn(priv);
}

static int rtw_usb_cancel_bulk_out(void *priv)
{
	return USBH_WIFI_CancelBulkOut(priv);
}

extern USB_BUS_OPS rtw_usb_bus_ops = {
	rtw_usb_probe,
	NULL,
	NULL,
	rtw_usb_ctrl_req,
	rtw_usb_get_speed_info,
	rtw_usb_get_in_ep_info,
	rtw_usb_get_out_ep_info,
	rtw_usb_get_bulk_in_pipe,
	rtw_usb_get_bulk_out_pipe,
	rtw_usb_bulk_in,
	rtw_usb_bulk_out,
	rtw_usb_cancel_bulk_in,
	rtw_usb_cancel_bulk_out
};
