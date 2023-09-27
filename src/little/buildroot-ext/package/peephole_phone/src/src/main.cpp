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
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include "mapi_sys_api.h"
#include "my_app.h"

static volatile int g_exit_flag = 0;

static int set_mallopt(void)
{
    mallopt(M_TRIM_THRESHOLD, 128 * 1024);
    mallopt(M_MMAP_THRESHOLD, 128 * 1024);
    mallopt(M_MMAP_MAX, 1024);
    return 0;
}

static void sigHandler(int sig_no)
{
    g_exit_flag = 1;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage : peephole_phone <dev_ip>\n");
        return -1;
    }

    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);

    set_mallopt();
    int ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_sys_init error: %x\n", ret);
        return -1 ;
    }

    MyApp::GetInstance()->Init();
    char *dev_ip = argv[1];
    MyApp::GetInstance()->Run(g_exit_flag, dev_ip, 9000);
    MyApp::GetInstance()->DeInit();

    kd_mapi_sys_deinit();
    return 0;
}
