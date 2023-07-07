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
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/ioctl.h"

/**
 * @brief 
 * io52  pwm4
 * io7   pwm2 测量点R77
 * io8   pwm3 测量点R79
 * 
 * pwm0-
 *      |_channel0
 *      |_channel1
 *      |_channel2
 * 
 * pwm1-
 *      |_channel3
 *      |_channel4
 *      |_channel5
 */

#define PWM_DEVICE_NAME     "/dev/pwm"
#define PWM_CHANNEL2           2
#define PWM_CHANNEL3           3
#define PWM_CHANNEL4           4
#define	KD_PWM_CMD_ENABLE           _IOW('P', 0, int)
#define	KD_PWM_CMD_DISABLE          _IOW('P', 1, int)
#define	KD_PWM_CMD_SET              _IOW('P', 2, int)
#define	KD_PWM_CMD_GET              _IOW('P', 3, int)

typedef struct
{
    unsigned int channel; /* 0-n */
    unsigned int period;  /* unit:ns 1ns~4.29s:1Ghz~0.23hz */
    unsigned int pulse;   /* unit:ns (pulse<=period) */
} pwm_config_t;

int main(int argc, char *argv[])
{
    int fd;
    pwm_config_t config;

    printf("open %s\n", PWM_DEVICE_NAME);
    fd = open(PWM_DEVICE_NAME, O_RDWR);
    if (fd < 0)
    {
        perror("open /dev/pwm err\n");
        pthread_exit((void *) "thread exit!");
    }
    printf("open %s OK!\n", PWM_DEVICE_NAME);
    config.channel = PWM_CHANNEL4;
    config.period = 100000;
    config.pulse = 25000;
    if (ioctl(fd, KD_PWM_CMD_SET, &config))
    {
        perror("set pwm err\n");
        pthread_exit((void *) "thread exit!");
    }
    config.channel = PWM_CHANNEL3;
    config.period = 100000;
    config.pulse = 50000;
    ioctl(fd, KD_PWM_CMD_SET, &config);
    ioctl(fd, KD_PWM_CMD_ENABLE, &config);


    config.channel = PWM_CHANNEL2;
    config.period = 100000;
    config.pulse = 75000;
    ioctl(fd, KD_PWM_CMD_SET, &config);
    ioctl(fd, KD_PWM_CMD_ENABLE, &config);
    printf("pwm 2 duty 75;\n");
    printf("pwm 3 duty 50;\n");
    printf("pwm 4 duty 25;\n");
    return 0;
}
