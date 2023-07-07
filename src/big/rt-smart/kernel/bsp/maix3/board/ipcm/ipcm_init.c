#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

rt_int32_t __attribute__((weak)) _ipcm_vdd_init(void)
{
    rt_kprintf("no ipcm library!\n");
    return RT_EOK;
}

extern rt_int32_t _ipcm_vdd_init(void);
rt_int32_t ipcm_module_init(void)
{
    return _ipcm_vdd_init();
}
