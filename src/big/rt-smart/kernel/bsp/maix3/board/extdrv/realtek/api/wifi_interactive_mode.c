#include "autoconf.h"
#if !defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "main.h"
#include <lwip_netconf.h>
#include "tcpip.h"
#include <wlan_test_inc.h>
#include <dhcp/dhcps.h>
#endif

#include <osdep_service.h>
#include <wifi/wifi_conf.h>
#include <wifi/wifi_util.h>
#include <basic_types.h>

#ifndef CONFIG_INTERACTIVE_EXT
#define CONFIG_INTERACTIVE_EXT  0
#endif
#ifndef CONFIG_SSL_CLIENT
#define CONFIG_SSL_CLIENT       0
#endif

#ifndef CONFIG_GOOGLENEST
#define CONFIG_GOOGLENEST          0
#endif
#if CONFIG_LWIP_LAYER
#ifndef CONFIG_WEBSERVER
#define CONFIG_WEBSERVER        0
#endif
#endif
#ifndef CONFIG_OTA_UPDATE
#define CONFIG_OTA_UPDATE       0
#endif
#ifndef CONFIG_BSD_TCP
#define CONFIG_BSD_TCP          0
#endif
#ifndef CONFIG_ENABLE_P2P
#define CONFIG_ENABLE_P2P       0
#endif
#define SCAN_WITH_SSID		1	

#if CONFIG_WPS
#define STACKSIZE               1280
#else
#define STACKSIZE               1024
#endif

#ifndef WLAN0_NAME
  #define WLAN0_NAME		"wlan0"
#endif
#ifndef WLAN1_NAME
  #define WLAN1_NAME		"wlan1"
#endif
/* Give default value if not defined */
#ifndef NET_IF_NUM
#if CONFIG_CONCURRENT_MODE
#define NET_IF_NUM 2
#else
#define NET_IF_NUM 1
#endif
#endif

/*Static IP ADDRESS*/
#ifndef IP_ADDR0
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   1
#define IP_ADDR3   80
#endif

/*NETMASK*/
#ifndef NETMASK_ADDR0
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0
#endif

/*Gateway Address*/
#ifndef GW_ADDR0
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   1
#endif

/*Static IP ADDRESS*/
#ifndef AP_IP_ADDR0
#define AP_IP_ADDR0   192
#define AP_IP_ADDR1   168
#define AP_IP_ADDR2   43
#define AP_IP_ADDR3   1
#endif
   
/*NETMASK*/
#ifndef AP_NETMASK_ADDR0
#define AP_NETMASK_ADDR0   255
#define AP_NETMASK_ADDR1   255
#define AP_NETMASK_ADDR2   255
#define AP_NETMASK_ADDR3   0
#endif

/*Gateway Address*/
#ifndef AP_GW_ADDR0
#define AP_GW_ADDR0   192
#define AP_GW_ADDR1   168
#define AP_GW_ADDR2   43
#define AP_GW_ADDR3   1  
#endif

static void cmd_help(int argc, char **argv);
#if CONFIG_SSL_CLIENT
extern void cmd_ssl_client(int argc, char **argv);
#endif

#if CONFIG_GOOGLENEST
extern void cmd_googlenest(int argc, char **argv);
#endif

#if CONFIG_BSD_TCP
extern void cmd_tcp(int argc, char **argv);
extern void cmd_udp(int argc, char **argv);
#endif

#if CONFIG_WLAN
static void cmd_wifi_on(int argc, char **argv);
static void cmd_wifi_off(int argc, char **argv);
static void cmd_wifi_disconnect(int argc, char **argv);
extern void cmd_promisc(int argc, char **argv);
extern void cmd_simple_config(int argc, char **argv);

#if CONFIG_OTA_UPDATE
extern void cmd_update(int argc, char **argv);
#endif

#if CONFIG_WEBSERVER
extern void  start_web_server(void);
extern void  stop_web_server(void);
#endif
extern void cmd_app(int argc, char **argv);
#if CONFIG_ENABLE_WPS
extern void cmd_wps(int argc, char **argv);
#endif
#if defined(CONFIG_ENABLE_WPS_AP) && CONFIG_ENABLE_WPS_AP
extern void cmd_ap_wps(int argc, char **argv);
extern int wpas_wps_dev_config(u8 *dev_addr, u8 bregistrar);
#endif //CONFIG_ENABLE_WPS_AP
#if CONFIG_ENABLE_P2P
extern void cmd_wifi_p2p_start(int argc, char **argv);
extern void cmd_wifi_p2p_stop(int argc, char **argv);
extern void cmd_wifi_p2p_auto_go_start(int argc, char **argv);
extern void cmd_p2p_listen(int argc, char **argv);
extern void cmd_p2p_find(int argc, char **argv);
extern void cmd_p2p_peers(int argc, char **argv);
extern void cmd_p2p_info(int argc, char **argv);
extern void cmd_p2p_disconnect(int argc, char **argv);
extern void cmd_p2p_connect(int argc, char **argv);
#endif //CONFIG_ENABLE_P2P
#if defined(CONFIG_RTL8195A) || defined(CONFIG_RTL8711B) || defined(CONFIG_RTL8721D)
extern u32 CmdDumpWord(IN u16 argc, IN u8 *argv[]);
extern u32 CmdWriteWord(IN u16 argc, IN u8 *argv[]);
#endif
#if CONFIG_LWIP_LAYER
extern struct netif xnetif[NET_IF_NUM]; 
#endif
#if CONFIG_CONCURRENT_MODE
static void cmd_wifi_sta_and_ap(int argc, char **argv)
{
	int timeout = 20;//, mode;
#if CONFIG_LWIP_LAYER
#if !defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	struct netif * pnetiff = (struct netif *)&xnetif[1];
#endif
#endif
	int channel;

	if((argc != 3) && (argc != 4)) {
		DBG_INFO("Usage: wifi_ap SSID CHANNEL [PASSWORD]");
		return;
	}

	if(atoi((const char *)argv[2]) > 14){
		DBG_INFO("bad channel!Usage: wifi_ap SSID CHANNEL [PASSWORD]");
		return;
	}

	if(strlen((const char *)argv[1]) > 32){
		DBG_INFO("Usage: wifi_ap SSID length can't exceed 32\n\r");
		return;
	}
	
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	//TODO
#else
	dhcps_deinit();
#endif
#endif
        
#if 0
	//Check mode
	wext_get_mode(WLAN0_NAME, &mode);

	switch(mode) {
		case IW_MODE_MASTER:	//In AP mode
			cmd_wifi_off(0, NULL);
			cmd_wifi_on(0, NULL);
			break;
		case IW_MODE_INFRA:		//In STA mode
			if(wext_get_ssid(WLAN0_NAME, ssid) > 0)
				cmd_wifi_disconnect(0, NULL);
	}
#endif
	wifi_off();

	rtw_mdelay_os(20);

	if (wifi_on(RTW_MODE_STA_AP) < 0){
		DBG_INFO("ERROR: Wifi on failed!");
		return;
	}

	DBG_INFO("Starting AP ...");
	channel = atoi((const char *)argv[2]);
	if(channel > 13){
		DBG_INFO("Channel is from 1 to 13. Set channel 1 as default!\n");
		channel = 1;
	}

	if(argc == 4) {
		if(wifi_start_ap(argv[1],
							 RTW_SECURITY_WPA2_AES_PSK,
							 argv[3],
							 strlen((const char *)argv[1]),
							 strlen((const char *)argv[3]),
							 channel
							 ) != RTW_SUCCESS) {
			DBG_INFO("ERROR: Operation failed!");
			return;
		}
	}
	else {
		if(wifi_start_ap(argv[1],
							 RTW_SECURITY_OPEN,
							 NULL,
							 strlen((const char *)argv[1]),
							 0,
							 channel
							 ) != RTW_SUCCESS) {
			DBG_INFO("ERROR: Operation failed!");
			return;
		}
	}

	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN1_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)argv[1]) == 0) {
				DBG_INFO("%s started", argv[1]);
				break;
			}
		}

		if(timeout == 0) {
			DBG_INFO("ERROR: Start AP timeout!");
			break;
		}

		rtw_mdelay_os(1000);
		timeout --;
	}
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	//TODO
#else
	LwIP_UseStaticIP(&xnetif[1]);
	dhcps_init(pnetiff);
#endif
#endif
}
#endif

static void cmd_wifi_ap(int argc, char **argv)
{
	int timeout = 20;
#if CONFIG_LWIP_LAYER 
#if !defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct netif * pnetif = &xnetif[0];
#endif
#endif
	int channel;

#if CONFIG_IEEE80211W
	if((argc != 3) && (argc != 4) && (argc != 5)) {
		DBG_INFO("Usage: wifi_ap SSID CHANNEL [PASSWORD] [MFP_SUPPORT]");
		return;
	}
#else
	if((argc != 3) && (argc != 4)) {
		DBG_INFO("Usage: wifi_ap SSID CHANNEL [PASSWORD]");
		return;
	}
#endif
	if(strlen((const char *)argv[1]) > 32){
		DBG_INFO("Usage: wifi_ap SSID length can't exceed 32\n\r");
		return;
	}

#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	//TODO
#else
	dhcps_deinit();
#if LWIP_VERSION_MAJOR >= 2
	IP4_ADDR(ip_2_ip4(&ipaddr), GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	IP4_ADDR(ip_2_ip4(&netmask), NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(ip_2_ip4(&gw), GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	netif_set_addr(pnetif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask),ip_2_ip4(&gw));
#else
	IP4_ADDR(&ipaddr, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	netif_set_addr(pnetif, &ipaddr, &netmask,&gw);
#endif
#endif
#endif
#if 0
	//Check mode
	wext_get_mode(WLAN0_NAME, &mode);

	switch(mode) {
		case IW_MODE_MASTER:	//In AP mode
			wifi_off();
			wifi_on(1);
			break;
		case IW_MODE_INFRA:		//In STA mode
			if(wext_get_ssid(WLAN0_NAME, ssid) > 0)
				cmd_wifi_disconnect(0, NULL);
	}
#else
	wifi_off();

	rtw_mdelay_os(20);
	
	if (wifi_on(RTW_MODE_AP) < 0){
		DBG_INFO("ERROR: Wifi on failed!");
		return;
	}
#endif

	DBG_INFO("Starting AP ...");
	channel = atoi((const char *)argv[2]);
	DBG_INFO("Set Channel is %d\n", channel);
#if NOT_SUPPORT_5G
	if(channel > 13){
		DBG_INFO("Channel is from 1 to 13. Set channel 1 as default!\n");
		channel = 1;
	}
#endif
#if defined(CONFIG_ENABLE_WPS_AP) && CONFIG_ENABLE_WPS_AP
	wpas_wps_dev_config(pnetif->hwaddr, 1);
#endif

	if(argc == 4) {
		if(wifi_start_ap(argv[1],
							 RTW_SECURITY_WPA2_AES_PSK,
							 argv[3],
							 strlen((const char *)argv[1]),
							 strlen((const char *)argv[3]),
							 channel
							 ) != RTW_SUCCESS) {
			DBG_INFO("ERROR: Operation failed!");
			return;
		}
	}
#if CONFIG_IEEE80211W
	else if(argc == 5) {
		
		rtw_security_t alg = RTW_SECURITY_WPA2_AES_CMAC;
		u8 mfp = atoi((const char *)argv[4]);
		
		if (mfp == 0 || mfp > 2)  {// not support
			alg = RTW_SECURITY_WPA2_AES_PSK;
			mfp = 0;
		}
		wifi_set_mfp_support(mfp);
		if (wifi_start_ap(argv[1],
						 alg,
						 argv[3],
						 strlen((const char *)argv[1]),
						 strlen((const char *)argv[3]),
						 channel
						 ) != RTW_SUCCESS) {
			DBG_INFO("ERROR: Operation failed!");
			return;
		}
	}
#endif
	else {
		if(wifi_start_ap(argv[1],
							 RTW_SECURITY_OPEN,
							 NULL,
							 strlen((const char *)argv[1]),
							 0,
							 channel
							 ) != RTW_SUCCESS) {
			DBG_INFO("ERROR: Operation failed!");
			return;
		}
	}

	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)argv[1]) == 0) {
				DBG_INFO("%s started\n", argv[1]);
				break;
			}
		}

		if(timeout == 0) {
			DBG_INFO("ERROR: Start AP timeout!");
			break;
		}

		rtw_mdelay_os(1000);

		timeout --;
	}
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	//TODO
#else
	//LwIP_UseStaticIP(pnetif);
	dhcps_init(pnetif);
#endif
#endif
}

static void cmd_wifi_connect(int argc, char **argv)
{
	int ret = RTW_ERROR;
	unsigned long tick1 = xTaskGetTickCount();
	unsigned long tick2, tick3;
	int mode;
	char 				*ssid;
	rtw_security_t	security_type;
	char 				*password;
	int 				ssid_len;
	int 				password_len;
	int 				key_id;
	void				*semaphore;

	if((argc != 2) && (argc != 3) && (argc != 4)) {
		DBG_INFO("Usage: wifi_connect SSID [WPA PASSWORD / (5 or 13) ASCII WEP KEY] [WEP KEY ID 0/1/2/3]");
		return;
	}
	
	//Check if in AP mode
	wext_get_mode(WLAN0_NAME, &mode);

	if(mode == IW_MODE_MASTER) {
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
		//TODO
#else
                dhcps_deinit();
#endif
#endif
		wifi_off();
		vTaskDelay(20);
		if (wifi_on(RTW_MODE_STA) < 0){
			DBG_INFO("ERROR: Wifi on failed!");
			return;
		}
	}

	ssid = argv[1];
	if(argc == 2){
		security_type = RTW_SECURITY_OPEN;
		password = NULL;
		ssid_len = strlen((const char *)argv[1]);
		password_len = 0;
		key_id = 0;
		semaphore = NULL;
	}else if(argc ==3){
		security_type = RTW_SECURITY_WPA2_AES_PSK;
		password = argv[2];
		ssid_len = strlen((const char *)argv[1]);
		password_len = strlen((const char *)argv[2]);
		key_id = 0;
		semaphore = NULL;
	}else{
		security_type = RTW_SECURITY_WEP_PSK;
		password = argv[2];
		ssid_len = strlen((const char *)argv[1]);
		password_len = strlen((const char *)argv[2]);
		key_id = atoi(argv[3]);
		if(( password_len != 5) && (password_len != 13)&&( password_len != 10) && (password_len != 26)) {
			DBG_INFO("Wrong WEP key length. Must be 5 or 13 ASCII characters or 10 or 26 hex.");
			return;
		}
		if((key_id < 0) || (key_id > 3)) {
			DBG_INFO("Wrong WEP key id. Must be one of 0,1,2, or 3.");
			return;
		}
		semaphore = NULL;
	}

	ret = wifi_connect(ssid, 
					security_type, 
					password, 
					ssid_len, 
					password_len, 
					key_id,
					semaphore);

	if(ret != RTW_SUCCESS) {
		DBG_INFO("ERROR: Operation failed!");
		return;
	} else {
		tick2 = xTaskGetTickCount();
		DBG_INFO("Connected after %dms.\n", (tick2-tick1));
	
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
		//TODO
#else
		/* Start DHCPClient */
		LwIP_DHCP(0, DHCP_START);
#endif
#endif
	}
	tick3 = xTaskGetTickCount();
	DBG_INFO("Got IP after %dms.\n", (tick3-tick1));
}

static void cmd_wifi_connect_bssid(int argc, char **argv)
{
	int ret = RTW_ERROR;
	unsigned long tick1 = xTaskGetTickCount();
	unsigned long tick2, tick3;
	int mode;
	unsigned char 	bssid[ETH_ALEN];
	char 			*ssid = NULL;
	rtw_security_t		security_type;
	char 			*password;
	int 				bssid_len;
	int 				ssid_len = 0;
	int 				password_len;
	int 				key_id;
	void				*semaphore;
	u32 				mac[ETH_ALEN];
	u32				i;
	u32				index = 0;
	
	if((argc != 3) && (argc != 4) && (argc != 5) && (argc != 6)) {
		DBG_INFO("Usage: wifi_connect_bssid 0/1 [SSID] BSSID / xx:xx:xx:xx:xx:xx [WPA PASSWORD / (5 or 13) ASCII WEP KEY] [WEP KEY ID 0/1/2/3]");
		return;
	}
	
	//Check if in AP mode
	wext_get_mode(WLAN0_NAME, &mode);

	if(mode == IW_MODE_MASTER) {
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
		//TODO
#else

        dhcps_deinit();
#endif
#endif
		wifi_off();
		vTaskDelay(20);
		if (wifi_on(RTW_MODE_STA) < 0){
			DBG_INFO("ERROR: Wifi on failed!");
			return;
		}
	}
	//check ssid
	if(!(memcmp(argv[1], "0", 1))){
		index = 1;
		ssid_len = strlen((const char *)argv[2]);
		if((ssid_len <= 0) || (ssid_len > 32)) {
			DBG_INFO("Wrong ssid. Length must be less than 32.");
			return;
		}
		ssid = argv[2];	
	}
	sscanf(argv[2 + index], MAC_FMT, mac, mac + 1, mac + 2, mac + 3, mac + 4, mac + 5);
	for(i=0; i<ETH_ALEN; i++)
		bssid[i] = (u8)mac[i]&0xFF;
	
	if(argc == 3 + index){
		security_type = RTW_SECURITY_OPEN;
		password = NULL;
		bssid_len = ETH_ALEN;
		password_len = 0;
		key_id = 0;
		semaphore = NULL;
	}else if(argc ==4 + index){
		security_type = RTW_SECURITY_WPA2_AES_PSK;
		password = argv[3 + index];
		bssid_len = ETH_ALEN;
		password_len = strlen((const char *)argv[3 + index]);
		key_id = 0;
		semaphore = NULL;
	}else{
		security_type = RTW_SECURITY_WEP_PSK;
		password = argv[3 + index];
		bssid_len = ETH_ALEN;
		password_len = strlen((const char *)argv[3 + index]);
		key_id = atoi(argv[4 + index]);
		if(( password_len != 5) && (password_len != 13)&&( password_len != 10) && (password_len != 26)) {
			DBG_INFO("Wrong WEP key length. Must be 5 or 13 ASCII characters or 10 or 26 hex.");
			return;
		}
		if((key_id < 0) || (key_id > 3)) {
			DBG_INFO("Wrong WEP key id. Must be one of 0,1,2, or 3.");
			return;
		}
		semaphore = NULL;
	}

	ret = wifi_connect_bssid(bssid, 
					ssid,
					security_type, 
					password, 
					bssid_len, 
					ssid_len, 					
					password_len, 
					key_id,
					semaphore);

	if(ret != RTW_SUCCESS) {
		DBG_INFO("ERROR: Operation failed!");
		return;
	} else {
		tick2 = xTaskGetTickCount();
		DBG_INFO("Connected after %dms.\n", (tick2-tick1));
		
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
		//TODO
#else

		/* Start DHCPClient */
		LwIP_DHCP(0, DHCP_START);
#endif
#endif
	}
	tick3 = xTaskGetTickCount();
	DBG_INFO("Got IP after %dms.\n", (tick3-tick1));
}

static void cmd_wifi_disconnect(int argc, char **argv)
{
	int timeout = 20;
	char essid[33];

	DBG_INFO("Deassociating AP ...");

	if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
		DBG_INFO("WIFI disconnected");
		return;
	}

	if(wifi_disconnect() < 0) {
		DBG_INFO("ERROR: Operation failed!");
		return;
	}

	while(1) {
		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
			DBG_INFO("WIFI disconnected");
			break;
		}

		if(timeout == 0) {
			DBG_INFO("ERROR: Deassoc timeout!");
			break;
		}

		rtw_mdelay_os(1000);

		timeout --;
	}
}
extern void dump_drv_version();
static void cmd_wifi_info(int argc, char **argv)
{
	int i = 0;
#if CONFIG_LWIP_LAYER
#if !defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	u8 *mac = LwIP_GetMAC(&xnetif[0]);
	u8 *ip = LwIP_GetIP(&xnetif[0]);
	u8 *gw = LwIP_GetGW(&xnetif[0]);
#endif
#endif
	u8 *ifname[2] = {WLAN0_NAME,WLAN1_NAME};
#ifdef CONFIG_MEM_MONITOR
	extern int min_free_heap_size;
#endif
	dump_drv_version();
	rtw_wifi_setting_t setting;
	for(i=0;i<NET_IF_NUM;i++){
		if(rltk_wlan_running(i)){
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
			//TODO
#else
			mac = LwIP_GetMAC(&xnetif[i]);
			ip = LwIP_GetIP(&xnetif[i]);
			gw = LwIP_GetGW(&xnetif[i]);
#endif			
#endif
			DBG_INFO("WIFI %s Status: Running",  ifname[i]);
			DBG_INFO("==============================");
			
			rltk_wlan_statistic(i);
			
			wifi_get_setting((const char*)ifname[i],&setting);
			wifi_show_setting((const char*)ifname[i],&setting);
#if CONFIG_LWIP_LAYER
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
			//TODO
#else
			DBG_INFO("Interface (%s)", ifname[i]);
			DBG_INFO("==============================");
			DBG_INFO("\tMAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
			DBG_INFO("\tIP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			DBG_INFO("\tGW  => %d.%d.%d.%d\n\r", gw[0], gw[1], gw[2], gw[3]);
#endif
#endif
			if(setting.mode == RTW_MODE_AP || i == 1)
			{
				int client_number;
				struct {
					int    count;
					rtw_mac_t mac_list[AP_STA_NUM];
				} client_info;

				client_info.count = AP_STA_NUM;
				wifi_get_associated_client_list(&client_info, sizeof(client_info));

				DBG_INFO("Associated Client List:");
				DBG_INFO("==============================");

				if(client_info.count == 0)
					DBG_INFO("Client Num: 0\n\r");
				else
				{
				      	DBG_INFO("Client Num: %d", client_info.count);
				      	for( client_number=0; client_number < client_info.count; client_number++ )
				      	{
						DBG_INFO("Client [%d]:", client_number);
						DBG_INFO("\tMAC => "MAC_FMT"",
										MAC_ARG(client_info.mac_list[client_number].octet));
					}
					DBG_INFO("");
				}
			}

			{
				int error = wifi_get_last_error();
				DBG_INFO("Last Link Error");
				DBG_INFO("==============================");
				switch(error)
				{
					case RTW_NO_ERROR:
						DBG_INFO("\tNo Error");
						break;
					case RTW_NONE_NETWORK:
						DBG_INFO("\tTarget AP Not Found");
						break;
					case RTW_CONNECT_FAIL:
						DBG_INFO("\tAssociation Failed");
						break;
					case RTW_WRONG_PASSWORD:
						DBG_INFO("\tWrong Password");
						break;
					case RTW_DHCP_FAIL:
						DBG_INFO("\tDHCP Failed");
						break;
					default:
						DBG_INFO("\tUnknown Error(%d)", error);
				}
				DBG_INFO("");
			}
		}		
	}

#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1)
	{
		signed char pcWriteBuffer[1024];
		vTaskList((char*)pcWriteBuffer);
		DBG_INFO("Task List: \n%s", pcWriteBuffer);
	}
#endif	
#ifdef CONFIG_MEM_MONITOR
	DBG_INFO("Memory Usage");
	DBG_INFO("==============================");
	DBG_INFO("Min Free Heap Size:  %d", min_free_heap_size);
	DBG_INFO("Cur Free Heap Size:  %d\n", xPortGetFreeHeapSize());
#endif
}

static void cmd_wifi_on(int argc, char **argv)
{
	if(wifi_on(RTW_MODE_STA)<0){
		DBG_INFO("ERROR: Wifi on failed!\n");
	}
}

static void cmd_wifi_off(int argc, char **argv)
{
#if CONFIG_WEBSERVER
	stop_web_server();
#endif
#if CONFIG_ENABLE_P2P
	cmd_wifi_p2p_stop(0, NULL);
#else
	wifi_off();
#endif
}

static void print_scan_result( rtw_scan_result_t* record )
{
    printk( "%s\t ", ( record->bss_type == RTW_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra" );
    printk( MAC_FMT, MAC_ARG(record->BSSID.octet) );
    printk( " %d\t ", record->signal_strength );
    printk( " %d\t  ", record->channel );
    printk( " %d\t  ", record->wps_type );
    printk( "%s\t\t ", ( record->security == RTW_SECURITY_OPEN ) ? "Open" :
                                 ( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
                                 ( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP PSK" :
								 ( record->security == RTW_SECURITY_WPA_TKIP_8021X ) ? "WPA TKIP 8021X" :
                                 ( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES PSK" :
								 ( record->security == RTW_SECURITY_WPA_AES_8021X ) ? "WPA AES 8021X" :
                                 ( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES PSK" :
								 ( record->security == RTW_SECURITY_WPA2_AES_8021X ) ? "WPA2 AES 8021X" :
                                 ( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP PSK" :
								 ( record->security == RTW_SECURITY_WPA2_TKIP_8021X ) ? "WPA2 TKIP 8021X" :
                                 ( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed PSK" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED_PSK ) ? "WPA/WPA2 PSK" :
								 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED_8021X ) ? "WPA/WPA2 8021X" :
#if CONFIG_SAE_SUPPORT
								 ( record->security == RTW_SECURITY_WPA3_AES_PSK) ? "WP3-SAE AES" :
#endif

                                 "Unknown" );

    printk( " %s ", record->SSID.val );
}

static rtw_result_t app_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
	static int ApNum = 0;

	if (malloced_scan_result->scan_complete != RTW_TRUE) {
		rtw_scan_result_t* record = &malloced_scan_result->ap_details;
		record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

		printk( "\r\n%d\t ", ++ApNum );

		print_scan_result(record);
	} else{
		ApNum = 0;
	}
	return RTW_SUCCESS;
}
#if SCAN_WITH_SSID
static void cmd_wifi_scan_with_multiple_ssid(int argc,char **argv)
{
	int num_ssid,scan_buflen,i;
	scan_ssid Ssid[3];
	if(argc < 2||argc>4){
		DBG_INFO("For Scan all channel Usage: wifi_scan_with_multissid ssid... (num<=3!)");
		return ;
	}
	for(i = 1;i<argc;i++){
		Ssid[i-1].ssidlength = strlen((const char *)argv[i]);
		memcpy(&(Ssid[i-1].ssid), argv[i], Ssid[i-1].ssidlength);
		DBG_INFO("%s: Ssid[%d].Ssid = %s, Ssid[%d].SsidLength = %d",__FUNCTION__,i-1,Ssid[i-1].ssid,i-1,Ssid[i-1].ssidlength);
	}
	scan_buflen = 200;
	num_ssid = argc -1;
	if(wifi_scan_networks_with_multissid(NULL,NULL, scan_buflen, Ssid ,num_ssid) != RTW_SUCCESS){
		DBG_INFO("ERROR: wifi scan failed");
	}
	return ;
}

static void cmd_wifi_scan_with_ssid(int argc, char **argv)
{

	u8 *channel_list = NULL;
	u8 *pscan_config = NULL;
	
	char *ssid = NULL;
	int ssid_len = 0;
	//Fully scan
	int scan_buf_len = 500;	
	if(argc == 3 && argv[1] && argv[2]){
		ssid = argv[1];		
		ssid_len = strlen((const char *)argv[1]);
		if((ssid_len <= 0) || (ssid_len > 32)) {
			DBG_INFO("Wrong ssid. Length must be less than 32.");
			goto exit;
		}
		scan_buf_len = atoi(argv[2]);
		if(scan_buf_len < 36){
			DBG_INFO("BUFFER_LENGTH too short\n\r");
			goto exit;
		}
	}else if(argc > 3){
		int i = 0;
		int num_channel = atoi(argv[2]);
		ssid = argv[1];		
		ssid_len = strlen((const char *)argv[1]);
		if((ssid_len <= 0) || (ssid_len > 32)) {
			DBG_INFO("Wrong ssid. Length must be less than 32.");
			goto exit;
		}		
		channel_list = (u8*)pvPortMalloc(num_channel);
		if(!channel_list){
			DBG_INFO("ERROR: Can't malloc memory for channel list");
			goto exit;
		}
		pscan_config = (u8*)pvPortMalloc(num_channel);
		if(!pscan_config){
			DBG_INFO("ERROR: Can't malloc memory for pscan_config");
			goto exit;
		}		
		//parse command channel list
		for(i = 3; i <= argc -1 ; i++){
			*(channel_list + i - 3) = (u8)atoi(argv[i]);	
			*(pscan_config + i - 3) = PSCAN_ENABLE;			
		}
		if(wifi_set_pscan_chan(channel_list, pscan_config, num_channel) < 0){
		    DBG_INFO("ERROR: wifi set partial scan channel fail");
		    goto exit;
		}
	}else{
	    DBG_INFO("For Scan all channel Usage: wifi_scan_with_ssid ssid BUFFER_LENGTH");
	    DBG_INFO("For Scan partial channel Usage: wifi_scan_with_ssid ssid num_channels channel_num1 ...");
	    return;
	}
	
	if(wifi_scan_networks_with_ssid(NULL, NULL, scan_buf_len, ssid, ssid_len) != RTW_SUCCESS){
		DBG_INFO("ERROR: wifi scan failed");
		goto exit;
	}

exit:
	if(argc > 2 && channel_list)
		vPortFree(channel_list);
	if(argc > 2 && pscan_config)
		vPortFree(pscan_config);
	
}
#endif
static void cmd_wifi_scan(int argc, char **argv)
{

	u8 *channel_list = NULL;
	u8 *pscan_config = NULL;

	if(argc > 2){
		int i = 0;
		int num_channel = atoi(argv[1]);

		channel_list = (u8*)pvPortMalloc(num_channel);
		if(!channel_list){
			DBG_INFO("ERROR: Can't malloc memory for channel list");
			goto exit;
		}
		pscan_config = (u8*)pvPortMalloc(num_channel);
		if(!pscan_config){
			DBG_INFO("ERROR: Can't malloc memory for pscan_config");
			goto exit;
		}
		//parse command channel list
		for(i = 2; i <= argc -1 ; i++){
			*(channel_list + i - 2) = (u8)atoi(argv[i]);		    
			*(pscan_config + i - 2) = PSCAN_ENABLE;
		}
		
		if(wifi_set_pscan_chan(channel_list, pscan_config, num_channel) < 0){
		    DBG_INFO("ERROR: wifi set partial scan channel fail");
		    goto exit;
		}
		
	}

	if(wifi_scan_networks(app_scan_result_handler, NULL ) != RTW_SUCCESS){
		DBG_INFO("ERROR: wifi scan failed");
		goto exit;
	}
exit:
	if(argc > 2 && channel_list)
		vPortFree(channel_list);
	if(argc > 2 && pscan_config)
		vPortFree(pscan_config);

}

static void cmd_wifi_reorder_scan(int argc, char **argv)
{

	u8 *channel_list = NULL;
	int i = 0;
	int num_channel = 13;
	char channel_reorder[]= {1,3,5,7,9,2,4,6,8,10,11,12,13};//set channel order
	channel_list = (u8*)pvPortMalloc(num_channel);
	if(!channel_list){
		DBG_INFO("ERROR: Can't malloc memory for channel_list");
		goto exit;
	}
	for(i=0;i<num_channel;i++)
		*(channel_list+i) = channel_reorder[i];
	if(wifi_set_scan_reorderchan(channel_list, num_channel) < 0){
		DBG_INFO("ERROR: wifi set reoder scan channel fail");
		goto exit;
	}
	if(wifi_scan_networks(app_scan_result_handler, NULL ) != RTW_SUCCESS){
		DBG_INFO("ERROR: wifi scan failed");
		goto exit;
	}
exit:
	if(channel_list)
		vPortFree(channel_list);
}
#if CONFIG_WEBSERVER
static void cmd_wifi_start_webserver(int argc, char **argv)
{
	start_web_server();
}
#endif

static void cmd_wifi_iwpriv(int argc, char **argv)
{
	if(argc == 2 && argv[1]) {
		wext_private_command(WLAN0_NAME, argv[1], 1);
	}
	else {
		DBG_INFO("Usage: iwpriv COMMAND PARAMETERS");
	}
}
#endif	//#if CONFIG_WLAN

static void cmd_ping(int argc, char **argv)
{
#if !defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	if(argc == 2) {
		do_ping_call(argv[1], 0, 5);	//Not loop, count=5
	}
	else if(argc == 3) {
		if(strcmp(argv[2], "loop") == 0)
			do_ping_call(argv[1], 1, 0);	//loop, no count
		else
			do_ping_call(argv[1], 0, atoi(argv[2]));	//Not loop, with count
	}
	else {
		DBG_INFO("Usage: ping IP [COUNT/loop]");
	}
#else
	DBG_INFO("unsupported cmd for customer platform!!!!!!!!");
#endif	
}
#if ( configGENERATE_RUN_TIME_STATS == 1 )
static char cBuffer[ 512 ];
static void cmd_cpustat(int argc, char **argv)
{
	vTaskGetRunTimeStats( ( char * ) cBuffer );
	DBG_INFO("%s",cBuffer);
}
#endif
#if defined(CONFIG_RTL8195A) || defined(CONFIG_RTL8711B) || defined(CONFIG_RTL8721D)
static void cmd_dump_reg(int argc, char **argv)
{
	CmdDumpWord(argc-1, (u8**)(argv+1));
}
static void cmd_edit_reg(int argc, char **argv)
{
	CmdWriteWord(argc-1, (u8**)(argv+1));
}
#endif
static void cmd_exit(int argc, char **argv)
{
	DBG_INFO("Leave INTERACTIVE MODE");
	vTaskDelete(NULL);
}

static void cmd_debug(int argc, char **argv)
{
	if(strcmp(argv[1], "ready_trx") == 0) {
		DBG_INFO("%d", wifi_is_ready_to_transceive((rtw_interface_t)rtw_atoi((u8*)argv[2])));
	} else if(strcmp(argv[1], "is_up") == 0) {
		DBG_INFO("%d", wifi_is_up((rtw_interface_t)rtw_atoi((u8*)argv[2])));
	} else if(strcmp(argv[1], "set_mac") == 0) {
		DBG_INFO("%d", wifi_set_mac_address(argv[2]));
	} else if(strcmp(argv[1], "get_mac") == 0) {
		u8 mac[18] = {0};
		wifi_get_mac_address((char*)mac);
		DBG_INFO("%s", mac);
	} else if(strcmp(argv[1], "set_bt_mac") == 0){
		DBG_INFO("%d", bt_set_mac_address(argv[2]));
	}else if(strcmp(argv[1], "get_bt_mac") == 0){
		u8 mac[18] = {0};
		bt_get_mac_address((char*)mac);
		DBG_INFO("%s", mac);
	}else if(strcmp(argv[1], "ps_on") == 0) {
		DBG_INFO("%d", wifi_enable_powersave());
	} else if(strcmp(argv[1], "ps_off") == 0) {
		DBG_INFO("%d", wifi_disable_powersave());
#if 0 //TODO		
	} else if(strcmp(argv[1], "get_txpwr") == 0) {
		int idx;
		wifi_get_txpower(&idx);
		DBG_INFO("%d", idx);
	} else if(strcmp(argv[1], "set_txpwr") == 0) {
		DBG_INFO("%d", wifi_set_txpower(rtw_atoi((u8*)argv[2])));
#endif		
	} else if(strcmp(argv[1], "get_clientlist") == 0) {
		int client_number;
		struct {
			int    count;
			rtw_mac_t mac_list[3];
		} client_info;

		client_info.count = 3;

		DBG_INFO("%d\r\n", wifi_get_associated_client_list(&client_info, sizeof(client_info)));

        if( client_info.count == 0 )
        {
            DBG_INFO("Clients connected 0..\r\n");
        }
        else
        {
            DBG_INFO("Clients connected %d..\r\n", client_info.count);
            for( client_number=0; client_number < client_info.count; client_number++ )
            {
				DBG_INFO("------------------------------------\r\n");
				DBG_INFO("| %d | "MAC_FMT" |\r\n",
									client_number,
									MAC_ARG(client_info.mac_list[client_number].octet)
							);
            }
            DBG_INFO("------------------------------------\r\n");
        }
	} else if(strcmp(argv[1], "get_apinfo") == 0) {
		rtw_bss_info_t ap_info;
		rtw_security_t sec;
		if(wifi_get_ap_info(&ap_info, &sec) == RTW_SUCCESS) {
			DBG_INFO("SSID  : %s\r\n", (char*)ap_info.SSID);
			DBG_INFO("BSSID : "MAC_FMT"\r\n", MAC_ARG(ap_info.BSSID.octet));
			DBG_INFO("RSSI  : %d\r\n", ap_info.RSSI);
			//DBG_INFO("SNR   : %d\r\n", ap_info.SNR);
			DBG_INFO("Beacon period : %d\r\n", ap_info.beacon_period);
			DBG_INFO("Security : %s\r\n", ( sec == RTW_SECURITY_OPEN )           ? "Open" :
													( sec == RTW_SECURITY_WEP_PSK )        ? "WEP" :
													( sec == RTW_SECURITY_WPA_TKIP_PSK )   ? "WPA TKIP" :
													( sec == RTW_SECURITY_WPA_AES_PSK )    ? "WPA AES" :
													( sec == RTW_SECURITY_WPA2_AES_PSK )   ? "WPA2 AES" :
													( sec == RTW_SECURITY_WPA2_TKIP_PSK )  ? "WPA2 TKIP" :
													( sec == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
													"Unknown");
		}
	} 
#if CONFIG_MULTICAST
	else if(strcmp(argv[1], "reg_mc") == 0) {
		rtw_mac_t mac;
		sscanf(argv[2], MAC_FMT, (int*)(mac.octet+0), (int*)(mac.octet+1), (int*)(mac.octet+2), (int*)(mac.octet+3), (int*)(mac.octet+4), (int*)(mac.octet+5));
		DBG_INFO("%d", wifi_register_multicast_address(&mac));
	} else if(strcmp(argv[1], "unreg_mc") == 0) {
		rtw_mac_t mac;
		sscanf(argv[2], MAC_FMT, (int*)(mac.octet+0), (int*)(mac.octet+1), (int*)(mac.octet+2), (int*)(mac.octet+3), (int*)(mac.octet+4), (int*)(mac.octet+5));
		DBG_INFO("%d", wifi_unregister_multicast_address(&mac));
	} 
#endif
	else if(strcmp(argv[1], "get_rssi") == 0) {
		int rssi = 0;
		wifi_get_rssi(&rssi);
		DBG_INFO("wifi_get_rssi: rssi = %d", rssi);
	}else if(strcmp(argv[1], "dbg") == 0) {
		char buf[32] = {0};
		char * copy = buf;
        int i = 0;
       	int len = 0;
		for(i=1;i<argc;i++){
			strcpy(&buf[len], argv[i]);
			len = strlen(copy);
			buf[len++] = ' ';
			buf[len] = '\0';
		}	
		wext_private_command(WLAN0_NAME, copy, 1);
#if CONFIG_IEEE80211W
	} else if(strcmp(argv[1], "11w_sa") == 0) {
		rltk_wlan_tx_sa_query(atoi((const char *)argv[2]));
	} else if(strcmp(argv[1], "11w_deauth") == 0) {
		rltk_wlan_tx_deauth(atoi((const char *)argv[2]), atoi((const char *)argv[3]));
	} else if(strcmp(argv[1], "11w_auth") == 0) {
		rltk_wlan_tx_auth();
#endif
	} else if(strcmp(argv[1], "log") == 0) {
		extern void rtlk_wlan_set_wifi_log(u8 enable);
		extern void rtlk_wlan_set_btco_log(u8 enable);		
		extern void rtlk_wlan_set_fa_log(u8 enable);	
		if(strcmp(argv[2], "wifi") == 0){//open close almost all wifi log
			if(strcmp(argv[3], "on") == 0)
				rtlk_wlan_set_wifi_log(1);
			if(strcmp(argv[3], "off") == 0)
				rtlk_wlan_set_wifi_log(0);
		}else if(strcmp(argv[2], "btco") == 0){//open close btco log
			if(strcmp(argv[3], "on") == 0)
				rtlk_wlan_set_btco_log(1);
			if(strcmp(argv[3], "off") == 0)
				rtlk_wlan_set_btco_log(0);
		}else if(strcmp(argv[2], "fa") == 0){//open fa log
			if(strcmp(argv[3], "on") == 0)
				rtlk_wlan_set_fa_log(1);
			if(strcmp(argv[3], "off") == 0)
				rtlk_wlan_set_fa_log(0);
		}
	} else {
		DBG_INFO("Unknown CMD\r\n");
	}
}
#if CONFIG_SUSPEND
extern int rtw_suspend(rtw_mode_t mode);
extern int rtw_resume(rtw_mode_t mode);
static void cmd_suspend(int argc, char **argv)
{
	rtw_suspend(NULL);
}
static void cmd_resume(int argc, char **argv)
{
	rtw_resume(NULL);
}
#endif

#if 0/*it does't work normally and is for specific needs*/ //CONFIG_CUSTOMER_EE_REQUEST
static void cmd_stop_ap(int argc, char **argv)
{
	DBG_INFO("stop AP ...\n");
	wifi_stop_ap();
}
static void cmd_resume_ap(int argc, char **argv)
{
	int timeout = 20;

	int channel;

	if((argc != 3) && (argc != 4)) {
		DBG_INFO("Usage: wifi_ap SSID CHANNEL [PASSWORD]");
		return;
	}

	if(strlen((const char *)argv[1]) > 32){
		DBG_INFO("Usage: wifi_ap SSID length can't exceed 32\n\r");
		return;
	}


	DBG_INFO("Resume AP ...\n");
	channel = atoi((const char *)argv[2]);
	DBG_INFO("Set Channel is %d\n", channel);
#if NOT_SUPPORT_5G
	if(channel > 13){
		DBG_INFO("Channel is from 1 to 13. Set channel 1 as default!\n");
		channel = 1;
	}
#endif

	if(argc == 4) {
		if(wifi_start_ap(argv[1],
							 RTW_SECURITY_WPA2_AES_PSK,
							 argv[3],
							 strlen((const char *)argv[1]),
							 strlen((const char *)argv[3]),
							 channel
							 ) != RTW_SUCCESS) {
			DBG_INFO("ERROR: Operation failed!");
			return;
		}
	}
	else {
		if(wifi_start_ap(argv[1],
							 RTW_SECURITY_OPEN,
							 NULL,
							 strlen((const char *)argv[1]),
							 0,
							 channel
							 ) != RTW_SUCCESS) {
			DBG_INFO("ERROR: Operation failed!");
			return;
		}
	}

	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)argv[1]) == 0) {
				DBG_INFO("%s Restarted\n", argv[1]);
				break;
			}
		}

		if(timeout == 0) {
			DBG_INFO("ERROR: Resume AP timeout!");
			break;
		}

		rtw_mdelay_os(1000);

		timeout --;
	}
}
#endif
#if CONFIG_ACS
static void cmd_get_auto_chl(int argc, char **argv)
{
	unsigned char channel_set[7];
	int auto_chl = 0;
	
    channel_set[0] = 36;
    channel_set[1] = 52;
    channel_set[2] = 40;
	channel_set[3] = 44;
	channel_set[4] = 56;
	channel_set[5] = 48;
	channel_set[6] = 153;
	//channel_set[7] = 56;

	auto_chl = wext_get_auto_chl("wlan0", channel_set, sizeof(channel_set)/sizeof(channel_set[0]));
	DBG_INFO("auto_chl = %d!",auto_chl);
}
#endif
#ifdef CONFIG_CMW500_TEST
static void cmd_testcmw500_enable(int argc, char **argv)
{
	int i = 0;
	u8 fix_rate = 0;
	u8 txpower = 0;
	u8 pwrtrack_en = 2;//default open, 0:disable 1:enable
	DBG_INFO("start test for CMW500!\r\n");
	if((1 != argc)&&(3 != argc)&&(5 != argc)&&(7 != argc)) {
		DBG_INFO("Usage: open_test_cmw500 -r fixrate -t txpower -p pwrtrack_en[0/1]\r\n");
		return;
	}
	for(i=1;i<argc;i++){
		if(strcmp(argv[i], "-r") == 0){
			fix_rate = atoi((const char *)argv[i+1]);
		}
		if(strcmp(argv[i], "-t") == 0){
			txpower = atoi((const char *)argv[i+1]);
		}
		if(strcmp(argv[i], "-p") == 0){
			pwrtrack_en = atoi((const char *)argv[i+1]);
		}
	}
	wext_enable_testcmw500(WLAN0_NAME, fix_rate, txpower, pwrtrack_en);
}

static void cmd_testcmw500_disable(int argc, char **argv)
{
	char* fix_rate = NULL;
	DBG_INFO("stop test for CMW500!\r\n");
	if(argc != 1) {
		DBG_INFO("Usage: error");
		return;
	}
	wext_disable_testcmw500(WLAN0_NAME);//modified
}
#endif

#if CONFIG_SET_PRIORITY
static void cmd_set_priority(int argc, char **argv)
{
	int priority;
	if(2 != argc) {
		DBG_INFO("Usage: set_pri num\r\n");
		return;
	}
	priority = atoi((const char *)argv[1]);
	DBG_INFO("set_pri num = %d \r\n",priority);
	rltk_wlan_set_priority(priority);	
}
#endif
#ifdef CONFIG_RAWDATA
int frame_handler(const unsigned char* frame_buf, unsigned int frame_len)
{
	DBG_INFO("%s----received raw data and print lenfgth is %d!\n",__FUNCTION__,frame_len);
	int i = 0;
	for(;i<frame_len;i++){
		printk("%0x ",*frame_buf);
		frame_buf++;
		if((i+1)%12 == 0)
			printk("\r\n");
	}
}
static void cmd_rawdata_enable(int argc, char **argv)
{
	DBG_INFO("Enable the receive raw data!");
	DBG_INFO("the callback function pointer is 0x%0x",frame_handler);
	wext_enable_rawdata_recv(WLAN0_NAME, (void*)frame_handler);
}
static void cmd_rawdata_disable(int argc, char **argv)
{
	DBG_INFO("Disable the receive raw data!");
	wext_disable_rawdata_recv(WLAN0_NAME);
}
char Frame_buf[] = {0x40, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0, 0x4c, 0x87, 0x13, 0x30, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,0x0,0x0,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd};
static void cmd_rawdata_sendData(int argc, char **argv)
{
	DBG_INFO("Send management frame and the length is %d!",sizeof(Frame_buf));
	wext_send_rawdata(WLAN0_NAME,Frame_buf, sizeof(Frame_buf));
}
#endif
#if CONFIG_CUSTOM_IE
// Example for custom ie
u8 test_1[] = {221, 2, 2, 2};
u8 test_2[] = {221, 2, 1, 1};
rtw_custom_ie_t buf[2] = {{test_1, PROBE_REQ},
		 {test_2, PROBE_RSP | BEACON}};
u8 buf_test2[] = {221, 2, 1, 3} ;
rtw_custom_ie_t buf_update = {buf_test2, PROBE_REQ};

// add ie list 
static void cmd_add_ie(int argc, char **argv)
{
	 wifi_add_custom_ie((void *)buf, 2);
}

//update current ie
static void cmd_update_ie(int argc, char **argv)
{
 wifi_update_custom_ie(&buf_update, 2);
}

// delete all ie
static void cmd_del_ie(int argc, char **argv)
{
 wifi_del_custom_ie();
}
#endif

typedef struct _cmd_entry {
	char *command;
	void (*function)(int, char **);
} cmd_entry;

static const cmd_entry cmd_table[] = {
#if CONFIG_WLAN
	{"wifi_connect", cmd_wifi_connect},
	{"wifi_connect_bssid", cmd_wifi_connect_bssid},
	{"wifi_disconnect", cmd_wifi_disconnect},
	{"wifi_info", cmd_wifi_info},
	{"wifi_on", cmd_wifi_on},
	{"wifi_off", cmd_wifi_off},
	{"wifi_ap", cmd_wifi_ap},
	{"wifi_scan", cmd_wifi_scan},
	{"wifi_reoder_scan", cmd_wifi_reorder_scan},
#if SCAN_WITH_SSID	
	{"wifi_scan_with_ssid", cmd_wifi_scan_with_ssid},
	{"wifi_scan_with_multissid", cmd_wifi_scan_with_multiple_ssid},
#endif
	{"iwpriv", cmd_wifi_iwpriv},
#if CONFIG_PROMISC
	{"wifi_promisc", cmd_promisc},
#endif
#if (CONFIG_INCLUDE_SIMPLE_CONFIG)	
	{"wifi_simple_config", cmd_simple_config},
#endif	
#if CONFIG_WPS
#if CONFIG_ENABLE_WPS
	{"wifi_wps", cmd_wps},
#endif	
#if defined(CONFIG_ENABLE_WPS_AP) && CONFIG_ENABLE_WPS_AP
	{"wifi_ap_wps", cmd_ap_wps},
#endif
#if CONFIG_ENABLE_P2P
	{"wifi_p2p_start", cmd_wifi_p2p_start},
	{"wifi_p2p_stop", cmd_wifi_p2p_stop},
	{"wifi_p2p_auto_go", cmd_wifi_p2p_auto_go_start},	
	{"p2p_listen", cmd_p2p_listen},
	{"p2p_find", cmd_p2p_find},	
	{"p2p_peers", cmd_p2p_peers},
	{"p2p_info", cmd_p2p_info},
	{"p2p_disconnect", cmd_p2p_disconnect},
	{"p2p_connect", cmd_p2p_connect},
#endif
#endif	
#if CONFIG_CONCURRENT_MODE
	{"wifi_sta_ap",cmd_wifi_sta_and_ap},
#endif	

	{"wifi_debug", cmd_debug},
#endif
#if CONFIG_LWIP_LAYER
//	{"app", cmd_app},
#if CONFIG_BSD_TCP
	{"tcp", cmd_tcp},
	{"udp", cmd_udp},
#endif
	{"ping", cmd_ping},
#endif
#if ( configGENERATE_RUN_TIME_STATS == 1 )
	{"cpu", cmd_cpustat},
#endif
#if CONFIG_SUSPEND
	{"wifi_suspend",cmd_suspend},
	{"wifi_resume",cmd_resume},
#endif
#if 0  /*it does't work normal and is for specific needs*/ //CONFIG_CUSTOMER_EE_REQUEST
	{"wifi_stop_ap",cmd_stop_ap},
	{"wifi_resume_ap",cmd_resume_ap},
#endif
#if CONFIG_ACS
	{"get_auto_chl", cmd_get_auto_chl},
#endif
#ifdef CONFIG_RAWDATA
	{"rawdata_enable",cmd_rawdata_enable},
	{"rawdata_disable",cmd_rawdata_disable},
	{"rawdata_send",cmd_rawdata_sendData},
#endif
#ifdef CONFIG_CMW500_TEST
	{"open_test_cmw500", cmd_testcmw500_enable},
	{"close_test_cmw500", cmd_testcmw500_disable},
#endif
#if CONFIG_SET_PRIORITY
	{"set_pri", cmd_set_priority},
#endif
#if CONFIG_CUSTOM_IE
	{"add_ie", cmd_add_ie},
	{"update_ie", cmd_update_ie},	
	{"del_ie", cmd_del_ie},
#endif
	{"exit", cmd_exit},
	{"help", cmd_help}
};

#if CONFIG_INTERACTIVE_EXT
/* must include here, ext_cmd_table in wifi_interactive_ext.h uses struct cmd_entry */
#include <wifi_interactive_ext.h>
#endif

static void cmd_help(int argc, char **argv)
{
	int i;

	DBG_INFO("COMMAND LIST:");
	DBG_INFO("==============================");

	for(i = 0; i < sizeof(cmd_table) / sizeof(cmd_table[0]); i ++)
		DBG_INFO("    %s", cmd_table[i].command);
#if CONFIG_INTERACTIVE_EXT
	for(i = 0; i < sizeof(ext_cmd_table) / sizeof(ext_cmd_table[0]); i ++)
		DBG_INFO("    %s", ext_cmd_table[i].command);
#endif
}

#define MAX_ARGC	10

static int parse_cmd(char *buf, char **argv)
{
	int argc = 0;

	memset(argv, 0, sizeof(argv)*MAX_ARGC);
	while((argc < MAX_ARGC) && (*buf != '\0')) {
		argv[argc] = buf;
		argc ++;
		buf ++;

		while((*buf != ' ') && (*buf != '\0'))
			buf ++;

		while(*buf == ' ') {
			*buf = '\0';
			buf ++;
		}
		// Don't replace space
		if(argc == 1){
			if(strcmp(argv[0], "iwpriv") == 0){
				if(*buf != '\0'){
					argv[1] = buf;
					argc ++;
				}
				break;
			}
		}
	}

	return argc;
}

char uart_buf[100];

void interactive_mode(void *param)
{
	int i, argc;
	char *argv[MAX_ARGC];
	char temp_uart_buf[100];
#if defined(CONFIG_PLATFOMR_CUSTOMER_RTOS)
	//TODO
	//Get content from uart
#else
	extern xSemaphoreHandle	uart_rx_interrupt_sema;

	DBG_INFO("Enter INTERACTIVE MODE\n\r");
	DBG_INFO("# ");

	while(1){
		while(rtw_down_sema((_sema *)&uart_rx_interrupt_sema) != _TRUE);

		rtw_memcpy(temp_uart_buf,uart_buf,100);
		if((argc = parse_cmd(temp_uart_buf, argv)) > 0) {
			int found = 0;

			for(i = 0; i < sizeof(cmd_table) / sizeof(cmd_table[0]); i ++) {
				if(strcmp((const char *)argv[0], (const char *)(cmd_table[i].command)) == 0) {
					cmd_table[i].function(argc, argv);
					found = 1;
					break;
				}
			}
#if CONFIG_INTERACTIVE_EXT
			if(!found) {
				for(i = 0; i < sizeof(ext_cmd_table) / sizeof(ext_cmd_table[0]); i ++) {
					if(strcmp(argv[0], ext_cmd_table[i].command) == 0) {
						ext_cmd_table[i].function(argc, argv);
						found = 1;
						break;
					}
				}
			}
#endif
			if(!found)
				DBG_INFO("unknown command '%s'", argv[0]);
			DBG_INFO("[MEM] After do cmd, available heap %d\n\r", xPortGetFreeHeapSize());
		}

		printk("\r\n\n# ");
		uart_buf[0] = '\0';
		temp_uart_buf[0] = '\0';
	}
#endif	
}

void start_interactive_mode(void)
{
#ifdef SERIAL_DEBUG_RX
	struct task_struct interactive_mode_task;
	interactive_mode_task.task = NULL;
	//if(xTaskCreate(interactive_mode, (char const *)"interactive_mode", STACKSIZE, NULL, tskIDLE_PRIORITY + 4, NULL) != pdPASS)
	if(rtw_create_task(&interactive_mode_task, (char const *)"interactive_mode", STACKSIZE, tskIDLE_PRIORITY + 5, interactive_mode, NULL) != pdPASS)
		DBG_INFO("%s xTaskCreate failed", __FUNCTION__);
#else
	DBG_INFO("ERROR: No SERIAL_DEBUG_RX to support interactive mode!");
#endif
}
