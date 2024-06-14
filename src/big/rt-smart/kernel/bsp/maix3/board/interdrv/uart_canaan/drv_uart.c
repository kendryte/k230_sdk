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

#include <rthw.h>
#include <rtdevice.h>
#include <ioremap.h>
#include "board.h"
#include "drv_uart.h"
#include "riscv_io.h"
#include "board.h"
#include "sysctl_clk.h"
#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#endif
#define DBG_TAG "uart"
#ifdef RT_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_WARNING
#endif
#define DBG_COLOR
#include <rtdbg.h>

#define UART_RBR (0x00)       /* receive buffer register */
#define UART_THR (0x00)       /* transmit holding register */
#define UART_DLL (0x00)       /* divisor latch low register */
#define UART_DLH (0x04)       /* diviso latch high register */
#define UART_IER (0x04)       /* interrupt enable register */
#define UART_IIR (0x08)       /* interrupt identity register */
#define UART_FCR (0x08)       /* FIFO control register */
#define UART_LCR (0x0c)       /* line control register */
#define UART_MCR (0x10)       /* modem control register */
#define UART_LSR (0x14)       /* line status register */
#define UART_MSR (0x18)       /* modem status register */
#define UART_SCH (0x1c)       /* scratch register */
#define UART_USR (0x7c)       /* status register */
#define UART_TFL (0x80)       /* transmit FIFO level */
#define UART_RFL (0x84)       /* RFL */
#define UART_HALT (0xa4)      /* halt tx register */
#define UART_DLF (0xc0)       /* Divisor Latch Fraction Register */

#define BIT(x) (1 << x)

/* Line Status Rigster */
#define UART_LSR_RXFIFOE    (BIT(7))
#define UART_LSR_TEMT       (BIT(6))
#define UART_LSR_THRE       (BIT(5))
#define UART_LSR_BI         (BIT(4))
#define UART_LSR_FE         (BIT(3))
#define UART_LSR_PE         (BIT(2))
#define UART_LSR_OE         (BIT(1))
#define UART_LSR_DR         (BIT(0))
#define UART_LSR_BRK_ERROR_BITS 0x1E /* BI, FE, PE, OE bits */

/* Line Control Register */
#define UART_LCR_DLAB       (BIT(7))
#define UART_LCR_SBC        (BIT(6))
#define UART_LCR_PARITY_MASK    (BIT(5)|BIT(4))
#define UART_LCR_EPAR       (1 << 4)
#define UART_LCR_OPAR       (0 << 4)
#define UART_LCR_PARITY     (BIT(3))
#define UART_LCR_STOP       (BIT(2))
#define UART_LCR_DLEN_MASK  (BIT(1)|BIT(0))
#define UART_LCR_WLEN5      (0)
#define UART_LCR_WLEN6      (1)
#define UART_LCR_WLEN7      (2)
#define UART_LCR_WLEN8      (3)

/* Halt Register */
#define UART_HALT_LCRUP     (BIT(2))
#define UART_HALT_FORCECFG  (BIT(1))
#define UART_HALT_HTX       (BIT(0))

/* Interrupt Enable Register */
#define UART_IER_MASK       (0xff)
#define UART_IER_PTIME      (BIT(7))
#define UART_IER_RS485      (BIT(4))
#define UART_IER_MSI        (BIT(3))
#define UART_IER_RLSI       (BIT(2))
#define UART_IER_THRI       (BIT(1))
#define UART_IER_RDI        (BIT(0))

/* Interrupt ID Register */
#define UART_IIR_FEFLAG_MASK    (BIT(6)|BIT(7))
#define UART_IIR_IID_MASK   (BIT(0)|BIT(1)|BIT(2)|BIT(3))
#define UART_IIR_IID_MSTA   (0)
#define UART_IIR_IID_NOIRQ  (1)
#define UART_IIR_IID_THREMP (2)
#define UART_IIR_IID_RXDVAL (4)
#define UART_IIR_IID_LINESTA    (6)
#define UART_IIR_IID_BUSBSY (7)
#define UART_IIR_IID_CHARTO (12)


#define	IOC_SET_BAUDRATE            _IOW('U', 0x40, int)

#define DEFAULT_BAUDRATE      (115200)
#define UART1_IRQ              0x11
#define UART2_IRQ              0x12
#define UART4_IRQ              0x14

struct kd_uart_device {
    struct rt_device kd_uart;
    volatile void *base;
    struct rt_serial_rx_fifo *kd_rx_fifo;
    struct rt_timer timer;
    volatile uint8_t t_flag;
    int id;
};

static uint32_t timeout = UART_TIMEOUT;

#define write32(addr, val) writel(val, (void*)(addr))
#define read32(addr) readl((void*)(addr))

static int uart_fops_open(struct dfs_fd *fd)
{
    rt_device_t uart = (rt_device_t)fd->fnode->data;
    RT_ASSERT(uart != RT_NULL);
    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)uart->user_data;

    rt_timer_start(&kd_uart_device->timer);
    return 0;
}

static int uart_fops_close(struct dfs_fd *fd)
{
    rt_device_t uart = (rt_device_t)fd->fnode->data;
    RT_ASSERT(uart != RT_NULL);
    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)uart->user_data;

    rt_timer_stop(&kd_uart_device->timer);
    return 0;
}

static int uart_fops_ioctl(struct dfs_fd *fd, int cmd, void *args)
{
    rt_device_t uart = (rt_device_t)fd->fnode->data;
    RT_ASSERT(uart != RT_NULL);

    if (uart->ops->control)
    {
        return uart->ops->control(uart, cmd, args);
    }

    return -RT_EINVAL;
}

static int uart_fops_read(struct dfs_fd *fd, void *buf, size_t count)
{
    int ret = 0;
    rt_device_t uart = (rt_device_t)fd->fnode->data;
    RT_ASSERT(uart != RT_NULL);

    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)uart->user_data;
    struct rt_serial_rx_fifo *kd_rx_fifo = kd_uart_device->kd_rx_fifo;


    if (uart->ops->read)
    {
        ret = uart->ops->read(uart, 0, buf, count);
    }

    rt_base_t level;
    level = rt_hw_interrupt_disable();

    if (kd_rx_fifo->put_index == kd_rx_fifo->get_index)
    {        
        kd_uart_device->t_flag = 0;
    }

    rt_hw_interrupt_enable(level);
    return ret;
}

static int uart_fops_write(struct dfs_fd *fd, const void *buf, size_t count)
{
    rt_device_t uart = (rt_device_t)fd->fnode->data;
    RT_ASSERT(uart != RT_NULL);

    if (uart->ops->write)
    {
        uart->ops->write(uart, 0, buf, count);
    }
}

static int uart_fops_poll(struct dfs_fd *fd, struct rt_pollreq *req)
{
    int flags = 0;
    rt_device_t uart = (rt_device_t)fd->fnode->data;
    RT_ASSERT(uart != RT_NULL);

    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)uart->user_data;
    struct rt_serial_rx_fifo *kd_rx_fifo = kd_uart_device->kd_rx_fifo;

    rt_base_t level;
    level = rt_hw_interrupt_disable();
    rt_poll_add(&(uart->wait_queue), req);

    if (kd_uart_device->t_flag)
    {
        kd_uart_device->t_flag = 0;
        return POLLIN;
    } else
    if ((kd_rx_fifo->put_index - kd_rx_fifo->get_index) > 0)
    {
        if ((kd_rx_fifo->put_index - kd_rx_fifo->get_index) >= POLLIN_SIZE)
        {
            kd_uart_device->t_flag = 0;
            return POLLIN;
        }
    } else
    if ((kd_rx_fifo->get_index - kd_rx_fifo->put_index) > 0)
    {
        if (UART_BUFFER_SIZE - (kd_rx_fifo->get_index - kd_rx_fifo->put_index) >= POLLIN_SIZE)
        {
            kd_uart_device->t_flag = 0;
            return POLLIN;
        }
    }

    rt_hw_interrupt_enable(level);
    return 0;
}

const static struct dfs_file_ops _uart_fops =
{
    uart_fops_open,
    uart_fops_close,
    uart_fops_ioctl,
    uart_fops_read,
    uart_fops_write,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    uart_fops_poll,
};

static void kd_uart_init(volatile void *uart_base, int index)
{
    float calc_baudrate;
    float fdiv;
    uint8_t frac;
    uint16_t dl;
    uint32_t uart_clock;

    sysctl_clk_set_leaf_div(SYSCTL_CLK_UART_0_CLK + index, 1, 1);
    uart_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_UART_0_CLK + index);

    fdiv = 1.0f * uart_clock / DEFAULT_BAUDRATE;
    dl = fdiv / 16;
    frac = 0.5f + fdiv - dl * 16;
    if (dl == 0) {
        dl = 1;
        frac = 0;
    }
    calc_baudrate = 1.0f * uart_clock / (dl * 16 + frac);
    if ((calc_baudrate - DEFAULT_BAUDRATE) / DEFAULT_BAUDRATE > 0.02f) {
        LOG_E("The baud rate error is too large, %u", (uint32_t)calc_baudrate);
        return;
    }

    write32(uart_base + UART_LCR, 0x00);
    /* Disable all interrupts */
    write32(uart_base + UART_IER, 0x00);
    /* Enable DLAB */
    write32(uart_base + UART_LCR, 0x80);
    /* Set divisor low byte */
    write32(uart_base + UART_DLL, (uint8_t)dl);
    /* Set divisor high byte */
    write32(uart_base + UART_DLH, (uint8_t)(dl >> 8));
    /* Set divisor fraction byte*/
    write32(uart_base + UART_DLF, frac);
    /* 8 bits, no parity, one stop bit */
    write32(uart_base + UART_LCR, 0x03);
    /* Enable FIFO */
    write32(uart_base + UART_FCR, 0x01 | UART_RECEIVE_FIFO_16 << 6);
    /* No modem control DTR RTS */
    write32(uart_base + UART_MCR, 0x00);
    /* Clear line status */
    read32(uart_base + UART_LSR);
    /* Read receive buffer */
    read32(uart_base + UART_RBR);
    read32(uart_base + UART_USR);
    read32(uart_base + UART_FCR);
    /* Set scratchpad */
    write32(uart_base + UART_SCH, 0x00);
    //enable uart rx irq
    write32(uart_base + UART_IER, 0x01);
}

static rt_err_t uart_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

rt_err_t uart_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t uart_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)dev->user_data;
    struct rt_serial_rx_fifo *kd_rx_fifo = kd_uart_device->kd_rx_fifo;

    uint32_t length;
    uint8_t *data;

    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    length = size;
    data = (uint8_t*)buffer;

    while (length)
    {
        uint32_t ch;
        rt_base_t level;
        level = rt_hw_interrupt_disable();

        if ((kd_rx_fifo->get_index == kd_rx_fifo->put_index) && (kd_rx_fifo->is_full == RT_FALSE))
        {
            /* no data, enable interrupt and break out */
            rt_hw_interrupt_enable(level);
            break;
        }

        ch = kd_rx_fifo->buffer[kd_rx_fifo->get_index];
        kd_rx_fifo->get_index += 1;

        if (kd_rx_fifo->get_index >= UART_BUFFER_SIZE) kd_rx_fifo->get_index = 0;

        if (kd_rx_fifo->is_full == RT_TRUE)
        {
            kd_rx_fifo->is_full = RT_FALSE;
        }

        rt_hw_interrupt_enable(level);

        *data = ch & 0xff;
        data ++; length --;

    }
    return size - length;
}

static rt_size_t uart_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)dev->user_data;
    volatile void *base_addr = kd_uart_device->base;

    uint32_t i;
    volatile uint32_t *sed_buf;
    volatile uint32_t *sta;
    uint8_t *buf = (uint8_t *)buffer;
    int length = size;

    sed_buf = (uint32_t *)(base_addr + UART_THR);
    sta = (uint32_t *)(base_addr + UART_USR);

    while (size)
    {
        while (!(read32(base_addr + UART_LSR) & 0x20));
        *sed_buf = *buf;
        buf ++; size --;
    }

    return (length);
}


static rt_err_t  uart_control(rt_device_t dev, int cmd, void *args)
{
    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)dev->user_data;
    volatile void *uart_base = kd_uart_device->base;
    int id = kd_uart_device->id;
    float calc_baudrate;
    float fdiv;
    uint8_t frac;
    uint16_t dl;
    uint32_t uart_clock;
    uint32_t value;
    struct uart_configure *config = (struct uart_configure*)args;

    if (cmd != IOC_SET_BAUDRATE)
        return -RT_EINVAL;
    if (config == RT_NULL || uart_base == RT_NULL)
        return -RT_EINVAL;

    uart_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_UART_0_CLK + id);

    fdiv = 1.0f * uart_clock / config->baud_rate;
    dl = fdiv / 16;
    frac = 0.5f + fdiv - dl * 16;
    if (dl == 0) {
        dl = 1;
        frac = 0;
    }
    calc_baudrate = 1.0f * uart_clock / (dl * 16 + frac);
    if ((calc_baudrate - config->baud_rate) / config->baud_rate > 0.02f) {
        LOG_E("The baud rate error is too large, %u", (uint32_t)calc_baudrate);
        return -RT_EINVAL;
    }

    /* stop uart */
    write32(uart_base + UART_LCR, 0x00);
    write32(uart_base + UART_IER, 0x00);
    write32(uart_base + UART_LCR, 0x80);
    /* caculate diciver */
    write32(uart_base + UART_DLL, (uint8_t)dl);
    write32(uart_base + UART_DLH, (uint8_t)(dl >> 8));
    write32(uart_base + UART_DLF, frac);

    value = (config->data_bits - 5) | (config->stop_bits << 2) | (config->parity << 3);
    write32(uart_base + UART_LCR, value & ~0x80);

    write32(uart_base + UART_FCR, 0x01 | config->fifo_lenth << 6);

    if (config->auto_flow)
        write32(uart_base + UART_MCR, (0x1 << 5 | 0x1 << 1));  //auto flow
    else
        write32(uart_base + UART_MCR, 0x0);

    read32(uart_base + UART_LSR);
    read32(uart_base + UART_RBR);
    read32(uart_base + UART_USR);
    read32(uart_base + UART_FCR);
    write32(uart_base + UART_IER, 0x01);

    return RT_EOK;
}

const static struct rt_device_ops uart_ops =
{
    RT_NULL,
    uart_open,
    uart_close,
    uart_read,
    uart_write,
    uart_control
};

static void uart_check_buffer_size(void)
{
    static rt_bool_t already_output = RT_FALSE;

    if (already_output == RT_FALSE)
    {
        rt_kprintf("Warning: There is no enough buffer for saving data,"
              " please increase the UART_BUFFER_SIZE option.\n");
        already_output = RT_TRUE;
    }
}

static void uart_rx_fifo_get_char(struct rt_serial_rx_fifo *rx_fifo, uint8_t data)
{
    rt_base_t level;
    level = rt_hw_interrupt_disable();
    rx_fifo->buffer[rx_fifo->put_index] = data;
    rx_fifo->put_index += 1;

    if (rx_fifo->put_index >= UART_BUFFER_SIZE) rx_fifo->put_index = 0;

    if (rx_fifo->put_index == rx_fifo->get_index)
    {
        rx_fifo->get_index += 1;
        rx_fifo->is_full = RT_TRUE;
        if (rx_fifo->get_index >= UART_BUFFER_SIZE) rx_fifo->get_index = 0;

        uart_check_buffer_size();
    }
    rt_hw_interrupt_enable(level);
}

static void rt_hw_uart_isr(int irq, void *param)
{
    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)param;
    rt_device_t uart = &kd_uart_device->kd_uart;
    volatile void *base_addr = kd_uart_device->base;
    struct rt_serial_rx_fifo *kd_rx_fifo = kd_uart_device->kd_rx_fifo;

    uint32_t iir, lsr;
    uint32_t level;

    iir = readb(base_addr + UART_IIR) & UART_IIR_IID_MASK;
    lsr = readb(base_addr + UART_LSR);

    if (iir == UART_IIR_IID_BUSBSY)
    {
        (void)readb(base_addr + UART_USR);
    }
    else if (lsr & (UART_LSR_DR | UART_LSR_BI))
    {
        uint8_t data;
        RT_ASSERT(kd_rx_fifo != RT_NULL);
        do {
            data = readb((void*)(base_addr + UART_RBR));
            uart_rx_fifo_get_char(kd_rx_fifo, data);
            lsr = readb((void*)(base_addr + UART_LSR));
        } while(lsr & UART_LSR_DR);

    }
    else if (iir & UART_IIR_IID_CHARTO)
    {
        readb((void*)(base_addr + UART_RBR));
    }

    return;
}

static void uart_timeout(void* parameter)
{
    struct kd_uart_device *kd_uart_device = (struct kd_uart_device *)parameter;
    rt_device_t uart = &kd_uart_device->kd_uart;
    struct rt_serial_rx_fifo *kd_rx_fifo = kd_uart_device->kd_rx_fifo;
    /* 检查是否有数据 */
    uint32_t ret;

    rt_interrupt_enter();

    if ((kd_rx_fifo->put_index != kd_rx_fifo->get_index) && !kd_uart_device->t_flag)  /* 缓冲区有数据，且没有报超时 */
    {
        kd_uart_device->t_flag = 1;
    
        rt_wqueue_wakeup(&(uart->wait_queue), (void *)POLLIN);
    }
    rt_interrupt_leave();

}

int kd_hw_uart_init(void)
{
    rt_err_t ret = RT_EOK;
    struct kd_uart_device *kd_uart_device;
    int i;

#ifdef RT_USING_UART1
    kd_uart_device = rt_malloc(sizeof(struct kd_uart_device));
    kd_uart_device->base = rt_ioremap((void *)UART1_BASE_ADDR, UART1_IO_SIZE);
    kd_uart_device->kd_uart.ops = &uart_ops;
    kd_uart_device->kd_uart.user_data = (void *)kd_uart_device;
    kd_uart_device->id = 1;
    ret = rt_device_register(&kd_uart_device->kd_uart, "uart1", RT_DEVICE_FLAG_RDWR);
    kd_uart_device->kd_uart.fops = &_uart_fops;

    kd_uart_init(kd_uart_device->base, kd_uart_device->id);

    /* create rx_fifo */
    kd_uart_device->kd_rx_fifo = rt_malloc(sizeof(struct rt_serial_rx_fifo) + UART_BUFFER_SIZE);
    kd_uart_device->kd_rx_fifo->buffer = (uint8_t*) (kd_uart_device->kd_rx_fifo + 1);
    rt_memset(kd_uart_device->kd_rx_fifo->buffer, 0, UART_BUFFER_SIZE);
    kd_uart_device->kd_rx_fifo->put_index = 0;
    kd_uart_device->kd_rx_fifo->get_index = 0;
    kd_uart_device->kd_rx_fifo->is_full = RT_FALSE;

    /* register interrupt handler */
    rt_hw_interrupt_install(UART1_IRQ, rt_hw_uart_isr, kd_uart_device, "uart1");
    rt_hw_interrupt_umask(UART1_IRQ);

    /* create timer */
    rt_timer_init(&kd_uart_device->timer, "uart1_timer", uart_timeout, (void*)kd_uart_device, UART_TIMEOUT, RT_TIMER_FLAG_PERIODIC);

    rt_wqueue_init(&kd_uart_device->kd_uart.wait_queue);
#ifndef RT_FASTBOOT
    rt_kprintf("k230 uart1 register OK.\n");
#endif
#endif

#ifdef RT_USING_UART2
    kd_uart_device = rt_malloc(sizeof(struct kd_uart_device));
    kd_uart_device->base = rt_ioremap((void *)UART2_BASE_ADDR, UART2_IO_SIZE);
    kd_uart_device->kd_uart.ops = &uart_ops;
    kd_uart_device->kd_uart.user_data = (void *)kd_uart_device;
    kd_uart_device->id = 2;
    ret = rt_device_register(&kd_uart_device->kd_uart, "uart2", RT_DEVICE_FLAG_RDWR);
    kd_uart_device->kd_uart.fops = &_uart_fops;

    kd_uart_init(kd_uart_device->base, kd_uart_device->id);

    /* create rx_fifo */
    kd_uart_device->kd_rx_fifo = rt_malloc(sizeof(struct rt_serial_rx_fifo) + UART_BUFFER_SIZE);
    kd_uart_device->kd_rx_fifo->buffer = (uint8_t*) (kd_uart_device->kd_rx_fifo + 1);
    rt_memset(kd_uart_device->kd_rx_fifo->buffer, 0, UART_BUFFER_SIZE);
    kd_uart_device->kd_rx_fifo->put_index = 0;
    kd_uart_device->kd_rx_fifo->get_index = 0;
    kd_uart_device->kd_rx_fifo->is_full = RT_FALSE;

    /* register interrupt handler */
    rt_hw_interrupt_install(UART2_IRQ, rt_hw_uart_isr, kd_uart_device, "uart2");
    rt_hw_interrupt_umask(UART2_IRQ);

    /* create timer */
    rt_timer_init(&kd_uart_device->timer, "uart2_timer", uart_timeout, (void*)kd_uart_device, UART_TIMEOUT, RT_TIMER_FLAG_PERIODIC);

    rt_wqueue_init(&kd_uart_device->kd_uart.wait_queue);
#ifndef RT_FASTBOOT
    rt_kprintf("k230 uart2 register OK.\n");
#endif
#endif

#ifdef RT_USING_UART4
    kd_uart_device = rt_malloc(sizeof(struct kd_uart_device));
    kd_uart_device->base = rt_ioremap((void *)UART4_BASE_ADDR, UART4_IO_SIZE);
    kd_uart_device->kd_uart.ops = &uart_ops;
    kd_uart_device->kd_uart.user_data = (void *)kd_uart_device;
    kd_uart_device->id = 4;
    ret = rt_device_register(&kd_uart_device->kd_uart, "uart4", RT_DEVICE_FLAG_RDWR);
    kd_uart_device->kd_uart.fops = &_uart_fops;

    kd_uart_init(kd_uart_device->base, kd_uart_device->id);

    /* create rx_fifo */
    kd_uart_device->kd_rx_fifo = rt_malloc(sizeof(struct rt_serial_rx_fifo) + UART_BUFFER_SIZE);
    kd_uart_device->kd_rx_fifo->buffer = (uint8_t*) (kd_uart_device->kd_rx_fifo + 1);
    rt_memset(kd_uart_device->kd_rx_fifo->buffer, 0, UART_BUFFER_SIZE);
    kd_uart_device->kd_rx_fifo->put_index = 0;
    kd_uart_device->kd_rx_fifo->get_index = 0;
    kd_uart_device->kd_rx_fifo->is_full = RT_FALSE;

    /* register interrupt handler */
    rt_hw_interrupt_install(UART4_IRQ, rt_hw_uart_isr, kd_uart_device, "uart4");
    rt_hw_interrupt_umask(UART4_IRQ);

    /* create timer */
    rt_timer_init(&kd_uart_device->timer, "uart4_timer", uart_timeout, (void*)kd_uart_device, UART_TIMEOUT, RT_TIMER_FLAG_PERIODIC);

    rt_wqueue_init(&kd_uart_device->kd_uart.wait_queue);
#ifndef RT_FASTBOOT
    rt_kprintf("k230 uart4 register OK.\n");
#endif
#endif

    return ret;
}
INIT_DEVICE_EXPORT(kd_hw_uart_init);
