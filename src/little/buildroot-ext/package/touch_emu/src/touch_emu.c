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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/input.h>

#define TOUCH_DEV   "/dev/input/event0"
#define SILDING_STEP    10
int touch_fd = 0;

int report_key(int fd, u_int16_t type, u_int16_t code, int32_t value)
{
    int ret = 0;
    struct input_event event;

    memset(&event, 0x00, sizeof(struct input_event));
    event.type = type;
    event.code = code;
    event.value = value;
    gettimeofday(&event.time, NULL);
    ret = write(fd, &event, sizeof(struct input_event));
    if (ret < 0) {
        perror("write event error");
        return -1;
    }

    return 0;
}

int emulate_click(int fd, int32_t pos_x, int32_t pos_y)
{
    report_key(fd, EV_ABS, ABS_MT_SLOT, 0);
    report_key(fd, EV_ABS, ABS_MT_TRACKING_ID, 100);
    report_key(fd, EV_ABS, ABS_MT_POSITION_X, pos_x);
    report_key(fd, EV_ABS, ABS_MT_POSITION_Y, pos_y);
    report_key(fd, EV_ABS, ABS_MT_TOUCH_MAJOR, 26);
    report_key(fd, EV_ABS, ABS_MT_WIDTH_MAJOR, 26);
    report_key(fd, EV_KEY, BTN_TOUCH, 1);
    report_key(fd, EV_ABS, ABS_X, pos_x);
    report_key(fd, EV_ABS, ABS_Y, pos_y);
    report_key(fd, EV_ABS, ABS_MT_TRACKING_ID, 0);
    report_key(fd, EV_SYN, EV_SYN, EV_SYN);
    usleep(10000);
    report_key(fd, EV_KEY, BTN_TOUCH, 0);
    report_key(fd, EV_SYN, EV_SYN, EV_SYN);
}

int emulate_silde(int fd, int start_x,int start_y,int end_x,int end_y ,int sec)
{
    int next_x, next_y;

    if(start_x == end_x && start_y == end_y)
        return 0;

    report_key(fd, EV_ABS, ABS_MT_SLOT, 0);
    report_key(fd, EV_ABS,ABS_MT_TRACKING_ID, 100);
    report_key(fd, EV_ABS,ABS_MT_POSITION_X, start_x);
    report_key(fd, EV_ABS,ABS_MT_POSITION_Y, start_y);
    report_key(fd, EV_ABS,ABS_MT_TRACKING_ID, 0);
    report_key(fd, EV_ABS,ABS_PRESSURE, 200);
    report_key(fd, EV_SYN, EV_SYN, EV_SYN);
    usleep(10000);

    next_x = start_x;
    next_y = start_y;
    if(end_x == start_x) {
        next_y = end_y > start_y ? (next_y + SILDING_STEP):(next_y - SILDING_STEP);
        while (abs(next_y-start_y) < abs(end_y-start_y))
        {
            report_key(fd, EV_ABS,ABS_MT_POSITION_X, end_x);
            report_key(fd, EV_ABS,ABS_MT_POSITION_Y, next_y);
            report_key(fd, EV_SYN,EV_SYN,EV_SYN);
            usleep(10000);
            next_y = end_y > start_y ? (next_y + SILDING_STEP) : (next_y - SILDING_STEP);
        }
    } else if(end_y == start_y) {
        next_x = end_x > start_x ? (next_x + SILDING_STEP) : (next_x - SILDING_STEP);
        while (abs(next_x-start_x) < abs(end_x-start_x))
        {
            report_key(fd, EV_ABS, ABS_MT_POSITION_X, next_x);
            report_key(fd, EV_ABS, ABS_MT_POSITION_Y, end_y);
            report_key(fd, EV_SYN, EV_SYN, EV_SYN);
            usleep(10000);
            next_x = end_x > start_x ? (next_x + SILDING_STEP) : (next_x - SILDING_STEP);
        }
    }
    report_key(fd, EV_ABS,ABS_MT_TRACKING_ID, -1);
    report_key(fd, EV_SYN,EV_SYN,EV_SYN);

    return 0;
}
 
int main(int argc, char **argv)
{
    printf("touchscreen emulation start \n");

    touch_fd = open(TOUCH_DEV, O_RDWR);
    if (touch_fd < 0) {
        perror("open touch_dev error");
        return -1;
    }

    while (1) {
        // auto adjust slider
        emulate_silde(touch_fd, 100, 1260, 450, 1260, 200);
        sleep (2);
        emulate_silde(touch_fd, 450, 1260, 100, 1260, 200);
        sleep (2);
        // auto pop up drop-down menu
        emulate_click(touch_fd, 860, 1150);
        usleep (10000);
        emulate_click(touch_fd, 860, 1150);
        sleep (2);
    }

    close(touch_fd);

    return 0;
}
