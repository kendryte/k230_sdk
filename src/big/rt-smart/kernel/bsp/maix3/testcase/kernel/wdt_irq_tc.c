#include <rtthread.h>
#include <rtdevice.h>
#include <drv_wdt.h>
#include "utest.h"

#define WDT_DEVICE_NAME    "watchdog1"    /* 看门狗设备名称 */
static rt_device_t wdt_dev;         /* 看门狗设备句柄 */
static void idle_hook(void)
{
    /* 在空闲线程的回调函数里喂狗 */
    rt_device_control(wdt_dev, KD_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
    LOG_D("feed dog!\n ");
}
static void wdt_sample(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t timeout = 5, i;    /* 溢出时间 */

    /* 根据设备名称查找看门狗设备，获取设备句柄 */
    wdt_dev = rt_device_find(WDT_DEVICE_NAME);
    if (!wdt_dev)
    {
        LOG_E("find watchdog failed!\n");
        return;
    }
    /* 初始化设备 */
    ret = rt_device_init(wdt_dev);
    if (ret != RT_EOK)
    {
        LOG_E("initialize watchdog failed!\n");
        return;
    }
    /* 设置看门狗溢出时间 */
    ret = rt_device_control(wdt_dev, KD_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (ret != RT_EOK)
    {
        LOG_E("set watchdog timeout failed!\n");
        return;
    }
    LOG_D("set timeout is %d\n", timeout);

    timeout = 0;
    rt_device_control(wdt_dev, KD_DEVICE_CTRL_WDT_GET_TIMEOUT, &timeout);
    LOG_D("get timeout is %d\n", timeout);

    rt_device_control(wdt_dev, KD_DEVICE_CTRL_WDT_START, RT_NULL);

    /* 设置空闲线程回调函数 */
    // rt_thread_idle_sethook(idle_hook);
    for (i = 0; i < 5; i++) {
        idle_hook();
    }

    LOG_D("wait for system reboot");

    while(1)
    {
        LOG_D(".");
        rt_thread_delay(200);
    }
    return;
}

/* 导出到 testcase 命令列表中 */
static void testcase(void)
{
    UTEST_UNIT_RUN(wdt_sample);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.watchdog_test", utest_tc_init, utest_tc_cleanup, 10);
