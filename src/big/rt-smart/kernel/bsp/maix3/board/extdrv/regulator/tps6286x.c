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

#include <rtthread.h>
#include <rtdevice.h>
#include "regulator.h"
#define DBG_TAG  "tps6286x"
#ifdef RT_DEBUG
#define DBG_LVL          DBG_LOG
#else
#define DBG_LVL          DBG_WARNING
#endif
#define DBG_COLOR
#include <rtdbg.h>

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

// tps6286x reg
#define TPS6286X_VOUT1          0x01
#define TPS6286X_CONTROL        0x03
#define TPS6286X_CONTROL_FPWM	0x10
#define TPS6286X_CONTROL_SWEN	0x20

#define TPS6286X_MIN_MV         400
#define TPS6286X_MAX_MV         1675
#define TPS6286X_STEP_MV        5

struct tps6286x_dev {
    struct regulator_dev dev;
    const char *dev_name;
    const char *i2c_name;
    rt_uint16_t i2c_addr;
    struct rt_i2c_bus_device *bus;
    int base_uv;
    int min_uv;
    int max_uv;
    int step_uv;
};

static int tps6286x_write_reg(struct tps6286x_dev *pdev, uint8_t reg,
        uint8_t val)
{
    struct rt_i2c_msg msgs;
    uint8_t buf[] = {reg, val};

    msgs.addr  = pdev->i2c_addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf   = buf;
    msgs.len   = sizeof(buf);

    if (rt_i2c_transfer(pdev->bus, &msgs, 1) != 1)
        return -RT_ERROR;

    return RT_EOK;
}

static int tps6286x_read_reg(struct tps6286x_dev *pdev, uint8_t reg,
        uint8_t *pval)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = pdev->i2c_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = pdev->i2c_addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = pval;
    msgs[1].len   = 1;

    if (rt_i2c_transfer(pdev->bus, msgs, 2) != 2)
        return -RT_ERROR;

    return RT_EOK;
}

static int tps6286x_update_reg(struct tps6286x_dev *pdev, uint8_t reg,
        uint8_t val, uint8_t mask)
{
    uint8_t reg_val;

    if (tps6286x_read_reg(pdev, reg, &reg_val) != RT_EOK)
        return -RT_ERROR;

    reg_val &= ~mask;
    reg_val |= val;

    if (tps6286x_write_reg(pdev, reg, reg_val) != RT_EOK)
        return -RT_ERROR;

    return RT_EOK;
}

static int tps6286x_init(struct regulator_dev *pdev)
{
    int ret;
    struct tps6286x_dev *tps_dev = (struct tps6286x_dev *)pdev;

    tps_dev->bus = (struct rt_i2c_bus_device *)rt_device_find(
        tps_dev->i2c_name);
    if (tps_dev->bus == RT_NULL) {
        LOG_E("Can't find %s device", tps_dev->i2c_name);
        return -RT_ERROR;
    }

    ret = rt_device_open((rt_device_t)tps_dev->bus, RT_DEVICE_FLAG_RDWR);
    if (ret != RT_EOK) {
        LOG_E("open %s device failed: %d", tps_dev->i2c_name, ret);
        return -RT_ERROR;
    }

    return ret;
}

static int tps6286x_deinit(struct regulator_dev *pdev)
{
    return 0;
}

static int tps6286x_enable(struct regulator_dev *pdev, bool enable)
{
    struct tps6286x_dev *tps_dev = (struct tps6286x_dev *)pdev;

    return tps6286x_update_reg(tps_dev, TPS6286X_CONTROL, enable ?
        TPS6286X_CONTROL_SWEN : 0, TPS6286X_CONTROL_SWEN);
}

static int tps6286x_set_voltage(struct regulator_dev *pdev, int vol_uv)
{
    struct tps6286x_dev *tps_dev = (struct tps6286x_dev *)pdev;

    if (vol_uv > tps_dev->max_uv)
        vol_uv = tps_dev->max_uv;
    else if (vol_uv < tps_dev->min_uv)
        vol_uv = tps_dev->min_uv;
    vol_uv -= tps_dev->base_uv;

    return tps6286x_write_reg(tps_dev, TPS6286X_VOUT1,
        vol_uv / tps_dev->step_uv);
}

static int tps6286x_get_voltage(struct regulator_dev *pdev, int *pvol_uv)
{
    int ret;
    uint8_t val;
    struct tps6286x_dev *tps_dev = (struct tps6286x_dev *)pdev;

    ret = tps6286x_read_reg(tps_dev, TPS6286X_VOUT1, &val);
    if (ret != RT_EOK)
        return -RT_ERROR;
    *pvol_uv = val * tps_dev->step_uv + tps_dev->base_uv;

    return RT_EOK;
}

static struct regulator_ops tps6286x_ops = {
    .init = tps6286x_init,
    .deinit = tps6286x_deinit,
    .enable = tps6286x_enable,
    .set_voltage = tps6286x_set_voltage,
    .get_voltage = tps6286x_get_voltage,
};

static struct tps6286x_dev dev[2] = {
    {
        .dev_name = "regulator_cpu",
        .i2c_name = PMIC_CPU_I2C_DEV,
        .i2c_addr = PMIC_CPU_I2C_ADDR,
        .base_uv = 400000,
        .min_uv = 400000,
        .max_uv = 1675000,
        .step_uv = 5000,
    },
    {
        .dev_name = "regulator_kpu",
        .i2c_name = PMIC_KPU_I2C_DEV,
        .i2c_addr = PMIC_KPU_I2C_ADDR,
        .base_uv = 400000,
        .min_uv = 400000,
        .max_uv = 1675000,
        .step_uv = 5000,
    },
};

int regulator_tps6286x_init(void)
{
    int ret;

    for (int i = 0; i < ARRAY_SIZE(dev); i++) {
        ret = regulator_dev_register(&dev[i].dev, &tps6286x_ops,
            dev[i].dev_name);
        if (ret) {
            LOG_E("tps6286x %d register fial: %d\n", i, ret);
            break;
        }
    }

    return ret;
}
INIT_COMPONENT_EXPORT(regulator_tps6286x_init);
