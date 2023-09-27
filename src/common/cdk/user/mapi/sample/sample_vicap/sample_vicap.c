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
#include <getopt.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

// argparse
struct option long_options[] = {
    {"dev",    required_argument, NULL, 'd'},
    {"sensor", required_argument, NULL, 's'},
    {"chn",    required_argument, NULL, 'c'},
    {"ofmt",    required_argument, NULL, 'f'},
    {"width",     required_argument, NULL, 'W'},
    {"height",     required_argument, NULL, 'H'},
    {"dewarp", no_argument, NULL, 'e'},
    {"help",   no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

typedef struct {
    k_u32 buffsize_with_align;
    k_u32 buffsize;
    k_char *suffix;
} buf_size_calc;

typedef struct {
    k_u32 page_size;
    k_u64 page_mask;
} kd_ts_sys_paga_vi_cap;

static kd_ts_sys_paga_vi_cap sys_page = {0};
static k_video_frame_info dump_pic_info = {0};
static k_s32 mmap_fd = -1;

void kd_ts_mmap(k_u64 phys_addr, k_u32 size, k_u32 **v_addr)
{
    sys_page.page_size = sysconf(_SC_PAGESIZE);
    sys_page.page_mask = sys_page.page_size - 1;
#ifdef USE_RT_SMART
    *v_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
#else
    void *mmap_addr = NULL;
    k_u32 mmap_size = (((size) + (sys_page.page_size) - 1) & ~((sys_page.page_size) - 1));

    if(mmap_fd == -1)
    {
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, phys_addr & ~sys_page.page_mask);

    if(mmap_addr)
    {
        *v_addr = mmap_addr + (phys_addr & sys_page.page_mask);
    }
#endif
    if(*v_addr == NULL)
    {
        printf("mmap error\n");
        return;
    }
    printf("mmap success\n");
    return;
}

static k_vicap_sensor_type get_sensor_type(k_u16 sensor)
{
    switch (sensor)
    {
    case 0:
        return OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
    case 1:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR;
    case 2:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;
    case 3:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    case 4:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 5:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 6:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR;
    case 7:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    case 8:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    default:
        printf("unsupport sensor type %d, use default IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR\n", sensor);
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    }
    return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
}

static k_pixel_format get_pixel_format(k_u32 pix_fmt)
{
    switch (pix_fmt)
    {
    case 0:
        return PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    case 1:
        return PIXEL_FORMAT_RGB_888;
    case 2:
        return PIXEL_FORMAT_BGR_888_PLANAR;
    case 3:
        return PIXEL_FORMAT_RGB_BAYER_10BPP;
    default:
        printf("unsupported pixel format %d, use dafault YUV_SEMIPLANAR_420\n", pix_fmt);
        break;
    }
}

static k_s32 calc_buffer_size(k_pixel_format pix_fmt, k_u32 width, k_u32 height, buf_size_calc * size_calc)
{
    switch (pix_fmt)
    {
    case PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3 / 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3 / 2;
        size_calc->suffix = "yuv420sp";
        break;
    case PIXEL_FORMAT_RGB_888:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3;
        size_calc->suffix = "rgb888p";
        break;
    case PIXEL_FORMAT_BGR_888_PLANAR:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3;
        size_calc->suffix = "rgb888";
        break;
    case PIXEL_FORMAT_RGB_BAYER_10BPP:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 2;
        size_calc->suffix = "raw10";
        break;
    default:
        printf("unsupported pixel format %d, use dafault YUV_SEMIPLANAR_420 size\n", pix_fmt);
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3 / 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3 / 2;
        size_calc->suffix = "unkown";
    }
    return K_SUCCESS;
}

static k_s32 sample_vicap_vb_init(k_bool dw_en, k_u32 dev_num, k_u32 chn_num, buf_size_calc in_size, buf_size_calc out_size, buf_size_calc dw_size)
{
    printf("sample_vicap_vb_init start\n");
    k_s32 ret = 0;
    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));

    media_attr.media_config.vb_config.max_pool_cnt = 64;
    int k = 0;
    for(int i = 0; i <= dev_num; i++) {
        for(int j = 0; j <= chn_num; j++) {
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[k].blk_size = out_size.buffsize_with_align;
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            k++;
        }
        if(dw_en)
        {
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[k].blk_size = dw_size.buffsize_with_align;
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            k++;
        }
    }

    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }
    printf("sample_vicap_vb_init end\n");
    return 0;
}

int main(int argc, char * const argv[])
{

    // argparse param init
    k_u32 out_width = 1088;
    k_u32 out_height = 1920;
    k_vicap_dev arg_dev = VICAP_DEV_ID_0; // 0
    k_vicap_chn arg_chn = VICAP_CHN_ID_0; // 0
    k_pixel_format arg_pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420; // 30
    k_vicap_sensor_type arg_sensor = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR; // 0
    k_bool dw_en = 0; // 0

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "d:s:c:f:W:H:eh", long_options, &option_index)) != -1)
    {
        switch(c)
        {
        case 'd':
            arg_dev = (k_vicap_dev)atoi(optarg);
            printf("dev: %d\n", arg_dev);
            break;
        case 's':
            arg_sensor = get_sensor_type(atoi(optarg));
            printf("sensor type: %d\n", arg_sensor);
            break;
        case 'c':
            arg_chn = (k_vicap_chn)atoi(optarg);
            printf("chn: %d\n", arg_chn);
            break;
        case 'f':
            arg_pixel_format = get_pixel_format(atoi(optarg));
            printf("pixel format: %d\n", arg_pixel_format);
            break;
        case 'W':
            out_width = (k_u32)atoi(optarg);
            printf("out width: %d\n", out_width);
            break;
        case 'H':
            out_height = (k_u32)atoi(optarg);
            printf("out_height: %d\n", out_height);
            break;
        case 'e':
            printf("dewarp enable\n");
            dw_en = 1;
            break;
        case 'h':
            printf("Usage: %s -d 0 -s 3 -c 0 -f 0 -W 1088 -H 1920 -e\n", argv[0]);
            printf("          -d or --dev device num, 0, 1, 2\n");
            printf("          -s or --sensor sensor num, \n");
            printf("               0: ov9732\n");
            printf("               1: ov9286 ir\n");
            printf("               2: ov9286 speckle\n");
            printf("               3: imx335 2LANE 1920Wx1080H\n");
            printf("               4: imx335 2LANE 2592Wx1944H\n");
            printf("               5: imx335 4LANE 2592Wx1944H\n");
            printf("               6: imx335 2LANE MCLK 7425 1920Wx1080H\n");
            printf("               7: imx335 2LANE MCLK 7425 2592Wx1944H\n");
            printf("               8: imx335 4LANE MCLK 7425 2592Wx1944H\n");
            printf("          -c or --chn channel num, 0, 1, 2\n");
            printf("          -f or --ofmt out pixel format, 0: yuv420sp, 1: rgb888, 2: rgb888p, 3: raw10\n");
            printf("          -W or --width output width\n");
            printf("          -H or --height output height\n");
            printf("          -e or --dewarp dewarp mode, action store_true, default disable, will enable when use -e or --dewarp, \n");
            printf("               will force disable with raw10 pixel format.\n");
            printf("          -h or --help will print this usage\n");
            return 0;
        default:
            fprintf(stderr, "Invalid option\n");
            break;
        }
    }

    k_s32 ret = 0;

    // ipcmsg service init
    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS)
    {
        printf("kd_mapi_sys_init error: %x\n", ret);
        goto exit;
    }

    // phy2vir
    if(mmap_fd == -1)
    {
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    }
    printf("opne mem success \n");

    /* use mapi */

    // sensor info
    k_vicap_sensor_info sensor_info;
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    sensor_info.sensor_type = arg_sensor;
    ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
    if(!ret)
    {
        printf("kd_mapi_vicap_get_sensor_info ok\n");
    }

    // get resolution
    k_u32 in_width = sensor_info.width;
    k_u32 in_height = sensor_info.height;

    // calc buffer size
    buf_size_calc size_calc_in;
    buf_size_calc size_calc_out;
    buf_size_calc size_calc_dw;
    calc_buffer_size(arg_pixel_format, in_width, in_height, &size_calc_in);
    calc_buffer_size(arg_pixel_format, out_width, out_height, &size_calc_out);
    calc_buffer_size(PIXEL_FORMAT_YUV_SEMIPLANAR_420, in_width, in_height, &size_calc_dw);

    // init vb
    ret = sample_vicap_vb_init(dw_en, (k_u32)arg_dev, (k_u32)arg_chn, size_calc_in, size_calc_out, size_calc_dw);
    if(ret)
    {
        ret |= kd_mapi_media_deinit();
    }

    // set dev attr
    for(k_u32 i = 0; i <= (k_u32)arg_dev; i++)
    {
        k_vicap_dev_set_info dev_attr_info;
        memset(&dev_attr_info, 0, sizeof(k_vicap_dev_set_info));
        dev_attr_info.dw_en = dw_en;
        dev_attr_info.pipe_ctrl.data = 0xFFFFFFFF;
        dev_attr_info.sensor_type = arg_sensor;
        dev_attr_info.vicap_dev = (k_vicap_dev)i;
        ret = kd_mapi_vicap_set_dev_attr(dev_attr_info);
    }

    // set chn attr
    for(k_u32 i = 0; i <= (k_u32)arg_chn; i++)
    {
        k_vicap_chn_set_info chn_attr_info;
        memset(&chn_attr_info, 0, sizeof(k_vicap_chn_set_info));
        chn_attr_info.crop_en = K_FALSE;
        chn_attr_info.scale_en = K_FALSE;
        chn_attr_info.chn_en = K_TRUE;
        chn_attr_info.crop_h_start = 0;
        chn_attr_info.crop_v_start = 0;
        chn_attr_info.out_height = out_height;
        chn_attr_info.out_width = out_width;
        chn_attr_info.pixel_format = arg_pixel_format;
        chn_attr_info.vicap_dev = arg_dev;
        chn_attr_info.vicap_chn = (k_vicap_chn)i;

        if(arg_pixel_format != PIXEL_FORMAT_RGB_BAYER_10BPP && arg_pixel_format != PIXEL_FORMAT_RGB_BAYER_12BPP)
        {
            // dewarp can enable in bayer pixel format
            if(dw_en && i == 0)
            {
                printf("dw buf\n");
                chn_attr_info.buf_size = size_calc_dw.buffsize_with_align;
            }
            else
            {
                printf("dw disable buf\n");
                chn_attr_info.buf_size = size_calc_out.buffsize_with_align;
            }
        }
        else
        {
            printf("dw is force disabled, becase of pixformat BAYER\n");
            dw_en = 0;
            chn_attr_info.buf_size = size_calc_out.buffsize_with_align;
        }
        ret = kd_mapi_vicap_set_chn_attr(chn_attr_info);
    }

    if(!ret)
    {
        printf("kd_mapi_vicap_set_attr ok\n");
    }
    ret = kd_mapi_vicap_start(arg_dev);
    if(!ret)
    {
        printf("vicap start...\n");
    }

    // dump test
    k_video_frame dump_vf_info;
    k_u32 *dump_virt_addr = NULL;
    k_char filename[256];
    k_u32 num[VICAP_DEV_ID_MAX][VICAP_CHN_ID_MAX] = {0};
    char select;
    while(1)
    {
        printf("Press Enter to continue, q will exit!!!!\n");
        select = (char)getchar();
        if(select == 'q')
        {
            break;
        }
        for(k_u32 dev_num = 0; dev_num <= (k_u32)arg_dev; dev_num++)
        {
            for(k_u32 chn_num = 0; chn_num <= (k_u32)arg_chn; chn_num++)
            {
                ret = kd_mapi_vicap_dump_frame((k_vicap_dev)dev_num, (k_vicap_chn)chn_num, VICAP_DUMP_YUV, &dump_pic_info, 100);
                dump_vf_info = dump_pic_info.v_frame;
                printf("ret is %d dump_pic_info phy addr is %x \n", ret, dump_pic_info.v_frame.phys_addr[0]);
                printf("dump_vf_info.width: %d, dump_vf_info.height: %d\n", dump_pic_info.v_frame.width, dump_pic_info.v_frame.height);

                kd_ts_mmap(dump_vf_info.phys_addr[0], size_calc_out.buffsize, &dump_virt_addr); // map to dump_virt_addr

                memset(filename, 0, sizeof(filename));
                snprintf(filename, sizeof(filename), "dev_%02d_chn_%02d_%dx%d_%04d.%s", \
                                    dev_num, chn_num, dump_vf_info.width, dump_vf_info.height, num[dev_num][chn_num], size_calc_out.suffix);

                printf("save data file %s\n", filename);
                FILE *fd = fopen(filename, "wb");
                fwrite(dump_virt_addr, 1, size_calc_out.buffsize, fd);
                fclose(fd);

                if(ret >= 0)
                {
                    sleep(1);
                    ret = kd_mapi_vicap_release_frame(dev_num, chn_num, &dump_pic_info);
                }
                else
                {
                    sleep(1);
                }
                num[dev_num][chn_num]++;
            }
        }
    }
    printf("exit\n");

exit:
    kd_mapi_vicap_stop(arg_dev);
    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
}
