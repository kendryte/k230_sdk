#include <rthw.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <tick.h>
#include <board.h>


#define TIMER_FREQ TIMER_CLK_FREQ

/* Use Cycle counter of Data Watchpoint and Trace Register for CPU time */

static float riscv_cputime_getres(void)
{
    float ret = 1000 * 1000 * 1000;
    ret = ret / TIMER_FREQ;
    return ret;
}

static uint64_t riscv_cputime_gettime(void)
{
    uint64_t time_elapsed;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

const static struct rt_clock_cputime_ops _riscv_ops =
    {
        riscv_cputime_getres,
        riscv_cputime_gettime};

int riscv_cputime_init(void)
{
    clock_cpu_setops(&_riscv_ops);
    return 0;
}
INIT_BOARD_EXPORT(riscv_cputime_init);
