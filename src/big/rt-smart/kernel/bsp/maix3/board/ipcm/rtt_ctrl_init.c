#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

int __attribute__((weak)) rc_server_init()
{
    rt_kprintf("no rtt ctrl device library!\n");
    return RT_EOK;
}
