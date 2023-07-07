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

#include "sample_dpu_vo.h"

/* dpu file path define */
#define PARAM_PATH          "/sharefs/H1280W720_conf.bin"
#define REF_PATH            "/sharefs/H1280W720_ref.bin"

k_dpu_init_t dpu_init;
k_dpu_dev_attr_t dpu_dev_attr;
k_dpu_chn_lcn_attr_t lcn_attr;
k_dpu_chn_ir_attr_t ir_attr;

/* dpu global variable */
k_dpu_user_space_t g_temp_space;

int sample_dv_dpu_init()
{
    k_s32 ret;

    /************************************************************
     * This part is the demo that actually starts to use DPU 
     ***********************************************************/
    /* dpu init */
    dpu_init.start_num = 0;
    dpu_init.buffer_num = 3;
    ret = kd_mpi_dpu_init(&dpu_init);
    if (ret) {
        printf("kd_mpi_dpu_init failed\n");
        goto err_return;
    }

    /* parse file */
    ret = kd_mpi_dpu_parse_file(PARAM_PATH, 
                                &dpu_dev_attr.dev_param, 
                                &lcn_attr.lcn_param,
                                &ir_attr.ir_param,
                                &g_temp_space);
    // printf("g_temp_space.virt_addr:%p, g_temp_space.phys_addr:%lx\n", 
    //     g_temp_space.virt_addr, g_temp_space.phys_addr);
    if (g_temp_space.virt_addr == NULL) {
        printf("g_temp_space.virt_addr is NULL\n");
        goto err_return;
    }
    if (ret) {
        printf("kd_mpi_dpu_parse_file failed\n");
        goto err_return;
    }

    /* set device attribute */
    dpu_dev_attr.mode = DPU_BIND;
    dpu_dev_attr.tytz_temp_recfg = K_TRUE;
    dpu_dev_attr.align_depth_recfg = K_TRUE;
    dpu_dev_attr.param_valid = 123;
    dpu_dev_attr.dev_param.spp.flag_align = K_FALSE;

    ret = kd_mpi_dpu_set_dev_attr(&dpu_dev_attr);
    if (ret) {
        printf("kd_mpi_dpu_set_dev_attr failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_dev_attr success\n");

    /* set reference image */
    ret = kd_mpi_dpu_set_ref_image(REF_PATH);
    if (ret) {
        printf("kd_mpi_dpu_set_ref_image failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_ref_image success\n");

    /* set template image */
    ret = kd_mpi_dpu_set_template_image(&g_temp_space);
    if (ret) {
        printf("kd_mpi_dpu_set_template_image failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_template_image success\n");

    /* start dev */
    ret = kd_mpi_dpu_start_dev();
    if (ret) {
        printf("kd_mpi_dpu_start_dev failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_start_dev success\n");

    /* set chn attr */
    lcn_attr.chn_num = 0;
    lcn_attr.param_valid = 0;
    ir_attr.chn_num = 1;
    ir_attr.param_valid = 0;
    ret = kd_mpi_dpu_set_chn_attr(&lcn_attr, &ir_attr);
    if (ret) {
        printf("kd_mpi_dpu_set_chn_attr failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_set_chn_attr success\n");

    /* start channel 0 */
    ret = kd_mpi_dpu_start_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_start_chn 0 failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_start_chn lcn success\n");

    return K_SUCCESS;

    /************************************************************
     * This part is used to stop the DPU 
     ***********************************************************/
    ret = kd_mpi_dpu_stop_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

err_dpu_dev:
    ret = kd_mpi_dpu_stop_dev();
    if (ret) {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_dev success\n");

err_dpu_delet:
    kd_mpi_dpu_delete();

err_return:
    return K_FAILED;
}

int sample_dv_dpu_delete()
{
    k_s32 ret;

    ret = kd_mpi_dpu_stop_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

    ret = kd_mpi_dpu_stop_dev();
    if (ret) {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_dev success\n");

    kd_mpi_dpu_delete();

    return K_SUCCESS;
}