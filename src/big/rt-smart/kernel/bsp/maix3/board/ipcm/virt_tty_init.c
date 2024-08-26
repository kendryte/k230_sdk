#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

int __attribute__((weak)) rt_virt_tty_device_init()
{
    rt_kprintf("no virt tty device library!\n");
    return RT_EOK;
}

int __attribute__((weak)) virt_tty_client_init(void)
{
    rt_kprintf("no virt tty client library!\n");
    return RT_EOK;
}