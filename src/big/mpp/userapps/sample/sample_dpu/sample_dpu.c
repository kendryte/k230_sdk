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
#include <sys/stat.h>
#include <pthread.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_dpu_comm.h"
#include "k_vvi_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_dpu_api.h"
#include "mpi_vvi_api.h"
#include "mpi_sys_api.h"

#define PARAM_PATH          "base_4_param.bin"
#define REF_PATH            "base_4_ref.bin"
#define PROC_REF_PATH       "base_4_proc_ref.bin"
#define LCN_PATH            "lcn.bin"
#define IR_PATH             "ir.bin"

#define DEPTH_OUT           "gb_align_depth_out_addr.bin"
#define QUANLITY_OUT        "gb_quanlity_out_addr.bin"
#define IR_OUT              "gb_align_ir_out_addr.bin"
#define DISP_OUT_X          "gb_sad_disp_out_x_addr0.bin"
#define DISP_OUT_XY         "gb_init_sad_disp_out_xy_addr.bin"

#define MAX_PATH_LENGTH     100
#define ERROR_BYTES_THRESHOLD 5

char g_param_path[MAX_PATH_LENGTH];
char g_ref_path[MAX_PATH_LENGTH];
char g_proc_ref_path[MAX_PATH_LENGTH];
char g_lcn_path[MAX_PATH_LENGTH];
char g_ir_path[MAX_PATH_LENGTH];

char g_depth_out[MAX_PATH_LENGTH];
char g_quanlity_out[MAX_PATH_LENGTH];
char g_ir_out[MAX_PATH_LENGTH];
char g_disp_out_x[MAX_PATH_LENGTH];
char g_disp_out_xy[MAX_PATH_LENGTH];

#define SAMPE_VVI_PIPE_NUMS     2
#define SAMPLE_VVI_FRAME_RATE   1
#define DPU_FRAME_COUNT         3

#define VB_BLK_SIZE         (1024 * 1024 * 2)

k_dpu_user_space_t g_lcn_space;
k_dpu_user_space_t g_ir_space;
k_dpu_user_space_t g_temp_space;

k_dpu_user_space_t g_depth_out_space;
k_dpu_user_space_t g_disp_out_space;
k_dpu_user_space_t g_ir_out_space;
k_dpu_user_space_t g_disp_out_x_space;
k_dpu_user_space_t g_disp_out_xy_space;
k_dpu_user_space_t g_qlt_out_space;

k_video_frame_info insert_pic_info[2];

static pthread_t tid1;
k_bool thread_exit = K_FALSE;

typedef struct {
    k_u32 dev_num;
    k_u32 chn_num;
    k_u32 dev_height;
    k_u32 dev_width;
    k_pixel_format dev_format;
    k_u32 chn_height;
    k_u32 chn_width;
    k_pixel_format chn_format;
} sample_vvi_pipe_conf_t;

sample_vvi_pipe_conf_t g_pipe_conf[SAMPE_VVI_PIPE_NUMS] =
{
    {
        0,
        0,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        VB_BLK_SIZE / 4,
        1,
        PIXEL_FORMAT_ARGB_8888,
    },
    {
        0,
        1,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        VB_BLK_SIZE / 4,
        1,
        PIXEL_FORMAT_ARGB_8888,
    },
};


static k_s32 sample_dpu_help_argu(int argc, char *argv[], k_dpu_mode_e *dpu_mode, k_bool *use_proc_ref)
{
    if (argc < 2) {
        return K_FAILED;
    }

    memset(g_param_path, 0, MAX_PATH_LENGTH);
    strcat(g_param_path, argv[1]);
    strcat(g_param_path, PARAM_PATH);

    memset(g_ref_path, 0, MAX_PATH_LENGTH);
    strcat(g_ref_path, argv[1]);
    strcat(g_ref_path, REF_PATH);

    memset(g_proc_ref_path, 0, MAX_PATH_LENGTH);
    strcat(g_proc_ref_path, argv[1]);
    strcat(g_proc_ref_path, PROC_REF_PATH);

    memset(g_lcn_path, 0, MAX_PATH_LENGTH);
    strcat(g_lcn_path, argv[1]);
    strcat(g_lcn_path, LCN_PATH);

    memset(g_ir_path, 0, MAX_PATH_LENGTH);
    strcat(g_ir_path, argv[1]);
    strcat(g_ir_path, IR_PATH);

    memset(g_depth_out, 0, MAX_PATH_LENGTH);
    strcat(g_depth_out, argv[1]);
    strcat(g_depth_out, DEPTH_OUT);

    memset(g_quanlity_out, 0, MAX_PATH_LENGTH);
    strcat(g_quanlity_out, argv[1]);
    strcat(g_quanlity_out, QUANLITY_OUT);

    memset(g_ir_out, 0, MAX_PATH_LENGTH);
    strcat(g_ir_out, argv[1]);
    strcat(g_ir_out, IR_OUT);

    memset(g_disp_out_x, 0, MAX_PATH_LENGTH);
    strcat(g_disp_out_x, argv[1]);
    strcat(g_disp_out_x, DISP_OUT_X);

    memset(g_disp_out_xy, 0, MAX_PATH_LENGTH);
    strcat(g_disp_out_xy, argv[1]);
    strcat(g_disp_out_xy, DISP_OUT_XY);

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Please input:\n");
            printf("\"./sample_dpu.elf file_path BOUND ORIGIN_REF\" or \"./sample_dpu.elf file_path UNBOUND ORIGIN_REF\" or \n \
                  \r\"./sample_dpu.elf file_path BOUND PROC_REF  \" or \"./sample_dpu.elf file_path UNBOUND PROC_REF  \". \n \
                  \rThe default mode is UNBOUND, use origin reference image.\n");
            return K_SUCCESS;
        } else if (strcmp(argv[i], "BOUND") == 0) {
            *dpu_mode = DPU_BIND;
        } else if (strcmp(argv[i], "UNBOUND") == 0) {
            *dpu_mode = DPU_UNBIND;
        } else if (strcmp(argv[i], "ORIGIN_REF") == 0) {
            *use_proc_ref = K_FALSE;
        } else if (strcmp(argv[i], "PROC_REF") == 0) {
            *use_proc_ref = K_TRUE;
        }
    }

    return K_SUCCESS;
}

static k_s32 dpu_vb_init()
{
    k_s32 ret;
    k_vb_config config;
    k_s32 i;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    for (i = 0; i < 1; i++) {
        config.comm_pool[i].blk_cnt = DPU_FRAME_COUNT + 4;
        config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
        config.comm_pool[i].blk_size = VB_BLK_SIZE;
    }
    ret = kd_mpi_vb_set_config(&config);
    if(ret)
        printf("vb_set_config failed ret:%d\n", ret);

    k_vb_supplement_config supplement_config;
    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    if(ret)
        printf("vb_set_supplement_config failed ret:%d\n", ret);
    ret = kd_mpi_vb_init();
    if(ret)
        printf("vb_init failed ret:%d\n", ret);
    return ret;
}

static k_s32 dpu_vb_exit()
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static k_s32 sample_vvi_bind_dpu()
{
    k_s32 ret;
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn dpu_mpp_chn;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 0;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_bind(&vvi_mpp_chn, &dpu_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
        return ret;
    }

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 1;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_bind(&vvi_mpp_chn, &dpu_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x:%x\n", ret);
        return ret;
    }

    return K_SUCCESS;
}

static k_s32 sample_vvi_unbind_dpu()
{
    k_s32 ret;
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn dpu_mpp_chn;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 0;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_unbind(&vvi_mpp_chn, &dpu_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
        return ret;
    }

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 1;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_unbind(&vvi_mpp_chn, &dpu_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x:%x\n", ret);
        return ret;
    }

    return K_SUCCESS;
}

static k_s32 dpu_userspace_mmz(k_dpu_user_space_t *data, const char *name, const char *path)
{
    k_s32 ret;
    k_u32 size;
    struct stat file_stat;
    FILE *file;

    data->virt_addr = NULL;

    /* Get file size */
    if (stat(path, &file_stat)) {
        printf("Failed to get file %-40s size\n", path);
        return K_ERR_DPU_ILLEGAL_PARAM;
    }
    size = file_stat.st_size;
    printf("%-46s size:0x%x\n", path, size);
    ret = kd_mpi_sys_mmz_alloc_cached(&data->phys_addr, &data->virt_addr,
        name, "anonymous", size);
    if (ret != K_ERR_OK) {
        printf("[ERROR]: kd_mpi_sys_mmz_alloc return 0x%08x\n",ret);
        return K_FAILED;
    } else {
        // printf("data alloc result phyaddr:0x%lx virtaddr:%p\n",
        //     data->phys_addr, data->virt_addr);
    }
    data->size = size;

    /* Get file */
    file = fopen(path, "rb");
    if (file == NULL) {
        printf("Open file failed\n");
        return K_ERR_DPU_ILLEGAL_PARAM;
    }
    ret = fread(data->virt_addr, 1, size, file);
    if (ret != size) {
        printf("Read %-40s error\n", path);
        return K_ERR_DPU_ILLEGAL_PARAM;
    }
    kd_mpi_sys_mmz_flush_cache(data->phys_addr, data->virt_addr, size);
    fclose(file);

    return K_SUCCESS;
}

static k_s32 dpu_golden_data_prepare()
{
    k_s32 ret;

    ret = dpu_userspace_mmz(&g_lcn_space, "lcn", g_lcn_path);
    if (ret)
        return K_ERR_DPU_NOMEM;

    ret = dpu_userspace_mmz(&g_ir_space, "ir", g_ir_path);
    if (ret)
        return K_ERR_DPU_NOMEM;

    ret = dpu_userspace_mmz(&g_depth_out_space, "depth", g_depth_out);
    if (ret)
        return K_ERR_DPU_NOMEM;

    ret = dpu_userspace_mmz(&g_ir_out_space, "ir", g_ir_out);
    if (ret)
        return K_ERR_DPU_NOMEM;

    ret = dpu_userspace_mmz(&g_disp_out_x_space, "disp_x", g_disp_out_x);
    if (ret)
        return K_ERR_DPU_NOMEM;

    ret = dpu_userspace_mmz(&g_disp_out_xy_space, "disp_xy", g_disp_out_xy);
    if (ret)
        return K_ERR_DPU_NOMEM;

    ret = dpu_userspace_mmz(&g_qlt_out_space, "qlt", g_quanlity_out);
    if (ret)
        return K_ERR_DPU_NOMEM;

    return K_SUCCESS;
}

static k_s32 dpu_golden_data_release()
{
    kd_mpi_sys_mmz_free(g_lcn_space.phys_addr, NULL);
    kd_mpi_sys_mmz_free(g_ir_space.phys_addr, NULL);
    kd_mpi_sys_mmz_free(g_depth_out_space.phys_addr, NULL);
    kd_mpi_sys_mmz_free(g_ir_out_space.phys_addr, NULL);
    kd_mpi_sys_mmz_free(g_disp_out_x_space.phys_addr, NULL);
    kd_mpi_sys_mmz_free(g_disp_out_xy_space.phys_addr, NULL);
    kd_mpi_sys_mmz_free(g_qlt_out_space.phys_addr, NULL);

    return K_SUCCESS;
}

static k_s32 dpu_input_data_prepare_bind(k_video_frame_info *vf_info, k_u8 *src_addr, void **dst_vaddr, k_u32 length)
{
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_s32 size;

    if (vf_info == NULL)
        return K_FALSE;

    size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    printf("%s,%d, size:%x\n", __func__, __LINE__, size);
    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, size, NULL);
    if(handle == VB_INVALID_HANDLE) {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if(pool_id == VB_INVALID_POOLID) {
        printf("%s get pool id error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if(phys_addr == 0) {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }
    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
    memcpy(virt_addr, src_addr, length);

    vf_info->mod_id = K_ID_V_VI;
    vf_info->pool_id = pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    vf_info->v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    vf_info->v_frame.virt_addr[0] = 0;
    *dst_vaddr = virt_addr;
    // printf("%s,%d, pool_id:%d, phys_addr:%lx\n",
    //     __func__, __LINE__, vf_info->pool_id, vf_info->v_frame.phys_addr[0]);

    return K_SUCCESS;
}

static k_s32 dpu_input_data_release_bind(k_video_frame_info *vf_info, const void *virt_addr)
{
    k_s32 ret;
    k_s32 size;
    k_vb_blk_handle handle;

    size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    if (vf_info == NULL)
        return K_FALSE;
    handle = kd_mpi_vb_phyaddr_to_handle(vf_info->v_frame.phys_addr[0]);
    if(handle != VB_INVALID_HANDLE) {
        ret = kd_mpi_vb_release_block(handle);
    } else {
        ret = K_FAILED;
    }
    if(virt_addr) {
        kd_mpi_sys_munmap((void *)virt_addr, size);
        printf("%s,%d\n", __func__, __LINE__);
    }

    return ret;
}

static k_s32 sample_vvi_prepare(sample_vvi_pipe_conf_t* pipe_conf)
{
    k_s32 i;
    k_vvi_dev_attr dev_attr;
    k_vvi_chn_attr chn_attr;

    for(i = 0; i < SAMPE_VVI_PIPE_NUMS; i++) {
        memset(&chn_attr, 0, sizeof(chn_attr));
        memset(&dev_attr, 0, sizeof(dev_attr));
        dev_attr.format = pipe_conf[i].dev_format;
        dev_attr.height = pipe_conf[i].dev_height;
        dev_attr.width = pipe_conf[i].dev_width;
        chn_attr.frame_rate = SAMPLE_VVI_FRAME_RATE;
        chn_attr.format = pipe_conf[i].chn_format;
        chn_attr.height = pipe_conf[i].chn_height;
        chn_attr.width = pipe_conf[i].chn_width;
        printf("pipe[%d] dev[%d] h:%d w:%d chn[%d] h:%d w:%d \n", i,
                pipe_conf[i].dev_num, pipe_conf[i].dev_height, pipe_conf[i].dev_width,
                pipe_conf[i].chn_num, pipe_conf[i].chn_height, pipe_conf[i].chn_width);
        kd_mpi_vvi_set_dev_attr(pipe_conf[i].dev_num, &dev_attr);
        kd_mpi_vvi_set_chn_attr(pipe_conf[i].chn_num, &chn_attr);
    }

    for(i = 0; i < SAMPE_VVI_PIPE_NUMS; i++) {
        kd_mpi_vvi_start_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    }

    kd_mpi_vvi_chn_insert_pic(g_pipe_conf[0].chn_num, &insert_pic_info[0]);
    kd_mpi_vvi_chn_insert_pic(g_pipe_conf[1].chn_num, &insert_pic_info[1]);

    return K_SUCCESS;
}

static void sample_vvi_stop(sample_vvi_pipe_conf_t* pipe_conf)
{
    k_s32 i;

    for(i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
        kd_mpi_vvi_stop_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    return;
}

static k_s32 result_check(k_u8 *data, k_u8 *golden, k_u32 length)
{
    k_u32 i;
    k_s32 err_cnt=0;

    for (i = 0; i < length; i++) {
        if (data[i] != golden[i]) {
            //printf("compare error! data[%-5d]:%02x, golden[%-5d]:%02x\n", i, data[i], i, golden[i]);
            err_cnt++;
        }
    }
    return err_cnt;
}

static k_s32 dpu_result_check(k_dpu_chn_result_u *result, k_u32 chn_num)
{
    k_s32 ret;
    k_u8 *virt_result;
    k_dpu_chn_lcn_result_t *lcn;
    k_dpu_chn_ir_result_t *ir;

    if (chn_num == 0) {
        lcn = &result->lcn_result;
        if (lcn->depth_out.valid) {
            // printf("%s,%d, depth_phys_addr:%lx, vb.length:%d, array.length:%d\n",
            //     __func__, __LINE__, lcn->depth_out.depth_phys_addr, lcn->depth_out.length, g_depth_out_space.size);
            virt_result = (k_u8 *)kd_mpi_sys_mmap(lcn->depth_out.depth_phys_addr, lcn->depth_out.length);
            ret = result_check(virt_result, g_depth_out_space.virt_addr, lcn->depth_out.length);
            kd_mpi_sys_munmap(virt_result, lcn->depth_out.length);
            if (ret > ERROR_BYTES_THRESHOLD)
            {
                printf("depth_out error bytes %d\n", ret);
                return ret;
            }
            printf("%-15s check pass\n", "depth_out");

        }
        if (lcn->disp_out.valid) {
            // printf("%s,%d, disp_phys_addr:%lx, vb.length:%d, array.length:%d\n",
            //     __func__, __LINE__, lcn->disp_out.disp_phys_addr, lcn->disp_out.length, g_disp_out_space.size);
            virt_result = (k_u8 *)kd_mpi_sys_mmap(lcn->disp_out.disp_phys_addr, lcn->disp_out.length);
            ret = result_check(virt_result, g_disp_out_space.virt_addr, lcn->disp_out.length);
            kd_mpi_sys_munmap(virt_result, lcn->disp_out.length);
            if (ret > ERROR_BYTES_THRESHOLD)
            {
                printf("disp_out error bytes %d\n", ret);
                return ret;
            }
            printf("%-15s check pass\n", "disp_out");
        }
        if (lcn->qlt_out.valid) {
            // printf("%s,%d, qlt_phys_addr:%lx, vb.length:%d, array.length:%d\n",
            //     __func__, __LINE__, lcn->qlt_out.qlt_phys_addr, lcn->qlt_out.qlt_length, g_qlt_out_space.size);
            virt_result = (k_u8 *)kd_mpi_sys_mmap(lcn->qlt_out.qlt_phys_addr, lcn->qlt_out.qlt_length);
            ret = result_check(virt_result, g_qlt_out_space.virt_addr, lcn->qlt_out.qlt_length);
            kd_mpi_sys_munmap(virt_result, lcn->qlt_out.qlt_length);
            if (ret > 3)
            {
                printf("qlt_out error bytes %d\n", ret);
                return ret;
            }
            printf("%-15s check pass\n", "qlt_out");

            // printf("%s,%d, sad_disp_phys_addr:%lx, vb.length:%d, array.length:%d\n",
            //     __func__, __LINE__, lcn->qlt_out.sad_disp_phys_addr, lcn->qlt_out.sad_disp_length, g_disp_out_x_space.size);
            virt_result = (k_u8 *)kd_mpi_sys_mmap(lcn->qlt_out.sad_disp_phys_addr, lcn->qlt_out.sad_disp_length);
            ret = result_check(virt_result, g_disp_out_x_space.virt_addr, lcn->qlt_out.sad_disp_length);
            kd_mpi_sys_munmap(virt_result, lcn->qlt_out.sad_disp_length);
            if (ret > ERROR_BYTES_THRESHOLD)
            {
                printf("sad_disp error bytes %d\n", ret);
                return ret;
            }
            printf("%-15s check pass\n", "sad_disp");

            // printf("%s,%d, init_sad_disp_phys_addr:%lx, vb.length:%d, array.length:%d\n",
            //     __func__, __LINE__, lcn->qlt_out.init_sad_disp_phys_addr, lcn->qlt_out.init_sad_disp_length, g_disp_out_xy_space.size);
            virt_result = (k_u8 *)kd_mpi_sys_mmap(lcn->qlt_out.init_sad_disp_phys_addr, lcn->qlt_out.init_sad_disp_length);
            ret = result_check(virt_result, g_disp_out_xy_space.virt_addr, lcn->qlt_out.init_sad_disp_length);
            kd_mpi_sys_munmap(virt_result, lcn->qlt_out.init_sad_disp_length);
            if (ret > ERROR_BYTES_THRESHOLD)
            {
                printf("init_sad_disp error bytes %d\n", ret);
                return ret;
            }
            printf("%-15s check pass\n", "init_sad_disp");
        }
    } else if (chn_num == 1) {
        ir = &result->ir_result;
        if (ir->ir_out.valid) {
            virt_result = (k_u8 *)kd_mpi_sys_mmap(ir->ir_out.ir_phys_addr, ir->ir_out.length);
            ret = result_check(virt_result, g_ir_out_space.virt_addr, ir->ir_out.length);
            kd_mpi_sys_munmap(virt_result, ir->ir_out.length);
            if (ret > ERROR_BYTES_THRESHOLD)
            {
                printf("ir_out error bytes %d\n", ret);
                return ret;
            }
            printf("%-15s check pass\n", "ir");
        }
    } else {
        printf("illegal parameter\n");
        return K_FAILED;
    }

    printf("chn %d check pass\n", chn_num);
    return K_SUCCESS;
}


k_s32 sample_dpu_unbound_mode(k_dpu_dev_attr_t *dev_attr, k_dpu_chn_lcn_attr_t *lcn_attr,
    k_dpu_chn_ir_attr_t *ir_attr)
{
    k_s32 ret;
    k_s32 test_nums = 10;
    k_s32 err_times = 0;
    k_dpu_chn_result_u lcn_result = {0};
    k_dpu_chn_result_u ir_result = {0};

    for (int tmp = 0; tmp < test_nums; tmp++) {
        if (tmp == 5) {
            dev_attr->mode = DPU_UNBIND;
            dev_attr->tytz_temp_recfg = K_TRUE;
            dev_attr->align_depth_recfg = K_TRUE;
            dev_attr->param_valid = 144;

            /* set device attribute */
            ret = kd_mpi_dpu_set_dev_attr(dev_attr);
            if (ret) {
                printf("kd_mpi_dpu_set_dev_attr failed\n");
                return 0;
            }
        }

        if (tmp == 7) {
            /* set chn attr */
            lcn_attr->chn_num = 0;
            lcn_attr->param_valid = 121;
            ir_attr->chn_num = 1;
            ir_attr->param_valid = 131;
            ret = kd_mpi_dpu_set_chn_attr(lcn_attr, ir_attr);
            if (ret) {
                printf("kd_mpi_dpu_set_chn_attr failed\n");
                return 0;
            }
        }

        ret = kd_mpi_dpu_send_frame(0, g_lcn_space.phys_addr, 100);
        if (ret) {
            printf("kd_mpi_dpu_send_frame lcn failed\n");
            return 0;
        }

        ret = kd_mpi_dpu_send_frame(1, g_ir_space.phys_addr, 100);
        if (ret) {
            printf("kd_mpi_dpu_send_frame ir failed\n");
            return 0;
        }

        ret = kd_mpi_dpu_get_frame(0, &lcn_result, 100);
        if (ret) {
            printf("kd_mpi_dpu_get_frame failed\n");
            return 0;
        }
        if (dpu_result_check(&lcn_result, 0))
            err_times++;


        ret = kd_mpi_dpu_get_frame(1, &ir_result, 100);
        if (ret) {
            printf("kd_mpi_dpu_get_frame failed\n");
            return 0;
        }
        if (dpu_result_check(&ir_result, 1))
            err_times++;

        kd_mpi_dpu_release_frame();

        usleep(30000);

        printf("\n****************frame num %d end*******************\n", tmp);
    }
    printf("all times:%d, error times:%d\n", test_nums*2, err_times);

    return K_SUCCESS;
}

static void *bind_mode_get_frame(void *parameter)
{
    k_s32 ret;
    k_dpu_chn_result_u lcn_result = {0};
    k_dpu_chn_result_u ir_result = {0};
    k_dpu_chn_result_u lcn_result_save = {0};

    printf("%s,%d\n", __func__, __LINE__);

    while(1) {
        ret = kd_mpi_dpu_get_frame(0, &lcn_result, 1100);
        ret = kd_mpi_dpu_get_frame(1, &ir_result, 1100);

        if (SAMPLE_VVI_FRAME_RATE == 1) {
            /* Save result. If the frame rate is set to 30, please
            remove this 'if' code, because saving the file will take a long time. */
            if (ret == K_SUCCESS) {
                void *depth_virt_addr = NULL;
                k_char depth_filename[256] = "/sharefs/depthout.bin";
                depth_virt_addr = kd_mpi_sys_mmap(lcn_result.lcn_result.depth_out.depth_phys_addr, lcn_result.lcn_result.depth_out.length);
                if (depth_virt_addr) {
                    FILE *file = fopen(depth_filename, "wb+");
                    if (file) {
                        fwrite(depth_virt_addr, 1, lcn_result.lcn_result.depth_out.length, file);
                        fclose(file);
                        printf("save depthout file success\n");
                    } else {
                        printf("can't create file\n");
                    }
                } else {
                    printf("save depthout file failed\n");
                }
            }
        } else {
            if (ret == K_SUCCESS) {
                memcpy(&lcn_result_save, &lcn_result, sizeof(k_dpu_chn_result_u));
            }
        }

        if (ret == K_SUCCESS) {
            printf("Successfully obtained and will release.\n");
            kd_mpi_dpu_release_frame();
        }

        if (thread_exit) {
            while (1) {
                ret = kd_mpi_dpu_get_frame(0, &lcn_result, 30);
                ret = kd_mpi_dpu_get_frame(1, &ir_result, 30);
                if (ret) {
                    break;
                }
                kd_mpi_dpu_release_frame();
            }
            break;
        }
    }

    if (SAMPLE_VVI_FRAME_RATE > 1) {
        void *depth_virt_addr = NULL;
        k_char depth_filename[256] = "/sharefs/depthout.bin";
        depth_virt_addr = kd_mpi_sys_mmap(lcn_result_save.lcn_result.depth_out.depth_phys_addr, lcn_result_save.lcn_result.depth_out.length);
        if (depth_virt_addr) {
            FILE *file = fopen(depth_filename, "wb+");
            if (file) {
                fwrite(depth_virt_addr, 1, lcn_result_save.lcn_result.depth_out.length, file);
                fclose(file);
                printf("save depthout file success\n");
            } else {
                printf("can't create file\n");
            }
        } else {
            printf("save depthout file failed\n");
        }
    }


    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    k_s32 ret;
    k_dpu_init_t dpu_init;
    k_dpu_dev_attr_t dev_attr;
    k_dpu_chn_lcn_attr_t lcn_attr;
    k_dpu_chn_ir_attr_t ir_attr;
    k_dpu_mode_e dpu_mode = DPU_UNBIND;
    k_bool use_proc_ref = K_FALSE;
    void* lcn_vaddr = NULL;
    void* ir_vaddr = NULL;

    printf("dpu sample case test1\n");

    ret = sample_dpu_help_argu(argc, argv, &dpu_mode, &use_proc_ref);
    if (ret) {
        printf("Enter ./sample_dpu.elf --help to view the correct input format\n");
        return K_SUCCESS;
    }
    printf("begin %-10s mode test\n", dpu_mode ? "UNBOUND" : "BOUND");

    /************************************************************
     * This part is doing the preparatory work, which includes
     * the init of the vb pool and whether to bind the pipeline.
     ***********************************************************/
    ret = dpu_vb_init(argc, argv);
    if (ret) {
        printf("dpu_vb_init failed\n");
        return 0;
    }

    ret = dpu_golden_data_prepare();
    if (ret) {
        printf("dpu_input_data_prepare_unbind failed\n");
        goto err_vb_exit;
    }

    if (dpu_mode == DPU_BIND) {
        ret = sample_vvi_bind_dpu();
        if (ret) {
            printf("sample_vvi_bind_dpu failed\n");
            goto err_input_rels;
        } else {
            printf("sample_vvi_bind_dpu success\n");
        }

        for (int i = 0; i < SAMPE_VVI_PIPE_NUMS; i++) {
            insert_pic_info[i].v_frame.width = g_pipe_conf[i].chn_width;
            insert_pic_info[i].v_frame.height = g_pipe_conf[i].chn_height;
            insert_pic_info[i].v_frame.pixel_format = g_pipe_conf[i].chn_format;
        }

        ret = dpu_input_data_prepare_bind(&insert_pic_info[0], (k_u8 *)g_lcn_space.virt_addr, &lcn_vaddr, g_lcn_space.size);
        if (ret) {
            printf("dpu_input_data_prepare_bind failed\n");
            goto err_unbind;
        }
        ret = dpu_input_data_prepare_bind(&insert_pic_info[1], (k_u8 *)g_ir_space.virt_addr, &ir_vaddr, g_ir_space.size);
        if (ret) {
            printf("dpu_input_data_prepare_bind failed\n");
            goto err_unbind;
        }
    }

    /************************************************************
     * This part is the demo that actually starts to use DPU
     ***********************************************************/
    /* dpu init */
    dpu_init.start_num = 0;
    dpu_init.buffer_num = 3;
    ret = kd_mpi_dpu_init(&dpu_init);
    if (ret) {
        printf("kd_mpi_dpu_init failed\n");
        goto err_delet;
    }

    /* parse file */
    ret = kd_mpi_dpu_parse_file(g_param_path,
                                &dev_attr.dev_param,
                                &lcn_attr.lcn_param,
                                &ir_attr.ir_param,
                                &g_temp_space);
    // printf("g_temp_space.virt_addr:%p, g_temp_space.phys_addr:%lx\n",
    //     g_temp_space.virt_addr, g_temp_space.phys_addr);
    if (g_temp_space.virt_addr == NULL) {
        printf("g_temp_space.virt_addr is NULL\n");
        goto err_delet;
    }
    if (ret) {
        printf("kd_mpi_dpu_parse_file failed\n");
        goto err_delet;
    }

    /* set device attribute */
    dev_attr.mode = dpu_mode;
    dev_attr.tytz_temp_recfg = K_TRUE;
    dev_attr.align_depth_recfg = K_TRUE;
    dev_attr.ir_never_open = K_FALSE;
    dev_attr.param_valid = 123;

    ret = kd_mpi_dpu_set_dev_attr(&dev_attr);
    if (ret) {
        printf("kd_mpi_dpu_set_dev_attr failed\n");
        goto err_delet;
    }
    printf("kd_mpi_dpu_set_dev_attr success\n");

    /* set reference image */
    if (!use_proc_ref) {
        ret = kd_mpi_dpu_set_ref_image(g_ref_path);
        if (ret) {
            printf("kd_mpi_dpu_set_ref_image failed\n");
            goto err_delet;
        }
        printf("kd_mpi_dpu_set_ref_image success\n");
    } else {
        ret = kd_mpi_dpu_set_processed_ref_image(g_proc_ref_path);
        if (ret) {
            printf("kd_mpi_dpu_set_processed_ref_image failed\n");
            goto err_delet;
        }
        printf("kd_mpi_dpu_set_processed_ref_image success\n");
    }


    /* set template image */
    ret = kd_mpi_dpu_set_template_image(&g_temp_space);
    if (ret) {
        printf("kd_mpi_dpu_set_template_image failed\n");
        goto err_delet;
    }
    printf("kd_mpi_dpu_set_template_image success\n");

    /* start dev */
    ret = kd_mpi_dpu_start_dev();
    if (ret) {
        printf("kd_mpi_dpu_start_dev failed\n");
        goto err_delet;
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
        goto err_stop_dev;
    }
    printf("kd_mpi_dpu_set_chn_attr success\n");
#if 1
    /* start channel 0 */
    ret = kd_mpi_dpu_start_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_start_chn 0 failed\n");
        goto err_stop_dev;
    }
    printf("kd_mpi_dpu_start_chn lcn success\n");

    /* start channel 1 */
    ret = kd_mpi_dpu_start_chn(1);
    if (ret) {
        printf("kd_mpi_dpu_start_chn failed\n");
        goto err_stop_chn0;
    }
    printf("kd_mpi_dpu_start_chn ir success\n");
#endif

    if (dpu_mode == DPU_BIND) {
        /* Config and start the previous module of the dpu, in the case is vvi. */
        printf("begin bound mode\n");
        sample_vvi_prepare(g_pipe_conf);
        ret = pthread_create(&tid1, NULL, bind_mode_get_frame, NULL);
        while ((char)getchar() != 'q');
        sample_vvi_stop(g_pipe_conf);
        thread_exit = K_TRUE;
        pthread_join(tid1, NULL);
    } else if (dpu_mode == DPU_UNBIND) {
        printf("begin unbound mode\n");
        sample_dpu_unbound_mode(&dev_attr, &lcn_attr, &ir_attr);
    }


    ret = kd_mpi_dpu_stop_chn(1);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn ir failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

err_stop_chn0:
    ret = kd_mpi_dpu_stop_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

err_stop_dev:
    ret = kd_mpi_dpu_stop_dev();
    if (ret) {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_dev success\n");

err_delet:
    kd_mpi_dpu_delete();

    if (dpu_mode == DPU_BIND) {
        dpu_input_data_release_bind(&insert_pic_info[0], lcn_vaddr);
        dpu_input_data_release_bind(&insert_pic_info[1], ir_vaddr);
    }

err_unbind:
    if (dpu_mode == DPU_BIND) {
        sample_vvi_unbind_dpu();
        printf("%s,%d\n", __func__, __LINE__);
    }

err_input_rels:
    dpu_golden_data_release();
    // printf("%s,%d\n", __func__, __LINE__);

err_vb_exit:
    dpu_vb_exit();
    // printf("%s,%d\n", __func__, __LINE__);
    printf("sample dpu done!");
    return 0;
}