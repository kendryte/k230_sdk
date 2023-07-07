#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>



extern rt_int32_t ipcm_module_init(void);
extern rt_int32_t mpp_init(void);

/*sdk kernel space init*/
rt_int32_t sdk_kernel_init(void)
{
    ipcm_module_init();
#if 0
    interdrv_init();
    extdrv_init();
#endif
    mpp_init();
    return RT_EOK;
}

INIT_COMPONENT_EXPORT(sdk_kernel_init);