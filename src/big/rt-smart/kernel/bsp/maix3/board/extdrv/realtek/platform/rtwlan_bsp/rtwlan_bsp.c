#include <stdlib.h>
#include "rtthread.h"
#include "rtdevice.h"
#include "drivers/sdio.h"
#include "card.h"
#include "wifi/wifi_conf.h"
#include "net_stack_intf.h"
#include "customer_rtos_service.h"

static rt_int32_t realtek_probe(struct rt_mmcsd_card* card);

struct sdio_func* wifi_sdio_func;
struct rt_sdio_function* rtt_sdio_func;
static struct rt_wlan_device wlan_sta, wlan_ap;
static uint8_t sta_disconnect_wait = 0;

void Set_WLAN_Power_On(void)
{
}

void Set_WLAN_Power_Off(void)
{
}

static rt_int32_t realtek_remove(struct rt_mmcsd_card* card)
{
}

static struct rt_sdio_device_id realtek_id[] = {
    { 1, 0x024c, 0xf179 },
};

static struct rt_sdio_driver realtek_drv = {
    "realtek-wifi",
    realtek_probe,
    realtek_remove,
    realtek_id,
};

int realtek_init(void)
{
#if REALTEK_SDIO_DEV == 0
    kd_sdhci0_reset(0);
    rt_thread_mdelay(20);
    kd_sdhci0_reset(1);
    rt_thread_mdelay(50);
#endif
    sdio_register_driver(&realtek_drv);
    kd_sdhci_change(REALTEK_SDIO_DEV);

    return 0;
}
INIT_COMPONENT_EXPORT(realtek_init);

void wlan_event_indication(rtw_event_indicate_t event, char* buf, int buf_len)
{
    if (event == WIFI_EVENT_FOURWAY_HANDSHAKE_DONE) {
        rt_wlan_dev_indicate_event_handle(&wlan_sta, RT_WLAN_DEV_EVT_CONNECT, NULL);
    } else if (event == WIFI_EVENT_DISCONNECT) {
        if (sta_disconnect_wait) {
            rt_wlan_dev_indicate_event_handle(&wlan_sta, RT_WLAN_DEV_EVT_DISCONNECT, NULL);
            sta_disconnect_wait = 0;
        }
    } else if (event == WIFI_EVENT_STA_ASSOC || event == WIFI_EVENT_STA_DISASSOC) {
        struct rt_wlan_buff buff;
        struct rt_wlan_info wlan_info;
        memset(&wlan_info, 0, sizeof(wlan_info));
        buff.data = &wlan_info;
        buff.len = sizeof(struct rt_wlan_info);
        if (event == WIFI_EVENT_STA_ASSOC) {
            memcpy(wlan_info.bssid, buf + 10, sizeof(wlan_info.bssid));
            rt_wlan_dev_indicate_event_handle(&wlan_ap, RT_WLAN_DEV_EVT_AP_ASSOCIATED, &buff);
        } else {
            memcpy(wlan_info.bssid, buf, sizeof(wlan_info.bssid));
            rt_wlan_dev_indicate_event_handle(&wlan_ap, RT_WLAN_DEV_EVT_AP_DISASSOCIATED, &buff);
        }
    }
}

void ethernetif_recv(int idx, int total_len)
{
    struct eth_drv_sg sg_list;

    if (!rltk_wlan_running(idx))
        return;

    sg_list.buf = rt_malloc(total_len);
    if (sg_list.buf == NULL)
        return;
    sg_list.len = total_len;

    rltk_wlan_recv(idx, &sg_list, 1);
    rt_wlan_dev_report_data(idx == 0 ? &wlan_sta : &wlan_ap, sg_list.buf, total_len);
    rt_free(sg_list.buf);
}

static rt_err_t wlan_init(struct rt_wlan_device* wlan)
{
    return 0;
}

static rt_err_t wlan_mode(struct rt_wlan_device* wlan, rt_wlan_mode_t mode)
{
    return 0;
}

static rtw_result_t scan_result_handler(rtw_scan_handler_result_t* malloced_scan_result)
{
    struct rt_wlan_buff buff;
    struct rt_wlan_info wlan_info;
    struct rt_wlan_device* wlan = malloced_scan_result->user_data;

    if (malloced_scan_result->scan_complete == RTW_TRUE) {
        rt_wlan_dev_indicate_event_handle(wlan, RT_WLAN_DEV_EVT_SCAN_DONE, NULL);
        return RTW_SUCCESS;
    }
    rtw_scan_result_t* record = &malloced_scan_result->ap_details;
    wlan_info.security = record->security;
    wlan_info.band = RT_802_11_BAND_2_4GHZ;
    wlan_info.datarate = 0;
    wlan_info.channel = record->channel;
    wlan_info.rssi = record->signal_strength;
    wlan_info.ssid.len = record->SSID.len;
    memcpy(wlan_info.ssid.val, record->SSID.val, sizeof(wlan_info.ssid.val));
    memcpy(wlan_info.bssid, record->BSSID.octet, sizeof(wlan_info.bssid));
    wlan_info.hidden = 0;
    buff.data = &wlan_info;
    buff.len = sizeof(struct rt_wlan_info);
    rt_wlan_dev_indicate_event_handle(wlan, RT_WLAN_DEV_EVT_SCAN_REPORT, &buff);

    return RTW_SUCCESS;
}

static rt_err_t wlan_scan(struct rt_wlan_device* wlan, struct rt_scan_info* scan_info)
{
    int ret;

    ret = wifi_scan_networks(scan_result_handler, wlan);

    return ret ? -RT_ERROR : 0;
}

static rt_err_t wlan_join(struct rt_wlan_device* wlan, struct rt_sta_info* sta_info)
{
    int ret;

    ret = wifi_connect(sta_info->ssid.val, sta_info->security, sta_info->key.val,
        sta_info->ssid.len, sta_info->key.len, 0, NULL);

    return ret ? -RT_ERROR : 0;
}

static rt_err_t wlan_softap(struct rt_wlan_device* wlan, struct rt_ap_info* ap_info)
{
    int ret;

    wifi_off_coAP();
    ret = wifi_on_coAP(RTW_MODE_STA_AP);
    if (ret >= 0)
        ret = wifi_start_ap(ap_info->ssid.val, ap_info->security, ap_info->key.val,
            ap_info->ssid.len, ap_info->key.len, ap_info->channel);

    rt_wlan_dev_indicate_event_handle(&wlan_ap, ret < 0 ? RT_WLAN_DEV_EVT_AP_STOP : RT_WLAN_DEV_EVT_AP_START, NULL);

    return ret;
}

static rt_err_t wlan_disconnect(struct rt_wlan_device* wlan)
{
    sta_disconnect_wait = 1;
    return wifi_disconnect();
}

static rt_err_t wlan_ap_stop(struct rt_wlan_device* wlan)
{
    wifi_off_coAP();
    rt_wlan_dev_indicate_event_handle(&wlan_ap, RT_WLAN_DEV_EVT_AP_STOP, NULL);
    return 0;
}

static rt_err_t wlan_ap_deauth(struct rt_wlan_device* wlan, rt_uint8_t mac[])
{
    return 0;
}

static rt_err_t wlan_scan_stop(struct rt_wlan_device* wlan)
{
    return 0;
}

static int wlan_get_rssi(struct rt_wlan_device* wlan)
{
    int32_t rssi;

    wifi_get_rssi(&rssi);

    return rssi;
}

static rt_err_t wlan_set_powersave(struct rt_wlan_device* wlan, int level)
{
    return 0;
}

static int wlan_get_powersave(struct rt_wlan_device* wlan)
{
    return 0;
}

static rt_err_t wlan_cfg_promisc(struct rt_wlan_device* wlan, rt_bool_t start)
{
    return 0;
}

static rt_err_t wlan_cfg_filter(struct rt_wlan_device* wlan, struct rt_wlan_filter* filter)
{
    return 0;
}

static rt_err_t wlan_cfg_mgnt_filter(struct rt_wlan_device* wlan, rt_bool_t start)
{
    return 0;
}

static rt_err_t wlan_set_channel(struct rt_wlan_device* wlan, int channel)
{
    return 0;
}

static int wlan_get_channel(struct rt_wlan_device* wlan)
{
    return 0;
}

static rt_err_t wlan_set_country(struct rt_wlan_device* wlan, rt_country_code_t country_code)
{
    return 0;
}

static rt_country_code_t wlan_get_country(struct rt_wlan_device* wlan)
{
    return 0;
}

static rt_err_t wlan_set_mac(struct rt_wlan_device* wlan, rt_uint8_t mac[])
{
    return 0;
}

static rt_err_t wlan_get_mac(struct rt_wlan_device* wlan, rt_uint8_t mac[])
{
    char addr[32];

    wifi_get_mac_address(addr);
    sscanf(addr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", mac, mac + 1, mac + 2, mac + 3, mac + 4, mac + 5);

    return 0;
}

static int wlan_recv(struct rt_wlan_device* wlan, void* buff, int len)
{
}

static int wlan_send(struct rt_wlan_device* wlan, void* buff, int len)
{
    struct eth_drv_sg sg_list;
    int idx = wlan == &wlan_sta ? 0 : 1;

    if (!rltk_wlan_running(idx))
        return -1;

    sg_list.buf = buff;
    sg_list.len = len;

    rltk_wlan_send(idx, &sg_list, 1, len);
    return 0;
}

static int wlan_send_raw_frame(struct rt_wlan_device* wlan, void* buff, int len)
{
    return 0;
}

static const struct rt_wlan_dev_ops ops = {
    .wlan_init = wlan_init,
    .wlan_mode = wlan_mode,
    .wlan_scan = wlan_scan,
    .wlan_join = wlan_join,
    .wlan_softap = wlan_softap,
    .wlan_disconnect = wlan_disconnect,
    .wlan_ap_stop = wlan_ap_stop,
    .wlan_ap_deauth = wlan_ap_deauth,
    .wlan_scan_stop = wlan_scan_stop,
    .wlan_get_rssi = wlan_get_rssi,
    .wlan_set_powersave = wlan_set_powersave,
    .wlan_get_powersave = wlan_get_powersave,
    .wlan_cfg_promisc = wlan_cfg_promisc,
    .wlan_cfg_filter = wlan_cfg_filter,
    .wlan_cfg_mgnt_filter = wlan_cfg_mgnt_filter,
    .wlan_set_channel = wlan_set_channel,
    .wlan_get_channel = wlan_get_channel,
    .wlan_set_country = wlan_set_country,
    .wlan_get_country = wlan_get_country,
    .wlan_set_mac = wlan_set_mac,
    .wlan_get_mac = wlan_get_mac,
    .wlan_recv = wlan_recv,
    .wlan_send = wlan_send,
    .wlan_send_raw_frame = wlan_send_raw_frame,
};

static rt_int32_t realtek_probe(struct rt_mmcsd_card* card)
{
    wifi_sdio_func = rt_malloc(sizeof(struct sdio_func));
    if (wifi_sdio_func == NULL)
        return -ENOMEM;

    rtt_sdio_func = card->sdio_function[1];
    sdio_set_drvdata(rtt_sdio_func, wifi_sdio_func);
    sdio_enable_func(rtt_sdio_func);
    sdio_set_block_size(rtt_sdio_func, 512);

    wifi_sdio_func->max_blksize = rtt_sdio_func->max_blk_size;
    wifi_sdio_func->cur_blksize = rtt_sdio_func->cur_blk_size;
    wifi_sdio_func->enable_timeout = rtt_sdio_func->enable_timeout_val;
    wifi_sdio_func->num = rtt_sdio_func->num;
    wifi_sdio_func->vendor = rtt_sdio_func->manufacturer;
    wifi_sdio_func->device = rtt_sdio_func->product;
    wifi_sdio_func->num_info = 0;

    if (wifi_on(RTW_MODE_STA_AP) < 0) {
        rt_kprintf("RTL8189 init error\n");
        return -1;
    }

    rt_wlan_dev_register(&wlan_sta, RT_WLAN_DEVICE_STA_NAME, &ops, 0, NULL);
    rt_wlan_dev_register(&wlan_ap, RT_WLAN_DEVICE_AP_NAME, &ops, 0, NULL);
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

    wifi_start_ap("", 0, 0, 0, 0, 1);

    return 0;
}
