/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include <drivers/spi.h>

#define RT_SPI_DEV_CTRL_CONFIG       (RT_DEVICE_CTRL_BASE(SPIBUS) + 0x01)
#define RT_SPI_DEV_CTRL_RW           (RT_DEVICE_CTRL_BASE(SPIBUS) + 0x02)
#define RT_SPI_DEV_CTRL_CLK          (RT_DEVICE_CTRL_BASE(SPIBUS) + 0x03)
#define SPI_BUS_NAME                "spi1"
#define SPI1_DEVICE                  "spi_dev"
struct rt_spi_device *spi_device;

struct rt_spi_priv_data {
    const void           *send_buf;
    rt_size_t             send_length;
    void                 *recv_buf;
    rt_size_t             recv_length;
};

/* SPI bus device interface, compatible with RT-Thread 0.3.x/1.0.x */
static rt_size_t _spi_bus_device_read(rt_device_t dev,
                                      rt_off_t    pos,
                                      void       *buffer,
                                      rt_size_t   size)
{
    struct rt_spi_bus *bus;

    bus = (struct rt_spi_bus *)dev;
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(bus->owner != RT_NULL);

    return rt_spi_transfer(bus->owner, RT_NULL, buffer, size);
}

static rt_size_t _spi_bus_device_write(rt_device_t dev,
                                       rt_off_t    pos,
                                       const void *buffer,
                                       rt_size_t   size)
{
    struct rt_spi_bus *bus;

    bus = (struct rt_spi_bus *)dev;
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(bus->owner != RT_NULL);

    return rt_spi_transfer(bus->owner, buffer, RT_NULL, size);
}

static rt_err_t _spi_bus_device_control(rt_device_t dev,
                                        int         cmd,
                                        void       *args)
{
    struct rt_spi_bus *bus;

    bus = (struct rt_spi_bus *)dev;
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(bus->owner != RT_NULL);
    struct rt_spi_priv_data *priv_data;

    /* TODO: add control command handle */
    switch (cmd)
    {
        case RT_SPI_DEV_CTRL_CONFIG:
            bus->owner->config = *(struct rt_spi_configuration *)args;
            rt_spi_configure(bus->owner, args);
            break;
        case RT_SPI_DEV_CTRL_RW: /* set device */
            priv_data = (struct rt_spi_priv_data *)args;
            rt_spi_send_then_recv(bus->owner, priv_data->send_buf, priv_data->send_length,
                                priv_data->recv_buf, priv_data->recv_length);
            break;
        default:
            break;
    }

    return RT_EOK;
}

rt_err_t  _spi_bus_device_open(rt_device_t dev, rt_uint16_t oflag)
{

    struct rt_spi_bus *bus;
    rt_err_t res;
    struct rt_spi_configuration cfg = {
        .mode = 1,
        .data_width = 8,
        .max_hz = 20,
    };

    spi_device = rt_malloc(sizeof(struct rt_spi_device ));

    res = rt_spi_bus_attach_device(spi_device, SPI1_DEVICE, SPI_BUS_NAME, spi_device);
    if (res != RT_EOK)
    {
        rt_kprintf("rt_spi_bus_attach_device() failed!\n");
        return res;
    }

    spi_device->bus->owner = spi_device;

    return 0;
}

rt_err_t  _spi_bus_device_close(rt_device_t dev)
{
    rt_err_t res;
    struct rt_spi_bus *bus = (struct rt_spi_bus *)dev;

    res = rt_device_unregister(&bus->owner->parent);
    if (res != RT_EOK)
    {
        rt_kprintf("device unregister failed!\n");
        return res;
    }
    spi_device->bus->owner = NULL;
    rt_free(spi_device);

    return 0;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops spi_bus_ops = 
{
    RT_NULL,
    _spi_bus_device_open,
    _spi_bus_device_close,
    _spi_bus_device_read,
    _spi_bus_device_write,
    _spi_bus_device_control
};
#endif

rt_err_t rt_spi_bus_device_init(struct rt_spi_bus *bus, const char *name)
{
    struct rt_device *device;
    RT_ASSERT(bus != RT_NULL);

    device = &bus->parent;

    /* set device type */
    device->type    = RT_Device_Class_SPIBUS;
    /* initialize device interface */
#ifdef RT_USING_DEVICE_OPS
    device->ops     = &spi_bus_ops;
#else
    device->init    = RT_NULL;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = _spi_bus_device_read;
    device->write   = _spi_bus_device_write;
    device->control = _spi_bus_device_control;
#endif

    /* register to device manager */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
}

/* SPI Dev device interface, compatible with RT-Thread 0.3.x/1.0.x */
static rt_size_t _spidev_device_read(rt_device_t dev,
                                     rt_off_t    pos,
                                     void       *buffer,
                                     rt_size_t   size)
{
    struct rt_spi_device *device;

    device = (struct rt_spi_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    return rt_spi_transfer(device, RT_NULL, buffer, size);
}

static rt_size_t _spidev_device_write(rt_device_t dev,
                                      rt_off_t    pos,
                                      const void *buffer,
                                      rt_size_t   size)
{
    struct rt_spi_device *device;

    device = (struct rt_spi_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    return rt_spi_transfer(device, buffer, RT_NULL, size);
}

static rt_err_t _spidev_device_control(rt_device_t dev,
                                       int         cmd,
                                       void       *args)
{
    switch (cmd)
    {
    case 0: /* set device */
        break;
    case 1: 
        break;
    }

    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops spi_device_ops = 
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    _spidev_device_read,
    _spidev_device_write,
    _spidev_device_control
};
#endif

rt_err_t rt_spidev_device_init(struct rt_spi_device *dev, const char *name)
{
    struct rt_device *device;
    RT_ASSERT(dev != RT_NULL);

    device = &(dev->parent);

    /* set device type */
    device->type    = RT_Device_Class_SPIDevice;
#ifdef RT_USING_DEVICE_OPS
    device->ops     = &spi_device_ops;
#else
    device->init    = RT_NULL;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = _spidev_device_read;
    device->write   = _spidev_device_write;
    device->control = _spidev_device_control;
#endif

    /* register to device manager */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
}
