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
#include <pthread.h>
#include <unistd.h>
#include "log.h"
#include "kuvc.h"
#include "uvc_gadgete.h"

static pthread_t g_uvc_send_data_pid;
static int running = 0;
static int __init()
{
    return 0;
}

static int __open()
{
    running = 1;
    const char *devpath = "/dev/video0";

    return open_uvc_device(devpath);
}

static int __close()
{
    running = 0;
    sleep(2);
    return close_uvc_device();
}

void* uvc_send_data_thread(void *p)
{
    int status = 0;

    while (running)
    {
        status = run_uvc_data();
        if (status < 0)
        {
            break;
        }
    }

    RLOG("uvc_send_data_thread exit, status: %d.\n", status);

    return NULL;
}

static int __run()
{
    int status = 0;

    pthread_create(&g_uvc_send_data_pid, 0, uvc_send_data_thread, NULL);

    while (running)
    {
        status = run_uvc_device();

        if (status < 0)
        {
            break;
        }
        // be careful. if return code is timeout,
        // maybe the host is disconnected,so here to start device again
        // it maybe to find a another nice method to checking host connects or disconnects
    }

    pthread_join(g_uvc_send_data_pid, NULL);
    RLOG("run_uvc_device exit, status: %d.\n", status);
    return 0;
}

/* ---------------------------------------------------------------------- */

static kuvc __k_uvc =
{
    .init = __init,
    .open = __open,
    .close = __close,
    .run = __run,
};

kuvc* get_kuvc()
{
    return &__k_uvc;
}

