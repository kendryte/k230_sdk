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

#include "ui_common.h"
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "lv_port.h"
#include "vo_cfg.h"

static int set_priority(void)
{
    int max_prio = sched_get_priority_max(SCHED_RR);
    int min_prio = sched_get_priority_min(SCHED_RR);
    struct sched_param sp = {(max_prio + min_prio) / 4};

    return pthread_setschedparam(pthread_self(), SCHED_RR, &sp);
}

static int set_mallopt(void)
{
    mallopt(M_TRIM_THRESHOLD, 128 * 1024);
    mallopt(M_MMAP_THRESHOLD, 128 * 1024);
    mallopt(M_MMAP_MAX, 1024);

    return 0;
}

static void sigHandler(int sig_no)
{
    intercom_deinit();
    // lv_deinit();
    return;
}

int main(int argc, char *argv[])
{
    int ret = 0;

    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);

    set_mallopt();
    ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_sys_init error: %x\n", ret);
        // return -1 ;
    }

    display_hardware_init();
    sample_dwc_dsi_init(0);
    vo_layer_init();

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    setup_scr_scr_main(argv[1]);
    msg_proc_init();
    set_priority();

    jump_to_scr_main();
    while (1)
    {
        lv_timer_handler();
        if (ui_msg_proc() < 0)
            usleep(1 * 1000);
    }

    // kd_mapi_sys_deinit();
    return 0;
}
