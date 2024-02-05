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
#include <stdint.h>
#include <signal.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_venc_api.h"
#include "mpi_sys_api.h"
#include "k_venc_comm.h"

#define OSD_MAX_IN_FRAMES   30

#ifdef ENABLE_VENC_DEBUG
    #define venc_debug  printf
#else
    #define venc_debug(ARGS...)
#endif

#define MAX_WIDTH 128
#define MAX_HEIGHT 64
#define STREAM_BUF_SIZE MAX_WIDTH*MAX_HEIGHT
#define FRAME_BUF_SIZE MAX_WIDTH*MAX_HEIGHT*2
#define INPUT_BUF_CNT   6
#define OUTPUT_BUF_CNT  6
#define DUMP_STREAM_CNT  3

extern const unsigned int osd_data;
extern const int osd_data_size;

typedef struct
{
    k_u32 chn_id;
    k_u32 video_width;
    k_u32 video_height;
    k_u32 video_file_size;
    k_u32 input_frames;
    k_u32 send_cnt;
    k_u32 get_cnt;
    k_u64 video_phys_addr[OSD_MAX_IN_FRAMES][3];
    k_u8 *video_virt_addr[OSD_MAX_IN_FRAMES][3];
    pthread_t input_tid;
    k_video_frame_info *frame;
    k_u32 osd_width;
    k_u32 osd_height;
    k_u32 osd_file_size;
    k_u32 osd_phys_addr[OSD_MAX_IN_FRAMES][3];
    k_video_frame_info *osd_frame;
    k_s32 video_pool_id;
    k_s32 osd_pool_id;
    k_u32 osd_startx;
    k_u32 osd_starty;
    k_venc_2d_src_dst_fmt video_fmt;
    k_venc_2d_osd_fmt osd_fmt;
    k_u16 bg_alpha;
    k_u16 osd_alpha;
    k_u16 video_alpha;
    k_venc_2d_add_order add_order;
    k_u32 bg_color;
    k_u16 osd_coef[K_VENC_2D_COEF_NUM];
    k_u8 osd_region_num;
    k_bool osd_matrix_en;
    k_vb_blk_handle *video_blk_handle;
    k_vb_blk_handle osd_blk_handle;

    char out_filename[20];
    FILE *video_file;
    FILE *output_file;

} sample_osd_conf_t;

static sample_osd_conf_t g_osd_conf;

static inline void CHECK_RET(k_s32 ret)
{
    if (ret)
        printf("error %s %d\n", __func__, __LINE__);
}

static k_s32 sample_vb_init(sample_osd_conf_t *osd_conf)
{
    k_s32 ret;
    k_vb_config config;
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u8 *virt_addr_Y;
    k_u8 *virt_addr_U;
    k_u8 *virt_addr_V;
    k_u8 *virt_addr_osd;
    int i;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 2;
    config.comm_pool[0].blk_cnt = 6;
    config.comm_pool[0].blk_size = osd_conf->video_width * osd_conf->video_height * 2;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;//VB_REMAP_MODE_NOCACHE;//VB_REMAP_MODE_CACHED;
    config.comm_pool[1].blk_cnt = 6;
    config.comm_pool[1].blk_size = osd_conf->osd_width * osd_conf->osd_height * 4;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;//VB_REMAP_MODE_NOCACHE;//VB_REMAP_MODE_CACHED;

    ret = kd_mpi_vb_set_config(&config);

    printf("-----------osd sample test------------------------\n");

    if (ret)
        printf("vb_set_config failed ret:%d\n", ret);

    ret = kd_mpi_vb_init();
    if (ret)
        printf("vb_init failed ret:%d\n", ret);

    printf("%s>input frames %d\n", __func__, osd_conf->input_frames);

    osd_conf->video_blk_handle = malloc(osd_conf->input_frames * sizeof(osd_conf->video_blk_handle));

    for (i = 0; i < osd_conf->input_frames; i++)
    {
        int stride;

        handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, config.comm_pool[0].blk_size, NULL);
        if (handle == VB_INVALID_HANDLE)
        {
            printf("%s video get vb block error\n", __func__);
            break;
        }

        osd_conf->video_blk_handle[i] = handle;

        pool_id = kd_mpi_vb_handle_to_pool_id(handle);
        if (pool_id == VB_INVALID_POOLID)
        {
            printf("%s video get pool id error\n", __func__);
            break;
        }
        osd_conf->video_pool_id = pool_id;
        printf("%s>osd_conf->video_pool_id = %d\n", __func__, osd_conf->video_pool_id);

        phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
        if (phys_addr == 0)
        {
            printf("%s video get phys addr error\n", __func__);
            break;
        }

        printf("%s>i %d, phys_addr 0x%lx, blk_size %ld\n", __func__, i, phys_addr, config.comm_pool[0].blk_size);
        virt_addr_Y = (k_u8 *)kd_mpi_sys_mmap_cached(phys_addr, config.comm_pool[0].blk_size);
        if (virt_addr_Y == NULL)
        {
            printf("%s video mmap error\n", __func__);
            break;
        }
        printf("%s>i %d, video virt_addr_Y %p\n", __func__, i, virt_addr_Y);

        if (osd_conf->video_file_size > 0)
        {
            int i;
            k_u8 *src = virt_addr_Y;
            k_u64 addr;

            //read Y
            stride = osd_conf->video_width;
            for (i = 0; i < osd_conf->video_height; i++)
            {
                fread(src, 1, stride, osd_conf->video_file);
                src += stride;
            }

            //read U
            stride = osd_conf->video_width;
            addr = (k_u64)src;
            addr = (addr + 0xfff) & ~0xfff;
            src = (k_u8 *)addr;
            virt_addr_U = src;
            printf("%s>video virt_addr_U %p\n", __func__, virt_addr_U);
            for (i = 0; i < osd_conf->video_height / 4; i++)
            {
                fread(src, 1, stride, osd_conf->video_file);
                src += stride;
            }

            //read V
            stride = osd_conf->video_width;
            addr = (k_u64)src;
            addr = (addr + 0xfff) & ~0xfff;
            src = (k_u8 *)addr;
            virt_addr_V = src;
            printf("%s>video virt_addr_V %p\n", __func__, virt_addr_V);
            for (i = 0; i < osd_conf->video_height / 4; i++)
            {
                fread(src, 1, stride, osd_conf->video_file);
                src += stride;
            }
        }
        osd_conf->video_phys_addr[i][0] = phys_addr;
        osd_conf->video_phys_addr[i][1] = phys_addr + (virt_addr_U - virt_addr_Y);
        osd_conf->video_phys_addr[i][2] = phys_addr + (virt_addr_V - virt_addr_Y);
        osd_conf->video_virt_addr[i][0] = virt_addr_Y;
        osd_conf->video_virt_addr[i][1] = virt_addr_U;
        osd_conf->video_virt_addr[i][2] = virt_addr_V;

        kd_mpi_sys_mmz_flush_cache(osd_conf->video_phys_addr[i][0], osd_conf->video_virt_addr[i][0], stride * osd_conf->video_height);
        kd_mpi_sys_mmz_flush_cache(osd_conf->video_phys_addr[i][1], osd_conf->video_virt_addr[i][1], stride * osd_conf->video_height / 4);
        kd_mpi_sys_mmz_flush_cache(osd_conf->video_phys_addr[i][2], osd_conf->video_virt_addr[i][2], stride * osd_conf->video_height / 4);

    }

    for (i = 0; i < 1; i++)
    {
        handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, config.comm_pool[1].blk_size, NULL);

        if (handle == VB_INVALID_HANDLE)
        {
            printf("%s osd get vb block error\n", __func__);
            break;
        }
        osd_conf->osd_blk_handle = handle;

        pool_id = kd_mpi_vb_handle_to_pool_id(handle);
        if (pool_id == VB_INVALID_POOLID)
        {
            printf("%s osd get pool id error\n", __func__);
            break;
        }
        osd_conf->osd_pool_id = pool_id;
        printf("%s>osd osd_conf->osd_pool_id = %d\n", __func__, osd_conf->osd_pool_id);

        phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
        if (phys_addr == 0)
        {
            printf("%s osd get phys addr error\n", __func__);
            break;
        }

        printf("%s>i %d, phys_addr 0x%lx, blk_size %ld\n", __func__, i, phys_addr, config.comm_pool[1].blk_size);
        virt_addr_osd = (k_u8 *)kd_mpi_sys_mmap_cached(phys_addr, config.comm_pool[1].blk_size);

        if (virt_addr_osd == NULL)
        {
            printf("%s osd mmap error\n", __func__);
            break;
        }
        printf("%s>i %d, osd virt_addr_osd %p\n", __func__, i, virt_addr_osd);


        memcpy(virt_addr_osd, &osd_data, osd_data_size);

        osd_conf->osd_phys_addr[i][0] = phys_addr;
        kd_mpi_sys_mmz_flush_cache(osd_conf->osd_phys_addr[i][0], virt_addr_osd, osd_conf->osd_width * osd_conf->osd_height * 4);
    }

    return ret;
}

static k_s32 sample_vb_exit(void)
{
    printf("%s\n", __FUNCTION__);
    k_s32 ret;

    for (int i = 0; i < g_osd_conf.input_frames; i++)
    {
        kd_mpi_vb_release_block(g_osd_conf.video_blk_handle[i]);
    }
    kd_mpi_vb_release_block(g_osd_conf.osd_blk_handle);
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static void sample_osd_start(int ch)
{
    printf("%s\n", __FUNCTION__);
    k_venc_2d_osd_attr attr;
    int ret = 0;
    memset(&attr, 0, sizeof(attr));

    sample_osd_conf_t *osd_conf = &g_osd_conf;

    ret = kd_mpi_venc_start_2d_chn(ch);
    CHECK_RET(ret);

    ret = kd_mpi_venc_set_2d_mode(ch, K_VENC_2D_CALC_MODE_OSD);
    CHECK_RET(ret);

    attr.width = osd_conf->osd_width;
    attr.height = osd_conf->osd_height;
    attr.startx = osd_conf->osd_startx;
    attr.starty = osd_conf->osd_starty;
    attr.phys_addr[0] = osd_conf->osd_phys_addr[0][0];
    attr.phys_addr[1] = osd_conf->osd_phys_addr[0][1];
    attr.phys_addr[2] = osd_conf->osd_phys_addr[0][2];
    attr.bg_alpha = osd_conf->bg_alpha;
    attr.osd_alpha = osd_conf->osd_alpha;
    attr.video_alpha = osd_conf->video_alpha;
    attr.add_order = osd_conf->add_order;
    attr.bg_color = osd_conf->bg_color;
    attr.fmt = osd_conf->osd_fmt;
    ret = kd_mpi_venc_set_2d_osd_param(ch, 0, &attr);
    CHECK_RET(ret);

    return;
}

static void *input_thread(void *arg)
{
    int i;

    k_video_frame_info frame;


    for (i = 0; i < g_osd_conf.input_frames; i++)
    {
        frame.mod_id = K_ID_VENC;
        frame.pool_id = g_osd_conf.video_pool_id;
        frame.v_frame.phys_addr[0] = g_osd_conf.video_phys_addr[i % g_osd_conf.input_frames][0];
        frame.v_frame.phys_addr[1] = g_osd_conf.video_phys_addr[i % g_osd_conf.input_frames][1];
        frame.v_frame.phys_addr[2] = g_osd_conf.video_phys_addr[i % g_osd_conf.input_frames][2];
        frame.v_frame.virt_addr[0] = (k_u64)g_osd_conf.video_virt_addr[i % g_osd_conf.input_frames][0];
        frame.v_frame.virt_addr[1] = (k_u64)g_osd_conf.video_virt_addr[i % g_osd_conf.input_frames][1];
        frame.v_frame.virt_addr[2] = (k_u64)g_osd_conf.video_virt_addr[i % g_osd_conf.input_frames][2];
        frame.v_frame.pixel_format = PIXEL_FORMAT_YVU_PLANAR_420;
        frame.v_frame.width = g_osd_conf.video_width;
        frame.v_frame.height = g_osd_conf.video_height;

        printf("send frame %d: phys_addr 0x%lx 0x%lx 0x%lx\n", i, frame.v_frame.phys_addr[0], frame.v_frame.phys_addr[1], frame.v_frame.phys_addr[2]);

        kd_mpi_venc_send_2d_frame(0, &frame);
        g_osd_conf.send_cnt++;
    }

    return arg;
}

static void *output_thread(void *arg)
{
    k_video_frame_info output;
    int ret = 0;
    g_osd_conf.get_cnt = 0;
    while (g_osd_conf.get_cnt < g_osd_conf.input_frames)
    {
        if (g_osd_conf.get_cnt < g_osd_conf.send_cnt)
        {
            ret = kd_mpi_venc_get_2d_frame(g_osd_conf.chn_id, &output);
            printf("%s>get frame ret = %d\n", __FUNCTION__, ret);
            if (!g_osd_conf.output_file)
            {
                if ((g_osd_conf.output_file = fopen(g_osd_conf.out_filename, "wb")) == NULL)
                {
                    printf("Cannot open output file!!!\n");
                }
            }
            fwrite((void *)output.v_frame.virt_addr[0], 1, output.v_frame.width * output.v_frame.height, g_osd_conf.output_file);
            fwrite((void *)output.v_frame.virt_addr[1], 1, output.v_frame.width * output.v_frame.height / 4, g_osd_conf.output_file);
            fwrite((void *)output.v_frame.virt_addr[2], 1, output.v_frame.width * output.v_frame.height / 4, g_osd_conf.output_file);
            g_osd_conf.get_cnt++;
        }
    }

    return arg;
}

int main(int argc, char *argv[])
{
    int ch = 0;
    int width, height;
    int i;

    printf("%s\n", __FUNCTION__);
    memset(&g_osd_conf, 0, sizeof(sample_osd_conf_t));

    for (i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            printf("Please input:\n");
            printf("-i: input file name\n");
            printf("-w: input width\n");
            printf("-h: input height\n");
            printf("-o: output file name\n");
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            venc_debug("infilename: %s\n", argv[i + 1]);
            if ((g_osd_conf.video_file = fopen(argv[i + 1], "rb")) == NULL)
            {
                venc_debug("Cannot open input file!!!\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            width = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            height = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            strcpy(g_osd_conf.out_filename, argv[i + 1]);
        }
    }

    if (g_osd_conf.video_file)
    {
        fseek(g_osd_conf.video_file, 0L, SEEK_END);
        g_osd_conf.video_file_size = ftell(g_osd_conf.video_file);
        fseek(g_osd_conf.video_file, 0, SEEK_SET);
    }
    else
    {
        g_osd_conf.video_file_size = 0;
        return -1;
    }
    printf("video input yuv size %d\n", g_osd_conf.video_file_size);


    g_osd_conf.input_frames = g_osd_conf.video_file_size / (width * height * 3 / 2);
    g_osd_conf.video_width = width;
    g_osd_conf.video_height = height;
    g_osd_conf.osd_width = 40;
    g_osd_conf.osd_height = 40;
    g_osd_conf.osd_startx = 4;
    g_osd_conf.osd_starty = 4;
    g_osd_conf.video_fmt = K_VENC_2D_SRC_DST_FMT_YUV420_I420;
    g_osd_conf.osd_fmt = K_VENC_2D_OSD_FMT_ARGB8888;
    g_osd_conf.bg_alpha = 200;
    g_osd_conf.osd_alpha = 200;
    g_osd_conf.video_alpha = 200;
    g_osd_conf.add_order = K_VENC_2D_ADD_ORDER_VIDEO_OSD;
    g_osd_conf.bg_color = (200 << 16) | (128 << 8) | (128 << 0);

    if (g_osd_conf.input_frames >= OSD_MAX_IN_FRAMES)
    {
        printf("too many input frames %d\n", g_osd_conf.input_frames);
        return -1;
    }

    g_osd_conf.frame = (k_video_frame_info *)malloc(sizeof(k_video_frame_info) * g_osd_conf.input_frames);
    if (g_osd_conf.frame == NULL)
        printf("error:no space\n");

    if (sample_vb_init(&g_osd_conf))
    {
        return -1;
    }

    sample_osd_start(ch);

    pthread_create(&g_osd_conf.input_tid, NULL, input_thread, &g_osd_conf);
    pthread_create(&g_osd_conf.input_tid, NULL, output_thread, &g_osd_conf);

    while (g_osd_conf.get_cnt != g_osd_conf.input_frames)
    {
        usleep(50000);
    }

    kd_mpi_venc_stop_2d_chn(ch);

    printf("%s>close files\n", __FUNCTION__);
    fclose(g_osd_conf.video_file);
    fclose(g_osd_conf.output_file);

    printf("%s>free frame\n", __FUNCTION__);
    free(g_osd_conf.frame);

    printf("%s>kill threads\n", __FUNCTION__);
    pthread_kill(g_osd_conf.input_tid, SIGALRM);
    pthread_join(g_osd_conf.input_tid, NULL);

    printf("%s>exit\n", __FUNCTION__);
    sample_vb_exit();

    return 0;
}
