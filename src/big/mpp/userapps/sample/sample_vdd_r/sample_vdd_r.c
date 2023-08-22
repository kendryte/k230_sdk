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

#include "sample_vdd_r.h"

sample_vdd_cfg_t g_vdd_cfg[SAMPLE_VDD_CHN_NUM];

static pthread_t tid1;
k_bool thread_exit = K_FALSE;

static k_s32 sample_vdd_vb_init()
{
    k_s32 ret;
    k_vb_config config;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    /* vi vb init */
    config.comm_pool[0].blk_cnt = DPU_FRAME_COUNT * 2 + 3;
    config.comm_pool[0].blk_size = g_vdd_cfg[0].img_height * g_vdd_cfg[0].img_width * 3 / 2;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;

    /* dma vb init */
    config.comm_pool[1].blk_cnt = DPU_FRAME_COUNT;
    config.comm_pool[1].blk_size = g_vdd_cfg[0].img_height * g_vdd_cfg[0].img_width;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    /* dpu vb init */
    config.comm_pool[2].blk_cnt = (DPU_FRAME_COUNT);
    config.comm_pool[2].blk_size = 5 * 1024 * 1024;
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;

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

static k_s32 sample_vdd_vb_exit()
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static k_s32 sample_vdd_bind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn dma_mpp_chn;
    k_mpp_chn dpu_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    dma_mpp_chn.mod_id = K_ID_DMA;
    dma_mpp_chn.dev_id = 0;
    dma_mpp_chn.chn_id = 0;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 0;

    ret = kd_mpi_sys_bind(&vi_mpp_chn, &dma_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_bind(&dma_mpp_chn, &dpu_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    return ret;
}

static k_s32 sample_vdd_unbind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn dma_mpp_chn;
    k_mpp_chn dpu_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    dma_mpp_chn.mod_id = K_ID_DMA;
    dma_mpp_chn.dev_id = 0;
    dma_mpp_chn.chn_id = 0;
    dpu_mpp_chn.mod_id = K_ID_DPU;
    dpu_mpp_chn.dev_id = 0;
    dpu_mpp_chn.chn_id = 0;

    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &dma_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_unbind(&dma_mpp_chn, &dpu_mpp_chn);
    VDD_CHECK_RET(ret, __func__, __LINE__);

    return ret;
}


static void *bind_mode_get_frame(void *parameter)
{
    k_s32 ret;
    k_dpu_chn_result_u lcn_result = {0};
    k_dpu_chn_result_u lcn_result_save = {0};

    printf("%s,%d\n", __func__, __LINE__);

    while(1) {
        ret = kd_mpi_dpu_get_frame(0, &lcn_result, 1100);

        if (ret == K_SUCCESS) {
            memcpy(&lcn_result_save, &lcn_result, sizeof(k_dpu_chn_result_u));
        }


        if (ret == K_SUCCESS) {
            printf("Successfully obtained and will release.\n");
            kd_mpi_dpu_release_frame();
        }

        if (thread_exit) {
            while (1) {
                ret = kd_mpi_dpu_get_frame(0, &lcn_result, 30);
                if (ret) {
                    break;
                }
                kd_mpi_dpu_release_frame();
            }
            break;
        }
    }

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


    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    k_s32 ret;

    g_vdd_cfg[0].img_height = DMA_CHN0_HEIGHT;
    g_vdd_cfg[0].img_width =DMA_CHN0_WIDTH;

    ret = sample_vdd_vb_init();
    if (ret) {
        printf("sample_vdd_vb_init failed\n");
        return ret;
    }

    ret = sample_vdd_bind();
    if (ret) {
        printf("sample_vdd_bind failed\n");
        goto err_vb_exit;
    }

    ret = sample_vdd_dpu_init();
    if (ret) {
        printf("sample_dpu_init failed\n");
        goto err_unbind;
    }

    ret = sample_vdd_dma_init();
    if (ret) {
        printf("sample_dma_init failed\n");
        goto err_dpu_delete;
    }

    /* vi config and start */
    sample_vdd_vicap_config(SAMPLE_VDD_CHN);
    sample_vdd_vicap_start(SAMPLE_VDD_CHN);
    ret = pthread_create(&tid1, NULL, bind_mode_get_frame, NULL);
    while ((char)getchar() != 'q');
    sample_vdd_vicap_stop(SAMPLE_VDD_CHN);
    thread_exit = K_TRUE;
    pthread_join(tid1, NULL);

    ret = sample_vdd_dma_delete();
    if (ret) {
        printf("sample_dma_delete failed\n");
        return 0;
    }

err_dpu_delete:
    ret = sample_vdd_dpu_delete();
    if (ret) {
        printf("sample_dpu_delete failed\n");
        return 0;
    }

err_unbind:
    sample_vdd_unbind();

err_vb_exit:
    sample_vdd_vb_exit();
    printf("%s,%d\n", __func__, __LINE__);

    return ret;
}