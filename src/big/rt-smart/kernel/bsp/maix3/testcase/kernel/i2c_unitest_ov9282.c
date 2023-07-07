#include <unistd.h>
#include <stdio.h>
#include <rtdef.h>
#include <drivers/i2c.h>
#include "board.h"
#define LOG_TAG     "drv.ov9282"
#include <rtdbg.h>
#include <riscv_io.h>

struct rt_i2c_bus_device *i2c_bus  = RT_NULL;
#define I2C_NAME        "i2c0"
#define CHIP_ADDRESS    (0x60) /* slave address */

static rt_err_t read_reg(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msg[2] = {0, 0};
    static rt_uint8_t i2c_reg[2] = {0, 0};

    RT_ASSERT(bus != RT_NULL);

    i2c_reg[0] = ((uint16_t)(reg >> 8) & 0xFF);
    i2c_reg[1] = ((uint16_t)(reg & 0xFF));

    msg[0].addr  = CHIP_ADDRESS;
    msg[0].flags = RT_I2C_WR;
    msg[0].buf   = i2c_reg;
    msg[0].len   = 2;

    msg[1].addr  = CHIP_ADDRESS;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = len;
    msg[1].buf   = buf;

    if (rt_i2c_transfer(bus, msg, 2) == 2)
    {
        LOG_E("read: [0x%x] = [0x%x]\n", reg, *buf);
        return RT_EOK;
    }

    return RT_ERROR;
}

static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;

    RT_ASSERT(bus != RT_NULL);

    buf[0] = ((uint16_t)(reg >> 8) & 0xFF);
    buf[1] = ((uint16_t)(reg)&0xFF);

    buf[2] = data;

    msgs.addr = CHIP_ADDRESS;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 3;

    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        LOG_E("write: [0x%x]  [0x%x]\n", reg, data);
        return RT_EOK;
    }

    return RT_ERROR;
}


static rt_err_t ov9282_reg_init(struct rt_i2c_bus_device *bus)
{
    rt_uint8_t read_value;

    write_reg(bus, 0x0103,0x01);
    write_reg(bus, 0x0302,0x32);                  // pll1 clk mul
    write_reg(bus, 0x3001,0x00);                  // pll1 clk mul
    write_reg(bus, 0x030d,0x50);                  // pll2 clk mul   
    write_reg(bus, 0x030e,0x02);                  // pll2 clk sys div
    write_reg(bus, 0x3004,0x00);                  // pll1 mipi div
    write_reg(bus, 0x3005,0x00);                  // pll1 sys per div
    write_reg(bus, 0x3006,0x04);                  // pll1 sys div
    write_reg(bus, 0x3011,0x0a);                  // pll2 bypass
    write_reg(bus, 0x3013,0x18);      
    write_reg(bus, 0x3022,0x01);                  // debug control /mipi_lvds_mode
    write_reg(bus, 0x3030,0x10);              
    write_reg(bus, 0x3039,0x32);                  // mipi lan_num, dvp_en mipi_en 
    write_reg(bus, 0x303a,0x00);                  // mipi lan disable 
    write_reg(bus, 0x3500,0x00);
    write_reg(bus, 0x3501,0x2a);
    write_reg(bus, 0x3502,0x90);
    write_reg(bus, 0x3503,0x08);
    write_reg(bus, 0x3505,0x8c);
    write_reg(bus, 0x3507,0x03);
    write_reg(bus, 0x3508,0x00);
    write_reg(bus, 0x3509,0x10);
    write_reg(bus, 0x3610,0x80);
    write_reg(bus, 0x3611,0xa0);
    write_reg(bus, 0x3620,0x6f);
    write_reg(bus, 0x3632,0x56);
    write_reg(bus, 0x3633,0x78);
    write_reg(bus, 0x3662,0x05);
    write_reg(bus, 0x3666,0x00);
    write_reg(bus, 0x366f,0x5a);
    write_reg(bus, 0x3680,0x84);
    write_reg(bus, 0x3712,0x80);
    write_reg(bus, 0x372d,0x22);
    write_reg(bus, 0x3731,0x80);
    write_reg(bus, 0x3732,0x30);
    write_reg(bus, 0x3778,0x00);              // v binning disable 
    write_reg(bus, 0x377d,0x22);
    write_reg(bus, 0x3788,0x02);
    write_reg(bus, 0x3789,0xa4);
    write_reg(bus, 0x378a,0x00);
    write_reg(bus, 0x378b,0x4a);
    write_reg(bus, 0x3799,0x20);
    write_reg(bus, 0x3800,0x00);          //array h start     0
    write_reg(bus, 0x3801,0x00);
    write_reg(bus, 0x3802,0x00);          //array v start       0
    write_reg(bus, 0x3803,0x00);
    write_reg(bus, 0x3804,0x05);          //array h end       1295
    write_reg(bus, 0x3805,0x0f);
    write_reg(bus, 0x3806,0x03);          // array v end       815   
    write_reg(bus, 0x3807,0x2f);
    write_reg(bus, 0x3808,0x05);          // output size h    1280
    write_reg(bus, 0x3809,0x00);
    write_reg(bus, 0x380a,0x03);          // output size v    800
    write_reg(bus, 0x380b,0x20);
    write_reg(bus, 0x380c,0x05);          // total size h 1456
    write_reg(bus, 0x380d,0xb0);
    write_reg(bus, 0x380e,0x0e);          // total size v  e38 = 3640? 3 8e? 910  
    write_reg(bus, 0x380f,0x38);
    write_reg(bus, 0x3810,0x00);          // windows offset h  8
    write_reg(bus, 0x3811,0x08);
    write_reg(bus, 0x3812,0x00);          // windows offset v  8
    write_reg(bus, 0x3813,0x08);
    write_reg(bus, 0x3814,0x11);
    write_reg(bus, 0x3815,0x11);
    write_reg(bus, 0x3820,0x40);
    write_reg(bus, 0x3821,0x00);
    write_reg(bus, 0x3881,0x42);
    write_reg(bus, 0x38b1,0x00);
    write_reg(bus, 0x3920,0xff);
    write_reg(bus, 0x4003,0x40);
    write_reg(bus, 0x4008,0x04);
    write_reg(bus, 0x4009,0x0b);
    write_reg(bus, 0x400c,0x00);
    write_reg(bus, 0x400d,0x07);
    write_reg(bus, 0x4010,0x40);
    write_reg(bus, 0x4043,0x40);
    write_reg(bus, 0x4307,0x30);
    write_reg(bus, 0x4317,0x00);
    write_reg(bus, 0x4501,0x00);
    write_reg(bus, 0x4507,0x00);
    write_reg(bus, 0x4509,0x00);
    write_reg(bus, 0x450a,0x08);
    write_reg(bus, 0x4601,0x04);
    write_reg(bus, 0x470f,0x00);
    write_reg(bus, 0x4f07,0x00);
    write_reg(bus, 0x4800,0x00);
    write_reg(bus, 0x5000,0x9f);
    write_reg(bus, 0x5001,0x00);
    write_reg(bus, 0x5e00,0x00);
    write_reg(bus, 0x5d00,0x07);
    write_reg(bus, 0x5d01,0x00);
    write_reg(bus, 0x0100,0x01);

    read_reg(bus, 0x0103, 1, &read_value);           //rst
    read_reg(bus, 0x3001, 1, &read_value);           // pll1 clk mul
    read_reg(bus, 0x0302, 1, &read_value);  
    read_reg(bus, 0x030d, 1, &read_value);           // pll2 clk mul
    read_reg(bus, 0x030e, 1, &read_value);           // pll2 clk sys div
    read_reg(bus, 0x3004, 1, &read_value);
	read_reg(bus, 0x304, 1, &read_value);
	read_reg(bus, 0x300, 1, &read_value);
    read_reg(bus, 0x3005, 1, &read_value);
    read_reg(bus, 0x3006, 1, &read_value);
    read_reg(bus, 0x3011, 1, &read_value);
    read_reg(bus, 0x3013, 1, &read_value);
    read_reg(bus, 0x3022, 1, &read_value);
    read_reg(bus, 0x3030, 1, &read_value);
    read_reg(bus, 0x3039, 1, &read_value);
    read_reg(bus, 0x303a, 1, &read_value);
    read_reg(bus, 0x3500, 1, &read_value);
    read_reg(bus, 0x3501, 1, &read_value);
    read_reg(bus, 0x3502, 1, &read_value);
    read_reg(bus, 0x3503, 1, &read_value);
    read_reg(bus, 0x3505, 1, &read_value);
    read_reg(bus, 0x3507, 1, &read_value);
    read_reg(bus, 0x3508, 1, &read_value);
    read_reg(bus, 0x3509, 1, &read_value);
    read_reg(bus, 0x3610, 1, &read_value);
    read_reg(bus, 0x3611, 1, &read_value);
    read_reg(bus, 0x3620, 1, &read_value);
    read_reg(bus, 0x3632, 1, &read_value);
    read_reg(bus, 0x3633, 1, &read_value);
    read_reg(bus, 0x3662, 1, &read_value);
    read_reg(bus, 0x3666, 1, &read_value);
    read_reg(bus, 0x366f, 1, &read_value);
    read_reg(bus, 0x3680, 1, &read_value);
    read_reg(bus, 0x3712, 1, &read_value);
    read_reg(bus, 0x372d, 1, &read_value);
    read_reg(bus, 0x3731, 1, &read_value);
    read_reg(bus, 0x3732, 1, &read_value);
    read_reg(bus, 0x3778, 1, &read_value);
    read_reg(bus, 0x377d, 1, &read_value);
    read_reg(bus, 0x3788, 1, &read_value);
    read_reg(bus, 0x3789, 1, &read_value);
    read_reg(bus, 0x378a, 1, &read_value);
    read_reg(bus, 0x378b, 1, &read_value);
    read_reg(bus, 0x3799, 1, &read_value);
    read_reg(bus, 0x3800, 1, &read_value);
    read_reg(bus, 0x3801, 1, &read_value);
    read_reg(bus, 0x3802, 1, &read_value);
    read_reg(bus, 0x3803, 1, &read_value);
    read_reg(bus, 0x3804, 1, &read_value);
    read_reg(bus, 0x3805, 1, &read_value);
    read_reg(bus, 0x3806, 1, &read_value);
    read_reg(bus, 0x3807, 1, &read_value);
    read_reg(bus, 0x3808, 1, &read_value);
    read_reg(bus, 0x3809, 1, &read_value);
    read_reg(bus, 0x380a, 1, &read_value);
    read_reg(bus, 0x380b, 1, &read_value);
    read_reg(bus, 0x380c, 1, &read_value);
    read_reg(bus, 0x380d, 1, &read_value);
    read_reg(bus, 0x380e, 1, &read_value);
    read_reg(bus, 0x380f, 1, &read_value);
    read_reg(bus, 0x3810, 1, &read_value);
    read_reg(bus, 0x3811, 1, &read_value);
    read_reg(bus, 0x3812, 1, &read_value);
    read_reg(bus, 0x3813, 1, &read_value);
    read_reg(bus, 0x3814, 1, &read_value);
    read_reg(bus, 0x3815, 1, &read_value);
    read_reg(bus, 0x3820, 1, &read_value);
    read_reg(bus, 0x3821, 1, &read_value);
    read_reg(bus, 0x3881, 1, &read_value);
    read_reg(bus, 0x38b1, 1, &read_value);
    read_reg(bus, 0x3920, 1, &read_value);
    read_reg(bus, 0x4003, 1, &read_value);
    read_reg(bus, 0x4008, 1, &read_value);
    read_reg(bus, 0x4009, 1, &read_value);
    read_reg(bus, 0x400c, 1, &read_value);
    read_reg(bus, 0x400d, 1, &read_value);
    read_reg(bus, 0x4010, 1, &read_value);
    read_reg(bus, 0x4043, 1, &read_value);
    read_reg(bus, 0x4307, 1, &read_value);
    read_reg(bus, 0x4317, 1, &read_value);
    read_reg(bus, 0x4501, 1, &read_value);
    read_reg(bus, 0x4507, 1, &read_value);
    read_reg(bus, 0x4509, 1, &read_value);
    read_reg(bus, 0x450a, 1, &read_value);
    read_reg(bus, 0x4601, 1, &read_value);
    read_reg(bus, 0x470f, 1, &read_value);
    read_reg(bus, 0x4f07, 1, &read_value);
    read_reg(bus, 0x4800, 1, &read_value);
    read_reg(bus, 0x5000, 1, &read_value);
    read_reg(bus, 0x5001, 1, &read_value);
    read_reg(bus, 0x5e00, 1, &read_value);
    read_reg(bus, 0x5d00, 1, &read_value);
    read_reg(bus, 0x5d01, 1, &read_value);

    return RT_EOK;
}




int rt_hw_ov9282_init(void)
{
    LOG_E("ov9282 i2c test start.........\n");
    i2c_bus = rt_i2c_bus_device_find(I2C_NAME);
    if (i2c_bus == RT_NULL)
    {
        LOG_E("can't find %s deivce", I2C_NAME);
        return RT_ERROR;
    }

    ov9282_reg_init(i2c_bus);

    LOG_E("ov9282 i2c test end.........\n");
    return 0;
}

INIT_COMPONENT_EXPORT(rt_hw_ov9282_init);

