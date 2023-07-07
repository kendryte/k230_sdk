
#include <clint.h>
#include <io.h>
#include "encoding.h"
//#include <riscv64.h>
#include "ioremap.h"

static inline uint64_t counter(void)
{
    uint64_t cnt;
    __asm__ __volatile__("csrr %0, time\n" : "=r"(cnt) :: "memory");
    return cnt;
}

//soft intterupt
//set MSIPR/SSIPR bit0 1 soft interrupt
//clear MSIPR/SSIPR bit0 0 soft interrupt

void clint_soft_irq_init(void)
{
    // clear_csr(sie, SIE_SSIE);
    set_csr(sie, SIE_SSIE); //set s-mode ssie
}

void clint_soft_irq_start(void)
{
    size_t reg;
    reg = (size_t)rt_ioremap_nocache((void*)(CLINT + CLINT_SSIP0_OFFSET), 4);
    write32(reg, 1);
    rt_iounmap((void*)reg);
}

void clint_soft_irq_clear(void)
{
    size_t reg;

    reg = (size_t)rt_ioremap_nocache((void*)(CLINT + CLINT_SSIP0_OFFSET), 4);
    write32(reg, 0);
    rt_iounmap((void*)reg);
}

static size_t clint_cmp_reg;

//timer
//riscv need 64 bit mtime
void clint_timer_init(void)
{
    rt_uint64_t cur_cnt = counter();
    rt_uint32_t tick_l = (cur_cnt & 0xffffffff);
    rt_uint32_t tick_h = (cur_cnt >> 32) & 0xffffffff;

    (*(volatile uint32_t *)((size_t)rt_ioremap((void *)0x91108020, 8))) = (1);
    (*(volatile uint32_t *)((size_t)rt_ioremap((void *)0x91108030, 8))) = (1);

    clint_cmp_reg = (size_t)rt_ioremap_nocache((void*)(CLINT + CLINT_STIMECMPL0), 8);

    write32(clint_cmp_reg, tick_l);
    write32(clint_cmp_reg + 4, tick_h);
    rt_iounmap((void *)0x91108020);
    rt_iounmap((void *)0x91108030);
    set_csr(sie,  SIE_SSIE | SIE_STIE | SIE_SEIE); //set m-mode sip
}

void clint_timer_cmp_set_val(unsigned long val)
{
    //MIE
    // 17   16-12   11  10  9    8   7    6    5     4    3      2     1    0
    //MOIE         MEIE    SEIE     MTIE      STIE       MSIE         SSIE
    //now we can user MTIE
    //24M 24000 000
    rt_uint64_t cur_cnt = counter() + val;
    rt_uint32_t tick_l = (cur_cnt & 0xffffffff);
    rt_uint32_t tick_h = (cur_cnt >> 32) & 0xffffffff;

    write32(clint_cmp_reg, tick_l);
    write32(clint_cmp_reg + 4, tick_h);
}
