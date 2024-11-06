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
#include <stdio.h>
#include <fcntl.h>
#include "msg_ai.h"
#include "mapi_ai_api.h"
#include "mpi_ai_api.h"
#include "mapi_ai_comm.h"
#include "mapi_sys_api.h"
#include "k_acodec_comm.h"
#include "k_datafifo.h"

typedef struct
{
    pthread_t get_ai_aec_frame_tid;
    k_handle ai_hdl;
    k_bool start;

} ai_aec_server_chn_ctl;

static ai_aec_server_chn_ctl g_ai_aec_chn_ctl[AI_MAX_CHN_NUMS];
static k_ai_aec_datafifo g_ai_aec_chn_datafifo[AI_MAX_CHN_NUMS];
static k_msg_ai_frame_t g_msg_ai_aec_frame[AI_MAX_CHN_NUMS];
static k_ai_vqe_enable g_vqe_enable[AI_MAX_CHN_NUMS];

#define CHECK_MAPI_AI_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_ai_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AI_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

static k_s32 g_acodec_fd = -1;
static k_s32 acodec_check_open(void)
{
    if (g_acodec_fd < 0)
    {
        g_acodec_fd = open("/dev/acodec_device", O_RDWR);
        if (g_acodec_fd < 0)
        {
            perror("open err\n");
            return -1;
        }
    }
    return 0;
}

static k_s32 acodec_check_close(void)
{
    if (g_acodec_fd >= 0)
    {
        close(g_acodec_fd);
        g_acodec_fd = -1;
    }
    return 0;
}

static k_s32 _ai_aec_datafifo_init_master(k_ai_aec_datafifo *info, k_u64 *phy_addr)
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
        mapi_ai_error_trace("%s open datafifo error:%x\n", __FUNCTION__, s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_GET_PHY_ADDR, phy_addr);
    if (K_SUCCESS != s32Ret)
    {
        mapi_ai_error_trace("%s get datafifo phy addr error:%x\n", __FUNCTION__, s32Ret);
        return K_FAILED;
    }

    if (info->release_func != NULL)
    {
        s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, info->release_func);
        if (K_SUCCESS != s32Ret)
        {
            mapi_ai_error_trace("%s set release func callback error:%x\n", __FUNCTION__, s32Ret);
            return K_FAILED;
        }
    }

    // printf("@@@@@@@%s phy_addr:0x%x,datafifo handle:0x%x\n",__FUNCTION__,*phy_addr,info->data_hdl);

    return K_SUCCESS;
}

k_s32 ai_aec_datafifo_init(k_handle ai_hdl, k_u64 *phy_addr)
{
    k_s32 ret;
    g_ai_aec_chn_datafifo[ai_hdl].item_count = K_AI_AEC_DATAFIFO_ITEM_COUNT;
    g_ai_aec_chn_datafifo[ai_hdl].item_size = K_AI_AEC_DATAFIFO_ITEM_SIZE;
    g_ai_aec_chn_datafifo[ai_hdl].open_mode = DATAFIFO_READER;
    g_ai_aec_chn_datafifo[ai_hdl].release_func = NULL;

    ret = _ai_aec_datafifo_init_master(&g_ai_aec_chn_datafifo[ai_hdl], phy_addr);

    mapi_ai_error_trace("_ai_aec_datafifo_init_master phy_addr:0x%x,ret:%d,datafifo handle:0x%x\n", *phy_addr, ret, g_ai_aec_chn_datafifo[ai_hdl].data_hdl);

    return ret;
}

static k_s32 _ai_aec_datafifo_deinit(k_datafifo_handle data_hdl, K_DATAFIFO_OPEN_MODE_E open_mode)
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

k_s32 ai_aec_datafifo_deinit(k_handle ai_hdl)
{
    return _ai_aec_datafifo_deinit(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, DATAFIFO_READER);
}

static k_s32 _do_datafifo_frame(k_datafifo_handle data_hdl, void *ppData, k_u32 data_len)
{
    k_s32 ret;
    // mapi_adec_error_trace("==========_do_datafifo_stream\n");
    if (data_len != K_AI_AEC_DATAFIFO_ITEM_SIZE)
    {
        mapi_ai_error_trace("datafifo_read_func len error %d(%d)\n", data_len, sizeof(k_msg_ai_frame_t));
        return K_FAILED;
    }

    k_msg_ai_frame_t *msg_ai_aec_frame = (k_msg_ai_frame_t *)ppData;
    k_handle ai_hdl = msg_ai_aec_frame->ai_hdl;

    if (ai_hdl < 0 || ai_hdl >= AI_MAX_CHN_NUMS)
    {
        mapi_ai_error_trace("ai_hdl error %d\n", ai_hdl);
        return K_FAILED;
    }

    g_msg_ai_aec_frame[ai_hdl] = *msg_ai_aec_frame;

    ret = kd_mapi_ai_send_far_echo_frame(ai_hdl, &g_msg_ai_aec_frame[ai_hdl].audio_frame, g_msg_ai_aec_frame[ai_hdl].milli_sec);

    // mapi_adec_error_trace("==========_do_datafifo_stream3:%d\n",ret);

    return ret;
}

static void *ai_aec_chn_get_frame_threads(void *arg)
{
    k_handle ai_hdl = *((k_handle *)arg);

    k_s32 s32Ret = K_FAILED;
    k_u32 readLen = 0;
    void *pdata = NULL;

    while (g_ai_aec_chn_ctl[ai_hdl].start)
    {
        s32Ret = kd_datafifo_cmd(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            mapi_ai_error_trace("%s get available read len error:%x\n", __FUNCTION__, s32Ret);
            break;
        }

        if (readLen > 0)
        {
            while (readLen >= K_AI_AEC_DATAFIFO_ITEM_SIZE)
            {
                s32Ret = kd_datafifo_read(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, &pdata);
                // mapi_adec_error_trace("========kd_datafifo_read end:%d,ret:%d\n", readLen, s32Ret);
                if (K_SUCCESS != s32Ret)
                {
                    mapi_ai_error_trace("%s read error:%x\n", __FUNCTION__, s32Ret);
                    break;
                }

                _do_datafifo_frame(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, pdata, K_AI_AEC_DATAFIFO_ITEM_SIZE);

                s32Ret = kd_datafifo_cmd(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, DATAFIFO_CMD_READ_DONE, pdata);
                if (K_SUCCESS != s32Ret)
                {
                    mapi_ai_error_trace("%s read done error:%x\n", __FUNCTION__, s32Ret);
                    break;
                }
                readLen -= K_AI_AEC_DATAFIFO_ITEM_SIZE;
            }
        }
        else
        {
            usleep(1000);
        }
    }
    return NULL;
}

static void _clean_datafifo(k_handle ai_hdl)
{
    k_s32 s32Ret = K_FAILED;
    k_u32 readLen = 0;
    void *pdata = NULL;

    while (1)
    {
        s32Ret = kd_datafifo_cmd(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            mapi_ai_error_trace("%s get available read len error:%x\n", __FUNCTION__, s32Ret);
            break;
        }
        if (readLen <= 0)
        {
            break;
        }

        while (readLen >= K_AI_AEC_DATAFIFO_ITEM_SIZE)
        {
            s32Ret = kd_datafifo_read(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, &pdata);
            if (K_SUCCESS != s32Ret)
            {
                mapi_ai_error_trace("%s read error:%x\n", __FUNCTION__, s32Ret);
                break;
            }

            s32Ret = kd_datafifo_cmd(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, DATAFIFO_CMD_READ_DONE, pdata);
            if (K_SUCCESS != s32Ret)
            {
                mapi_ai_error_trace("%s read done error:%x\n", __FUNCTION__, s32Ret);
                break;
            }
            readLen -= K_AI_AEC_DATAFIFO_ITEM_SIZE;
        }
        usleep(10000);
    }
}

k_s32 kd_mapi_ai_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ai_hdl)
{
    CHECK_MAPI_AI_NULL_PTR("dev_attr", dev_attr);
    k_s32 ret;
    ret = kd_mpi_ai_set_pub_attr(dev, dev_attr);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_set_pub_attr failed:0x%x\n", ret);
        return K_FAILED;
    }

    *ai_hdl = AI_CREATE_HANDLE(dev,chn);

    memset(&g_vqe_enable,0,sizeof(g_vqe_enable));

    return ret;
}

k_s32 kd_mapi_ai_deinit(k_handle ai_hdl)
{
    acodec_check_close();
    return K_SUCCESS;
}

k_s32 kd_mapi_ai_start(k_handle ai_hdl)
{
    k_s32 ret;
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    //i2s
    if (0 == dev)
    {
        if (chn <0 || chn >1)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    //pdm
    else if (1 == dev)
    {
        if (chn <0 || chn > 3)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    else
    {
        mapi_ai_error_trace("dev value not supported\n");
        return K_FAILED;
    }


    ret = kd_mpi_ai_enable(dev);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_enable failed:0x%x\n", ret);
        return K_FAILED;
    }

    if (1 == dev)
    {
        //pdm enable must from small to large
        for (int i =0;i <= chn;i++)
        {
           ret = kd_mpi_ai_enable_chn(dev,i);
        }
    }
    else
    {
        ret = kd_mpi_ai_enable_chn(dev,chn);
    }


    if (!g_ai_aec_chn_ctl[ai_hdl].start)
    {
        g_ai_aec_chn_ctl[ai_hdl].start = K_TRUE;
    }
    else
    {
        mapi_ai_error_trace("ai aec handle:%d already start\n", ai_hdl);
    }

    if (g_vqe_enable[ai_hdl].aec_enable)
    {
        _clean_datafifo(ai_hdl);
        g_ai_aec_chn_ctl[ai_hdl].ai_hdl = ai_hdl;
        g_ai_aec_chn_ctl[ai_hdl].start = K_TRUE;
        pthread_create(&g_ai_aec_chn_ctl[ai_hdl].get_ai_aec_frame_tid, NULL, ai_aec_chn_get_frame_threads, &g_ai_aec_chn_ctl[ai_hdl].ai_hdl);
        struct sched_param param;
        param.sched_priority = 17;
        pthread_setschedparam(g_ai_aec_chn_ctl[ai_hdl].get_ai_aec_frame_tid, SCHED_FIFO, &param);
    }

    return ret;
}

k_s32 kd_mapi_ai_stop(k_handle ai_hdl)
{
    k_s32 ret;
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    //i2s
    if (0 == dev)
    {
        if (chn <0 || chn >1)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    //pdm
    else if (1 == dev)
    {
        if (chn <0 || chn > 3)
        {
            mapi_ai_error_trace("chn value not supported\n");
            return K_FAILED;
        }
    }
    else
    {
        mapi_ai_error_trace("dev value not supported\n");
        return K_FAILED;
    }

    if (g_vqe_enable[ai_hdl].aec_enable)
    {
        if (g_ai_aec_chn_ctl[ai_hdl].get_ai_aec_frame_tid != 0)
        {
            g_ai_aec_chn_ctl[ai_hdl].start = K_FALSE;
            pthread_join(g_ai_aec_chn_ctl[ai_hdl].get_ai_aec_frame_tid, NULL);
            g_ai_aec_chn_ctl[ai_hdl].get_ai_aec_frame_tid = 0;
        }
        _clean_datafifo(ai_hdl);
    }

    if (1 == dev)
    {
        //pdm disable must from large to small
        for (int i = chn; i >= 0; i --)
        {
            ret = kd_mpi_ai_disable_chn(dev, i);
        }
    }
    else
    {
        ret = kd_mpi_ai_disable_chn(dev,chn);
    }

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_disable_chn failed:0x%x,dev:%d,channel:%d\n", ret,dev,chn);
        return K_FAILED;
    }

    ret = kd_mpi_ai_disable(dev);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("kd_mpi_ai_disable failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

k_s32 kd_mapi_ai_get_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_get_frame(dev,chn,frame,1000);
}

k_s32 kd_mapi_ai_release_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_release_frame(dev,chn,frame);
}

k_s32 kd_mapi_ai_set_pitch_shift_attr(k_handle ai_hdl, const k_ai_chn_pitch_shift_param *param)
{
    CHECK_MAPI_AI_NULL_PTR("param", param);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_set_pitch_shift_attr(dev,chn, param);
}

k_s32 kd_mapi_ai_get_pitch_shift_attr(k_handle ai_hdl, k_ai_chn_pitch_shift_param *param)
{
    CHECK_MAPI_AI_NULL_PTR("param", param);
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_get_pitch_shift_attr(dev,chn, param);
}

k_s32 kd_mapi_ai_bind_ao(k_handle ai_hdl,k_handle ao_hdl)
{
    k_mpp_chn ai_mpp_chn;
    k_mpp_chn ao_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ((ai_hdl) >> 16) & 0xFFFF;
    ai_mpp_chn.chn_id = (ai_hdl) & 0xFFFF;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ((ao_hdl) >> 16) & 0xFFFF;
    ao_mpp_chn.chn_id = (ao_hdl) & 0xFFFF;

    return kd_mpi_sys_bind(&ai_mpp_chn, &ao_mpp_chn);
}

k_s32 kd_mapi_ai_unbind_ao(k_handle ai_hdl,k_handle ao_hdl)
{
    k_mpp_chn ai_mpp_chn;
    k_mpp_chn ao_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ((ai_hdl) >> 16) & 0xFFFF;
    ai_mpp_chn.chn_id = (ai_hdl) & 0xFFFF;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ((ao_hdl) >> 16) & 0xFFFF;
    ao_mpp_chn.chn_id = (ao_hdl) & 0xFFFF;

    return kd_mpi_sys_unbind(&ai_mpp_chn, &ao_mpp_chn);
}

k_s32 kd_mapi_ai_set_volume(k_handle ai_hdl,float volume)
{
    k_s32 ret;
    float gain_value = volume;
    if (acodec_check_open())
        return K_FAILED;

    ioctl(g_acodec_fd, k_acodec_set_alc_gain_micl, &gain_value);
    ioctl(g_acodec_fd, k_acodec_set_alc_gain_micr, &gain_value);

    return 0;
}

k_s32 kd_mapi_acodec_reset()
{
    if (acodec_check_open())
        return K_FAILED;

    ioctl(g_acodec_fd, k_acodec_reset, NULL);
    return 0;
}

k_s32 kd_mapi_ai_set_vqe_attr(k_handle ai_hdl,const k_ai_vqe_enable vqe_enable)
{
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);
    g_vqe_enable[ai_hdl] = vqe_enable;
    return kd_mpi_ai_set_vqe_attr(dev,chn, vqe_enable);
}

k_s32 kd_mapi_ai_get_vqe_attr(k_handle ai_hdl, k_ai_vqe_enable *vqe_enable)
{
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_get_vqe_attr(dev,chn, vqe_enable);
}

k_s32 kd_mapi_ai_send_far_echo_frame(k_handle ai_hdl, const k_audio_frame *audio_frame, k_s32 milli_sec)
{
    k_u32 dev;
    k_u32 chn;
    AI_GET_DEVICE_AND_CHANNEL(ai_hdl,dev,chn);

    return kd_mpi_ai_send_far_echo_frame(dev,chn, audio_frame, milli_sec);
    // mapi_ai_error_trace("========kd_mapi_ai_send_far_echo_frame\n");
    // return K_SUCCESS;
}

