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
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "camera.h"
#include "frame_cache.h"
#include "mapi_dpu_comm.h"

unsigned int g_exit = 0;
unsigned int g_bulk = 1;
unsigned int g_standalone = 0;

#if 0
unsigned int g_imagesize = 4096000;
unsigned int g_bulk_size = 4096000;
unsigned int g_cache_count = 10;
#endif

unsigned int g_imagesize = 409600;
unsigned int g_bulk_size = 409600;
unsigned int g_cache_count = 40;

extern unsigned int g_sensor0;
extern unsigned int g_sensor1;
char g_out_path[50] = {0};

extern k_dpu_info_t g_dpu_init;

static void sig_handler(int sig) {
    get_kcamera()->close();
}

void usage()
{
    fprintf(stderr, "Available options are\n");
    // fprintf(stderr, " -i		Use ISO mode\n");
    // fprintf(stderr, " -s		Do not use any real camera device\n");
    // fprintf(stderr, " -h		Print this help screen and exit\n");
    // fprintf(stderr, " -c		Number of frame cache buffers\n");
    // fprintf(stderr, " -m		Size of iso image\n");
    // fprintf(stderr, " -u		Size of bulk image\n");
    fprintf(stderr, " -o		depth output path\n");
    fprintf(stderr, " -s0		dev0 sensor type[see K230_Camera_Sensor_Adaptation_Guide.md]\n");
    fprintf(stderr, " -s1		dev1 sensor type[see K230_Camera_Sensor_Adaptation_Guide.md]\n");

}

int main(int argc, char *argv[])
{
    int i = argc;

    signal(SIGINT, sig_handler);

    if (argc > 1) {
        for (int i = 1; i < argc; i += 2) {
            if (strcmp(argv[i], "-i") == 0) {
                g_bulk = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-m") == 0) {
                g_imagesize = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-u") == 0) {
                g_bulk_size = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-s0") == 0) {
                g_sensor0 = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-s1") == 0) {
                g_sensor1 = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-o") == 0) {
                strcpy(g_out_path, argv[i + 1]);
            }
            else if (strcmp(argv[i], "-help") == 0) {
                usage();
                return 0;
            }
            else
            {
                printf("Invalid option\n");
                usage();
                return 0;
            }
        }
    }

    if (create_uvc_cache() != 0)
    {
        goto err_exit;
    }

    if (get_kcamera()->init() != 0 ||
        get_kcamera()->open() != 0 ||
        get_kcamera()->run() != 0)
    {
        get_kcamera()->close();
        goto err_exit;
    }

    printf("UVC sample exit!\n");
err_exit:
    destroy_uvc_cache();

    printf("camera exit\n");
    return 0;
}
