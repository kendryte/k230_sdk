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
#include <rtdevice.h>
#include "riscv_io.h"
#include "board.h"
#include "ioremap.h"
#include <rtdbg.h>
#include <stdbool.h>
#include <stdio.h>
#include <rthw.h>
#include "sysctl_clk.h"
#include "sysctl_rst.h"
#include "drv_timer.h"
#include <dfs_posix.h>
#include <lwp_user_mm.h>

struct kd_timer_Type {
    kendryte_timer_t* base_addr;
    uint32_t id;
    struct rt_work send_sig_work;
    int pid;
    int signo;
    void* sigval;
};

static void kd_timer_clk_set(rt_hwtimer_t* timer)
{
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;
    uint8_t id = kd_timer->id;
    sysctl_clk_node_e clk, src;
    uint32_t div;

    if (id >= 0 && id <= 5) {
        clk = SYSCTL_CLK_TIMER_0_CLK + id * 2;
        src = SYSCTL_CLK_TIMER_0_SRC + id * 2;

        sysctl_clk_set_leaf_parent(clk, src);
        if (timer->freq < timer->info->minfreq) {
            timer->freq = timer->info->minfreq;
            sysctl_clk_set_leaf_div(src, 1, 8);
        } else if (timer->freq > timer->info->maxfreq) {
            timer->freq = timer->info->maxfreq;
            sysctl_clk_set_leaf_div(src, 1, 1);
        } else {
            div = timer->info->maxfreq / (timer->freq + 1) + 1;
            sysctl_clk_set_leaf_div(src, 1, div);
            timer->freq = timer->info->maxfreq / div;
        }
    }
}

static void kd_timer_init(rt_hwtimer_t* timer, rt_uint32_t state)
{
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;

    if (state == 0) {
        timer->ops->stop(timer);
    } else if (state == 1) {
        kd_timer_clk_set(timer);
    }
}

static rt_err_t kd_timer_start(rt_hwtimer_t* timer, rt_uint32_t cnt, rt_hwtimer_mode_t mode)
{
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;
    uint8_t id = kd_timer->id;
    kendryte_timer_t* reg = kd_timer->base_addr;

    reg->channel[id].load_count = cnt;
    reg->channel[id].control &= ~(TIMER_CR_INTERRUPT_MASK);
    reg->channel[id].control |= (TIMER_CR_USER_MODE | TIMER_CR_ENABLE);

    return RT_EOK;
}

static void kd_timer_stop(rt_hwtimer_t* timer)
{
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;
    uint8_t id = kd_timer->id;
    kendryte_timer_t* reg = kd_timer->base_addr;

    reg->channel[id].control &= ~TIMER_CR_ENABLE;
    reg->channel[id].control |= TIMER_CR_INTERRUPT_MASK;
}

static rt_uint32_t kd_timer_get(rt_hwtimer_t* timer)
{
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;
    uint8_t id = kd_timer->id;
    kendryte_timer_t* reg = kd_timer->base_addr;

    return reg->channel[id].current_value;
}

static void send_sig_work(struct rt_work* work, void* param)
{
    siginfo_t info;
    struct kd_timer_Type* kd_timer = param;

    rt_memset(&info, 0, sizeof(info));
    info.si_code = SI_TIMER;
    info.si_ptr = kd_timer->sigval;
    lwp_kill_ext(kd_timer->pid, kd_timer->signo, &info);
}

static rt_err_t timer_timeout_ind(rt_device_t dev, rt_size_t size)
{
    rt_hwtimer_t* timer = (rt_hwtimer_t*)dev;
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;

    rt_work_init(&kd_timer->send_sig_work, send_sig_work, kd_timer);
    rt_work_submit(&kd_timer->send_sig_work, 0);
}

static rt_err_t kd_timer_ctrl(rt_hwtimer_t* timer, rt_uint32_t cmd, void* arg)
{
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;

    if (cmd == HWTIMER_CTRL_FREQ_SET) {
        timer->freq = *((rt_uint32_t*)arg);
        kd_timer_clk_set(timer);
    }

    return RT_EOK;
}

static int timer_fops_open(struct dfs_fd* fd)
{
    rt_device_t device = (rt_device_t)fd->fnode->data;
    int ret = rt_device_open(device, RT_DEVICE_OFLAG_RDWR);

    return ret;
}

static int timer_fops_close(struct dfs_fd* fd)
{
    rt_device_t device = (rt_device_t)fd->fnode->data;
    int ret = rt_device_close(device);

    return ret;
}

static int timer_fops_ioctl(struct dfs_fd* fd, int cmd, void* args)
{
    rt_device_t device = (rt_device_t)fd->fnode->data;
    int ret;

    cmd &= 0xFF;
    ret = rt_device_control(device, cmd, args);

    if (cmd == HWTIMER_CTRL_IRQ_SET) {
        rt_hwtimer_t* timer = (rt_hwtimer_t*)device;
        struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)timer->parent.user_data;
        hwtimer_irqcfg_t cfg;
        rt_base_t level;

        lwp_get_from_user(&cfg, args, sizeof(cfg));
        level = rt_hw_interrupt_disable();
        if (cfg.enable) {
            kd_timer->pid = lwp_getpid();
            kd_timer->signo = cfg.signo;
            kd_timer->sigval = cfg.sigval;
            rt_device_set_rx_indicate(&timer->parent, timer_timeout_ind);
        } else {
            rt_device_set_rx_indicate(&timer->parent, NULL);
        }
        rt_hw_interrupt_enable(level);
        ret = RT_EOK;
    }

    return ret;
}

static int timer_fops_read(struct dfs_fd* fd, void* buf, size_t count)
{
    rt_device_t device = (rt_device_t)fd->fnode->data;
    int ret = rt_device_read(device, 0, buf, count);

    return ret;
}

static int timer_fops_write(struct dfs_fd* fd, const void* buf, size_t count)
{
    rt_device_t device = (rt_device_t)fd->fnode->data;
    int ret = rt_device_write(device, 0, buf, count);

    return ret;
}

static const struct rt_hwtimer_info _info = {
    100000000, /* the maximum count frequency can be set */
    12500000, /* the minimum count frequency can be set */
    0xFFFFFFFF, /* the maximum counter value */
    HWTIMER_CNTMODE_DW, /* Increment or Decreasing count mode */
};

static const struct rt_hwtimer_ops _ops = {
    .init = kd_timer_init,
    .start = kd_timer_start,
    .stop = kd_timer_stop,
    .count_get = kd_timer_get,
    .control = kd_timer_ctrl,
};

static const struct dfs_file_ops timer_fops = {
    timer_fops_open,
    timer_fops_close,
    timer_fops_ioctl,
    timer_fops_read,
    timer_fops_write,
};

void kd_hwtimer_isr(int vector, void* param)
{
    uint32_t ret;
    rt_hwtimer_t* hwtimer = (rt_hwtimer_t*)param;
    struct kd_timer_Type* kd_timer = (struct kd_timer_Type*)hwtimer->parent.user_data;

    RT_ASSERT(kd_timer != RT_NULL);

    int id = kd_timer->id;
    kendryte_timer_t* reg = kd_timer->base_addr;

    ret = (reg->channel[id].eoi);

    rt_device_hwtimer_isr(hwtimer);
}

int rt_hw_timer_init(void)
{
    volatile void* addr = (void*)rt_ioremap((void*)HW_TIMER_BASE_ADDR, HW_TIMER_IO_SIZE);

#ifdef RT_USING_HW_TIMER0
    static rt_hwtimer_t _timer0;
    static struct kd_timer_Type kd_timer0;

    kd_timer0.base_addr = (kendryte_timer_t*)addr;
    kd_timer0.id = 0;

    _timer0.info = &_info;
    _timer0.ops = &_ops;
    rt_device_hwtimer_register(&_timer0, "hwtimer0", &kd_timer0);
    _timer0.parent.fops = &timer_fops;
    rt_hw_interrupt_install(IRQN_TIMER_0_INTERRUPT, kd_hwtimer_isr, &_timer0, "hwtimer0");
    rt_hw_interrupt_umask(IRQN_TIMER_0_INTERRUPT);
#endif

#ifdef RT_USING_HW_TIMER1
    static rt_hwtimer_t _timer1;
    static struct kd_timer_Type kd_timer1;

    kd_timer1.base_addr = (kendryte_timer_t*)addr;
    kd_timer1.id = 1;

    _timer1.info = &_info;
    _timer1.ops = &_ops;
    rt_device_hwtimer_register(&_timer1, "hwtimer1", &kd_timer1);
    _timer1.parent.fops = &timer_fops;
    rt_hw_interrupt_install(IRQN_TIMER_1_INTERRUPT, kd_hwtimer_isr, &_timer1, "hwtimer1");
    rt_hw_interrupt_umask(IRQN_TIMER_1_INTERRUPT);
#endif

#ifdef RT_USING_HW_TIMER2
    static rt_hwtimer_t _timer2;
    static struct kd_timer_Type kd_timer2;

    kd_timer2.base_addr = (kendryte_timer_t*)addr;
    kd_timer2.id = 2;

    _timer2.info = &_info;
    _timer2.ops = &_ops;
    rt_device_hwtimer_register(&_timer2, "hwtimer2", &kd_timer2);
    _timer2.parent.fops = &timer_fops;
    rt_hw_interrupt_install(IRQN_TIMER_2_INTERRUPT, kd_hwtimer_isr, &_timer2, "hwtimer2");
    rt_hw_interrupt_umask(IRQN_TIMER_2_INTERRUPT);
#endif

#ifdef RT_USING_HW_TIMER3
    static rt_hwtimer_t _timer3;
    static struct kd_timer_Type kd_timer3;

    kd_timer3.base_addr = (kendryte_timer_t*)addr;
    kd_timer3.id = 3;

    _timer3.info = &_info;
    _timer3.ops = &_ops;
    rt_device_hwtimer_register(&_timer3, "hwtimer3", &kd_timer3);
    _timer3.parent.fops = &timer_fops;
    rt_hw_interrupt_install(IRQN_TIMER_3_INTERRUPT, kd_hwtimer_isr, &_timer3, "hwtimer3");
    rt_hw_interrupt_umask(IRQN_TIMER_3_INTERRUPT);
#endif

#ifdef RT_USING_HW_TIMER4
    static rt_hwtimer_t _timer4;
    static struct kd_timer_Type kd_timer4;

    kd_timer4.base_addr = (kendryte_timer_t*)addr;
    kd_timer4.id = 4;

    _timer4.info = &_info;
    _timer4.ops = &_ops;
    rt_device_hwtimer_register(&_timer4, "hwtimer4", &kd_timer4);
    _timer4.parent.fops = &timer_fops;
    rt_hw_interrupt_install(IRQN_TIMER_4_INTERRUPT, kd_hwtimer_isr, &_timer4, "hwtimer4");
    rt_hw_interrupt_umask(IRQN_TIMER_4_INTERRUPT);
#endif

#ifdef RT_USING_HW_TIMER5
    static rt_hwtimer_t _timer5;
    static struct kd_timer_Type kd_timer5;

    kd_timer5.base_addr = (kendryte_timer_t*)addr;
    kd_timer5.id = 5;

    _timer5.info = &_info;
    _timer5.ops = &_ops;
    rt_device_hwtimer_register(&_timer5, "hwtimer5", &kd_timer5);
    _timer5.parent.fops = &timer_fops;
    rt_hw_interrupt_install(IRQN_TIMER_5_INTERRUPT, kd_hwtimer_isr, &_timer5, "hwtimer5");
    rt_hw_interrupt_umask(IRQN_TIMER_5_INTERRUPT);
#endif
}
INIT_BOARD_EXPORT(rt_hw_timer_init);
