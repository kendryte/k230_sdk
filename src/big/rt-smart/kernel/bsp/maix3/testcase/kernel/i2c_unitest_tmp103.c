#include <unistd.h>
#include <stdio.h>
#include <rtdef.h>
#include <drivers/i2c.h>
#include "board.h"
#include <rtdbg.h>
#include <riscv_io.h>
#include "utest.h"

#define LOG_TAG     "drv.tmp103"

static struct rt_i2c_bus_device *i2c_bus  = RT_NULL;
#define I2C_NAME        "i2c0"
#define CHIP_ADDRESS    (0x70) /* slave address */

static rt_err_t read_reg(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msg[2] = {0, 0};
    static rt_uint8_t i2c_reg[1] = {0};

    RT_ASSERT(bus != RT_NULL);

    i2c_reg[0] = ((uint16_t)reg);

    msg[0].addr  = CHIP_ADDRESS;
    msg[0].flags = RT_I2C_WR;
    msg[0].buf   = i2c_reg;
    msg[0].len   = 1;

    msg[1].addr  = CHIP_ADDRESS;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = len;
    msg[1].buf   = buf;

    if (rt_i2c_transfer(bus, msg, 2) == 2)
    {
        return RT_EOK;
    }

    return RT_ERROR;
}

static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2];
    struct rt_i2c_msg msgs;

    RT_ASSERT(bus != RT_NULL);

    buf[0] = (uint16_t)(reg);

    buf[1] = data;

    msgs.addr = CHIP_ADDRESS;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 2;

    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }

    return RT_ERROR;
}


static rt_err_t tmp103_rw_test(struct rt_i2c_bus_device *bus)
{
    rt_uint8_t read_value;
    rt_uint8_t count = 0;

    write_reg(bus, 0x01,0x02);
    read_reg(bus, 0x01, 1, &read_value);
    uassert_int_equal(read_value, 0x02);

    write_reg(bus, 0x01,0x03);
    read_reg(bus, 0x01, 1, &read_value);
    uassert_int_equal(read_value, 0x03);

    write_reg(bus, 0x02,0x0b);
    read_reg(bus, 0x02, 1, &read_value);
    uassert_int_equal(read_value, 0x0b);

    write_reg(bus, 0x02,0x0c);
    read_reg(bus, 0x02, 1, &read_value);
    uassert_int_equal(read_value, 0x0c);

    write_reg(bus, 0x03,0x1b);
    read_reg(bus, 0x03, 1, &read_value);
    uassert_int_equal(read_value, 0x1b);

    write_reg(bus, 0x03,0x1c);
    read_reg(bus, 0x03, 1, &read_value);
    uassert_int_equal(read_value, 0x1c);

    return 0;
}

void rt_hw_tmp103_test(void)
{
    i2c_bus = rt_i2c_bus_device_find(I2C_NAME);
    if (i2c_bus == RT_NULL)
    {
        LOG_E("can't find %s deivce", I2C_NAME);
        return;
    }

    tmp103_rw_test(i2c_bus);

}

static void i2c_testcase_tmp103(void)
{
    UTEST_UNIT_RUN(rt_hw_tmp103_test);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

UTEST_TC_EXPORT(i2c_testcase_tmp103, "testcases.kernel.i2c_testcase_tmp103", utest_tc_init, utest_tc_cleanup, 10);
