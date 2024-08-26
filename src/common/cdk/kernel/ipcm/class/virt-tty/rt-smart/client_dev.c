#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "client.h"

#define DBG_TAG    "VIRT-TTY"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

static struct rt_device g_virt_tty_device = {0};
static int disable_dev_read = 0;
static struct rt_device_notify rx_notify;
/* RT-Thread Device Interface */

static rt_err_t rt_serial_init(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);

    rt_memset(&rx_notify, 0, sizeof(struct rt_device_notify));

    return RT_EOK;
}

static rt_err_t rt_serial_open(struct rt_device *dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_serial_close(struct rt_device *dev)
{
    return RT_EOK;
}

static rt_size_t rt_serial_read(struct rt_device *dev,
                                rt_off_t          pos,
                                void             *buffer,
                                rt_size_t         size)
{
    if (disable_dev_read)
		return 0;

    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    return virt_tty_client_read(buffer, size);
}

static rt_size_t rt_serial_write(struct rt_device *dev,
                                 rt_off_t          pos,
                                 const void       *buffer,
                                 rt_size_t         size)
{
    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    return virt_tty_client_send(buffer, size);
}

static rt_err_t rt_serial_control(struct rt_device *dev,
                                  int              cmd,
                                  void             *args)
{
    rt_err_t ret = RT_EOK;
    struct rt_serial_device *serial;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    switch (cmd)
    {
        case RT_DEVICE_CTRL_NOTIFY_SET:
            if (args)
            {
                rt_memcpy(&rx_notify, args, sizeof(struct rt_device_notify));
            }
            break;

        case RT_DEVICE_CTRL_CONSOLE_OFLAG:
            if (args)
            {
                *(rt_uint16_t*)args = RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM;
            }
            break;
        default :
            break;
    }

    return ret;
}

const static struct rt_device_ops serial_ops =
{
    rt_serial_init,
    rt_serial_open,
    rt_serial_close,
    rt_serial_read,
    rt_serial_write,
    rt_serial_control
};


/*
 * serial register
 */
int rt_virt_tty_device_init(void)
{
    rt_err_t ret;
    struct rt_device *device;

    device = &g_virt_tty_device;

    device->type        = RT_Device_Class_Char;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    device->ops         = &serial_ops;

    /* register a character device */
    ret = rt_device_register(device, "virt-tty", RT_DEVICE_FLAG_RDWR);
    
    /* set console device */
    rt_console_set_device("virt-tty");

    return ret;
}

void virt_tty_client_recv_notify(void)
{
	if (disable_dev_read)
		return;

	if (rx_notify.notify)
    {
        rx_notify.notify(rx_notify.dev);
    }
}
