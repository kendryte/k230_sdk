#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "sys/ioctl.h"

typedef enum {
    HWTIMER_CTRL_FREQ_SET = 19 * 0x100 + 0x01,           /* set the count frequency */
    HWTIMER_CTRL_STOP = 19 * 0x100 + 0x02,               /* stop timer */
    HWTIMER_CTRL_INFO_GET = 19 * 0x100 + 0x03,           /* get a timer feature information */
    HWTIMER_CTRL_MODE_SET = 19 * 0x100 + 0x04,           /* Setting the timing mode(oneshot/period) */
    HWTIMER_CTRL_IRQ_SET = 19 * 0x100 + 0x10,
} hwtimer_ctrl_t;

typedef struct {
    int32_t sec;      /* second */
    int32_t usec;     /* microsecond */
} hwtimerval_t;

typedef struct {
    uint8_t enable;
    uint8_t signo;
    void *sigval;
} hwtimer_irqcfg_t;

typedef enum {
    HWTIMER_MODE_ONESHOT = 0x01,
    HWTIMER_MODE_PERIOD
} hwtimer_mode_t;

#define TIMER1_SIG SIGUSR1
#define TIMER2_SIG SIGUSR2

#define errExit(msg)        \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

int fd[2];

static void
print_siginfo(siginfo_t* si)
{
    printf("    si_signo = %d; ", si->si_signo);
    printf("    si_code = %d; ", si->si_code);
    printf("    si_errno = %d; ", si->si_errno);
    printf("    sival_ptr = %p; \n", si->si_ptr);
}

static void
handler(int sig, siginfo_t* si, void* uc)
{
    /* Note: calling printf() from a signal handler is not safe
       (and should not be done in production programs), since
       printf() is not async-signal-safe; see signal-safety(7).
       Nevertheless, we use printf() here as a simple way of
       showing that the handler was called. */

    printf("Caught signal %d\n", sig);
    print_siginfo(si);
}

int main(int argc, char* argv[])
{
    struct sigaction sa;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s timer1_timeout_s timer2_timeout_s\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(TIMER1_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    if (sigaction(TIMER2_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    fd[0] = open("/dev/hwtimer1", O_RDWR);
    fd[1] = open("/dev/hwtimer2", O_RDWR);

    hwtimer_irqcfg_t irqcfg;
    irqcfg.enable = 1;
    irqcfg.signo = TIMER1_SIG;
    irqcfg.sigval = (void *)(uint64_t)1;
    ioctl(fd[0], HWTIMER_CTRL_IRQ_SET, &irqcfg);
    irqcfg.enable = 1;
    irqcfg.signo = TIMER2_SIG;
    irqcfg.sigval = (void *)(uint64_t)2;
    ioctl(fd[1], HWTIMER_CTRL_IRQ_SET, &irqcfg);

    hwtimer_mode_t mode = HWTIMER_MODE_PERIOD;
    ioctl(fd[0], HWTIMER_CTRL_MODE_SET, &mode);
    ioctl(fd[1], HWTIMER_CTRL_MODE_SET, &mode);

    hwtimerval_t val;
    val.sec = atoi(argv[1]);
    val.usec = 0;
    printf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    write(fd[0], &val, sizeof(val));

    val.sec = atoi(argv[2]);
    val.usec = 0;
    printf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    write(fd[1], &val, sizeof(val));

    while (1) {
        char c = getchar();
        if (c == 'q')
            break;
        usleep(10000);
    }

    exit(EXIT_SUCCESS);
}
