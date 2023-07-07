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
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mapi_adec_api.h"
#include "msg_client_dispatch.h"
#include "mapi_adec_comm.h"
#include "mpi_adec_api.h"
#include "msg_adec.h"
#include "k_type.h"
#include "k_datafifo.h"

typedef struct
{
    pthread_t send_stream_tid;
    k_bool start;
    k_handle adec_hdl;

} adec_client_chn_ctl;

static k_adec_datafifo g_adec_chn_datafifo[ADEC_MAX_CHN_NUMS];
static adec_client_chn_ctl g_adec_chn_ctl[ADEC_MAX_CHN_NUMS];
static k_adec_callback_s g_adec_data_cb[ADEC_MAX_CHN_NUMS];
static sem_t g_adec_chn_send_stream_sem[ADEC_MAX_CHN_NUMS];
static k_msg_adec_stream_t  g_adec_chn_stream[ADEC_MAX_CHN_NUMS];
static k_bool g_init = K_FALSE;


#define CHECK_MAPI_ADEC_NULL_PTR(paraname, ptr)                                \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_adec_error_trace("%s is NULL pointer\n", paraname);           \
            return K_MAPI_ERR_ADEC_NULL_PTR;                                   \
        }                                                                      \
    } while (0)

#define CHECK_MAPI_ADEC_HANDLE_PTR(handle)                                     \
    do {                                                                       \
        if (handle < 0 || handle >= ADEC_MAX_CHN_NUMS)                         \
        {                                                                      \
            mapi_adec_error_trace("adec handle(%d) is not valid\n", handle);   \
            return K_MAPI_ERR_ADEC_INVALID_HANDLE;                             \
        }                                                                      \
    } while (0)

static k_s32 _reset_adec_ctl(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    g_adec_chn_ctl[adec_hdl].start = K_FALSE;
    g_adec_chn_ctl[adec_hdl].send_stream_tid = 0;
    return K_SUCCESS;
}

// client -> server,must client send mapi to server, server open datafifo and return phyaddr and client open datafifo by phyaddr
static k_s32 _adec_init_datafifo(k_handle adec_hdl, k_u64 *datafifo_phyaddr)
{
    k_s32 ret;
    k_msg_adec_datafifo_t adec_datafifo;
    adec_datafifo.adec_hdl = adec_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_INIT_DATAFIFO,
                         &adec_datafifo, sizeof(adec_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }

    *datafifo_phyaddr = adec_datafifo.phyAddr;

    return ret;
}

static k_s32 _adec_deinit_datafifo(k_handle adec_hdl)
{
    k_s32 ret;
    k_msg_adec_datafifo_t adec_datafifo;
    adec_datafifo.adec_hdl = adec_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_DEINIT_DATAFIFO,
                         &adec_datafifo, sizeof(adec_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

static k_s32 _adec_datafifo_init_slave(k_adec_datafifo *info, k_u64 phy_addr)
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

static void _datafifo_release_func(void* pStream)
{
    k_msg_adec_stream_t* msg_adec_stream = (k_msg_adec_stream_t*)pStream;
    k_handle adec_hdl = msg_adec_stream->adec_hdl;

    if (g_init)
    {
        mapi_adec_error_trace("_datafifo_release_func,adec_hdl:%d\n",adec_hdl);
        g_init = K_FALSE;
    }

    sem_post(&g_adec_chn_send_stream_sem[adec_hdl]);

}

static k_s32 _init_datafifo(k_handle adec_hdl, const k_adec_chn_attr *attr)
{
    k_s32 ret;
    k_u64 datafifo_phyaddr;
    ret = _adec_init_datafifo(adec_hdl, &datafifo_phyaddr);

    if (ret != K_SUCCESS)
    {
        mapi_adec_error_trace("_adec_init_datafifo failed\n");
    }
    else
    {
        g_adec_chn_datafifo[adec_hdl].item_count = K_ADEC_DATAFIFO_ITEM_COUNT;
        g_adec_chn_datafifo[adec_hdl].item_size = K_ADEC_DATAFIFO_ITEM_SIZE;
        g_adec_chn_datafifo[adec_hdl].open_mode = DATAFIFO_WRITER;
        g_adec_chn_datafifo[adec_hdl].release_func = _datafifo_release_func;

        ret = _adec_datafifo_init_slave(&g_adec_chn_datafifo[adec_hdl], datafifo_phyaddr);
        if (ret != K_SUCCESS)
        {
            mapi_adec_error_trace("_adec_datafifo_init_slave failed\n");
        }
        else
        {
            mapi_adec_error_trace("_adec_datafifo_init_slave ok,datafifo_phyaddr:0x%x,data_hdl:0x%x\n",
                                  datafifo_phyaddr, g_adec_chn_datafifo[adec_hdl].data_hdl);
        }
    }

    return ret;
}

k_s32 kd_mapi_adec_init(k_handle adec_hdl,const k_adec_chn_attr *attr)
{
    k_s32 ret;
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    CHECK_MAPI_ADEC_NULL_PTR("adec_attr", attr);

    k_msg_adec_pipe_attr_t adec_attr;
    adec_attr.adec_hdl = adec_hdl;
    adec_attr.attr = *attr;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_INIT,
        &adec_attr, sizeof(adec_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }

    _reset_adec_ctl(adec_hdl);
    g_adec_data_cb[adec_hdl].pfn_data_cb = NULL;
    // init datafifo
    ret = _init_datafifo(adec_hdl, attr);

    sem_init(&g_adec_chn_send_stream_sem[adec_hdl],0,0);

    g_init = K_TRUE;

    return ret;
}

k_s32 kd_mapi_adec_deinit(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_DEINIT,
        &adec_hdl, sizeof(adec_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }

    g_adec_data_cb[adec_hdl].pfn_data_cb = NULL;
    _reset_adec_ctl(adec_hdl);

    ret = _adec_deinit_datafifo(adec_hdl);

    sem_destroy(&g_adec_chn_send_stream_sem[adec_hdl]);

    g_init = K_FALSE;
    return ret;
}

static k_s32 _adec_datafifo_write_data(k_datafifo_handle data_hdl, void *data, k_u32 data_len)
{
    k_s32 s32Ret = K_FAILED;
    k_u32 availWriteLen = 0;

    // call write NULL to flush
    s32Ret = kd_datafifo_write(data_hdl, NULL);
    if (K_SUCCESS != s32Ret)
    {
        mapi_adec_error_trace("%s write error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
    if (K_SUCCESS != s32Ret)
    {
        mapi_adec_error_trace("%s get available write len error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    if (availWriteLen >= data_len)
    {
        s32Ret = kd_datafifo_write(data_hdl, data);
        if (K_SUCCESS != s32Ret)
        {
            mapi_adec_error_trace("%s write error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }

        s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_WRITE_DONE, NULL);
        if (K_SUCCESS != s32Ret)
        {
            mapi_adec_error_trace("%s write done error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }
    }
    else
    {
        mapi_adec_error_trace("%s no availWriteLen %d_%d\n",__FUNCTION__, availWriteLen, data_len);
        return K_FAILED;
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_adec_start(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_START,
        &adec_hdl, sizeof(adec_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_adec_stop(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_STOP,
        &adec_hdl, sizeof(adec_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_adec_registercallback(k_handle adec_hdl,k_adec_callback_s *adec_cb)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_msg_adec_callback_attr  attr;
    attr.adec_hdl = adec_hdl;
    attr.callback_attr = *adec_cb;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_REGISTER_CALLBACK,
        &attr, sizeof(attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_adec_unregistercallback(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_UNREGISTER_CALLBACK,
        &adec_hdl, sizeof(adec_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_adec_bind_ao(k_handle ao_hdl,k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_msg_adec_bind_ao_t adec_ao_info;
    adec_ao_info.ao_hdl = ao_hdl;
    adec_ao_info.adec_hdl = adec_hdl;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_BIND_AO,
        &adec_ao_info, sizeof(adec_ao_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_adec_unbind_ao(k_handle ao_hdl,k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    k_msg_adec_bind_ao_t adec_ao_info;
    adec_ao_info.ao_hdl = ao_hdl;
    adec_ao_info.adec_hdl = adec_hdl;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_UNBIND_AO,
        &adec_ao_info, sizeof(adec_ao_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_register_ext_audio_decoder(const k_adec_decoder *decoder,k_handle* decoder_hdl)
{
    CHECK_MAPI_ADEC_NULL_PTR("decoder",decoder);

    k_msg_ext_audio_decoder_t  decoder_info;
    decoder_info.decoder = *decoder;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_REGISTER_EXT_AUDIO_DECODER,
        &decoder_info, sizeof(decoder_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }

    *decoder_hdl = decoder_info.decoder_hdl;
    return ret;

}

k_s32 kd_mapi_unregister_ext_audio_decoder( k_handle decoder_hdl)
{
    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_ADEC, 0, 0), MSG_CMD_MEDIA_ADEC_UNREGISTER_EXT_AUDIO_DECODER,
        &decoder_hdl, sizeof(decoder_hdl), NULL);

    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_adec_send_stream(k_handle adec_hdl,const k_audio_stream *stream)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    CHECK_MAPI_ADEC_NULL_PTR("stream",stream);

    g_adec_chn_stream[adec_hdl].adec_hdl = adec_hdl;
    g_adec_chn_stream[adec_hdl].stream = *stream;

    _adec_datafifo_write_data(g_adec_chn_datafifo[adec_hdl].data_hdl,&g_adec_chn_stream[adec_hdl],K_ADEC_DATAFIFO_ITEM_SIZE);
    kd_datafifo_write(g_adec_chn_datafifo[adec_hdl].data_hdl, NULL);

    k_s32 ret;
    k_u32 try_count = 50;
    struct timespec ts;
    while(1)
    {
        clock_gettime(CLOCK_REALTIME,&ts);
        ts.tv_nsec += 10*1000*1000;//10ms
        ts.tv_sec += ts.tv_nsec/(1000 * 1000 *1000);
		ts.tv_nsec %= (1000 * 1000 *1000);
        ret = sem_timedwait(&g_adec_chn_send_stream_sem[adec_hdl],&ts);
        if (0 == ret)
        {
            break;
        }
        kd_datafifo_write(g_adec_chn_datafifo[adec_hdl].data_hdl, NULL);
        if (try_count -- <= 0)
        {
            mapi_adec_error_trace("sem_timedwait failed\n");
            return K_FAILED;
        }
    }

    return K_SUCCESS;

}