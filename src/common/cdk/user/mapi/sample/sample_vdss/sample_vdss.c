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
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_sys_api.h"
#include "mapi_vdss_api.h"
#include "k_vdss_comm.h"

#define SAMPE_VICAP_PIPE_NUMS   3
#define SAMPE_VICAP_DEV_NUMS   3

typedef struct {
    k_u32 chn_num;
    k_u32 chn_height;
    k_u32 chn_width;
    k_u32 enable;
    k_pixel_format chn_format;
}sample_vicap_chn_conf;

typedef struct {
    k_u32 sensor_type;
    k_u32 dev_num;
    k_u32 dev_height;
    k_u32 dev_width;
    k_vi_attr artr;
    k_u32 enable;
    sample_vicap_chn_conf chn_conf[3];
} sample_vicap_pipe_conf_t;

static sample_vicap_pipe_conf_t g_pipe_conf[SAMPE_VICAP_PIPE_NUMS] = {0};


void sample_vicap_pipe_config(void)
{
    // config dev1 pip0
    g_pipe_conf[0].dev_num = 0;
    g_pipe_conf[0].sensor_type = 3;
    g_pipe_conf[0].enable = 1;
    g_pipe_conf[0].artr.csi = CSI1;
    g_pipe_conf[0].dev_height = 720;
    g_pipe_conf[0].dev_width = 1280;

    g_pipe_conf[0].artr.type = FOLLOW_STROBE_MODE;
    g_pipe_conf[0].artr.mode = LINERA_MODE;
    g_pipe_conf[0].artr.dev_format[0] = RAW10;
    g_pipe_conf[0].artr.phy_attr.lan_num = MIPI_2LAN;
    g_pipe_conf[0].artr.phy_attr.freq = MIPI_800M;
    g_pipe_conf[0].artr.bind_dvp = DVP_CSI1_FLASE_TRIGGER0;

    g_pipe_conf[0].chn_conf[0].chn_num = 0;
    g_pipe_conf[0].chn_conf[0].enable = 1;
    g_pipe_conf[0].chn_conf[0].chn_width = 1280;
    g_pipe_conf[0].chn_conf[0].chn_height = 720;
    g_pipe_conf[0].chn_conf[0].chn_format= PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    g_pipe_conf[1].enable = 0;
    g_pipe_conf[2].enable = 0;
}


void sample_vicap_rgb_sensor_pipe_config(void)
{
    // config dev1 pip0
    g_pipe_conf[0].dev_num = 0;
    g_pipe_conf[0].sensor_type = 1;
    g_pipe_conf[0].enable = 1;
    g_pipe_conf[0].artr.csi = CSI0;
    g_pipe_conf[0].dev_height = 720;
    g_pipe_conf[0].dev_width = 1280;

    g_pipe_conf[0].artr.type = CLOSE_3D_MODE;
    g_pipe_conf[0].artr.mode = LINERA_MODE;
    g_pipe_conf[0].artr.dev_format[0] = RAW10;
    g_pipe_conf[0].artr.phy_attr.lan_num = MIPI_1LAN;
    g_pipe_conf[0].artr.phy_attr.freq = MIPI_800M;
    g_pipe_conf[0].artr.bind_dvp = DVP_CSI1_FLASE_TRIGGER0;

    g_pipe_conf[0].chn_conf[0].chn_num = 0;
    g_pipe_conf[0].chn_conf[0].enable = 1;
    g_pipe_conf[0].chn_conf[0].chn_width = 1280;
    g_pipe_conf[0].chn_conf[0].chn_height = 720;
    g_pipe_conf[0].chn_conf[0].chn_format= PIXEL_FORMAT_YVU_SEMIPLANAR_420;
}


static void sample_vicap_attr_set(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 dev_num, chn_num;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;

    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        printf("pipe_conf[dev_num].enable is %d \n", pipe_conf[dev_num].enable);
        if(pipe_conf[dev_num].enable == 1)
        {
            memset(&dev_attr, 0, sizeof(dev_attr));
            dev_attr.dev_num = pipe_conf[dev_num].dev_num;
            dev_attr.sensor_type = pipe_conf[dev_num].sensor_type;

            printf("pipe_conf[dev_num].sensor_type is %d dev_attr.sensor_type is %d \n", pipe_conf[dev_num].sensor_type, dev_attr.sensor_type);
            dev_attr.height = pipe_conf[dev_num].dev_height;
            dev_attr.width = pipe_conf[dev_num].dev_width;
            memcpy(&dev_attr.artr, &pipe_conf[dev_num].artr, sizeof(k_vi_attr));
            kd_mapi_vdss_set_dev_attr(&dev_attr);

            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++) {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                {
                    memset(&chn_attr, 0, sizeof(chn_attr));
                    chn_attr.format = pipe_conf[dev_num].chn_conf[chn_num].chn_format;
                    chn_attr.height = pipe_conf[dev_num].chn_conf[chn_num].chn_height;
                    chn_attr.width = pipe_conf[dev_num].chn_conf[chn_num].chn_width;
                    chn_attr.enable = pipe_conf[dev_num].chn_conf[chn_num].enable;

                    printf("pipe[%d] dev[%d] h:%d w:%d chn[%d] h:%d w:%d \n", dev_num,
                            pipe_conf[dev_num].dev_num, pipe_conf[dev_num].dev_height, pipe_conf[dev_num].dev_width,
                            pipe_conf[dev_num].chn_conf[chn_num].chn_num, pipe_conf[dev_num].chn_conf[chn_num].chn_height, pipe_conf[dev_num].chn_conf[chn_num].chn_width);

                    kd_mapi_vdss_set_chn_attr(dev_num, chn_num, &chn_attr);
                }
            }
        }

    }

    return;
}

static void sample_vicap_start(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 dev_num, chn_num;

    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        if(pipe_conf[dev_num].enable == 1)
        {
            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++)
            {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                    kd_mapi_vdss_start_pipe(dev_num, chn_num);
            }
        }

    }
    return;
}


static void sample_vicap_stop(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 dev_num, chn_num;

    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        if(pipe_conf[dev_num].enable == 1)
        {
            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++)
            {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                    kd_mapi_vdss_stop_pipe(dev_num, chn_num);
            }
        }

    }
    return;
}

static k_mapi_media_attr_t media_attr = {0};

static k_s32 sample_vb_init(sample_vicap_pipe_conf_t* pipe_conf)
{
    k_s32 dev_num, chn_num;
    k_s32 ret;

    media_attr.media_config.vb_config.max_pool_cnt = 64;
    for(dev_num = 0; dev_num < SAMPE_VICAP_DEV_NUMS; dev_num++)
    {
        if(pipe_conf[dev_num].enable == 1)
        {
            for(chn_num = 0; chn_num < SAMPE_VICAP_PIPE_NUMS; chn_num++)
            {
                if(pipe_conf[dev_num].chn_conf[chn_num].enable == 1)
                {
                    media_attr.media_config.vb_config.comm_pool[chn_num].blk_cnt = VDSS_MAX_FRAME_COUNT;//5;
                    media_attr.media_config.vb_config.comm_pool[chn_num].blk_size = pipe_conf[dev_num].chn_conf[chn_num].chn_height * pipe_conf[dev_num].chn_conf[chn_num].chn_width * 3 / 2;
                    media_attr.media_config.vb_config.comm_pool[chn_num].mode = VB_REMAP_MODE_NOCACHE;
                }
            }
        }
    }
    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }
}

#define RGB_DUMP_PICTURE            "dump_yuv422.bin"



static void vdss_save_yuv422_picture(FILE *fd, int mmap_fd, k_video_frame_info *vf_info)
{
    int ret = 0;
    void *mmap_addr = NULL;
    void *virt_addr;
    k_u64 phys_addr = 0;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u32 mmap_size ;
    k_u64 page_mask = (page_size - 1);
    k_u32 read_size;

    phys_addr = vf_info->v_frame.phys_addr[0];
    read_size = 1280 * 720 * 3 / 2;
    mmap_size = (((read_size) + (page_size) - 1) & ~((page_size) - 1));

    printf("vf_info phy addr is %x read_size is %x \n", vf_info->v_frame.phys_addr[0], read_size);

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, phys_addr & ~page_mask);
    if(mmap_addr)
        virt_addr = mmap_addr + (phys_addr & page_mask);

    ret = fwrite(virt_addr, read_size, 1, fd);
    if (ret <= 0)
    {
        printf("fwrite  picture_addr is failed ret is %d \n", ret);
    }

}
int main(void)
{
    k_s32 ret;
    k_video_frame_info vf_info;

    printf("start vdss \n");

    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_init error: %x\n", ret);
        goto exit;
    }

    // sample_vicap_pipe_config();
     sample_vicap_rgb_sensor_pipe_config();


    sample_vb_init(g_pipe_conf);

    printf("vb init success \n");

    kd_mapi_vdss_rst(2);           //rst ov9286



    sample_vicap_attr_set(g_pipe_conf);

    sample_vicap_start(g_pipe_conf);

    FILE *fd;
    static k_s32 mmap_fd = -1;
    // add picture
    fd = fopen(RGB_DUMP_PICTURE, "wb");


    if(mmap_fd == -1)
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);

    printf("opne mem success \n");

#if 0
    ret = kd_mpi_vdss_dump_frame(0, 0, &vf_info, 10);
    if(ret >= 0)
    {
        vdss_save_yuv422_picture(fd, mmap_fd,  &vf_info);
        kd_mpi_vdss_chn_release_frame(0, 0, &vf_info);
        printf("dump picture success \n");
    }
    else
        usleep(500000);
#else
    sleep(1);

    while(1)
    {
        // printf("----------------------------------ret is %d \n", ret);
        ret = kd_mapi_vdss_dump_frame(0, 0, &vf_info, 10);

        printf("ret is %d vf_info phy addr is %x \n", ret, vf_info.v_frame.phys_addr[0]);
        if(ret >= 0)
        {
            sleep(1);
            printf("release   -----------------vf_info phy addr is %x ----------  \n", vf_info.v_frame.phys_addr[0]);
            kd_mapi_vdss_chn_release_frame(0, 0, &vf_info);
        }
        else
            sleep(1);

        printf("start   ---------------------------  \n");
    }

#endif
    getchar();

    sample_vicap_stop(g_pipe_conf);

    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the virtual VO to release the VB block*/
    k_u32 display_ms = 1000 / 33;
    usleep(1000 * display_ms);



    printf("Press Enter to exit!!!!\n");
    getchar();
    ret |= kd_mapi_media_deinit();
    ret |= kd_mapi_sys_deinit();

    return 0;
exit:
    if(ret)
        printf("sample code run error\n");
    printf("Exit!!!!\n");
    return 0;
}



