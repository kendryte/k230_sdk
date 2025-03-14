#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "sys/ioctl.h"

#define	GPIO_WRITE_LOW           _IOW('G', 4, int)
#define	GPIO_WRITE_HIGH          _IOW('G', 5, int)

#define KD_GPIO_SET_MODE _IOW('G', 20, int)
#define KD_GPIO_GET_MODE _IOWR('G', 21, int)
#define KD_GPIO_SET_VALUE _IOW('G', 22, int)
#define KD_GPIO_GET_VALUE _IOWR('G', 23, int)
#define KD_GPIO_SET_IRQ _IOW('G', 24, int)
#define KD_GPIO_GET_IRQ _IOWR('G', 25, int)

typedef enum _gpio_pin_edge {
    GPIO_PE_RISING,
    GPIO_PE_FALLING,
    GPIO_PE_BOTH,
    GPIO_PE_HIGH,
    GPIO_PE_LOW,
} gpio_pin_edge_t;

typedef enum _gpio_drive_mode {
    GPIO_DM_OUTPUT,
    GPIO_DM_INPUT,
    GPIO_DM_INPUT_PULL_UP,
    GPIO_DM_INPUT_PULL_DOWN,
} gpio_drive_mode_t;

typedef enum _gpio_pin_value {
    GPIO_PV_LOW,
    GPIO_PV_HIGH
} gpio_pin_value_t;

typedef struct {
    uint16_t pin;
    uint16_t value;
} gpio_cfg_t;

typedef struct {
    uint16_t pin;
    uint8_t enable;
    uint8_t mode;
    uint16_t debounce;
    uint8_t signo;
    void* sigval;
} gpio_irqcfg_t;

#define KEY1_SIG SIGUSR1
#define KEY2_SIG SIGUSR2

#define errExit(msg)        \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

int fd;

static void
print_siginfo(siginfo_t* si)
{
    printf("    si_signo = %d; ", si->si_signo);
    printf("    si_code = %d; ", si->si_code);
    printf("    si_errno = %d; ", si->si_errno);
    printf("    sival_ptr = %p; \n", si->si_ptr);
    gpio_cfg_t cfg;
    cfg.pin = (uint64_t)si->si_ptr;
    ioctl(fd, KD_GPIO_GET_VALUE, &cfg);
    printf("input value= %d\n", cfg.value);
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
    int ret;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(KEY1_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    if (sigaction(KEY2_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    fd = open("/dev/gpio", O_RDWR);

    gpio_cfg_t cfg;
    cfg.pin = 29;
    cfg.value = GPIO_DM_INPUT;
    ioctl(fd, KD_GPIO_SET_MODE, &cfg);
    cfg.pin = 30;
    cfg.value = GPIO_DM_OUTPUT;
    ioctl(fd, KD_GPIO_SET_MODE, &cfg);
    cfg.pin = 30;
    cfg.value = GPIO_PV_HIGH;
    ret = ioctl(fd, GPIO_WRITE_HIGH, &cfg);
    if (ret)
        printf("ioctl /dev/pin err\n");

    gpio_irqcfg_t irqcfg;
    irqcfg.pin = 29;
    irqcfg.enable = 1;
    irqcfg.mode = GPIO_PE_BOTH;
    irqcfg.debounce = 200;
    irqcfg.signo = KEY1_SIG;
    irqcfg.sigval = (void *)(uint64_t)irqcfg.pin;
    ioctl(fd, KD_GPIO_SET_IRQ, &irqcfg);

    cfg.pin = 30;
    cfg.value = GPIO_PV_LOW;
    ret = ioctl(fd, GPIO_WRITE_LOW, &cfg);
    if (ret)
        printf("ioctl /dev/pin err\n");

    usleep(1000000);

    cfg.pin = 30;
    cfg.value = GPIO_PV_HIGH;
    ret = ioctl(fd, GPIO_WRITE_HIGH, &cfg);
    if (ret)
        printf("ioctl /dev/pin err\n");

    exit(EXIT_SUCCESS);
}
