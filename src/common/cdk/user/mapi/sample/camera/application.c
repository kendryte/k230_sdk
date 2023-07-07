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

unsigned int g_bulk = 1;
unsigned int g_standalone = 0;
unsigned int g_imagesize = 409600;
unsigned int g_bulk_size = 409600;
unsigned int g_cache_count = 20;
unsigned int g_sensor = 3;

void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s [options]\n", argv0);
    fprintf(stderr, "Available options are\n");
    fprintf(stderr, " -i		Use ISO mode\n");
    fprintf(stderr, " -s		Do not use any real camera device\n");
    fprintf(stderr, " -h		Print this help screen and exit\n");
    fprintf(stderr, " -c		Number of frame cache buffers\n");
    fprintf(stderr, " -m		Size of iso image\n");
    fprintf(stderr, " -u		Size of bulk image\n");
    fprintf(stderr, " -t		sensor type num\n");
    fprintf(stderr, "               0: ov9732\n");
    fprintf(stderr, "               1: ov9286 ir\n");
    fprintf(stderr, "               2: ov9286 speckle\n");
    fprintf(stderr, "               3: imx335 2LANE 1920Wx1080H\n");
    fprintf(stderr, "               4: imx335 2LANE 2592Wx1944H\n");
    fprintf(stderr, "               5: imx335 4LANE 2592Wx1944H\n");
    fprintf(stderr, "               6: imx335 2LANE MCLK 7425 1920Wx1080H\n");
    fprintf(stderr, "               7: imx335 2LANE MCLK 7425 2592Wx1944H\n");
    fprintf(stderr, "               8: imx335 4LANE MCLK 7425 2592Wx1944H\n");
}

int main(int argc, char *argv[])
{
    int i = argc;
    int opt;

    while ((opt = getopt(argc, argv, "ishc:m:u:t:")) != -1) {
        switch (opt) {
        case 'i':
            g_bulk = 0;
            break;

        case 's':
            g_standalone = 1;
            break;

        case 'h':
            usage(argv[0]);
            return 1;

        case 'c':
            if (atoi(optarg) < 2 || atoi(optarg) > 32) {
                usage(argv[0]);
                return 1;
            }

            g_cache_count = atoi(optarg);
            break;

        case 'm':
            g_imagesize = atoi(optarg);
            break;

        case 'u':
            g_bulk_size = atoi(optarg);
            break;

        case 't':
            g_sensor = atoi(optarg);
            break;
            
        default:
            printf("Invalid option '-%c'\n", opt);
            usage(argv[0]);
            return 1;
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
        goto err_exit;
    }

err_exit:
    get_kcamera()->close();

    destroy_uvc_cache();

    printf("camera exit\n");
    return 0;
}
