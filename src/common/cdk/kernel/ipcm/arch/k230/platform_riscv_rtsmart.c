#include <stdint.h>
#include <stdlib.h>
#include <rthw.h>
#include <ioremap.h>
#include "device_config.h"
#include "ipcm_platform.h"
#include "io.h"

extern int irq_callback(int irq, void *data);

void *mail_box_reg_base = NULL;

static void mailbox_write_reg(uint32_t reg, uint32_t value)
{
    writel(value, mail_box_reg_base + reg);
}

static uint32_t mailbox_read_reg(uint32_t reg)
{
    return readl(mail_box_reg_base + reg);
}

void __interrupt_trigger__(int cpu, int irq)
{
    mailbox_write_reg(DSP2CPU_INT_SET0, 0);
}

static void __irq_callback(int irq, void *data)
{
	irq_callback(irq, data);
    mailbox_write_reg(CPU2DSP_INT_CLEAR0, 0);
}

static void reset_delay()
{
    volatile int count = 100000;
    while(count--);
}

#if 0
#define SOC_CTL_RST_CTL (0x91101020UL)
static void reset_mailbox(void)
{
    uint32_t reg_value;
    void *soc_ctl_rst_ctl = rt_ioremap((void *)SOC_CTL_RST_CTL, 0x1000);
    if(!soc_ctl_rst_ctl) {
        rt_kprintf("SOC_CTL_RST_CTL ioremap error\n");
    }
    reg_value = readl(soc_ctl_rst_ctl);
    reg_value &= ~(1<<17);
    writel(reg_value, soc_ctl_rst_ctl);
    reset_delay();
    reg_value |= 1<<17;
    writel(reg_value, soc_ctl_rst_ctl);
    reset_delay();
    rt_iounmap(soc_ctl_rst_ctl);
    rt_kprintf("\nmailbox reset********** ok!\n");
}
#endif

int __arch_init__(void)
{
    int i = 0;
    mail_box_reg_base = rt_ioremap((void *)MAILBOX_REG_BASE, 0x10000);
    if(!mail_box_reg_base) {
        rt_kprintf("mail_box_reg_base ioremap error\n");
        return -1;
    }
    for(i = 0; i < 16; i++) {
        mailbox_write_reg(CPU2DSP_INT_CLEAR0, i);
    }
    rt_hw_interrupt_install(IRQ_NUM, __irq_callback, 0, "ipcm_irq");
    rt_hw_interrupt_umask(IRQ_NUM);
    mailbox_write_reg(CPU2DSP_INT_EN0, 0x1 << 16| 0x1 << 1| 0x1 << 0);
    mailbox_write_reg(DSP2CPU_INT_EN0, 0x1 << 16| 0x1 << 1| 0x1 << 0);
    return 0;
}

void __arch_free__(void)
{
    return;
}

void *__arch_io_mapping__(unsigned long addr, unsigned int sz)
{
	return rt_ioremap((void *)addr, sz);
}

void __arch_io_unmapping__(void *addr)
{
    rt_iounmap(addr);
    return;
}

void __barrier__(void)
{
    dmb();
}

