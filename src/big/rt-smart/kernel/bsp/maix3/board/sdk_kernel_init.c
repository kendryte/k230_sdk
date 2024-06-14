#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

extern rt_int32_t ipcm_module_init(void);
extern rt_int32_t mpp_init(void);

/*sdk kernel space init*/
rt_int32_t sdk_kernel_init(void)
{
#ifdef RT_USING_IPCM
    ipcm_module_init();
#endif

#ifdef RT_USING_MPP
    mpp_init();
#endif
    return RT_EOK;
}

INIT_COMPONENT_EXPORT(sdk_kernel_init);