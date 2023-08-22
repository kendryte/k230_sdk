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

/**
 * @file lv_port_indev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_port.h"
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/
#define TOUCHPAD_DEVNAME "/dev/input/event0"

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    int evdev_fd;
    int evdev_root_x;
    int evdev_root_y;
    int evdev_button;
    int evdev_key_val;
} evdev_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touchpad_init(void);
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static void map0(int *evdev_root_x, int *evdev_root_y);
static void map1(int *evdev_root_x, int *evdev_root_y);

/**********************
 *  STATIC VARIABLES
 **********************/
static evdev_t touchpad_evdev = {.evdev_fd = -1};
static void (*map)(int *evdev_root_x, int *evdev_root_y);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;

    touchpad_init();

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void touchpad_init(void)
{
    int evdev_fd = touchpad_evdev.evdev_fd;

    if (evdev_fd != -1)
        close(evdev_fd);

    evdev_fd = open(TOUCHPAD_DEVNAME, O_RDONLY | O_NOCTTY | O_NDELAY);

    if (evdev_fd == -1) {
        perror("unable to open evdev interface:");
        return;
    }

    fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);

    touchpad_evdev.evdev_fd = evdev_fd;
    touchpad_evdev.evdev_root_x = 0;
    touchpad_evdev.evdev_root_y = 0;
    touchpad_evdev.evdev_key_val = 0;
    touchpad_evdev.evdev_button = LV_INDEV_STATE_REL;
}

static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    struct input_event in;
    int evdev_fd = touchpad_evdev.evdev_fd;
    int evdev_root_x = touchpad_evdev.evdev_root_x;
    int evdev_root_y = touchpad_evdev.evdev_root_y;
    int evdev_button = touchpad_evdev.evdev_button;

    while (1) {
        int rbytes = read(evdev_fd, &in, sizeof(in));
        if (rbytes < (int)sizeof(in))
            break;
        if (in.type == EV_ABS) {
            if (in.code == ABS_X)
                evdev_root_x = in.value;
            else if (in.code == ABS_Y)
                evdev_root_y = in.value;
        } else if (in.type == EV_KEY) {
            if (in.code == BTN_TOUCH) {
                if (in.value == 0)
                    evdev_button = LV_INDEV_STATE_REL;
                else if (in.value == 1)
                    evdev_button = LV_INDEV_STATE_PR;
            }
        }
    }

    touchpad_evdev.evdev_root_x = evdev_root_x;
    touchpad_evdev.evdev_root_y = evdev_root_y;
    touchpad_evdev.evdev_button = evdev_button;

    if (map)
        map(&evdev_root_x, &evdev_root_y);

    data->state = evdev_button;
    data->point.x = evdev_root_x;
    data->point.y = evdev_root_y;
}

static void map0(int *evdev_root_x, int *evdev_root_y)
{
    int x,y;
    x = 1080 - *evdev_root_x;
    y = 1920 - *evdev_root_y;
    // y offset
    y = y - 1920 / 2;
    
    // y limit
    y = y < 0 ? 0 : y;
    x = x < 0 ? 0 : x;

    *evdev_root_y = y;
    *evdev_root_x = x;
}

static void map1(int *evdev_root_x, int *evdev_root_y)
{
    int x, y;

    x = *evdev_root_x;
    y = *evdev_root_y;
    // x,y limit
    x = x < 0 ? 0 : x >= 800 ? 799 : x;
    y = y < 0 ? 0 : y >= 1280 ? 1279 : y;

    *evdev_root_x = x;
    *evdev_root_y = y;
}

void input_map_set(int index)
{
    if (index == 0)
        map = map0;
    else if (index == 1)
        map = map1;
    else
        map = NULL;
}
