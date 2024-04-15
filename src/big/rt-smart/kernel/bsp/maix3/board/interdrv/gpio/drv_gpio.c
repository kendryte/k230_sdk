/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <rtdevice.h>
#include <riscv_io.h>
#include <rtdef.h>
#include "ioremap.h"
#include "drv_gpio.h"
#include "drv_hardlock.h"
#include "board.h"
#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#include <posix_termios.h>
#endif
#include <rtdbg.h>

#define DBG_TAG  "GPIO"
#ifdef RT_DEBUG
#define DBG_LVL          DBG_LOG
#else
#define DBG_LVL          DBG_WARNING
#endif
#define DBG_COLOR

static struct rt_device g_gpio_device = {0};
static char *kd_gpio[2];
static int hardlock;
static struct
{
    void (*hdr)(void *args);
    void* args;
    gpio_pin_edge_t edge;
} irq_table[GPIO_MAX_NUM];

struct rt_device_gpio
{
    rt_uint16_t pin;
    rt_uint16_t value;
};

static void kd_gpio_reg_writel(volatile char *reg, rt_size_t offset, rt_uint32_t value)
{
    /* hardlock try lock */
    while(0 != kd_hardlock_lock(hardlock));

    rt_uint32_t val = readl(reg);
    val &= ~(1 << offset);
    val |= (value << offset);
    writel(val, reg);
    kd_hardlock_unlock(hardlock);
}

static rt_uint32_t kd_gpio_reg_readl(volatile char *reg, rt_size_t offset)
{
    rt_uint32_t val = readl(reg);
    return (val & (1 << offset)) >> offset;
}

static int check_pin_valid(rt_base_t pin)
{
    if((rt_uint16_t)pin < 0 || (rt_uint16_t)pin > GPIO_MAX_NUM)
    {
        LOG_E("pin %d is not valid\n", pin);
        return -RT_ERROR;
    }
    return pin;
}

static int kd_set_drive_mode(rt_base_t pin, rt_base_t mode)
{
    uint32_t dir;
    switch (mode)
    {
        case GPIO_DM_INPUT:
            dir = 0;
            break;
        case GPIO_DM_INPUT_PULL_DOWN:
            dir = 0;
            break;
        case GPIO_DM_INPUT_PULL_UP:
            dir = 0;
            break;
        case GPIO_DM_OUTPUT:
            dir = 1;
            break;
        default:
            LOG_E("GPIO drive mode is not supported.");
            break;
    }

    if(pin < 32)
    {
        kd_gpio_reg_writel(kd_gpio[0] + DIRECTION, pin, dir);
    } else {
        pin -= 32;
        if(pin < 32)
            kd_gpio_reg_writel(kd_gpio[1] + DIRECTION, pin, dir);
        else {
            pin -= 32;
            kd_gpio_reg_writel(kd_gpio[1] + DIRECTION + DIRECTION_STRIDE, pin, dir);
        }
    }

    return RT_EOK;
}

static rt_base_t kd_get_drive_mode(rt_base_t pin)
{
    int ret = check_pin_valid(pin);
    if(ret == -1)   return -RT_ERROR;

    if(pin < 32)
        return kd_gpio_reg_readl(kd_gpio[0] + DIRECTION, pin);
    else {
        pin -= 32;
        if(pin < 32)
            return kd_gpio_reg_readl(kd_gpio[1] + DIRECTION, pin);
        else {
            pin -= 32;
            return kd_gpio_reg_readl(kd_gpio[1] + DIRECTION + DIRECTION_STRIDE, pin);
        }
    }
}

void kd_pin_mode(rt_base_t pin, rt_base_t mode)
{
    int ret = check_pin_valid(pin);
    if(ret == -1)   return;

    kd_set_drive_mode(pin, mode);
}

void kd_pin_write(rt_base_t pin, rt_base_t value)
{
    int ret = check_pin_valid(pin);
    if(ret == -1)   return;

    if(kd_get_drive_mode(pin) == 0)
    {
        LOG_E("pin %d is input mode, not write it", pin);
        return;
    }

    if(pin < 32)
        kd_gpio_reg_writel(kd_gpio[0] + DATA_OUTPUT, pin, value == KD_GPIO_HIGH ? GPIO_PV_HIGH : GPIO_PV_LOW);
    else {
        pin -= 32;
        if(pin < 32)
            kd_gpio_reg_writel(kd_gpio[1] + DATA_OUTPUT, pin, value == KD_GPIO_HIGH ? GPIO_PV_HIGH : GPIO_PV_LOW);
        else {
            pin -= 32;
            kd_gpio_reg_writel(kd_gpio[1] + DATA_OUTPUT + DATA_OUTPUT_STRIDE, pin, value == KD_GPIO_HIGH ? GPIO_PV_HIGH : GPIO_PV_LOW);
        }
    }
}

int kd_pin_read(rt_base_t pin)
{
    int ret = check_pin_valid(pin);
    if(ret == -1)
    {
        LOG_E("pin %d not valid for read", pin);
        return -RT_ERROR;
    }
    if(pin < 32)
        return kd_gpio_reg_readl(kd_gpio[0] + DATA_INPUT, pin) == GPIO_PV_HIGH ? PIN_HIGH : PIN_LOW;
    else {
        pin -= 32;
        if(pin < 32)
            return kd_gpio_reg_readl(kd_gpio[1] + DATA_INPUT, pin) == GPIO_PV_HIGH ? PIN_HIGH : PIN_LOW;
        else {
            pin -= 32;
            return kd_gpio_reg_readl(kd_gpio[1] + DATA_INPUT + DATA_INPUT_STRIDE, pin) == GPIO_PV_HIGH ? PIN_HIGH : PIN_LOW;
        }
    }
}

static void kd_set_pin_edge(rt_int32_t pin, gpio_pin_edge_t edge)
{
    char *reg;

    if(pin < 32)
        reg = kd_gpio[0];
    else {
        pin -= 32;
        if(pin < 32)
            reg = kd_gpio[1];
        else {
            LOG_E("pin %d not support interrupt", pin+32);
        }
    }

    switch (edge)
    {
        case GPIO_PE_RISING:
            kd_gpio_reg_writel(reg + INT_TYPE_LEVEL, pin, 0x1);
            kd_gpio_reg_writel(reg + INT_POLARITY, pin, 0x1);
            kd_gpio_reg_writel(reg + INT_BOTHEDGE, pin, 0x0);
            break;
        case GPIO_PE_FALLING:
            kd_gpio_reg_writel(reg + INT_TYPE_LEVEL, pin, 0x1);
            kd_gpio_reg_writel(reg + INT_POLARITY, pin, 0x0);
            kd_gpio_reg_writel(reg + INT_BOTHEDGE, pin, 0x0);
            break;
        case GPIO_PE_BOTH:
            // kd_gpio_reg_writel(reg + INT_TYPE_LEVEL, pin, 0x1);
            // kd_gpio_reg_writel(reg + INT_POLARITY, pin, 0x0);
            kd_gpio_reg_writel(reg + INT_BOTHEDGE, pin, 0x1);
            break;
        case GPIO_PE_LOW:
            kd_gpio_reg_writel(reg + INT_TYPE_LEVEL, pin, 0x0);
            kd_gpio_reg_writel(reg + INT_POLARITY, pin, 0x0);
            kd_gpio_reg_writel(reg + INT_BOTHEDGE, pin, 0x0);
            break;
        case GPIO_PE_HIGH:
            kd_gpio_reg_writel(reg + INT_TYPE_LEVEL, pin, 0x0);
            kd_gpio_reg_writel(reg + INT_POLARITY, pin, 0x1);
            kd_gpio_reg_writel(reg + INT_BOTHEDGE, pin, 0x0);
            break;
        default:
            break;
    }
    /* enable gpio interrupte */
    kd_gpio_reg_writel(reg + INT_ENABLE, pin, 0x1);
}

static void pin_irq(int vector, void *param)
{
    int pin = vector - IRQN_GPIO0_INTERRUPT;
    int pin_id = pin;
    char *reg;
    gpio_pin_edge_t edge = irq_table[pin_id].edge;
    if(pin < 32)
        reg = kd_gpio[0];
    else {
        pin -= 32;
        if(pin < 32)
            reg = kd_gpio[1];
        else {
            LOG_E("pin %d not support interrupt", pin+32);
        }
    }

    switch (edge)
    {
        case GPIO_PE_RISING:
        case GPIO_PE_FALLING:
        case GPIO_PE_BOTH:
            kd_gpio_reg_writel(reg + INT_CLEAR, pin, 0x1);
            break;
        case GPIO_PE_LOW:
        case GPIO_PE_HIGH:
            kd_gpio_reg_writel(reg + INT_MASK, pin, 0x1);  //mask level interrupt
            break;
        default:
            break;
    }

    if(irq_table[pin_id].hdr)
    {
        irq_table[pin_id].hdr(irq_table[pin_id].args);  //run the target irq handler
    }

    switch (edge)
    {
        case GPIO_PE_LOW:
        case GPIO_PE_HIGH:
            kd_gpio_reg_writel(reg + INT_MASK, pin, 0x0);  //unmask level interrupt
            break;
        default:
            break;
    }
}

rt_err_t kd_pin_attach_irq(rt_int32_t pin,rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    char irq_name[10];
    int ret = check_pin_valid(pin);
    if(ret == -1)
    {
        LOG_E("pin %d not valid for attach irq", pin);
        return -RT_ERROR;
    }

    irq_table[pin].hdr = hdr;
    irq_table[pin].args = args;

    if(mode < 0 || mode > 4)
        return -RT_ERROR;
    irq_table[pin].edge = mode;

    kd_set_pin_edge(pin, irq_table[pin].edge);

    rt_snprintf(irq_name, sizeof irq_name, "pin%d", pin);
    rt_hw_interrupt_install(IRQN_GPIO0_INTERRUPT + pin, pin_irq, RT_NULL, irq_name);

    return RT_EOK;
}

rt_err_t kd_pin_detach_irq(rt_int32_t pin)
{
    char *reg;
    int ret = check_pin_valid(pin);
    int pin_id = pin;
    if(ret == -1)
    {
        LOG_E("pin %d not valid for attach irq", pin);
        return -RT_ERROR;
    }

    if(pin < 32)
        reg = kd_gpio[0];
    else {
        pin -= 32;
        if(pin < 32)
            reg = kd_gpio[1];
        else {
            LOG_E("pin %d not support interrupt", pin+32);
        }
    }

    irq_table[pin_id].hdr = RT_NULL;
    irq_table[pin_id].args = RT_NULL;

    /* disable gpio interrupte */
    kd_gpio_reg_writel(reg + INT_ENABLE, pin, 0x0);

    return ret;
}

rt_err_t kd_pin_irq_enable(rt_base_t pin, rt_uint32_t enabled)
{
    int ret = check_pin_valid(pin);
    if(ret == -1)
    {
        LOG_E("pin %d not valid for attach irq", pin);
        return -RT_ERROR;
    }

    if(enabled)
    {
        rt_hw_interrupt_umask(IRQN_GPIO0_INTERRUPT + pin);
    }
    else
    {
        rt_hw_interrupt_mask(IRQN_GPIO0_INTERRUPT + pin);
    }

    return RT_EOK;
}

static rt_err_t  gpio_device_ioctl(rt_device_t dev, int cmd, void *args)
{
    struct rt_device_gpio *mode;

    /* check parameters */
    RT_ASSERT(dev != RT_NULL);

    mode = (struct rt_device_gpio *) args;
    if (mode == RT_NULL) return -RT_ERROR;

    switch (cmd)
    {
        /* kendryte gpio set direction */
        case KD_GPIO_DM_OUTPUT:
            kd_pin_mode((rt_base_t)mode->pin, GPIO_DM_OUTPUT);
            break;
        case KD_GPIO_DM_INPUT:
            kd_pin_mode((rt_base_t)mode->pin, GPIO_DM_INPUT);
            break;
        case KD_GPIO_DM_INPUT_PULL_UP:
            kd_pin_mode((rt_base_t)mode->pin, GPIO_DM_INPUT_PULL_UP);
            break;
        case KD_GPIO_DM_INPUT_PULL_DOWN:
            kd_pin_mode((rt_base_t)mode->pin, GPIO_DM_INPUT_PULL_DOWN);
            break;
        /* kendryte gpio set port value */
        case KD_GPIO_WRITE_LOW:
            kd_pin_write((rt_base_t)mode->pin, GPIO_PV_LOW);
            break;
        case KD_GPIO_WRITE_HIGH:
            kd_pin_write((rt_base_t)mode->pin, GPIO_PV_HIGH);
            break;
        case KD_GPIO_READ_VALUE:
            mode->value = kd_pin_read((rt_base_t)mode->pin);
            break;
        default:
            LOG_E("[ %s ] : cmd is not valid\n", __func__);
            return -RT_EINVAL;
    }

    return RT_EOK;
}

const static struct rt_device_ops gpio_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    gpio_device_ioctl
};

int rt_hw_gpio_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_device_t gpio_device = &g_gpio_device;

    kd_gpio[0] = rt_ioremap((void *)GPIO0_BASE_ADDR, GPIO0_IO_SIZE);
    kd_gpio[1] = rt_ioremap((void *)GPIO1_BASE_ADDR, GPIO1_IO_SIZE);

    ret = rt_device_register(gpio_device, "gpio", RT_DEVICE_FLAG_RDWR);
#ifndef RT_FASTBOOT
    if(!ret)
        rt_kprintf("canaan gpio driver register OK\n");
#endif
    gpio_device->ops = &gpio_ops;

    if(kd_request_lock(HARDLOCK_GPIO)) {
        rt_kprintf("fail to request hardlock-%d\n", HARDLOCK_GPIO);
        hardlock = -1;
    } else
        hardlock = HARDLOCK_GPIO;

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_gpio_init);
