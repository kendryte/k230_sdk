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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "sys/ioctl.h"
#include <stdio.h>
#include "stdio.h"
#include "k_autoconf_comm.h"
#define KD_GPIO_HIGH     1
#define KD_GPIO_LOW      0

/* ioctl */

#define	GPIO_DM_OUTPUT           _IOW('G', 0, int)
#define	GPIO_DM_INPUT            _IOW('G', 1, int)
#define	GPIO_DM_INPUT_PULL_UP    _IOW('G', 2, int)
#define	GPIO_DM_INPUT_PULL_DOWN  _IOW('G', 3, int)
#define	GPIO_WRITE_LOW           _IOW('G', 4, int)
#define	GPIO_WRITE_HIGH          _IOW('G', 5, int)

#define	GPIO_PE_RISING           _IOW('G', 7, int)
#define	GPIO_PE_FALLING          _IOW('G', 8, int)
#define	GPIO_PE_BOTH             _IOW('G', 9, int)
#define	GPIO_PE_HIGH             _IOW('G', 10, int)
#define	GPIO_PE_LOW              _IOW('G', 11, int)

#define GPIO_READ_VALUE       	_IOW('G', 12, int)


typedef struct kd_pin_mode
{
    unsigned short pin;     /* pin number, from 0 to 63 */
    unsigned short mode;    /* pin level status, 0 low level, 1 high level */
} pin_mode_t;


#if defined(CONFIG_BOARD_K230_CANMV)
#define LED_PIN_NUM1    52
#define LED_PIN_NUM2    53
#else
#define LED_PIN_NUM1    33
#define LED_PIN_NUM2    32
#endif

int unitest_gpio_read_write(int fd)
{
    int ret;
    pin_mode_t mode33;
    pin_mode_t mode27;

    mode33.pin = LED_PIN_NUM1;
    mode27.pin = LED_PIN_NUM2;

    ret = ioctl(fd, GPIO_DM_OUTPUT, &mode33);  //pin32 output
    if (ret)
    {
        perror("ioctl /dev/pin err\n");
        return -1;
    }

    ret = ioctl(fd, GPIO_DM_INPUT, &mode27);  //pin33 input
    if (ret)
    {
        perror("ioctl /dev/pin err\n");
        return -1;
    }

    while(1)
    {
        ioctl(fd, GPIO_WRITE_HIGH, &mode33);
        ioctl(fd, GPIO_READ_VALUE, &mode27);
        if(mode27.mode != KD_GPIO_HIGH)
            printf("[error] the output pin is not equal to input pin!\n");
        else
            printf("read pin level success!\n");
        sleep(1);
        ioctl(fd, GPIO_WRITE_LOW, &mode33);
        ioctl(fd, GPIO_READ_VALUE, &mode27);
        if(mode27.mode != KD_GPIO_LOW)
            printf("[error] the output pin is not equal to input pin!\n");
        else
            printf("read pin level success!\n");
        sleep(1);
    }
}

int unitest_gpio_key(int fd)
{
    int ret;
    pin_mode_t mode52;
    pin_mode_t mode53;

    mode52.pin = LED_PIN_NUM1;
    mode53.pin = LED_PIN_NUM2;

    ret = ioctl(fd, GPIO_DM_INPUT, &mode52);
    if (ret)
    {
        perror("ioctl /dev/pin err\n");
        return -1;
    }

    ret = ioctl(fd, GPIO_DM_INPUT, &mode53);
    if (ret)
    {
        perror("ioctl /dev/pin err\n");
        return -1;
    }

    while(1)
    {
        ioctl(fd, GPIO_READ_VALUE, &mode53);
        if(mode53.mode == KD_GPIO_LOW)
        {
            printf("[S1] key press!\n");
        }

        ioctl(fd, GPIO_READ_VALUE, &mode52);
        if(mode52.mode == KD_GPIO_LOW)
        {
            printf("[S2] key press!\n");
        }
        mode53.mode = KD_GPIO_HIGH;
        mode52.mode = KD_GPIO_HIGH;
    }
    return 0;
}


int main(void)
{
    int gpio_fd = -1;

    gpio_fd = open("/dev/gpio", O_RDWR);
    if (gpio_fd < 0)
    {
        perror("open /dev/pin err\n");
        return -1;
    }

#if defined(CONFIG_BOARD_K230_CANMV)
    unitest_gpio_key(gpio_fd);
#else
    unitest_gpio_read_write(gpio_fd);
#endif
    close(gpio_fd);

    return 0;
}
