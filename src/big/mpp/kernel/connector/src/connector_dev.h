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

#ifndef _CONNECTOR_DEV_H_
#define _CONNECTOR_DEV_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>
#include <ioremap.h>
#include <rthw.h>
#include "lwp_user_mm.h"
#include "k_type.h"

#include "k_vo_comm.h"
#include "connector_dev.h"
#include "k_connector_comm.h"


typedef struct {
    k_s32 (*connector_power) (void *ctx, k_s32 on);
    k_s32 (*connector_init) (void *ctx, k_connector_info *info);
    k_s32 (*connector_get_chip_id) (void *ctx, k_u32 *chip_id);
    k_s32 (*connector_get_negotiated_data) (void *ctx, k_connector_negotiated_data *negotiated_data);
    k_s32 (*connector_conn_check)(void *ctx, k_s32 *conn);
    k_s32 (*connector_set_mirror)(void *ctx, k_connector_mirror *mirror);
} k_connector_function;


struct connector_driver_dev 
{
    struct rt_device  parent;
    struct rt_mutex   connector_mutex;
    k_u8 *connector_name;
    k_connector_function connector_func;

    k_s32 backlight_gpio;
    k_s32 reset_gpio;

    void *driver_data;
};


extern struct connector_driver_dev *g_connector_drv[CONNECTOR_NUM_MAX];
extern struct connector_driver_dev *connector_drv_list[CONNECTOR_NUM_MAX];


extern k_u32 dwc_mipi_phy_config(k_vo_mipi_phy_attr *phy);
extern k_u32 dwc_lpdt_send_pkg(k_u8 *buf, k_u32 cmd_len);
extern k_u32 dwc_lpdt_read_pkg(k_u8 data_addr);
extern k_u32 k230_display_rst(void);
extern k_u32 kd_dsi_dsi_attr(k_vo_dsi_attr *attr);
extern k_u32 dwc_dsi_enable(k_u32 enable);
extern k_u32 dwc_dst_set_test_mode(void);
extern k_u32 vo_init(void);
extern k_s32 kd_vo_set_dev_param(k_vo_pub_attr *attr);
extern k_s32 kd_vo_set_dev_attr(k_vo_pub_attr *pub_attr, k_vo_sync_attr *sync_attr);
extern void kd_vo_enable(void);
extern void kd_vo_set_vtth_intr(k_bool status, k_u32 vpos);
extern void k230_set_pixclk(k_u32 div);

k_s32 connector_priv_ioctl(struct connector_driver_dev *dev, k_u32 cmd, void *args);
void connector_drv_list_init(struct connector_driver_dev *drv_list[]);
k_u32 connecter_dsi_send_pkg(k_u8 *buf, k_u32 cmd_len);
k_u32 connecter_dsi_read_pkg(k_u8 data_addr);
k_s32 connector_set_phy_freq(k_vo_mipi_phy_attr *phy);
k_s32 connector_set_dsi_attr(k_vo_dsi_attr *attr);
k_u32 connector_set_dsi_enable(k_u32 enable);
k_u32 connector_set_dsi_test_mode(void);
k_u32 connector_set_vo_init(void);
k_u32 connector_set_vo_param(k_vo_pub_attr *attr);
k_u32 connector_set_vo_attr(k_vo_pub_attr *pub_attr, k_vo_sync_attr *sync_attr);
void connector_set_vo_enable(void);
void connector_set_vtth_intr(k_bool status, k_u32 vpos);
void connector_delay_us(uint64_t us);
void connector_set_pixclk(k_u32 div);
#endif /* _SENSOR_DEV_H_ */
