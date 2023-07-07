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
#include "msg_aenc.h"
#include "mapi_aenc_api.h"
#include "mpi_aenc_api.h"
#include "mapi_aenc_comm.h"
#include "mapi_sys_api.h"
#include "mpi_sys_api.h"
#include "k_datafifo.h"

typedef struct
{
    pthread_t send_stream_tid;
    k_handle aenc_hdl;
    k_bool    start;

} aenc_server_chn_ctl;

static k_aenc_callback_s g_aenc_data_cb[AENC_MAX_CHN_NUMS];
static aenc_server_chn_ctl g_aenc_chn_ctl[AENC_MAX_CHN_NUMS];
static k_aenc_datafifo g_aenc_chn_datafifo[AENC_MAX_CHN_NUMS];
static k_msg_aenc_stream_t g_msg_aenc_stream[AENC_MAX_CHN_NUMS];

#define CHECK_MAPI_AENC_NULL_PTR(paraname, ptr)                                 \
    do {                                                                        \
        if ((ptr) == NULL) {                                                    \
            mapi_aenc_error_trace("%s is NULL pointer\n", paraname);            \
            return K_MAPI_ERR_AENC_NULL_PTR;                                    \
        }                                                                       \
    } while (0)

#define CHECK_MAPI_AENC_HANDLE_PTR(handle)                                      \
    do {                                                                        \
        if (handle < 0 || handle >= AENC_MAX_CHN_NUMS)                          \
        {                                                                       \
            mapi_aenc_error_trace("aenc handle(%d) is not valid\n", handle);    \
            return K_MAPI_ERR_AENC_INVALID_HANDLE;                              \
        }                                                                       \
    } while (0)

static k_s32 _aenc_datafifo_init_master(k_aenc_datafifo* info,k_u64* phy_addr)
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
        printf("%s open datafifo error:%x\n",__FUNCTION__,s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_GET_PHY_ADDR, phy_addr);
    if (K_SUCCESS != s32Ret)
    {
        printf("%s get datafifo phy addr error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    if (info->release_func != NULL)
    {
        s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, info->release_func);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s set release func callback error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }
    }

    //printf("@@@@@@@%s phy_addr:0x%x,datafifo handle:0x%x\n",__FUNCTION__,*phy_addr,info->data_hdl);


    return K_SUCCESS;
}

k_s32  aenc_datafifo_init(k_handle aenc_hdl,k_u64* phy_addr)
{
    k_s32 ret;
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    ret = _aenc_datafifo_init_master(&g_aenc_chn_datafifo[aenc_hdl],phy_addr);

    mapi_aenc_error_trace("_aenc_datafifo_init_master phy_addr:0x%x,ret:%d,datafifo handle:0x%x\n",*phy_addr,ret,g_aenc_chn_datafifo[aenc_hdl].data_hdl);

    return ret;
}

static k_s32 _audio_datafifo_deinit(k_datafifo_handle data_hdl, K_DATAFIFO_OPEN_MODE_E open_mode)
{
    k_s32 s32Ret = K_SUCCESS;
    if (DATAFIFO_WRITER == open_mode)
    {
        // call write NULL to flush and release stream buffer.
        s32Ret = kd_datafifo_write(data_hdl, NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s write error:%x\n", __FUNCTION__,s32Ret);
        }
    }

    return kd_datafifo_close(data_hdl);
}

k_s32  aenc_datafifo_deinit(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    return _audio_datafifo_deinit(g_aenc_chn_datafifo[aenc_hdl].data_hdl,DATAFIFO_WRITER);
}

static k_s32 _reset_aenc_ctl(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    g_aenc_chn_ctl[aenc_hdl].start = K_FALSE;
    g_aenc_chn_ctl[aenc_hdl].send_stream_tid = 0;
    return K_SUCCESS;
}

static void _datafifo_release_func(void* pStream)
{
    k_msg_aenc_stream_t* msg_aenc_stream = (k_msg_aenc_stream_t*)pStream;
    kd_mpi_aenc_release_stream(msg_aenc_stream->aenc_hdl, &msg_aenc_stream->stream);

    //mapi_aenc_error_trace("kd_mpi_aenc_release_stream physaddr:0x%x\n", msg_aenc_stream->stream.phys_addr);
}

k_s32 kd_mapi_aenc_init(k_handle aenc_hdl,const k_aenc_chn_attr *attr)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    CHECK_MAPI_AENC_NULL_PTR("dev_attr", attr);
    k_s32 ret;
    ret = kd_mpi_aenc_create_chn(aenc_hdl, attr);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("kd_mpi_aenc_create_chn failed:0x%x\n", ret);
        return K_FAILED;
    }

    g_aenc_chn_datafifo[aenc_hdl].item_count = K_AENC_DATAFIFO_ITEM_COUNT;
    g_aenc_chn_datafifo[aenc_hdl].item_size = K_AENC_DATAFIFO_ITEM_SIZE;
    g_aenc_chn_datafifo[aenc_hdl].open_mode = DATAFIFO_WRITER;
    g_aenc_chn_datafifo[aenc_hdl].release_func = _datafifo_release_func;

    g_aenc_data_cb[aenc_hdl].pfn_data_cb = NULL;

    _reset_aenc_ctl(aenc_hdl);

    return ret;
}

k_s32 kd_mapi_aenc_deinit(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    k_s32 ret;
    ret = kd_mpi_aenc_destroy_chn(aenc_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("kd_mpi_aenc_destroy_chn failed:0x%x\n", ret);
        return K_FAILED;
    }

    g_aenc_data_cb[aenc_hdl].pfn_data_cb = NULL;
    _reset_aenc_ctl(aenc_hdl);

    return ret;
}

static k_s32 _audio_datafifo_write_data(k_datafifo_handle data_hdl, void *data, k_u32 data_len)
{
    k_s32 s32Ret = K_FAILED;
    k_u32 availWriteLen = 0;
    // call write NULL to flush
    s32Ret = kd_datafifo_write(data_hdl, NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("%s write error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
    if (K_SUCCESS != s32Ret)
    {
        printf("%s get available write len error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    if (availWriteLen >= data_len)
    {
        s32Ret = kd_datafifo_write(data_hdl, data);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s write error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }

        s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_WRITE_DONE, NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s write done error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }
    }
    else
    {
        printf("%s no availWriteLen %d_%d\n",__FUNCTION__, availWriteLen, data_len);
        return K_FAILED;
    }

    return K_SUCCESS;
}

static void *aenc_chn_send_stream_threads(void *arg)
{
    k_handle aenc_hdl = *((k_handle *)arg);
    g_msg_aenc_stream[aenc_hdl].aenc_hdl = aenc_hdl;

    while(g_aenc_chn_ctl[aenc_hdl].start)
    {
        if (0 != kd_mpi_aenc_get_stream(aenc_hdl, &g_msg_aenc_stream[aenc_hdl].stream, 500))
        {
            //printf("kd_mpi_aenc_get_stream failed\n");
            continue;
        }
        else
        {
            //printf("========_audio_datafifo_write_data aenc_hdl:%d,datafifo hdl:0x%x,datalen:%d\n",aenc_hdl,g_aenc_chn_datafifo[aenc_hdl].data_hdl,sizeof(g_msg_aenc_stream[aenc_hdl]));
            if (0 != _audio_datafifo_write_data(g_aenc_chn_datafifo[aenc_hdl].data_hdl,&g_msg_aenc_stream[aenc_hdl],K_AENC_DATAFIFO_ITEM_SIZE))
            {
                printf("_audio_datafifo_write_data failed\n");
            }
        }
    }

    return NULL;
}

k_s32 kd_mapi_aenc_start(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    if (!g_aenc_chn_ctl[aenc_hdl].start)
    {
        g_aenc_chn_ctl[aenc_hdl].start = K_TRUE;
        g_aenc_chn_ctl[aenc_hdl].aenc_hdl = aenc_hdl;
    }
    else
    {
        mapi_aenc_error_trace("aenc handle:%d already start\n", aenc_hdl);
        return K_FAILED;
    }

    if (g_aenc_chn_ctl[aenc_hdl].send_stream_tid != 0)
    {
        g_aenc_chn_ctl[aenc_hdl].start = K_FALSE;
        pthread_join(g_aenc_chn_ctl[aenc_hdl].send_stream_tid,NULL);
        g_aenc_chn_ctl[aenc_hdl].send_stream_tid = 0;
    }
    pthread_create(&g_aenc_chn_ctl[aenc_hdl].send_stream_tid,NULL,aenc_chn_send_stream_threads,&g_aenc_chn_ctl[aenc_hdl].aenc_hdl);

    return K_SUCCESS;
}

k_s32 kd_mapi_aenc_stop(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);

    if (g_aenc_chn_ctl[aenc_hdl].start)
    {
        g_aenc_chn_ctl[aenc_hdl].start = K_FALSE;
    }
    else
    {
        mapi_aenc_error_trace("aenc handle:%d already stop\n", aenc_hdl);
    }

    if (g_aenc_chn_ctl[aenc_hdl].send_stream_tid != 0)
    {
        g_aenc_chn_ctl[aenc_hdl].start = K_FALSE;
        pthread_join(g_aenc_chn_ctl[aenc_hdl].send_stream_tid,NULL);
        g_aenc_chn_ctl[aenc_hdl].send_stream_tid = 0;
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_aenc_registercallback(k_handle aenc_hdl,k_aenc_callback_s *aenc_cb)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    g_aenc_data_cb[aenc_hdl].pfn_data_cb = aenc_cb->pfn_data_cb;
    g_aenc_data_cb[aenc_hdl].p_private_data = aenc_cb->p_private_data;
    return K_SUCCESS;
}

k_s32 kd_mapi_aenc_unregistercallback(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);

    g_aenc_data_cb[aenc_hdl].pfn_data_cb = NULL;
    return K_SUCCESS;
}

k_s32 kd_mapi_aenc_bind_ai(k_handle ai_hdl,k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ((ai_hdl) >> 16) & 0xFFFF;
    ai_mpp_chn.chn_id = (ai_hdl) & 0xFFFF;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_hdl;

    return kd_mpi_sys_bind(&ai_mpp_chn, &aenc_mpp_chn);
}

k_s32 kd_mapi_aenc_unbind_ai(k_handle ai_hdl,k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ((ai_hdl) >> 16) & 0xFFFF;
    ai_mpp_chn.chn_id = (ai_hdl) & 0xFFFF;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_hdl;

    return kd_mpi_sys_unbind(&ai_mpp_chn, &aenc_mpp_chn);
}

k_s32 kd_mapi_register_ext_audio_encoder(const k_aenc_encoder *encoder,k_handle* encoder_hdl)
{
    CHECK_MAPI_AENC_NULL_PTR("encoder", encoder);

    k_s32 ret;
    k_s32 enc_handle;
    ret = kd_mpi_aenc_register_encoder(&enc_handle,encoder);
    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("kd_mpi_aenc_register_encoder failed:0x%x\n", ret);
        return K_FAILED;
    }

    *encoder_hdl = enc_handle;
    return K_SUCCESS;
}

k_s32 kd_mapi_unregister_ext_audio_encoder( k_handle encoder_hdl)
{
    k_s32 ret;
    ret = kd_mpi_aenc_unregister_encoder(encoder_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("kd_mpi_aenc_unregister_encoder failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

k_s32 kd_mapi_aenc_send_frame(k_handle aenc_hdl,const k_audio_frame *frame)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    CHECK_MAPI_AENC_NULL_PTR("frame",frame);

    k_s32 ret;
    ret = kd_mpi_aenc_send_frame(aenc_hdl,frame);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("kd_mpi_aenc_send_frame failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}
