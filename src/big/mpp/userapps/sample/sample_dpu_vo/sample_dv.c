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

sample_dv_cfg_t g_vdd_cfg[SAMPLE_VDD_CHN_NUM];
k_u32 g_vo_pool_id;
k_video_frame_info g_vf_info;

static pthread_t tid1;
k_bool thread_exit = K_FALSE;

static k_s32 sample_dv_vb_init()
{
    k_s32 ret;
    k_vb_config config;
    k_vb_pool_config pool_config;
    k_u32 pool_id;

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

    /* vo vb init */
    config.max_pool_cnt = 10;
    config.comm_pool[3].blk_cnt = 1;
    config.comm_pool[3].blk_size = PRIVATE_POLL_SZE;          // osd0 - 3 argb 320 x 240
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;//VB_REMAP_MODE_NOCACHE;

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

    /* vo private vb */
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = PRIVATE_POLL_NUM;
    pool_config.blk_size = PRIVATE_POLL_SZE;
    pool_config.mode = VB_REMAP_MODE_NONE;
    pool_id = kd_mpi_vb_create_pool(&pool_config);      // osd0 - 3 argb 320 x 240

    g_vo_pool_id = pool_id;

    return ret;
}

static k_s32 sample_dv_vb_exit()
{
    k_s32 ret;
    ret = kd_mpi_vb_destory_pool(g_vo_pool_id);
    if (ret) {
        printf("private vb exit failed ret:%d\n", ret);
    }
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static k_s32 sample_dv_bind()
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

static k_s32 sample_dv_unbind()
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
    k_dpu_chn_result_u lcn_result;
    k_u32 cnt=0;

    printf("%s,%d\n", __func__, __LINE__);

    while(1) {
        ret = kd_mpi_dpu_get_frame(0, &lcn_result, 1100);

        if (ret == K_SUCCESS)
        {
            void *dpu_virt_addr = NULL;
            k_u32 length;
            k_u8 *dst;
            k_u16 *src;

            length = lcn_result.lcn_result.depth_out.length;
            dpu_virt_addr = kd_mpi_sys_mmap_cached(lcn_result.lcn_result.depth_out.depth_phys_addr, lcn_result.lcn_result.depth_out.length);
            src = (k_u16*)dpu_virt_addr;
            dst = (k_u8*)dpu_virt_addr;

            while(length > 0)
            {
                if(*src < 100 || *src > 10000)
                    *dst = 0;
                else
                    *dst = 51000/(*src);
                src++;
                dst++;
                length -= 2;
            }
            length = lcn_result.lcn_result.depth_out.length/2;

            //cnt++;

            if(dpu_virt_addr && cnt == 60)
            {
                k_char dpu_filename[256] = "/sharefs/depth_out_8bit.bin";
                FILE *file = fopen(dpu_filename, "wb+");

                if(file)
                {
                    fwrite(dpu_virt_addr, 1, length, file);
                    fclose(file);
                    printf("save depthout success\n");
                }
                else
                {
                    printf("can't create file\n");
                }
            }
            kd_mpi_sys_mmz_flush_cache(lcn_result.lcn_result.depth_out.depth_phys_addr, dpu_virt_addr, length);
            kd_mpi_sys_munmap(dpu_virt_addr, lcn_result.lcn_result.depth_out.length);
        }

        /* send depth result to vo */
        g_vf_info.mod_id = K_ID_VO;
        g_vf_info.pool_id = lcn_result.lcn_result.pool_id;
        g_vf_info.v_frame.phys_addr[0] = lcn_result.lcn_result.depth_out.depth_phys_addr;

        kd_mpi_vo_chn_insert_frame(K_VO_OSD3 + 3, &g_vf_info);  //K_VO_OSD0

        if (ret == K_SUCCESS) {
            //printf("Successfully obtained and will release.\n");
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



    pthread_exit(0);
}

static void usage(void)
{
    printf("Options:\n");
    printf(" -sensor:       sensor type[see K230_Camera_Sensor_Adaptation_Guide.md]\n");
    printf(" -mirror:       vo mirror[0: no change, 1: enable vo mirror]\n");
    printf(" -help:         print this help\n");
}

int main(int argc, char *argv[])
{
    k_s32 ret;
    k_s32 sensor_index = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;
    k_bool mirror=K_FALSE;

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            usage();
            return 0;
        }
        else if (strcmp(argv[i], "-sensor") == 0)
        {
            sensor_index = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-mirror") == 0)
        {
            mirror = atoi(argv[i + 1]);
        }
    }

    g_vdd_cfg[0].img_height = DMA_CHN0_HEIGHT;
    g_vdd_cfg[0].img_width =DMA_CHN0_WIDTH;

    ret = sample_dv_vb_init();
    if (ret) {
        printf("sample_dv_vb_init failed\n");
        return ret;
    }

    ret = sample_dv_bind();
    if (ret) {
        printf("sample_dv_bind failed\n");
        goto err_vb_exit;
    }

    ret = sample_dv_vo_init(mirror);
    if (ret) {
        printf("sample_dv_vo_init failed\n");
        goto err_unbind;
    }

    ret = sample_dv_dpu_init();
    if (ret) {
        printf("sample_dpu_init failed\n");
        goto err_vo_disable;
    }

    ret = sample_dv_dma_init();
    if (ret) {
        printf("sample_dma_init failed\n");
        goto err_dpu_delete;
    }

    /* vi config and start */
    sample_dv_vicap_config(SAMPLE_VDD_CHN, sensor_index);
    sample_dv_vicap_start(SAMPLE_VDD_CHN);
    ret = pthread_create(&tid1, NULL, bind_mode_get_frame, NULL);
    while ((char)getchar() != 'q');
    sample_dv_vicap_stop(SAMPLE_VDD_CHN);
    thread_exit = K_TRUE;
    pthread_join(tid1, NULL);

    printf("%s,%d\n", __func__, __LINE__);
    ret = sample_dv_dma_delete();
    if (ret) {
        printf("sample_dma_delete failed\n");
        return 0;
    }
    printf("%s,%d\n", __func__, __LINE__);

err_dpu_delete:
    ret = sample_dv_dpu_delete();
    if (ret) {
        printf("sample_dpu_delete failed\n");
        return 0;
    }
    printf("%s,%d\n", __func__, __LINE__);

err_vo_disable:
    kd_mpi_vo_osd_disable(K_VO_OSD3);

err_unbind:
    sample_dv_unbind();
    printf("%s,%d\n", __func__, __LINE__);

err_vb_exit:
    sample_dv_vb_exit();
    printf("%s,%d\n", __func__, __LINE__);

    return ret;
}