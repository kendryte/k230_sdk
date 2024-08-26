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
#include "k_connector_ioctl.h"
#include "k_connector_comm.h"
#include "tick.h"

extern struct connector_driver_dev hx8399_connector_drv;
extern struct connector_driver_dev lt9611_connector_drv;
extern struct connector_driver_dev st7701_connector_drv;
extern struct connector_driver_dev ili9806_connector_drv;
extern struct connector_driver_dev virtdev_connector_drv;

struct connector_driver_dev* connector_drv_list[CONNECTOR_NUM_MAX] = {
    &hx8399_connector_drv,
    &lt9611_connector_drv,
    &st7701_connector_drv,
    &ili9806_connector_drv,
    &virtdev_connector_drv,
};

void connector_drv_list_init(struct connector_driver_dev* drv_list[])
{
    for (k_u32 connector_id = 0; connector_id < CONNECTOR_NUM_MAX; connector_id++) {
        if (drv_list[connector_id] != NULL)
            g_connector_drv[connector_id] = drv_list[connector_id];
    }
}

k_s32 connector_priv_ioctl(struct connector_driver_dev* dev, k_u32 cmd, void* args)
{
    k_s32 ret = -1;
    if (!dev) {
        rt_kprintf("%s error, dev null\n", __func__);
        return ret;
    }

    switch (cmd) {
    case KD_IOC_CONNECTOR_S_POWER: {
        k_s32 power_on;
        if (dev->connector_func.connector_power == NULL) {
            rt_kprintf("%s (%s)connector_power is null\n", __func__, dev->connector_name);
            return -1;
        }

        if (sizeof(k_s32) != lwp_get_from_user(&power_on, args, sizeof(k_s32))) {
            rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
            return -1;
        }

        ret = dev->connector_func.connector_power(dev, power_on);
        break;
    }
    case KD_IOC_CONNECTOR_S_INIT: {
        k_connector_info connector_info;
        if (dev->connector_func.connector_init == NULL) {
            rt_kprintf("%s (%s)connector_init is null\n", __func__, dev->connector_name);
            return -1;
        }

        if (sizeof(connector_info) != lwp_get_from_user(&connector_info, args, sizeof(connector_info))) {
            rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
            return -1;
        }

        ret = dev->connector_func.connector_init(dev, &connector_info);
        break;
    }
    case KD_IOC_CONNECTOR_G_ID: {
        k_u32 chip_id = 0;
        if (dev->connector_func.connector_get_chip_id == NULL) {
            rt_kprintf("%s (%s)connector_get_chip_id is null\n", __func__, dev->connector_name);
            return -1;
        }
        ret = dev->connector_func.connector_get_chip_id(dev, &chip_id);
        if (ret) {
            rt_kprintf("%s (%s)connector_get_chip_id err\n", __func__, dev->connector_name);
            return -1;
        }
        if (sizeof(chip_id) != lwp_put_to_user(args, &chip_id, sizeof(chip_id))) {
            rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
            return -1;
        }
        break;
    }
    case KD_IOC_CONNECTOR_G_NEG_DATA: {
        k_connector_negotiated_data negotiated_data;
        if (dev->connector_func.connector_get_negotiated_data == NULL) {
            rt_kprintf("%s (%s)connector_get_negotiated_data is null\n", __func__, dev->connector_name);
            return -1;
        }
        ret = dev->connector_func.connector_get_negotiated_data(dev, &negotiated_data);
        if (ret == -1) {
            rt_kprintf("%s (%s)connector_get_negotiated_data err\n", __func__, dev->connector_name);
            return -1;
        }
        if (sizeof(negotiated_data) != lwp_put_to_user(args, &negotiated_data, sizeof(negotiated_data))) {
            rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
            return -1;
        }
        break;
    }
    case KD_IOC_CONNECTOR_S_MIRROR: {
        k_connector_mirror mirror;
        if (dev->connector_func.connector_set_mirror == NULL) {
            rt_kprintf("%s (%s)connector_set_mirror is null\n", __func__, dev->connector_name);
            return -1;
        }

        if (sizeof(mirror) != lwp_get_from_user(&mirror, args, sizeof(mirror))) {
            rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
            return -1;
        }

        ret = dev->connector_func.connector_set_mirror(dev, &mirror);
        break;
    }
    default:
        break;
    }
    return ret;
}

static uint64_t perf_get_times(void)
{
    uint64_t cnt;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(cnt));
    return cnt;
}

void connector_delay_us(uint64_t us)
{
    uint64_t delay = (TIMER_CLK_FREQ / 1000000) * us;
    volatile uint64_t cur_time = perf_get_times();
    while (1) {
        if ((perf_get_times() - cur_time) >= delay)
            break;
    }
}

k_u32 connecter_dsi_send_pkg(k_u8* buf, k_u32 cmd_len)
{
    return dwc_lpdt_send_pkg(buf, cmd_len);
}

k_u32 connecter_dsi_read_pkg(k_u8 data_addr)
{
    return dwc_lpdt_read_pkg(data_addr);
}

k_s32 connector_set_phy_freq(k_vo_mipi_phy_attr* phy)
{
    return dwc_mipi_phy_config(phy);
}

k_s32 connector_set_dsi_attr(k_vo_dsi_attr* attr)
{
    return kd_dsi_dsi_attr(attr);
}

k_u32 connector_set_dsi_enable(k_u32 enable)
{
    return dwc_dsi_enable(enable);
}

k_u32 connector_set_dsi_test_mode(void)
{
    return dwc_dst_set_test_mode();
}

k_u32 connector_set_vo_init(void)
{
    return vo_init();
}

k_u32 connector_set_vo_param(k_vo_pub_attr* attr)
{
    return kd_vo_set_dev_param(attr);
}

k_u32 connector_set_vo_attr(k_vo_pub_attr* pub_attr, k_vo_sync_attr* sync_attr)
{
    return kd_vo_set_dev_attr(pub_attr, sync_attr);
}

void connector_set_vo_enable(void)
{
    kd_vo_enable();
}

void connector_set_vtth_intr(k_bool status, k_u32 vpos)
{
    kd_vo_set_vtth_intr(status, vpos);
}

void connector_set_pixclk(k_u32 div)
{
    k230_set_pixclk(div);
}
