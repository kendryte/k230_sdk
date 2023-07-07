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
#include "msg_adec.h"
#include "mapi_adec_api.h"
#include "mpi_adec_api.h"
#include "mapi_adec_comm.h"
#include "mapi_sys_api.h"
#include "mpi_sys_api.h"
#include "k_datafifo.h"

typedef struct
{
    pthread_t get_stream_tid;
    k_handle adec_hdl;
    k_bool    start;

} adec_server_chn_ctl;

static adec_server_chn_ctl g_adec_chn_ctl[ADEC_MAX_CHN_NUMS];
static k_adec_callback_s g_adec_data_cb[ADEC_MAX_CHN_NUMS];
static k_adec_datafifo g_adec_chn_datafifo[ADEC_MAX_CHN_NUMS];
static k_msg_adec_stream_t g_msg_adec_stream[ADEC_MAX_CHN_NUMS];

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
    g_adec_chn_ctl[adec_hdl].get_stream_tid = 0;
    return K_SUCCESS;
}


k_s32 kd_mapi_adec_init(k_handle adec_hdl,const k_adec_chn_attr *attr)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    CHECK_MAPI_ADEC_NULL_PTR("dev_attr", attr);
    k_s32 ret;
    ret = kd_mpi_adec_create_chn(adec_hdl, attr);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("kd_mpi_adec_create_chn failed:0x%x\n", ret);
        return K_FAILED;
    }

    mapi_adec_error_trace("kd_mpi_adec_create_chn ok,adec_hdl:%d\n",adec_hdl);

    g_adec_chn_datafifo[adec_hdl].item_count = K_ADEC_DATAFIFO_ITEM_COUNT;
    g_adec_chn_datafifo[adec_hdl].item_size = K_ADEC_DATAFIFO_ITEM_SIZE;
    g_adec_chn_datafifo[adec_hdl].open_mode = DATAFIFO_READER;
    g_adec_chn_datafifo[adec_hdl].release_func = NULL;
    g_adec_data_cb[adec_hdl].pfn_data_cb = NULL;

    return ret;
}

static k_s32 _adec_datafifo_init_master(k_adec_datafifo* info,k_u64* phy_addr)
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
        mapi_adec_error_trace("%s open datafifo error:%x\n",__FUNCTION__,s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_GET_PHY_ADDR, phy_addr);
    if (K_SUCCESS != s32Ret)
    {
        mapi_adec_error_trace("%s get datafifo phy addr error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    if (info->release_func != NULL)
    {
        s32Ret = kd_datafifo_cmd(info->data_hdl, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, info->release_func);
        if (K_SUCCESS != s32Ret)
        {
            mapi_adec_error_trace("%s set release func callback error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }
    }

    //printf("@@@@@@@%s phy_addr:0x%x,datafifo handle:0x%x\n",__FUNCTION__,*phy_addr,info->data_hdl);


    return K_SUCCESS;
}

k_s32  adec_datafifo_init(k_handle adec_hdl,k_u64* phy_addr)
{
    k_s32 ret;
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    ret = _adec_datafifo_init_master(&g_adec_chn_datafifo[adec_hdl],phy_addr);

    mapi_adec_error_trace("_adec_datafifo_init_master phy_addr:0x%x,ret:%d,datafifo handle:0x%x\n",*phy_addr,ret,g_adec_chn_datafifo[adec_hdl].data_hdl);

    return ret;
}

static k_s32 _adec_datafifo_deinit(k_datafifo_handle data_hdl, K_DATAFIFO_OPEN_MODE_E open_mode)
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

k_s32  adec_datafifo_deinit(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    return _adec_datafifo_deinit(g_adec_chn_datafifo[adec_hdl].data_hdl,DATAFIFO_READER);
}

k_s32 kd_mapi_adec_deinit(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    k_s32 ret;
    ret = kd_mpi_adec_destroy_chn(adec_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("kd_mpi_adec_destroy_chn failed:0x%x\n", ret);
        return K_FAILED;
    }
    g_adec_data_cb[adec_hdl].pfn_data_cb = NULL;

    return ret;
}

static k_s32 _do_datafifo_stream(k_datafifo_handle data_hdl, void *ppData, k_u32 data_len)
{
    k_s32 ret;
    //mapi_adec_error_trace("==========_do_datafifo_stream\n");
    if (data_len != K_ADEC_DATAFIFO_ITEM_SIZE)
    {
        mapi_adec_error_trace("datafifo_read_func len error %d(%d)\n", data_len, sizeof(k_msg_adec_stream_t));
        return K_FAILED;
    }

    k_msg_adec_stream_t *msg_adec_stream = (k_msg_adec_stream_t *)ppData;
    k_handle adec_hdl = msg_adec_stream->adec_hdl;

    if (adec_hdl < 0 || adec_hdl >= ADEC_MAX_CHN_NUMS)
    {
        mapi_adec_error_trace("adec_hdl error %d\n", adec_hdl);
        return K_FAILED;
    }

    g_msg_adec_stream[adec_hdl] = *msg_adec_stream;

   // mapi_adec_error_trace("==========_do_datafifo_stream2,adec_hdl:%d\n",adec_hdl);
   // mapi_adec_error_trace("======stream info:viraddr:0x%x,phys_addr:0x%lx,len:%d,seq:%d\n",\
   // msg_adec_stream->stream.stream,msg_adec_stream->stream.phys_addr,msg_adec_stream->stream.len,msg_adec_stream->stream.seq);

    ret = kd_mpi_adec_send_stream(adec_hdl,&g_msg_adec_stream[adec_hdl].stream,K_TRUE);

   // mapi_adec_error_trace("==========_do_datafifo_stream3:%d\n",ret);

    return ret;
}

static void *adec_chn_get_stream_threads(void *arg)
{
    k_handle adec_hdl = *((k_handle *)arg);

    k_s32 s32Ret = K_FAILED;
    k_u32 readLen = 0;
    void *pdata = NULL;

    while (g_adec_chn_ctl[adec_hdl].start)
    {
        s32Ret = kd_datafifo_cmd(g_adec_chn_datafifo[adec_hdl].data_hdl, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            mapi_adec_error_trace("%s get available read len error:%x\n", __FUNCTION__, s32Ret);
            break;
        }
        //mapi_adec_error_trace("kd_datafifo_cmd readlen:%d\n",readLen);

        if (readLen > 0)
        {
            s32Ret = kd_datafifo_read(g_adec_chn_datafifo[adec_hdl].data_hdl, &pdata);
            //mapi_adec_error_trace("========kd_datafifo_read end:%d,ret:%d\n", readLen, s32Ret);
            if (K_SUCCESS != s32Ret)
            {
                mapi_adec_error_trace("%s read error:%x\n", __FUNCTION__, s32Ret);
                break;
            }

            _do_datafifo_stream(g_adec_chn_datafifo[adec_hdl].data_hdl, pdata, readLen);

            s32Ret = kd_datafifo_cmd(g_adec_chn_datafifo[adec_hdl].data_hdl, DATAFIFO_CMD_READ_DONE, pdata);
            if (K_SUCCESS != s32Ret)
            {
                mapi_adec_error_trace("%s read done error:%x\n", __FUNCTION__, s32Ret);
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

k_s32 kd_mapi_adec_start(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    if (!g_adec_chn_ctl[adec_hdl].start)
    {
        g_adec_chn_ctl[adec_hdl].start = K_TRUE;
    }
    else
    {
        mapi_adec_error_trace("adec handle:%d already start\n", adec_hdl);
    }

    g_adec_chn_ctl[adec_hdl].adec_hdl = adec_hdl;
    g_adec_chn_ctl[adec_hdl].start = K_TRUE;
    pthread_create(&g_adec_chn_ctl[adec_hdl].get_stream_tid, NULL, adec_chn_get_stream_threads, &g_adec_chn_ctl[adec_hdl].adec_hdl);

    return K_SUCCESS;
}

k_s32 kd_mapi_adec_stop(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    if (g_adec_chn_ctl[adec_hdl].start)
    {
        g_adec_chn_ctl[adec_hdl].start = K_FALSE;
    }
    else
    {
        mapi_adec_error_trace("adec handle:%d already stop\n", adec_hdl);
    }

    if (g_adec_chn_ctl[adec_hdl].get_stream_tid != 0)
    {
        g_adec_chn_ctl[adec_hdl].start = K_FALSE;
        pthread_join(g_adec_chn_ctl[adec_hdl].get_stream_tid, NULL);
        g_adec_chn_ctl[adec_hdl].get_stream_tid = 0;
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_adec_registercallback(k_handle adec_hdl,k_adec_callback_s *adec_cb)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    g_adec_data_cb[adec_hdl].pfn_data_cb = adec_cb->pfn_data_cb;
    g_adec_data_cb[adec_hdl].p_private_data = adec_cb->p_private_data;

    return K_SUCCESS;
}

k_s32 kd_mapi_adec_unregistercallback(k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    g_adec_data_cb[adec_hdl].pfn_data_cb = NULL;

    return K_SUCCESS;
}

k_s32 kd_mapi_adec_bind_ao(k_handle ao_hdl,k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;

    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_hdl;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ((ao_hdl) >> 16) & 0xFFFF;
    ao_mpp_chn.chn_id = (ao_hdl) & 0xFFFF;

    return kd_mpi_sys_bind(&adec_mpp_chn, &ao_mpp_chn);
}

k_s32 kd_mapi_adec_unbind_ao(k_handle ao_hdl,k_handle adec_hdl)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;

    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_hdl;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ((ao_hdl) >> 16) & 0xFFFF;
    ao_mpp_chn.chn_id = (ao_hdl) & 0xFFFF;

    return kd_mpi_sys_unbind(&adec_mpp_chn, &ao_mpp_chn);
}

k_s32 kd_mapi_register_ext_audio_decoder(const k_adec_decoder *decoder,k_handle* decoder_hdl)
{
    CHECK_MAPI_ADEC_NULL_PTR("decoder", decoder);

    k_s32 ret;
    k_s32 dec_handle;
    ret = kd_mpi_adec_register_decoder(&dec_handle,decoder);
    if (ret != K_SUCCESS)
    {
        mapi_adec_error_trace("kd_mpi_adec_register_decoder failed:0x%x\n", ret);
        return K_FAILED;
    }

    *decoder_hdl = dec_handle;
    return K_SUCCESS;
}

k_s32 kd_mapi_unregister_ext_audio_decoder( k_handle decoder_hdl)
{
    k_s32 ret;
    ret = kd_mpi_adec_unregister_decoder(decoder_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("kd_mpi_adec_unregister_decoder failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

k_s32 kd_mapi_adec_send_stream(k_handle adec_hdl,const k_audio_stream *stream)
{
    CHECK_MAPI_ADEC_HANDLE_PTR(adec_hdl);
    CHECK_MAPI_ADEC_NULL_PTR("stream", stream);

    k_s32 ret;
    ret = kd_mpi_adec_send_stream(adec_hdl,stream,K_TRUE);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("kd_mpi_adec_send_stream failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}
