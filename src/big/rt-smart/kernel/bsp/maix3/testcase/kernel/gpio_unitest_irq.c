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
static int cnt;
static char retn;
static char on;
void key_irq(void *args)
{
    cnt++;
    if(!on)
    {
        LOG_W("turn on led!\n");
        kd_pin_write(LED_PIN_NUM1, GPIO_PV_LOW);
        on = 1;
    } else {
        LOG_W("turn off led!\n");
        kd_pin_write(LED_PIN_NUM1, GPIO_PV_HIGH);
        on = 0;
    }
    if (cnt >= 10) {
        retn = 0;
        cnt = 0;
    }
}


static void gpio_key_led_int(void)
{
    retn = 1;
    LOG_W("gpio test irq falling\n");
    /* led引脚为输出模式 */
    kd_pin_mode(LED_PIN_NUM1, GPIO_DM_OUTPUT);
    /* 默认高电平 */
    kd_pin_write(LED_PIN_NUM1, GPIO_PV_HIGH);

    /* 按键1引脚为输入模式 */
    kd_pin_mode(KEY1_PIN_NUM, GPIO_DM_INPUT);
    /* 绑定中断，上升沿模式，回调函数名为 key_irq */
    kd_pin_attach_irq(KEY1_PIN_NUM,GPIO_PE_FALLING, key_irq, RT_NULL);
    /* 使能中断 */
    kd_pin_irq_enable(KEY1_PIN_NUM, KD_GPIO_IRQ_ENABLE);
    while(retn);
    kd_pin_detach_irq(KEY1_PIN_NUM);
}

static void gpio_key_led_int1(void)
{
    retn = 1;
    LOG_W("gpio test irq rising\n");
    /* led引脚为输出模式 */
    kd_pin_mode(LED_PIN_NUM1, GPIO_DM_OUTPUT);
    /* 默认低电平 */
    kd_pin_write(LED_PIN_NUM1, GPIO_PV_LOW);

    /* 按键1引脚为输入模式 */
    kd_pin_mode(KEY1_PIN_NUM, GPIO_DM_INPUT);
    /* 绑定中断，上升沿模式，回调函数名为 key_irq */
    kd_pin_attach_irq(KEY1_PIN_NUM,GPIO_PE_RISING, key_irq, RT_NULL);
    /* 使能中断 */
    kd_pin_irq_enable(KEY1_PIN_NUM, KD_GPIO_IRQ_ENABLE);
    while(retn);
    kd_pin_detach_irq(KEY1_PIN_NUM);
}

static void gpio_key_led_int2(void)
{
    retn = 1;
    LOG_W("gpio test irq both\n");
    /* led引脚为输出模式 */
    kd_pin_mode(LED_PIN_NUM1, GPIO_DM_OUTPUT);
    /* 默认低电平 */
    kd_pin_write(LED_PIN_NUM1, GPIO_PV_LOW);

    /* 按键1引脚为输入模式 */
    kd_pin_mode(KEY1_PIN_NUM, GPIO_DM_INPUT);
    /* 绑定中断，上升沿模式，回调函数名为 key_irq */
    kd_pin_attach_irq(KEY1_PIN_NUM,GPIO_PE_BOTH, key_irq, RT_NULL);
    /* 使能中断 */
    kd_pin_irq_enable(KEY1_PIN_NUM, KD_GPIO_IRQ_ENABLE);
    while(retn);
    kd_pin_detach_irq(KEY1_PIN_NUM);
}

static void testcase(void)
{
    UTEST_UNIT_RUN(gpio_key_led_int);
    UTEST_UNIT_RUN(gpio_key_led_int1);
    UTEST_UNIT_RUN(gpio_key_led_int2);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

UTEST_TC_EXPORT(testcase, "testcases.kernel.gpio_irq_test", utest_tc_init, utest_tc_cleanup, 100);
