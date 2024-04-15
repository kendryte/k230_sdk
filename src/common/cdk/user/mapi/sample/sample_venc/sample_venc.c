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
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include "mapi_sys_api.h"
#include "mapi_vvi_api.h"
#include "mapi_venc_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

#define MAX_CHN_NUM 3
#define VI_ALIGN_UP(addr, size) (((addr)+((size)-1U))&(~((size)-1U)))

static k_u8 g_exit = 0;
static FILE *fp[MAX_CHN_NUM] = {NULL};
static int stream_count[MAX_CHN_NUM] = {0};
static int is_jpeg_type[MAX_CHN_NUM] = {0};
static char jpeg_out_path[128];

static void sig_handler(int sig) {
    g_exit = 1;
}

k_s32 get_venc_stream(k_u32 chn_num, kd_venc_data_s* p_vstream_data, k_u8 *p_private_data)
{
    int cut = p_vstream_data->status.cur_packs;
    int i = 0;
    k_char * pdata  = NULL;

    if (!is_jpeg_type[chn_num]) {
        for(i = 0;i < cut;i++) {
            pdata =  p_vstream_data->astPack[i].vir_addr;
            if(stream_count[chn_num] % 90 == 0)
                printf("recv count:%d, chn:%d, phys_addr:0x%lx, len:%d, data:[%02x %02x %02x %02x %02x]\n",
                stream_count[chn_num], chn_num, p_vstream_data->astPack[i].phys_addr, p_vstream_data->astPack[i].len,pdata[0],pdata[1],pdata[2],pdata[3],pdata[4]);

            if (stream_count[chn_num] <= 1000) {
                fwrite(pdata, 1, p_vstream_data->astPack[i].len, fp[chn_num]);
                fflush(fp[chn_num]);
            }
        }
    } else {
        if (stream_count[chn_num] <= 10) {
            char jpg_name[128];
            snprintf(jpg_name, 128, "%s/chn%d_%d.jpg", jpeg_out_path, chn_num, stream_count[chn_num]);
            FILE *fp_jpg = fopen(jpg_name, "wb");
            for(i = 0; i < cut; i++) {
                pdata = p_vstream_data->astPack[i].vir_addr;
                printf("recv count:%d, chn:%d, phys_addr:0x%lx, len:%d, data:[%02x %02x %02x %02x %02x]\n",
                    stream_count[chn_num], chn_num, p_vstream_data->astPack[i].phys_addr, p_vstream_data->astPack[i].len,pdata[0],pdata[1],pdata[2],pdata[3],pdata[4]);

                fwrite(pdata, 1, p_vstream_data->astPack[i].len, fp_jpg);
                fflush(fp_jpg);
            }
            fclose(fp_jpg);
        }
    }
    stream_count[chn_num]++;
}

typedef struct {
    k_vicap_sensor_type sensor_type;
    k_payload_type type;
    k_u32 chn_num;
    char out_path[128];
} SampleCtx;

struct option long_options[] = {
    {"sensor_type", required_argument, NULL, 's'},
    {"chn_num", required_argument, NULL, 'n'},
    {"type", required_argument, NULL, 't'},
    {"out_path", required_argument, NULL, 'o'},
    {"help", required_argument, NULL, 'h'},
    {NULL, 0, NULL, 0},
};

static k_payload_type get_venc_type(k_u32 vtype_index) {
    switch (vtype_index) {
        case 0:
            return K_PT_H264;
        case 1:
            return K_PT_H265;
        case 2:
            return K_PT_JPEG;
        default:
            printf("encoder type %d is unsupported, use K_PT_H265 default.\n", vtype_index);
            return K_PT_H265;
    }
}

k_u32 parse_option(int argc, char *argv[], SampleCtx *psample_ctx) {
    if (argc > 1) {
        int c;
        int option_index = 0;
        while ((c = getopt_long(argc, argv, "s:n:t:o:h", long_options, &option_index)) != -1) {
            switch (c) {
                case 's': {
                    psample_ctx->sensor_type = (k_vicap_sensor_type)atoi(optarg);
                    printf("sensor type: %d.\n", psample_ctx->sensor_type);
                    break;
                }
                case 'n': {
                    psample_ctx->chn_num = atoi(optarg);
                    if (psample_ctx->chn_num < 1 || psample_ctx->chn_num > MAX_CHN_NUM) {
                        printf("sample not support chn_num > 3 or chn_num < 1, please check by %s -h \n", argv[0]);
                        return -1;
                    }
                    printf("chn_num: %d.\n", psample_ctx->chn_num);
                    break;
                }
                case 't': {
                    psample_ctx->type = get_venc_type(atoi(optarg));
                    printf("encoder payload type: %d.\n", psample_ctx->type);
                    break;
                }
                case 'o': {
                    if (access(optarg, F_OK) != 0) {
                        printf("output path %s is not exist.\n", optarg);
                        return -1;
                    }
                    strcpy(psample_ctx->out_path, optarg);
                    printf("output path: %s.\n", psample_ctx->out_path);
                    break;
                }
                case 'h': {
                    printf("Usage: %s -s 0 -n 2 -t 0\n", argv[0]);
                    printf("          -s or --sensor_type [sensor_index],\n");
                    printf("                                see vicap doc\n");
                    printf("          -n or --chn_num [number], 1, 2, 3.\n");
                    printf("          -t or --type [type_index],\n");
                    printf("                        0: h264 type\n");
                    printf("                        1: h265 type\n");
                    printf("                        2: jpeg type\n");
                    printf("          -o or --out_path [output_path].\n");
                    printf("          -h or --help, will print usage.\n");
                    return 1;
                }
                default: {
                    printf("Invalid option, please check by %s -h\n", argv[0]);
                    return -1;
                }
            }
        }
    }
    return K_SUCCESS;
}

int main(int argc, char *argv[]) {

    signal(SIGINT, sig_handler);

    SampleCtx sample_context = {
        .sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR,
        .type = K_PT_H264,
        .chn_num = 1,
        .out_path = "/tmp/"
    };

    k_u32 ret = parse_option(argc, argv, &sample_context);
    if (ret != K_SUCCESS) {
        if (ret > 0)
            return 0;

        printf("parse_option failed.\n");
        return -1;
    }

    strcpy(jpeg_out_path, sample_context.out_path);

    printf("mapi sample_venc...\n");

    ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_init failed, %x.\n", ret);
        return -1;
    }

    if(0)  //sample for nonai_2d
    {
        #include "mapi_nonai_2d_api.h"

        k_nonai_2d_chn_attr attr_2d;
        attr_2d.mode = K_NONAI_2D_CALC_MODE_CSC;
        attr_2d.dst_fmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        kd_mapi_nonai_2d_init(0, &attr_2d);
        kd_mapi_nonai_2d_start(0);
        kd_mapi_nonai_2d_stop(0);
        kd_mapi_nonai_2d_deinit(0);
    }

    // vicap init
    k_vicap_sensor_info sensor_info;
    memset(&sensor_info, 0, sizeof(sensor_info));
    sensor_info.sensor_type = sample_context.sensor_type;
    ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_get_sensor_info failed, %x.\n", ret);
        goto venc_deinit;
    }

    k_vicap_dev_set_info dev_attr_info;
    memset(&dev_attr_info, 0, sizeof(dev_attr_info));
    if (sample_context.sensor_type <= OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE || sample_context.sensor_type >= SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR)
        dev_attr_info.dw_en = K_FALSE;
    else
        dev_attr_info.dw_en = K_TRUE;

    k_u32 pic_width[MAX_CHN_NUM] = {1280, 960, 640 };
    k_u32 pic_height[MAX_CHN_NUM] = {720, 640, 480};
    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = sample_context.chn_num * 2 + 1;
    for (int i = 0; i < sample_context.chn_num; i++) {
        k_u32 pic_size = pic_width[i] * pic_height[i] * 3 / 2;
        k_u32 stream_size = pic_width[i] * pic_height[i] * 3 / 4;
        media_attr.media_config.vb_config.comm_pool[i * 2].blk_cnt = 6;
        media_attr.media_config.vb_config.comm_pool[i * 2].blk_size = ((pic_size + 0xfff) & ~0xfff);
        media_attr.media_config.vb_config.comm_pool[i * 2].mode = VB_REMAP_MODE_NOCACHE;
        media_attr.media_config.vb_config.comm_pool[i * 2 + 1].blk_cnt = 30;
        media_attr.media_config.vb_config.comm_pool[i * 2 + 1].blk_size = ((stream_size + 0xfff) & ~0xfff);
        media_attr.media_config.vb_config.comm_pool[i * 2 + 1].mode = VB_REMAP_MODE_NOCACHE;
    }

    if (dev_attr_info.dw_en) {
        media_attr.media_config.vb_config.comm_pool[sample_context.chn_num * 2].blk_cnt = 6;
        media_attr.media_config.vb_config.comm_pool[sample_context.chn_num * 2].blk_size = VI_ALIGN_UP(sensor_info.width * sensor_info.height * 3 / 2, 0x400);
        media_attr.media_config.vb_config.comm_pool[sample_context.chn_num * 2].mode = VB_REMAP_MODE_NOCACHE;
    }

    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_media_init failed, %x.\n", ret);
        goto sys_deinit;
    }

    dev_attr_info.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr_info.sensor_type = sample_context.sensor_type;
    dev_attr_info.vicap_dev = VICAP_DEV_ID_0;
    ret = kd_mapi_vicap_set_dev_attr(dev_attr_info);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_set_dev_attr failed, %x.\n", ret);
        goto sys_deinit;
    }

    k_venc_chn_attr venc_chn_attr;
    memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
    venc_chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
    venc_chn_attr.rc_attr.cbr.src_frame_rate = 30;
    venc_chn_attr.rc_attr.cbr.dst_frame_rate = 30;
    venc_chn_attr.rc_attr.cbr.bit_rate = 4000;
    venc_chn_attr.venc_attr.type = sample_context.type;
    if (sample_context.type == K_PT_H264) {
        venc_chn_attr.venc_attr.profile = VENC_PROFILE_H264_HIGH;
    } else if (sample_context.type == K_PT_H265) {
        venc_chn_attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
    } else if (sample_context.type == K_PT_JPEG) {
        venc_chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_FIXQP;
        venc_chn_attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
        venc_chn_attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
        venc_chn_attr.rc_attr.mjpeg_fixqp.q_factor = 45;
    }

    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        char filename[128];
        if (sample_context.type == K_PT_H264) {
            snprintf(filename, 128, "%s/stream_chn%d.264", sample_context.out_path, venc_idx);
        } else if (sample_context.type == K_PT_H265) {
            snprintf(filename, 128, "%s/stream_chn%d.265", sample_context.out_path, venc_idx);
        } else if (sample_context.type == K_PT_JPEG) {
            is_jpeg_type[venc_idx] = 1;
        }
        if (sample_context.type != K_PT_JPEG) {
            fp[venc_idx] = fopen(filename, "wb");
            if (fp[venc_idx] == NULL) {
                printf("stream file %s open failed.\n", filename);
                goto media_deinit;
            }
        }

        k_u32 stream_size = pic_width[venc_idx] * pic_height[venc_idx] * 3 / 4;
        venc_chn_attr.venc_attr.pic_width = pic_width[venc_idx];
        venc_chn_attr.venc_attr.pic_height = pic_height[venc_idx];
        venc_chn_attr.venc_attr.stream_buf_cnt = 30;
        venc_chn_attr.venc_attr.stream_buf_size = ((stream_size + 0xfff) & ~0xfff);
        ret = kd_mapi_venc_init(venc_idx, &venc_chn_attr);
        if (ret != K_SUCCESS) {
            printf("init venc %d failed, %x.\n", venc_idx, ret);
            for (int create_idx = 0; create_idx < venc_idx; create_idx++) {
                ret = kd_mapi_venc_deinit(create_idx);
                if (ret != K_SUCCESS) {
                    printf("deinit venc %d failed, %x.\n", create_idx, ret);
                    return -1;
                }
            }
            goto sys_deinit;
        }
    }

    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        int user_data = venc_idx;
        kd_venc_callback_s venc_cb;
        venc_cb.pfn_data_cb = get_venc_stream;
        venc_cb.p_private_data = (k_u8 *)&user_data;
        kd_mapi_venc_registercallback(venc_idx, &venc_cb);
    }

    for (int vichn_idx = 0; vichn_idx < sample_context.chn_num; vichn_idx++) {
        k_vicap_chn_set_info vi_chn_attr_info;
        memset(&vi_chn_attr_info, 0, sizeof(vi_chn_attr_info));

        vi_chn_attr_info.crop_en = K_FALSE;
        vi_chn_attr_info.scale_en = K_FALSE;
        vi_chn_attr_info.chn_en = K_TRUE;
        vi_chn_attr_info.crop_h_start = 0;
        vi_chn_attr_info.crop_v_start = 0;
        vi_chn_attr_info.out_width = VI_ALIGN_UP(pic_width[vichn_idx],16);
        vi_chn_attr_info.out_height = pic_height[vichn_idx];
        vi_chn_attr_info.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        vi_chn_attr_info.vicap_dev = VICAP_DEV_ID_0;
        vi_chn_attr_info.vicap_chn = (k_vicap_chn)vichn_idx;
        vi_chn_attr_info.buffer_num = 6;
        vi_chn_attr_info.alignment = 12;
        if (!dev_attr_info.dw_en)
            vi_chn_attr_info.buf_size = VI_ALIGN_UP(VI_ALIGN_UP(pic_width[vichn_idx],16) * pic_height[vichn_idx] * 3 / 2, 0x400);
        else
            vi_chn_attr_info.buf_size = VI_ALIGN_UP(sensor_info.width * sensor_info.height * 3 / 2, 0x400);
        ret = kd_mapi_vicap_set_chn_attr(vi_chn_attr_info);
        if (ret != K_SUCCESS) {
            printf("vicap chn %d set attr failed, %x.\n", vichn_idx, ret);
            goto venc_deinit;
        }
    }

    // chn start
    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        ret = kd_mapi_venc_start(venc_idx, -1);
        if (ret != K_SUCCESS) {
            printf("venc chn %d start failed, %x.\n", venc_idx, ret);
            for (int create_idx = 0; create_idx < venc_idx; create_idx++) {
                ret = kd_mapi_venc_stop(create_idx);
                if (ret != K_SUCCESS) {
                    printf("venc chn %d stop failed, %x.\n", create_idx, ret);
                    return -1;
                }
            }
            goto venc_deinit;
        }
    }

    k_s32 src_dev = VICAP_DEV_ID_0;
    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        k_s32 src_chn = venc_idx;
        ret = kd_mapi_venc_bind_vi(src_dev, src_chn, venc_idx);
        if (ret != K_SUCCESS) {
            printf("venc chn %d bind vi failed, %x.\n", venc_idx, ret);
            for (int idx = 0; idx < venc_idx; idx++) {
                ret = kd_mapi_venc_unbind_vi(src_dev, idx, idx);
                if (ret != K_SUCCESS) {
                    printf("venc idx %d unbind vi failed, %x.\n", idx, ret);
                    return -1;
                }
            }
            goto venc_stop;
        }
    }

    ret = kd_mapi_vicap_start(VICAP_DEV_ID_0);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_start failed, %x.\n", ret);
        goto vicap_stop;
    }

    // waiting thread
    while (!g_exit) {
        usleep(50000);
    }

    for (int idx = 0; idx < sample_context.chn_num; idx++) {
        ret = kd_mapi_venc_unbind_vi(src_dev, idx, idx);
        if (ret != K_SUCCESS) {
            printf("venc chn %d unbind vi failed, %x.\n", idx, ret);
            return -1;
        }
    }

vicap_stop:
    ret = kd_mapi_vicap_stop(VICAP_DEV_ID_0);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_stop failed, %x.\n", ret);
        return -1;
    }

venc_stop:
    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        ret = kd_mapi_venc_stop(venc_idx);
        if (ret != K_SUCCESS) {
            printf("venc chn %d stop failed, %x.\n", venc_idx, ret);
            return -1;
        }
    }

venc_deinit:
    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        ret = kd_mapi_venc_deinit(venc_idx);
        if (ret != K_SUCCESS) {
            printf("venc %d deinit failed, %x.\n", venc_idx, ret);
            return -1;
        }
    }

media_deinit:
    ret = kd_mapi_media_deinit();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_media_deinit failed, %x.\n", ret);
        return -1;
    }

    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        if (fp[venc_idx]) {
            fclose(fp[venc_idx]);
            fp[venc_idx] = NULL;
        }
    }

sys_deinit:
    ret = kd_mapi_sys_deinit();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_deinit failed, %x.\n", ret);
        return -1;
    }

    printf("mapi sample_venc end...\n");
    return 0;
}