#include <assert.h>
#include <stdlib.h>
#include "cyw43.h"
#include "cyw43_country.h"
#include "cyw43_internal.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_gpio.h"

typedef struct {
    cyw43_t* dev;
    int itf;
} wifi_dev_t;

static struct rt_mmcsd_card* cyw43_card;
static rt_mutex_t cyw43_thread_mutex;
static rt_sem_t cyw43_thread_sem;
static struct rt_wlan_device wlan_sta, wlan_ap;

void cyw43_thread_enter(void)
{
    rt_mutex_take(cyw43_thread_mutex, RT_WAITING_FOREVER);
}

void cyw43_thread_exit(void)
{
    rt_mutex_release(cyw43_thread_mutex);
}

void cyw43_thread_lock_check(void)
{
#ifndef NDEBUG
    rt_base_t level;
    struct rt_thread* thread;
    /* get current thread */
    thread = rt_thread_self();
    /* disable interrupt */
    level = rt_hw_interrupt_disable();
    RT_ASSERT(cyw43_thread_mutex->owner == thread);
    /* enable interrupt */
    rt_hw_interrupt_enable(level);
#endif
}

uint64_t cyw43_hal_ticks_us(void)
{
    return (uint64_t)rt_tick_get() * (1000000 / RT_TICK_PER_SECOND);
}

uint64_t cyw43_hal_ticks_ms(void)
{
    return (uint64_t)rt_tick_get() * (1000 / RT_TICK_PER_SECOND);
}

void cyw43_delay_us(uint64_t us)
{
    rt_thread_mdelay(us / 1000);
}

void cyw43_delay_ms(uint64_t ms)
{
    rt_thread_mdelay(ms);
}

void cyw43_hal_get_mac(int interface, uint8_t mac[6])
{
    srandom(rt_tick_get());
    mac[0] = (uint8_t)random();
    mac[1] = (uint8_t)random();
    mac[2] = (uint8_t)random();
    mac[3] = (uint8_t)random();
    mac[4] = (uint8_t)random();
    mac[5] = (uint8_t)random();
}

void cyw43_hal_generate_laa_mac(int interface, uint8_t mac[6])
{
    cyw43_hal_get_mac(interface, mac);
}

void cyw43_hal_pin_config(int pin, int mode, int pull, int alt)
{
    kd_pin_mode(pin, mode);
}

void cyw43_hal_pin_config_irq_falling(int pin, int enable)
{
}

int cyw43_hal_pin_read(int pin)
{
    return 0;
}

void cyw43_hal_pin_low(int pin)
{
    kd_pin_write(pin, GPIO_PV_LOW);
}

void cyw43_hal_pin_high(int pin)
{
    kd_pin_write(pin, GPIO_PV_HIGH);
}

void cyw43_schedule_internal_poll_dispatch(void (*func)(void))
{
    rt_sem_release(cyw43_thread_sem);
}

static void cyw43_sdio_irq(struct rt_sdio_function* rt_func)
{
    cyw43_card->host->flags &= ~MMCSD_SUP_SDIO_IRQ;
    rt_sem_release(cyw43_thread_sem);
}

void cyw43_sdio_init(void)
{
    sdio_set_block_size(cyw43_card->sdio_function[1], 64);
    sdio_set_block_size(cyw43_card->sdio_function[2], 64);
    sdio_attach_irq(cyw43_card->sdio_function[1], cyw43_sdio_irq);
    sdio_attach_irq(cyw43_card->sdio_function[2], cyw43_sdio_irq);
}

void cyw43_sdio_reinit(void)
{
}

void cyw43_sdio_deinit(void)
{
}

void cyw43_sdio_set_irq(bool enable)
{
    cyw43_card->host->flags |= MMCSD_SUP_SDIO_IRQ;
    cyw43_card->host->ops->enable_sdio_irq(cyw43_card->host, enable);
}

void cyw43_sdio_enable_high_speed_4bit(void)
{
}

int cyw43_sdio_transfer_cmd52(uint32_t fn, bool write, uint32_t addr, uint8_t val)
{
    int ret;

    mmcsd_host_lock(cyw43_card->host);
    if (write)
        ret = sdio_io_writeb(cyw43_card->sdio_function[fn], addr, val);
    else
        ret = sdio_io_readb(cyw43_card->sdio_function[fn], addr, NULL);
    mmcsd_host_unlock(cyw43_card->host);
    return ret;
}

int cyw43_sdio_transfer_cmd53(uint32_t fn, bool write, uint32_t addr, bool op, uint8_t* buf, size_t len)
{
    int ret;

    mmcsd_host_lock(cyw43_card->host);
    ret = sdio_io_rw_extended_block(cyw43_card->sdio_function[fn], write, addr, op, buf, len);
    mmcsd_host_unlock(cyw43_card->host);
    return ret;
}

static void cyw43_thread(void* parameter)
{
    while (1) {
        rt_sem_take(cyw43_thread_sem, RT_WAITING_FOREVER);
        CYW43_THREAD_ENTER;
        if (cyw43_poll)
            cyw43_poll();
        CYW43_THREAD_EXIT;
    }
}

void cyw43_cb_tcpip_init(cyw43_t* self, int itf)
{
}

void cyw43_cb_tcpip_deinit(cyw43_t* self, int itf)
{
}

void cyw43_cb_tcpip_set_link_up(cyw43_t* self, int itf)
{
    if (itf == CYW43_ITF_AP)
        rt_wlan_dev_indicate_event_handle(&wlan_ap, RT_WLAN_DEV_EVT_AP_START, NULL);
    else
        rt_wlan_dev_indicate_event_handle(&wlan_sta, RT_WLAN_DEV_EVT_CONNECT, NULL);
}

void cyw43_cb_tcpip_set_link_down(cyw43_t* self, int itf)
{
    if (itf == CYW43_ITF_AP)
        rt_wlan_dev_indicate_event_handle(&wlan_ap, RT_WLAN_DEV_EVT_AP_STOP, NULL);
    else
        rt_wlan_dev_indicate_event_handle(&wlan_sta, RT_WLAN_DEV_EVT_DISCONNECT, NULL);
}

void cyw43_cb_process_ethernet(void* cb_data, int itf, size_t len, const uint8_t* buf)
{
    rt_wlan_dev_report_data(itf == CYW43_ITF_STA ? &wlan_sta : &wlan_ap, buf, len);
}

static void cyw43_cb_async_event(void* cb_data, const cyw43_async_event_t* ev)
{
    cyw43_t* self = cb_data;

    if ((self->wifi_join_state & 0xf) > 1) {
        self->wifi_join_state = 0;
        cyw43_wifi_leave(self, CYW43_ITF_STA);
        rt_wlan_dev_indicate_event_handle(&wlan_sta, RT_WLAN_DEV_EVT_CONNECT_FAIL, NULL);
    }

    if (ev->interface == CYW43_ITF_AP) {
        if (ev->event_type == CYW43_EV_ASSOC_IND || ev->event_type == CYW43_EV_REASSOC_IND || ev->event_type == CYW43_EV_DISASSOC_IND) {
            struct rt_wlan_buff buff;
            struct rt_wlan_info wlan_info;
            memset(&wlan_info, 0, sizeof(wlan_info));
            memcpy(wlan_info.bssid, ev->_1 + 8, sizeof(wlan_info.bssid));
            buff.data = &wlan_info;
            buff.len = sizeof(struct rt_wlan_info);
            if (ev->event_type == CYW43_EV_DISASSOC_IND)
                rt_wlan_dev_indicate_event_handle(&wlan_ap, RT_WLAN_DEV_EVT_AP_DISASSOCIATED, &buff);
            else
                rt_wlan_dev_indicate_event_handle(&wlan_ap, RT_WLAN_DEV_EVT_AP_ASSOCIATED, &buff);
        }
    }
}

static rt_err_t wlan_init(struct rt_wlan_device* wlan)
{
    wifi_dev_t* dev = wlan->user_data;

    cyw43_wifi_set_up(dev->dev, dev->itf, true, CYW43_COUNTRY_WORLDWIDE);

    return 0;
}

static rt_err_t wlan_mode(struct rt_wlan_device* wlan, rt_wlan_mode_t mode)
{
    wifi_dev_t* dev = wlan->user_data;

    if (mode == RT_WLAN_NONE)
        cyw43_wifi_set_up(dev->dev, dev->itf, false, CYW43_COUNTRY_WORLDWIDE);
    else
        cyw43_wifi_set_up(dev->dev, dev->itf, true, CYW43_COUNTRY_WORLDWIDE);

    return 0;
}

static int cyw43_cb_wifi_scan(void* env, const cyw43_ev_scan_result_t* result)
{
    struct rt_wlan_buff buff;
    struct rt_wlan_info wlan_info;

    if (result == NULL) {
        rt_wlan_dev_indicate_event_handle(env, RT_WLAN_DEV_EVT_SCAN_DONE, NULL);
        return 0;
    }
    wlan_info.security = result->auth_mode == 3 ? SECURITY_WPA_TKIP_PSK : result->auth_mode == 5 ? SECURITY_WPA2_AES_PSK
        : result->auth_mode == 0                                                                 ? SECURITY_OPEN
                                                                                                 : SECURITY_UNKNOWN;
    wlan_info.band = RT_802_11_BAND_2_4GHZ;
    wlan_info.datarate = 0;
    wlan_info.channel = result->channel;
    wlan_info.rssi = result->rssi;
    wlan_info.ssid.len = result->ssid_len;
    memcpy(wlan_info.ssid.val, result->ssid, sizeof(wlan_info.ssid.val));
    memcpy(wlan_info.bssid, result->bssid, sizeof(wlan_info.bssid));
    wlan_info.hidden = 0;
    buff.data = &wlan_info;
    buff.len = sizeof(struct rt_wlan_info);
    rt_wlan_dev_indicate_event_handle(env, RT_WLAN_DEV_EVT_SCAN_REPORT, &buff);

    return 0;
}

static rt_err_t wlan_scan(struct rt_wlan_device* wlan, struct rt_scan_info* scan_info)
{
    wifi_dev_t* dev = wlan->user_data;
    cyw43_wifi_scan_options_t opt;
    int ret;

    memset(&opt, 0, sizeof(opt));
    if (scan_info) {
        opt.ssid_len = scan_info->ssid.len;
        memcpy(opt.ssid, scan_info->ssid.val, sizeof(opt.ssid));
        opt.scan_type = scan_info->passive;
    }
    ret = cyw43_wifi_scan(dev->dev, &opt, wlan, cyw43_cb_wifi_scan);

    return ret ? -RT_ERROR : 0;
}

static rt_err_t wlan_join(struct rt_wlan_device* wlan, struct rt_sta_info* sta_info)
{
    wifi_dev_t* dev = wlan->user_data;
    int ret;

    ret = cyw43_wifi_join(dev->dev, sta_info->ssid.len, sta_info->ssid.val,
        sta_info->key.len, sta_info->key.val, sta_info->security == 0 ? 0 : SECURITY_WPA2_MIXED_PSK, sta_info->bssid, sta_info->channel);

    return ret ? -RT_ERROR : 0;
}

static rt_err_t wlan_softap(struct rt_wlan_device* wlan, struct rt_ap_info* ap_info)
{
    wifi_dev_t* dev = wlan->user_data;

    cyw43_wifi_ap_set_channel(dev->dev, ap_info->channel);
    cyw43_wifi_ap_set_ssid(dev->dev, ap_info->ssid.len, ap_info->ssid.val);
    cyw43_wifi_ap_set_password(dev->dev, ap_info->key.len, ap_info->key.val);
    cyw43_wifi_ap_set_auth(dev->dev, ap_info->security);
    cyw43_wifi_set_up(dev->dev, dev->itf, true, CYW43_COUNTRY_WORLDWIDE);

    return 0;
}

static rt_err_t wlan_disconnect(struct rt_wlan_device* wlan)
{
    wifi_dev_t* dev = wlan->user_data;
    int ret;

    ret = cyw43_wifi_leave(dev->dev, dev->itf);

    return ret ? -RT_ERROR : 0;
}

static rt_err_t wlan_ap_stop(struct rt_wlan_device* wlan)
{
    wifi_dev_t* dev = wlan->user_data;

    cyw43_wifi_set_up(dev->dev, dev->itf, false, CYW43_COUNTRY_WORLDWIDE);

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
    wifi_dev_t* dev = wlan->user_data;
    int32_t rssi;

    cyw43_wifi_get_rssi(dev->dev, &rssi);

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
    wifi_dev_t* dev = wlan->user_data;

    memcpy(mac, dev->dev->mac, sizeof(dev->dev->mac));

    return 0;
}

static int wlan_recv(struct rt_wlan_device* wlan, void* buff, int len)
{
}

static int wlan_send(struct rt_wlan_device* wlan, void* buff, int len)
{
    wifi_dev_t* dev = wlan->user_data;
    int ret;

    ret = cyw43_send_ethernet(dev->dev, dev->itf, len, buff, false);

    return ret ? -RT_ERROR : 0;
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

static rt_int32_t cyw43_probe(struct rt_mmcsd_card* card)
{
    static wifi_dev_t wifi_sta, wifi_ap;
    rt_thread_t tid;

    cyw43_card = card;
    cyw43_thread_mutex = rt_mutex_create("cyw43", RT_IPC_FLAG_PRIO);
    cyw43_thread_sem = rt_sem_create("cyw43", 0, RT_IPC_FLAG_PRIO);

    cyw43_init(&cyw43_state);
    cyw43_state.async_event_cb = cyw43_cb_async_event;

    wifi_sta.dev = &cyw43_state;
    wifi_sta.itf = CYW43_ITF_STA;
    wifi_ap.dev = &cyw43_state;
    wifi_ap.itf = CYW43_ITF_AP;
    rt_wlan_dev_register(&wlan_sta, RT_WLAN_DEVICE_STA_NAME, &ops, 0, &wifi_sta);
    rt_wlan_dev_register(&wlan_ap, RT_WLAN_DEVICE_AP_NAME, &ops, 0, &wifi_ap);
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

    tid = rt_thread_create("cyw43", cyw43_thread, NULL, CYW43XX_THREAD_STACK_SIZE, CYW43XX_THREAD_PRIORITY, 10);
    rt_thread_startup(tid);

    return 0;
}

static rt_int32_t cyw43_remove(struct rt_mmcsd_card* card)
{
}

static struct rt_sdio_device_id cyw43_id[] = {
    { 0, 0x02d0, 0xa9a6 },
};

static struct rt_sdio_driver cyw43_drv = {
    "realtek-wifi",
    cyw43_probe,
    cyw43_remove,
    cyw43_id,
};

int cyw43_driver_init(void)
{
    // Reset and power up the WL chip
    cyw43_hal_pin_config(CYW43_PIN_WL_REG_ON, CYW43_HAL_PIN_MODE_OUTPUT, CYW43_HAL_PIN_PULL_NONE, 0);
    cyw43_hal_pin_low(CYW43_PIN_WL_REG_ON);
    cyw43_delay_ms(20);
    cyw43_hal_pin_high(CYW43_PIN_WL_REG_ON);
    cyw43_delay_ms(50);

    sdio_register_driver(&cyw43_drv);
    kd_sdhci_change(CYW43XX_SDIO_DEV);

    return 0;
}
INIT_COMPONENT_EXPORT(cyw43_driver_init);
