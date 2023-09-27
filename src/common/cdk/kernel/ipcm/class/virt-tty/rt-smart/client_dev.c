#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "drivers/serial.h"
#include "client.h"

static int disable_dev_read = 0;
#ifdef __VIRT_TTY_DEV__

#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#include <posix_termios.h>
#endif

#ifdef RT_USING_USERSPACE
#include <lwp_user_mm.h>
#endif

#define UART_DEFAULT_BAUDRATE               115200

static rt_err_t  virt_tty_configure(struct rt_serial_device *serial, struct serial_configure *cfg);
static rt_err_t virt_tty_control(struct rt_serial_device *serial, int cmd, void *arg);
static rt_size_t virt_tty_trans(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction);

static const struct rt_uart_ops _virt_tty_ops =
{
    virt_tty_configure,
    virt_tty_control,
	RT_NULL,
	RT_NULL,
	virt_tty_trans,
};

static struct rt_serial_device  virt_tty_serial;

/*
 * UART interface
 */
static rt_err_t virt_tty_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct device_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    serial->config = *cfg;

    return (RT_EOK);
}

static rt_err_t virt_tty_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct device_uart *uart;

    uart = serial->parent.user_data;
    rt_uint32_t channel = 1;

    RT_ASSERT(uart != RT_NULL);
    RT_ASSERT(channel != 3);

    return (RT_EOK);
}

static rt_size_t virt_tty_trans(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
	switch (direction) {
	case RT_SERIAL_DMA_RX:
		return virt_tty_client_read(buf, size);
	case RT_SERIAL_DMA_TX:
		return virt_tty_client_send(buf, size);
	default:
		return 0;
	}
	return 0;
}

/*
 * UART Initiation
 */
int virt_tty_uart_init(void)
{
    struct rt_serial_device *serial;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

	if (virt_tty_client_init()) {
		return -1;
	}

    {
        serial  = &virt_tty_serial;
        serial->ops              = &_virt_tty_ops;
        serial->config           = config;
        serial->config.baud_rate = UART_DEFAULT_BAUDRATE;

        rt_hw_serial_register(serial,
                              "virt-tty",
                              RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              RT_NULL);
    }

    return 0;
}

#endif