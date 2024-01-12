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
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <pthread.h>
#include "log.h"
#include "kuvc.h"
#include "kstream.h"
#include "camera.h"
/* -------------------------------------------------------------------------- */
extern unsigned int g_standalone;
static pthread_t g_uvc_pid;
extern void sample_dpu_config(void);

static int __init()
{
    if(g_standalone == 0)
    {
        sample_dpu_config();
    }

    if (get_kuvc()->init() != 0 ||
        kstream_init() != 0)
    {
        return -1;
    }

    return 0;
}

static int __open()
{
    if (get_kuvc()->open() != 0)
    {
        return -1;
    }

    return 0;
}

static int __close()
{
    get_kuvc()->close();
    kstream_deinit();

    return 0;
}

void* uvc_thread(void *p)
{
    get_kuvc()->run();
    return NULL;
}

static int __run()
{
    pthread_create(&g_uvc_pid, 0, uvc_thread, NULL);

    pthread_join(g_uvc_pid, NULL);

    return 0;
}

/* -------------------------------------------------------------------------- */

static kcamera __k_camera =
{
    .init = __init,
    .open = __open,
    .close = __close,
    .run = __run,
};

kcamera* get_kcamera()
{
    return &__k_camera;
}
