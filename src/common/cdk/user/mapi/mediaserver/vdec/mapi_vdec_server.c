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

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include "msg_vdec.h"
#include "mapi_vdec_api.h"
#include "mpi_vdec_api.h"
#include "mapi_vdec_comm.h"
#include "mapi_sys_api.h"
#include "mpi_sys_api.h"
#include "k_datafifo.h"

#define MPI_VO_TEST  0
#if MPI_VO_TEST
#include "mpi_vo_api.h"
#endif

typedef struct
{
    pthread_t get_stream_tid;
    k_s32 vdec_chn;
    k_bool start;

} vdec_server_chn_ctl;

static vdec_server_chn_ctl g_vdec_chn_ctl[VDEC_MAX_CHN_NUMS];
static k_vdec_datafifo g_vdec_chn_datafifo[VDEC_MAX_CHN_NUMS];
static k_msg_vdec_stream_t g_msg_vdec_stream[VDEC_MAX_CHN_NUMS];

#define CHECK_MAPI_VDEC_NULL_PTR(paraname, ptr)                      \
    do                                                               \
    {                                                                \
        if ((ptr) == NULL)                                           \
        {                                                            \
            mapi_vdec_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_VDEC_NULL_PTR;                         \
        }                                                            \
    } while (0)

#define CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn)                                  \
    do                                                                         \
    {                                                                          \
        if (vdec_chn < 0 || vdec_chn >= VDEC_MAX_CHN_NUMS)                     \
        {                                                                      \
            mapi_vdec_error_trace("vdec handle(%d) is not valid\n", vdec_chn); \
            return K_MAPI_ERR_VDEC_INVALID_HANDLE;                             \
        }                                                                      \
    } while (0)

static k_s32 _reset_vdec_ctl(k_u32 vdec_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);
    g_vdec_chn_ctl[vdec_chn].start = K_FALSE;
    g_vdec_chn_ctl[vdec_chn].get_stream_tid = 0;
    return K_SUCCESS;
}

k_s32 kd_mapi_vdec_init(k_u32 vdec_chn, const k_vdec_chn_attr *attr)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);
    CHECK_MAPI_VDEC_NULL_PTR("dev_attr", attr);
    k_s32 ret;
    ret = kd_mpi_vdec_create_chn(vdec_chn, (k_vdec_chn_attr *)attr);
    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("kd_mpi_vdec_create_chn failed:0x%x\n", ret);
        return K_FAILED;
    }

    mapi_vdec_error_trace("kd_mpi_vdec_create_chn ok,vdec_chn:%d\n", vdec_chn);

    g_vdec_chn_datafifo[vdec_chn].item_count = K_VDEC_DATAFIFO_ITEM_COUNT;
    g_vdec_chn_datafifo[vdec_chn].item_size = K_VDEC_DATAFIFO_ITEM_SIZE;
    g_vdec_chn_datafifo[vdec_chn].open_mode = DATAFIFO_READER;
    g_vdec_chn_datafifo[vdec_chn].release_func = NULL;

    return ret;
}

static k_s32 _vdec_datafifo_init_master(k_vdec_datafifo *info, k_u64 *phy_addr)
{
    k_datafifo_params_s datafifo_params;
    datafifo_params.u32EntriesNum = info->item_count;
    datafifo_params.u32CacheLineSize = info->item_size;
    datafifo_params.bDataReleaseByWriter = K_TRUE;
    datafifo_params.enOpenMode = info->open_mode;

    k_s32 s32Ret = K_FAILED;
    s32Ret = kd_datafifo_open(&info->data_hdl, &datafifo_params);
    if (K_SUCCESS != s32Ret)
    {
        mapi_vdec_error_trace("%s open datafifo error:%x\n", __FUNCTION__, s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_GET_PHY_ADDR, phy_addr);
    if (K_SUCCESS != s32Ret)
    {
        mapi_vdec_error_trace("%s get datafifo phy addr error:%x\n", __FUNCTION__, s32Ret);
        return K_FAILED;
    }

    if (info->release_func != NULL)
    {
        s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, info->release_func);
        if (K_SUCCESS != s32Ret)
        {
            mapi_vdec_error_trace("%s set release func callback error:%x\n", __FUNCTION__, s32Ret);
            return K_FAILED;
        }
    }

    // printf("@@@@@@@%s phy_addr:0x%x,datafifo handle:0x%x\n",__FUNCTION__,*phy_addr,info->data_hdl);

    return K_SUCCESS;
}

k_s32 vdec_datafifo_init(k_u32 vdec_chn, k_u64 *phy_addr)
{
    k_s32 ret;
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);
    ret = _vdec_datafifo_init_master(&g_vdec_chn_datafifo[vdec_chn], phy_addr);

    mapi_vdec_error_trace("_vdec_datafifo_init_master phy_addr:0x%x,ret:%d,datafifo handle:0x%x\n", *phy_addr, ret, g_vdec_chn_datafifo[vdec_chn].data_hdl);

    return ret;
}

static k_s32 _vdec_datafifo_deinit(k_datafifo_handle data_hdl, K_DATAFIFO_OPEN_MODE_E open_mode)
{
    k_s32 s32Ret = K_SUCCESS;
    if (DATAFIFO_WRITER == open_mode)
    {
        // call write NULL to flush and release stream buffer.
        s32Ret = kd_datafifo_write(data_hdl, NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s write error:%x\n", __FUNCTION__, s32Ret);
        }
    }

    return kd_datafifo_close(data_hdl);
}

k_s32 vdec_datafifo_deinit(k_u32 vdec_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);
    return _vdec_datafifo_deinit(g_vdec_chn_datafifo[vdec_chn].data_hdl, DATAFIFO_READER);
}

k_s32 kd_mapi_vdec_deinit(k_u32 vdec_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);
    k_s32 ret;
    ret = kd_mpi_vdec_destroy_chn(vdec_chn);
    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("kd_mpi_vdec_destroy_chn failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

static k_s32 _do_datafifo_stream(k_datafifo_handle data_hdl, void *ppData, k_u32 data_len)
{
    k_s32 ret;
    // mapi_vdec_error_trace("==========_do_datafifo_stream\n");
    if (data_len != K_VDEC_DATAFIFO_ITEM_SIZE)
    {
        mapi_vdec_error_trace("datafifo_read_func len error %d(%d)\n", data_len, sizeof(k_msg_vdec_stream_t));
        return K_FAILED;
    }

    k_msg_vdec_stream_t *msg_vdec_stream = (k_msg_vdec_stream_t *)ppData;
    k_s32 vdec_chn = msg_vdec_stream->vdec_chn;

    if (vdec_chn < 0 || vdec_chn >= VDEC_MAX_CHN_NUMS)
    {
        mapi_vdec_error_trace("vdec_hdl error %d\n", vdec_chn);
        return K_FAILED;
    }

    g_msg_vdec_stream[vdec_chn] = *msg_vdec_stream;

    // mapi_vdec_error_trace("==========_do_datafifo_stream2,vdec_chn:%d\n",vdec_chn);
    // mapi_vdec_error_trace("======stream info:viraddr:0x%x,phys_addr:0x%lx,len:%d,seq:%d\n",\
   // msg_vdec_stream->stream.stream,msg_vdec_stream->stream.phys_addr,msg_vdec_stream->stream.len,msg_vdec_stream->stream.seq);

    ret = kd_mpi_vdec_send_stream(vdec_chn, &g_msg_vdec_stream[vdec_chn].stream, -1);

    // mapi_adec_error_trace("==========_do_datafifo_stream3:%d\n",ret);

    return ret;
}

static void *vdec_chn_get_stream_threads(void *arg)
{
    k_s32 vdec_chn = *((k_handle *)arg);

    k_s32 s32Ret = K_FAILED;
    k_u32 readLen = 0;
    void *pdata = NULL;

    while (g_vdec_chn_ctl[vdec_chn].start)
    {
        s32Ret = kd_datafifo_cmd(g_vdec_chn_datafifo[vdec_chn].data_hdl, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            mapi_vdec_error_trace("%s get available read len error:%x\n", __FUNCTION__, s32Ret);
            break;
        }
        // mapi_adec_error_trace("kd_datafifo_cmd readlen:%d\n",readLen);

        if (readLen > 0)
        {
            s32Ret = kd_datafifo_read(g_vdec_chn_datafifo[vdec_chn].data_hdl, &pdata);
            // mapi_adec_error_trace("========kd_datafifo_read end:%d,ret:%d\n", readLen, s32Ret);
            if (K_SUCCESS != s32Ret)
            {
                mapi_vdec_error_trace("%s read error:%x\n", __FUNCTION__, s32Ret);
                break;
            }

            _do_datafifo_stream(g_vdec_chn_datafifo[vdec_chn].data_hdl, pdata, readLen);

            s32Ret = kd_datafifo_cmd(g_vdec_chn_datafifo[vdec_chn].data_hdl, DATAFIFO_CMD_READ_DONE, pdata);
            if (K_SUCCESS != s32Ret)
            {
                mapi_vdec_error_trace("%s read done error:%x\n", __FUNCTION__, s32Ret);
                break;
            }

            continue;
        }
        else
        {
            // printf("%s get available read len error:%x(%d)\n", __FUNCTION__, s32Ret, readLen);
            usleep(10000);
        }
    }
    return NULL;
}

k_s32 kd_mapi_vdec_start(k_u32 vdec_chn)
{
    k_s32 ret;
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);
    if (!g_vdec_chn_ctl[vdec_chn].start)
    {
        g_vdec_chn_ctl[vdec_chn].start = K_TRUE;
    }
    else
    {
        mapi_vdec_error_trace("vdec handle:%d already start\n", vdec_chn);
        return K_SUCCESS;
    }

    ret = kd_mpi_vdec_start_chn(vdec_chn);
    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("kd_mpi_vdec_start_chn failed:0x%x\n", ret);
        g_vdec_chn_ctl[vdec_chn].start = K_FALSE;
        return K_FAILED;
    }

    g_vdec_chn_ctl[vdec_chn].vdec_chn = vdec_chn;
    g_vdec_chn_ctl[vdec_chn].start = K_TRUE;
    pthread_create(&g_vdec_chn_ctl[vdec_chn].get_stream_tid, NULL, vdec_chn_get_stream_threads, &g_vdec_chn_ctl[vdec_chn].vdec_chn);

    return K_SUCCESS;
}

k_s32 kd_mapi_vdec_stop(k_u32 vdec_chn)
{
    k_s32 ret;
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);

    if (g_vdec_chn_ctl[vdec_chn].start)
    {
        g_vdec_chn_ctl[vdec_chn].start = K_FALSE;
    }
    else
    {
        mapi_vdec_error_trace("vdec channel:%d already stop\n", vdec_chn);
        return K_SUCCESS;
    }

    ret = kd_mpi_vdec_stop_chn(vdec_chn);
    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("kd_mpi_vdec_stop_chn failed:0x%x\n", ret);
        g_vdec_chn_ctl[vdec_chn].start = K_FALSE;
        return K_FAILED;
    }

    if (g_vdec_chn_ctl[vdec_chn].get_stream_tid != 0)
    {
        g_vdec_chn_ctl[vdec_chn].start = K_FALSE;
        pthread_join(g_vdec_chn_ctl[vdec_chn].get_stream_tid, NULL);
        g_vdec_chn_ctl[vdec_chn].get_stream_tid = 0;
    }

    return K_SUCCESS;
}

#if MPI_VO_TEST
#include "mpi_vo_api.h"
#include "k_vo_comm.h"

typedef struct
{
    k_u32 w;
    k_u32 h;
    k_vo_layer ch;
    k_vo_rotation ro;
} layer_bind_config;

typedef struct
{
    k_u64 layer_phy_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;

    //only layer0、layer1
    k_u32 func;
    // only layer0
    k_vo_scaler_attr attr;

} layer_info;

static k_vo_display_resolution hx8399[20] =
{
    // {74250, 445500, 1340, 1080, 20, 20, 220, 1938, 1920, 5, 8, 10},           // display  evblp3
    {74250, 445500, 1240, 1080, 20, 20, 120, 1988, 1920, 5, 8, 55},
};

static void hx8399_v2_init(k_u8 test_mode_en)
{
    k_u8 param1[] = {0xB9, 0xFF, 0x83, 0x99};
    k_u8 param21[] = {0xD2, 0xAA};
    k_u8 param2[] = {0xB1, 0x02, 0x04, 0x71, 0x91, 0x01, 0x32, 0x33, 0x11, 0x11, 0xab, 0x4d, 0x56, 0x73, 0x02, 0x02};
    k_u8 param3[] = {0xB2, 0x00, 0x80, 0x80, 0xae, 0x05, 0x07, 0x5a, 0x11, 0x00, 0x00, 0x10, 0x1e, 0x70, 0x03, 0xd4};
    k_u8 param4[] = {0xB4, 0x00, 0xFF, 0x02, 0xC0, 0x02, 0xc0, 0x00, 0x00, 0x08, 0x00, 0x04, 0x06, 0x00, 0x32, 0x04, 0x0a, 0x08, 0x21, 0x03, 0x01, 0x00, 0x0f, 0xb8, 0x8b, 0x02, 0xc0, 0x02, 0xc0, 0x00, 0x00, 0x08, 0x00, 0x04, 0x06, 0x00, 0x32, 0x04, 0x0a, 0x08, 0x01, 0x00, 0x0f, 0xb8, 0x01};
    k_u8 param5[] = {0xD3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x10, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x05, 0x05, 0x07, 0x00, 0x00, 0x00, 0x05, 0x40};
    k_u8 param6[] = {0xD5, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x21, 0x20, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x2f, 0x2f, 0x30, 0x30, 0x31, 0x31, 0x18, 0x18, 0x18, 0x18};
    k_u8 param7[] = {0xD6, 0x18, 0x18, 0x19, 0x19, 0x40, 0x40, 0x20, 0x21, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x2f, 0x2f, 0x30, 0x30, 0x31, 0x31, 0x40, 0x40, 0x40, 0x40};
    k_u8 param8[] = {0xD8, 0xa2, 0xaa, 0x02, 0xa0, 0xa2, 0xa8, 0x02, 0xa0, 0xb0, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00};
    k_u8 param9[] = {0xBD, 0x01};
    k_u8 param10[] = {0xD8, 0xB0, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x00, 0x00, 0xE2, 0xAA, 0x03, 0xF0, 0xE2, 0xAA, 0x03, 0xF0};
    k_u8 param11[] = {0xBD, 0x02};
    k_u8 param12[] = {0xD8, 0xE2, 0xAA, 0x03, 0xF0, 0xE2, 0xAA, 0x03, 0xF0};
    k_u8 param13[] = {0xBD, 0x00};
    k_u8 param14[] = {0xB6, 0x8D, 0x8D};
    k_u8 param15[] = {0xCC, 0x09};
    k_u8 param16[] = {0xC6, 0xFF, 0xF9};
    k_u8 param22[] = {0xE0, 0x00, 0x12, 0x1f, 0x1a, 0x40, 0x4a, 0x59, 0x55, 0x5e, 0x67, 0x6f, 0x75, 0x7a, 0x82, 0x8b, 0x90, 0x95, 0x9f, 0xa3, 0xad, 0xa2, 0xb2, 0xB6, 0x5e, 0x5a, 0x65, 0x77, 0x00, 0x12, 0x1f, 0x1a, 0x40, 0x4a, 0x59, 0x55, 0x5e, 0x67, 0x6f, 0x75, 0x7a, 0x82, 0x8b, 0x90, 0x95, 0x9f, 0xa3, 0xad, 0xa2, 0xb2, 0xB6, 0x5e, 0x5a, 0x65, 0x77};
    k_u8 param23[] = {0x11};
    k_u8 param24[] = {0x29};

    k_u8 pag20[50] = {0xB2, 0x0b, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};               // 蓝色


    kd_mpi_dsi_send_cmd(param1, sizeof(param1));
    kd_mpi_dsi_send_cmd(param21, sizeof(param21));
    kd_mpi_dsi_send_cmd(param2, sizeof(param2));
    kd_mpi_dsi_send_cmd(param3, sizeof(param3));
    kd_mpi_dsi_send_cmd(param4, sizeof(param4));
    kd_mpi_dsi_send_cmd(param5, sizeof(param5));
    kd_mpi_dsi_send_cmd(param6, sizeof(param6));
    kd_mpi_dsi_send_cmd(param7, sizeof(param7));
    kd_mpi_dsi_send_cmd(param8, sizeof(param8));
    kd_mpi_dsi_send_cmd(param9, sizeof(param9));

    if (test_mode_en == 1)
    {
        kd_mpi_dsi_send_cmd(pag20, 10);                   // test  mode
    }

    kd_mpi_dsi_send_cmd(param10, sizeof(param10));
    kd_mpi_dsi_send_cmd(param11, sizeof(param11));
    kd_mpi_dsi_send_cmd(param12, sizeof(param12));
    kd_mpi_dsi_send_cmd(param13, sizeof(param13));
    kd_mpi_dsi_send_cmd(param14, sizeof(param14));
    kd_mpi_dsi_send_cmd(param15, sizeof(param15));
    kd_mpi_dsi_send_cmd(param16, sizeof(param16));
    kd_mpi_dsi_send_cmd(param22, sizeof(param22));
    kd_mpi_dsi_send_cmd(param23, 1);
    usleep(300000);
    kd_mpi_dsi_send_cmd(param24, 1);
    usleep(100000);
}

static void _dwc_dsi_init(void)
{

    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];
    k_vo_dsi_attr attr;

    k_vo_mipi_phy_attr phy_attr;
    int enable = 1;
    int screen_test_mode = 0;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));


    // config phy
    phy_attr.phy_lan_num = K_DSI_4LAN;
    phy_attr.m = 295;
    phy_attr.n = 15;
    phy_attr.voc = 0x17;
    phy_attr.hs_freq = 0x96;
    kd_mpi_set_mipi_phy_attr(&phy_attr);


    attr.lan_num = K_DSI_4LAN;
    attr.cmd_mode = K_VO_LP_MODE;
    attr.lp_div = 8;
    memcpy(&attr.resolution, resolution, sizeof(k_vo_display_resolution));
    // set dsi timing
    kd_mpi_dsi_set_attr(&attr);

    // config scann
    hx8399_v2_init(screen_test_mode);

    // enable dsi
    kd_mpi_dsi_enable(enable);

}

static int vo_creat_layer_test(k_vo_layer chn_id, layer_info *info)
{
    k_vo_video_layer_attr attr;

    // check layer
    if ((chn_id >= K_MAX_VO_LAYER_NUM) || ((info->func & K_VO_SCALER_ENABLE) && (chn_id != K_VO_LAYER0))
            || ((info->func != 0) && (chn_id == K_VO_LAYER2)))
    {
        printf("input layer num failed \n");
        return -1 ;
    }

    // check scaler

    // set offset
    attr.display_rect = info->offset;
    // set act
    attr.img_size = info->act_size;
    // sget size
    info->size = info->act_size.height * info->act_size.width * 3 / 2;
    //set pixel format
    attr.pixel_format = info->format;
    if (info->format != PIXEL_FORMAT_YVU_PLANAR_420)
    {
        printf("input pix format failed \n");
        return -1;
    }
    // set stride
    attr.stride = (info->act_size.width / 8 - 1) + ((info->act_size.height - 1) << 16);
    // set function
    attr.func = info->func;
    // set scaler attr
    attr.scaler_attr = info->attr;

    // set video layer atrr
    kd_mpi_vo_set_video_layer_attr(chn_id, &attr);

    // enable layer
    kd_mpi_vo_enable_video_layer(chn_id);

    return 0;
}

static k_s32 _vo_layer_bind_config(layer_bind_config *config)
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    layer_info info;

    k_vo_layer chn_id = config->ch;

    // set hardware reset;
    kd_display_set_backlight();
    // rst display subsystem
    kd_display_reset();

    _dwc_dsi_init();

    memset(&attr, 0, sizeof(attr));
    memset(&info, 0, sizeof(info));

    attr.bg_color = 0x808000;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    // vo init
    kd_mpi_vo_init();
    // set vo timing
    kd_mpi_vo_set_dev_param(&attr);
    printf("%s>w %d, h %d\n", __func__, config->w, config->h);
    // config lyaer
    info.act_size.width = config->w;  // 1080;//640;//1080;
    info.act_size.height = config->h; // 1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = config->ro;
    info.global_alptha = 0xff;
    info.offset.x = 0; //(1080-w)/2,
    info.offset.y = 0; //(1920-h)/2;
    // info.attr.out_size.width = 1080;//640;
    // info.attr.out_size.height = 1920;//480;
    vo_creat_layer_test(chn_id, &info);

    // enable vo
    kd_mpi_vo_enable();

    // exit ;
    return 0;
}

#define ENABLE_VO_LAYER   1
static k_u32 g_vo_width  = 0;
static k_u32 g_vo_height  = 0;
static k_s32 _test_init_vo(k_vo_layer layer,k_u32 w,k_u32 h)
{
    if (0 == w || 0 == h)
    {
        w = 1920;
        h = 1080;
    }
    layer_bind_config config;
    config.ch = layer;

    if (w > 1080)
    {
        config.w = h; // 1080;
        config.h = w;  // 1920;
        config.ro = K_ROTATION_90;
    }
    else
    {
        config.w = w; // 1080;
        config.h = h;  // 1920;
        config.ro = 0;//K_ROTATION_0;
    }
    return _vo_layer_bind_config(&config);
}

static k_s32 _test_deinit_vo(k_vo_layer layer)
{
    return kd_mpi_vo_disable_video_layer(layer);
}

static pthread_t g_config_vo_tid;
static void *vo_init_thread(void *arg)
{
    k_s32 chn_num = *((k_handle *)arg);
    k_vdec_chn_status status;
    while(1)
    {
        k_s32 ret;
        ret = kd_mpi_vdec_query_status(chn_num, &status);
        if (ret == K_SUCCESS)
        {
            if (status.width != 0 && status.height != 0)
            {
                g_vo_width = status.width;
                g_vo_height = status.height;
                break;
            }
        }

        usleep(500*1000);
        continue;
    }

    mapi_vdec_error_trace("config vo width:%d,height:%d\n", g_vo_width,g_vo_height);
    _test_init_vo(ENABLE_VO_LAYER,g_vo_width,g_vo_height);

    return NULL;
}
#endif

k_s32 kd_mapi_vdec_bind_vo(k_u32 chn_num, k_u32 vo_dev, k_u32 vo_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);
#if MPI_VO_TEST
    pthread_create(&g_config_vo_tid, NULL, vo_init_thread, &g_vdec_chn_ctl[chn_num].vdec_chn);

    k_mpp_chn vdec_mpp_chn;
    k_mpp_chn vvo_mpp_chn;
    k_s32 ret;

    vdec_mpp_chn.mod_id = K_ID_VDEC;
    vdec_mpp_chn.dev_id = 0;
    vdec_mpp_chn.chn_id = chn_num;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = vo_dev;
    vvo_mpp_chn.chn_id = vo_chn;
    ret = kd_mpi_sys_bind(&vdec_mpp_chn, &vvo_mpp_chn);
    if (ret)
    {
        mapi_vdec_error_trace("kd_mpi_sys_bind failed:0x%x\n", ret);
    }
#else
    k_mpp_chn vdec_mpp_chn;
    k_mpp_chn vvo_mpp_chn;
    k_s32 ret;

    vdec_mpp_chn.mod_id = K_ID_VDEC;
    vdec_mpp_chn.dev_id = 0;
    vdec_mpp_chn.chn_id = chn_num;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = vo_dev;
    vvo_mpp_chn.chn_id = vo_chn;
    ret = kd_mpi_sys_bind(&vdec_mpp_chn, &vvo_mpp_chn);
    if (ret)
    {
        mapi_vdec_error_trace("kd_mpi_sys_bind failed:0x%x\n", ret);
    }
    return ret;
#endif

}

k_s32 kd_mapi_vdec_unbind_vo(k_u32 chn_num, k_u32 vo_dev, k_u32 vo_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);
    k_mpp_chn vdec_mpp_chn;
    k_mpp_chn vvo_mpp_chn;
    k_s32 ret;

    vdec_mpp_chn.mod_id = K_ID_VDEC;
    vdec_mpp_chn.dev_id = 0;
    vdec_mpp_chn.chn_id = chn_num;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = vo_dev;
    vvo_mpp_chn.chn_id = vo_chn;

    ret = kd_mpi_sys_unbind(&vdec_mpp_chn, &vvo_mpp_chn);
#if MPI_VO_TEST
    _test_deinit_vo(ENABLE_VO_LAYER);
    if (g_config_vo_tid != 0)
    {
        pthread_join(g_config_vo_tid, NULL);
        g_config_vo_tid = 0;
    }
#endif
    return ret;
}

k_s32 kd_mapi_vdec_send_stream(k_u32 chn_num, k_vdec_stream *stream, k_s32 milli_sec)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);
    CHECK_MAPI_VDEC_NULL_PTR("stream", stream);

    k_s32 ret;
    ret = kd_mpi_vdec_send_stream(chn_num, stream, milli_sec);
    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("kd_mpi_vdec_send_stream failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

k_s32 kd_mapi_vdec_query_status(k_u32 chn_num, k_vdec_chn_status *status)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);
    CHECK_MAPI_VDEC_NULL_PTR("status", status);

    k_s32 ret;
    ret = kd_mpi_vdec_query_status(chn_num, status);
    //mapi_vdec_error_trace("kd_mapi_vdec_query_status width:%d,height:%d,endstream:%d\n",\
    status->width,status->height,status->end_of_stream);
    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("kd_mpi_vdec_query_status failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}
