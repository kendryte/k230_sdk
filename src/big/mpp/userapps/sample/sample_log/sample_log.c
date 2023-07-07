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
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_dma_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"
#include "k_log_comm.h"

char buf[1024];
int main(void)
{
    int ret;
    k_log_level_conf conf;

    conf.level = 3;
    conf.mod_id = K_ID_VB;
    strcpy(conf.mod_name, "vb");
    ret = kd_mpi_log_set_level_conf(&conf);
    printf("\nset level conf ret:0x%08x level:%d\n", ret, conf.level);

    memset(&conf, 0, sizeof(conf));
    conf.mod_id = K_ID_VB;
    strcpy(conf.mod_name, "vb");
    ret = kd_mpi_log_get_level_conf(&conf);
    printf("get level conf ret:0x%08x level:%d\n", ret, conf.level);

    ret = kd_mpi_log_set_wait_flag(K_FALSE);
    printf("set wait flag 0 ret:0x%08x \n", ret);

    kd_mpi_log_read(buf, 1024);
    printf("log read ret:0x%08x buf:%s \n", ret, buf);

    return 0;
}