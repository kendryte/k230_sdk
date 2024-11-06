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
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "mapi_ai_api.h"
#include "msg_client_dispatch.h"
#include "mapi_ai_comm.h"
#include "mpi_ai_api.h"
#include "msg_ai.h"
#include "k_type.h"
#include "k_datafifo.h"

static k_ai_aec_datafifo g_ai_aec_chn_datafifo[AI_MAX_CHN_NUMS];
static sem_t g_ai_aec_chn_send_frame_sem[AI_MAX_CHN_NUMS];
static k_ai_vqe_enable g_vqe_enable[AI_MAX_CHN_NUMS];
static k_msg_ai_frame_t  g_ai_aec_chn_frame[AI_MAX_CHN_NUMS];

#define CHECK_MAPI_AI_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_ai_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AI_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

// client -> server,must client send mapi to server, server open datafifo and return phyaddr and client open datafifo by phyaddr
static k_s32 _ai_aec_init_datafifo(k_u32 ai_chn, k_u64 *datafifo_phyaddr)
{
    k_s32 ret;
    k_msg_ai_aec_datafifo_t ai_aec_datafifo;
    ai_aec_datafifo.ai_hdl = ai_chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_AEC_INIT_DATAFIFO,
                         &ai_aec_datafifo, sizeof(ai_aec_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    *datafifo_phyaddr = ai_aec_datafifo.phyAddr;

    return ret;
}

static k_s32 _ai_aec_deinit_datafifo(k_u32 ai_chn)
{
    k_s32 ret;
    k_msg_ai_aec_datafifo_t ai_aec_datafifo;
    ai_aec_datafifo.ai_hdl = ai_chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_AEC_DEINIT_DATAFIFO,
                         &ai_aec_datafifo, sizeof(ai_aec_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

static k_s32 _ai_aec_datafifo_init_slave(k_ai_aec_datafifo *info, k_u64 phy_addr)
{
    k_datafifo_params_s datafifo_params;
    datafifo_params.u32EntriesNum = info->item_count;
    datafifo_params.u32CacheLineSize = info->item_size;
    datafifo_params.bDataReleaseByWriter = K_TRUE;
    datafifo_params.enOpenMode = info->open_mode;

    k_s32 s32Ret = K_FAILED;
    s32Ret = kd_datafifo_open_by_addr(&info->data_hdl, &datafifo_params, phy_addr);
    if (K_SUCCESS != s32Ret)
    {
        printf("%s open datafifo error:%x\n", __FUNCTION__, s32Ret);
        return K_FAILED;
    }

    if (info->release_func != NULL)
    {
        s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, info->release_func);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s set release func callback error:%x\n", __FUNCTION__, s32Ret);
            return K_FAILED;
        }
    }

    //printf("@@@@@@@%s phy_addr:0x%x,datafifo handle:0x%x\n", __FUNCTION__, phy_addr, info->data_hdl);

    return K_SUCCESS;
}

static void _datafifo_release_func(void* pFrame)
{
    k_msg_ai_frame_t* msg_ai_aec_frame = (k_msg_ai_frame_t*)pFrame;
    k_handle ai_hdl = msg_ai_aec_frame->ai_hdl;

    sem_post(&g_ai_aec_chn_send_frame_sem[ai_hdl]);

}

static k_s32 _init_datafifo(k_handle ai_hdl)
{
    k_s32 ret;
    k_u64 datafifo_phyaddr;
    ret = _ai_aec_init_datafifo(ai_hdl, &datafifo_phyaddr);

    if (ret != K_SUCCESS)
    {
        mapi_ai_error_trace("_ai_init_datafifo failed\n");
    }
    else
    {
        g_ai_aec_chn_datafifo[ai_hdl].item_count = K_AI_AEC_DATAFIFO_ITEM_COUNT;
        g_ai_aec_chn_datafifo[ai_hdl].item_size = K_AI_AEC_DATAFIFO_ITEM_SIZE;
        g_ai_aec_chn_datafifo[ai_hdl].open_mode = DATAFIFO_WRITER;
        g_ai_aec_chn_datafifo[ai_hdl].release_func = _datafifo_release_func;

        ret = _ai_aec_datafifo_init_slave(&g_ai_aec_chn_datafifo[ai_hdl], datafifo_phyaddr);
        if (ret != K_SUCCESS)
        {
            mapi_ai_error_trace("_adec_datafifo_init_slave failed\n");
        }
        else
        {
            mapi_ai_error_trace("_ai_aec_datafifo_init_slave ok,datafifo_phyaddr:0x%x,data_hdl:0x%x\n",
                                  datafifo_phyaddr, g_ai_aec_chn_datafifo[ai_hdl].data_hdl);
        }
    }

    return ret;
}

static k_s32 _ai_aec_datafifo_write_data(k_datafifo_handle data_hdl, void *data, k_u32 data_len)
{
    k_s32 s32Ret = K_FAILED;
    k_u32 availWriteLen = 0;

    // call write NULL to flush
    s32Ret = kd_datafifo_write(data_hdl, NULL);
    if (K_SUCCESS != s32Ret)
    {
        mapi_ai_error_trace("%s write error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
    if (K_SUCCESS != s32Ret)
    {
        mapi_ai_error_trace("%s get available write len error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    if (availWriteLen >= data_len)
    {
        s32Ret = kd_datafifo_write(data_hdl, data);
        if (K_SUCCESS != s32Ret)
        {
            mapi_ai_error_trace("%s write error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }

        s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_WRITE_DONE, NULL);
        if (K_SUCCESS != s32Ret)
        {
            mapi_ai_error_trace("%s write done error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }
    }
    else
    {
        mapi_ai_error_trace("%s no availWriteLen %d_%d\n",__FUNCTION__, availWriteLen, data_len);
        return K_FAILED;
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_ai_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ai_hdl)
{
    k_s32 ret;
    CHECK_MAPI_AI_NULL_PTR("dev_attr", dev_attr);
    k_msg_ai_pipe_attr_t ai_attr;
    ai_attr.ai_dev = dev;
    ai_attr.ai_chn = chn;
    ai_attr.attr = *dev_attr;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_INIT,
        &ai_attr, sizeof(ai_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    *ai_hdl = ai_attr.ai_hdl;

    return ret;
}

k_s32 kd_mapi_ai_deinit(k_handle ai_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_DEINIT,
        &ai_hdl, sizeof(ai_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_start(k_handle ai_hdl)
{
    k_s32 ret;

    if (g_vqe_enable[ai_hdl].aec_enable)
    {
        // init datafifo
        ret = _init_datafifo(ai_hdl);
        if (ret != K_SUCCESS)
        {
            mapi_ai_error_trace("_init_datafifo failed\n");
        }
        sem_init(&g_ai_aec_chn_send_frame_sem[ai_hdl],0,0);
    }

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_START,
        &ai_hdl, sizeof(ai_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_stop(k_handle ai_hdl)
{
    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_STOP,
        &ai_hdl, sizeof(ai_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    if (g_vqe_enable[ai_hdl].aec_enable)
    {
        _ai_aec_deinit_datafifo(ai_hdl);
        sem_destroy(&g_ai_aec_chn_send_frame_sem[ai_hdl]);
    }
    return ret;
}

k_s32 kd_mapi_ai_get_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);

    k_s32 ret;
    k_msg_ai_frame_t ai_frame;
    ai_frame.ai_hdl = ai_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_GETFRAME,
        &ai_frame, sizeof(ai_frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    memcpy(frame, &ai_frame.audio_frame, sizeof(k_audio_frame));
    return ret;
}

k_s32 kd_mapi_ai_release_frame(k_handle ai_hdl, k_audio_frame *frame)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);

    k_s32 ret;
    k_msg_ai_frame_t ai_frame;
    ai_frame.ai_hdl = ai_hdl;
    ai_frame.audio_frame = *frame;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_RELEASEFRAME,
        &ai_frame, sizeof(ai_frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_set_pitch_shift_attr(k_handle ai_hdl, const k_ai_chn_pitch_shift_param *param)
{
    CHECK_MAPI_AI_NULL_PTR("param", param);

    k_s32 ret;
    k_msg_ai_pitch_shift_attr_t ai_ps_attr;
    ai_ps_attr.ai_hdl = ai_hdl;
    ai_ps_attr.param = *param;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_SET_PITCH_SHIFT_ATTR,
        &ai_ps_attr, sizeof(ai_ps_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_get_ptich_shift_attr(k_handle ai_hdl, k_ai_chn_pitch_shift_param *param)
{
    CHECK_MAPI_AI_NULL_PTR("param", param);

    k_s32 ret;
    k_msg_ai_pitch_shift_attr_t ai_ps_attr;
    ai_ps_attr.ai_hdl = ai_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_GET_PITCH_SHIFT_ATTR,
        &ai_ps_attr, sizeof(ai_ps_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    memcpy(param, &ai_ps_attr.param, sizeof(k_ai_chn_pitch_shift_param));
    return ret;
}

k_s32 kd_mapi_ai_bind_ao(k_handle ai_hdl,k_handle ao_hdl)
{
    k_msg_ai_bind_ao_t ai_ao_info;
    ai_ao_info.ai_hdl = ai_hdl;
    ai_ao_info.ao_hdl = ao_hdl;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_BIND_AO,
        &ai_ao_info, sizeof(ai_ao_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_unbind_ao(k_handle ai_hdl,k_handle ao_hdl)
{
    k_msg_ai_bind_ao_t ai_ao_info;
    ai_ao_info.ai_hdl = ai_hdl;
    ai_ao_info.ao_hdl = ao_hdl;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_UNBIND_AO,
        &ai_ao_info, sizeof(ai_ao_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_set_volume(k_handle ai_hdl,float volume)
{
    k_msg_ai_gain_info gain_info;
    gain_info.gain = volume;
    gain_info.ai_hdl = ai_hdl;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_SETVOLUME,
        &gain_info, sizeof(gain_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_ai_set_vqe_attr(k_handle ai_hdl,const k_ai_vqe_enable vqe_enable)
{
    k_s32 ret;
    k_msg_ai_vqe_attr ai_vqe_attr;
    ai_vqe_attr.ai_hdl = ai_hdl;
    ai_vqe_attr.vqe_enable = vqe_enable;
    g_vqe_enable[ai_hdl] = vqe_enable; // save vqe enable status

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_SET_VEQ_ATTR,
        &ai_vqe_attr, sizeof(ai_vqe_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_ai_get_vqe_attr(k_handle ai_hdl,  k_ai_vqe_enable *vqe_enable)
{
    k_s32 ret;
    k_msg_ai_vqe_attr ai_vqe_attr;
    ai_vqe_attr.ai_hdl = ai_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_AI_GET_VEQ_ATTR,
        &ai_vqe_attr, sizeof(ai_vqe_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }

    memcpy(vqe_enable, &ai_vqe_attr.vqe_enable, sizeof(k_ai_vqe_enable));
    return ret;
}

k_s32 kd_mapi_ai_send_far_echo_frame(k_handle ai_hdl, const k_audio_frame *frame, k_s32 milli_sec)
{
    CHECK_MAPI_AI_NULL_PTR("frame", frame);

    g_ai_aec_chn_frame[ai_hdl].ai_hdl = ai_hdl;
    g_ai_aec_chn_frame[ai_hdl].audio_frame = *frame;
    g_ai_aec_chn_frame[ai_hdl].milli_sec = milli_sec;

    _ai_aec_datafifo_write_data(g_ai_aec_chn_datafifo[ai_hdl].data_hdl,&g_ai_aec_chn_frame,K_AI_AEC_DATAFIFO_ITEM_SIZE);
    kd_datafifo_write(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, NULL);

    k_s32 ret;
    k_u32 try_count = 50;
    struct timespec ts;
    while(1)
    {
        clock_gettime(CLOCK_REALTIME,&ts);
        ts.tv_nsec += 1*1000*1000;//1ms
        ts.tv_sec += ts.tv_nsec/(1000 * 1000 *1000);
		ts.tv_nsec %= (1000 * 1000 *1000);
        ret = sem_timedwait(&g_ai_aec_chn_send_frame_sem[ai_hdl],&ts);
        if (0 == ret)
        {
            break;
        }
        kd_datafifo_write(g_ai_aec_chn_datafifo[ai_hdl].data_hdl, NULL);
        #if 1
        if (try_count -- <= 0)
        {
            mapi_ai_error_trace("sem_timedwait failed\n");
            return K_FAILED;
        }
        #endif
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_acodec_reset()
{
    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AI, 0, 0), MSG_CMD_MEDIA_ACODEC_RESET,
        NULL, 0, NULL);

    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}
