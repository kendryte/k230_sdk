/*
 * Copyright (C) 2022, Canaan Bright Sight Co., Ltd
 *
 * All enquiries to https://www.canaan-creative.com/
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <rtdbg.h>
#include "utest.h"
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_gpio.h"
#define LED_PIN_NUM1            33
#define KEY1_PIN_NUM            32

static void gpio_input_output_test(void)
{
    int cnt = 20;
    int level;
    LOG_D("gpio input and output test, gpio33 output and gpio32 input, gpio32 will read the level.");
    /* led引脚为输出模式 */
    kd_pin_mode(LED_PIN_NUM1, GPIO_DM_OUTPUT);

    /* key引脚为输入模式 */
    kd_pin_mode(KEY1_PIN_NUM, GPIO_DM_INPUT);

    while(1)
    {
        kd_pin_write(LED_PIN_NUM1, GPIO_PV_LOW);
        level = kd_pin_read(KEY1_PIN_NUM);
        uassert_int_equal(level, GPIO_PV_LOW);

        kd_pin_write(LED_PIN_NUM1, GPIO_PV_HIGH);
        level = kd_pin_read(KEY1_PIN_NUM);
        uassert_int_equal(level, GPIO_PV_HIGH);

        rt_thread_mdelay(500);

        if(0 == cnt--)
            break;
    }
}

static void testcase(void)
{
    UTEST_UNIT_RUN(gpio_input_output_test);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

UTEST_TC_EXPORT(testcase, "testcases.kernel.gpio_read_write_test", utest_tc_init, utest_tc_cleanup, 100);

