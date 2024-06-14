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

#ifndef _SENSOR_DEV_H_
#define _SENSOR_DEV_H_

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
#include <drivers/i2c.h>
#include "lwp_user_mm.h"
#include "k_type.h"
#include "k_sensor_comm.h"
#include "k_vicap_comm.h"

typedef struct {
    k_s32 (*sensor_power) (void *ctx, k_s32 on);
    k_s32 (*sensor_init) (void *ctx, k_sensor_mode mode);
    k_s32 (*sensor_get_chip_id)(void *ctx, k_u32 *chip_id);
    k_s32 (*sensor_get_mode)(void *ctx, k_sensor_mode *mode);
    k_s32 (*sensor_set_mode)(void *ctx, k_sensor_mode mode);
    k_s32 (*sensor_enum_mode)(void *ctx, k_sensor_enum_mode *enum_mode);
    k_s32 (*sensor_get_caps)(void *ctx, k_sensor_caps *caps);
    k_s32 (*sensor_conn_check)(void *ctx, k_s32 *conn);
    k_s32 (*sensor_set_stream)(void *ctx, k_s32 enable);
    k_s32 (*sensor_get_again)(void *ctx, k_sensor_gain *gain);
    k_s32 (*sensor_set_again)(void *ctx, k_sensor_gain gain);
    k_s32 (*sensor_get_dgain)(void *ctx, k_sensor_gain *gain);
    k_s32 (*sensor_set_dgain)(void *ctx, k_sensor_gain gain);
    k_s32 (*sensor_get_intg_time)(void *ctx, k_sensor_intg_time *time);
    k_s32 (*sensor_set_intg_time)(void *ctx, k_sensor_intg_time time);
    k_s32 (*sensor_get_exp_parm)(void *ctx, k_sensor_exposure_param *exp_parm);
    k_s32 (*sensor_set_exp_parm)(void *ctx, k_sensor_exposure_param exp_parm);
    k_s32 (*sensor_get_fps)(void *ctx, k_u32 *fps);
    k_s32 (*sensor_set_fps)(void *ctx, k_u32 fps);
    k_s32 (*sensor_get_isp_status)(void *ctx, k_sensor_isp_status *staus);
    k_s32 (*sensor_set_blc)(void *ctx, k_sensor_blc blc);
    k_s32 (*sensor_set_wb)(void *ctx, k_sensor_white_balance wb);
    k_s32 (*sensor_get_tpg)(void *ctx, k_sensor_test_pattern *tpg);
    k_s32 (*sensor_set_tpg)(void *ctx, k_sensor_test_pattern tpg);
    k_s32 (*sensor_get_expand_curve)(void *ctx, k_sensor_compand_curve *curve);
    k_s32 (*sensor_get_otp_data)(void *ctx, void *data);
    k_s32 (*sensor_set_otp_data)(void *ctx, void *data);
    k_s32 (*sensor_mirror_set)(void *ctx, k_vicap_mirror_mode mirror);
} k_sensor_function;

typedef struct {
    struct rt_i2c_bus_device *i2c_bus;
    const char *i2c_name;
    k_u16 slave_addr;
    k_sensor_reg_bits reg_addr_size;
    k_sensor_reg_bits reg_val_size;
} k_sensor_i2c_info;

struct sensor_driver_dev {
    struct rt_device  parent;
    struct rt_mutex   sensor_mutex;
    //struct rt_i2c_bus_device *i2c_bus;
    //const char *i2c_name;
    //k_u16 slave_addr;
    k_sensor_i2c_info i2c_info;

    k_u8 *sensor_name;
    //k_u8 sensor_id;

    k_sensor_function sensor_func;

    k_sensor_mode *sensor_mode;

    k_sensor_ae_info ae_info;
    k_s32 pwd_gpio;
    k_s32 reset_gpio;
};

extern struct sensor_driver_dev *g_sensor_drv[SENSOR_NUM_MAX];
extern struct sensor_driver_dev *sensor_drv_list[SENSOR_NUM_MAX];

void sensor_drv_list_init(struct sensor_driver_dev *drv_list[]);
k_s32 sensor_reg_read(k_sensor_i2c_info *i2c_info, k_u16 reg_addr, k_u16 *buf);
k_s32 sensor_reg_write(k_sensor_i2c_info *i2c_info, k_u16 reg_addr, k_u16 reg_val);
k_s32 sensor_reg_list_read(k_sensor_i2c_info *i2c_info, const k_sensor_reg *reg_list);
k_s32 sensor_reg_list_write(k_sensor_i2c_info *i2c_info, const k_sensor_reg *reg_list);
k_s32 sensor_priv_ioctl(struct sensor_driver_dev *dev, k_u32 cmd, void *args);

#endif /* _SENSOR_DEV_H_ */

