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
#include "mapi_vdec_api.h"
#include "msg_client_dispatch.h"
#include "mapi_vdec_comm.h"
#include "mpi_vdec_api.h"
#include "msg_vdec.h"
#include "k_type.h"
#include "k_datafifo.h"

typedef struct
{
    pthread_t send_stream_tid;
    k_bool start;
    k_u32 vdec_chn;

} vdec_client_chn_ctl;

static k_vdec_datafifo g_vdec_chn_datafifo[VDEC_MAX_CHN_NUMS];
static vdec_client_chn_ctl g_vdec_chn_ctl[VDEC_MAX_CHN_NUMS];
static sem_t g_vdec_chn_send_stream_sem[VDEC_MAX_CHN_NUMS];
static k_msg_vdec_stream_t  g_vdec_chn_stream[VDEC_MAX_CHN_NUMS];
static k_bool g_init = K_FALSE;


#define CHECK_MAPI_VDEC_NULL_PTR(paraname, ptr)                                \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_vdec_error_trace("%s is NULL pointer\n", paraname);           \
            return K_MAPI_ERR_VDEC_NULL_PTR;                                   \
        }                                                                      \
    } while (0)

#define CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn)                                     \
    do {                                                                       \
        if (vdec_chn < 0 || vdec_chn >= VDEC_MAX_CHN_NUMS)                         \
        {                                                                      \
            mapi_vdec_error_trace("vdec chn(%d) is not valid\n", vdec_chn);   \
            return K_MAPI_ERR_VDEC_INVALID_HANDLE;                             \
        }                                                                      \
    } while (0)

static k_s32 _reset_vdec_ctl(k_u32 vdec_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(vdec_chn);
    g_vdec_chn_ctl[vdec_chn].start = K_FALSE;
    g_vdec_chn_ctl[vdec_chn].send_stream_tid = 0;
    return K_SUCCESS;
}

// client -> server,must client send mapi to server, server open datafifo and return phyaddr and client open datafifo by phyaddr
static k_s32 _vdec_init_datafifo(k_u32 vdec_chn, k_u64 *datafifo_phyaddr)
{
    k_s32 ret;
    k_msg_vdec_datafifo_t vdec_datafifo;
    vdec_datafifo.vdec_chn = vdec_chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_INIT_DATAFIFO,
                         &vdec_datafifo, sizeof(vdec_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }

    *datafifo_phyaddr = vdec_datafifo.phyAddr;

    return ret;
}

static k_s32 _vdec_deinit_datafifo(k_u32 vdec_chn)
{
    k_s32 ret;
    k_msg_vdec_datafifo_t vdec_datafifo;
    vdec_datafifo.vdec_chn = vdec_chn;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_DEINIT_DATAFIFO,
                         &vdec_datafifo, sizeof(vdec_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }

    kd_datafifo_close(g_vdec_chn_datafifo[vdec_chn].data_hdl);
    return ret;
}

static k_s32 _vdec_datafifo_init_slave(k_vdec_datafifo *info, k_u64 phy_addr)
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
        mapi_vdec_error_trace("%s open datafifo error:%x\n", __FUNCTION__, s32Ret);
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

    //printf("@@@@@@@%s phy_addr:0x%x,datafifo handle:0x%x\n", __FUNCTION__, phy_addr, info->data_hdl);

    return K_SUCCESS;
}

static void _datafifo_release_func(void* pStream)
{
    k_msg_vdec_stream_t* msg_vdec_stream = (k_msg_vdec_stream_t*)pStream;
    k_u32 vdec_chn = msg_vdec_stream->vdec_chn;

    if (g_init)
    {
        mapi_vdec_error_trace("_datafifo_release_func,vdec channel:%d\n",vdec_chn);
        g_init = K_FALSE;
    }

    sem_post(&g_vdec_chn_send_stream_sem[vdec_chn]);
}

static k_s32 _init_datafifo(k_u32 vdec_chn, const k_vdec_chn_attr *attr)
{
    k_s32 ret;
    k_u64 datafifo_phyaddr;
    ret = _vdec_init_datafifo(vdec_chn, &datafifo_phyaddr);

    if (ret != K_SUCCESS)
    {
        mapi_vdec_error_trace("_vdec_init_datafifo failed\n");
    }
    else
    {
        g_vdec_chn_datafifo[vdec_chn].item_count = K_VDEC_DATAFIFO_ITEM_COUNT;
        g_vdec_chn_datafifo[vdec_chn].item_size = K_VDEC_DATAFIFO_ITEM_SIZE;
        g_vdec_chn_datafifo[vdec_chn].open_mode = DATAFIFO_WRITER;
        g_vdec_chn_datafifo[vdec_chn].release_func = _datafifo_release_func;

        ret = _vdec_datafifo_init_slave(&g_vdec_chn_datafifo[vdec_chn], datafifo_phyaddr);
        if (ret != K_SUCCESS)
        {
            mapi_vdec_error_trace("_vdec_datafifo_init_slave failed\n");
        }
        else
        {
            mapi_vdec_error_trace("_vdec_datafifo_init_slave ok,datafifo_phyaddr:0x%x,data_hdl:0x%x\n",
                                  datafifo_phyaddr, g_vdec_chn_datafifo[vdec_chn].data_hdl);
        }
    }

    return ret;
}

k_s32 kd_mapi_vdec_init(k_u32 chn_num,const k_vdec_chn_attr *attr)
{
    k_s32 ret;
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);
    CHECK_MAPI_VDEC_NULL_PTR("vdec_attr", attr);

    k_msg_vdec_pipe_attr_t vdec_attr;
    vdec_attr.vdec_chn = chn_num;
    vdec_attr.attr = *attr;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_INIT,
        &vdec_attr, sizeof(vdec_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }

    _reset_vdec_ctl(chn_num);
    // init datafifo
    ret = _init_datafifo(chn_num, attr);

    sem_init(&g_vdec_chn_send_stream_sem[chn_num],0,0);

    g_init = K_TRUE;
    return ret;
}

k_s32 kd_mapi_vdec_deinit(k_u32 chn_num)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_DEINIT,
        &chn_num, sizeof(chn_num), NULL);

    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }

    _reset_vdec_ctl(chn_num);
    ret = _vdec_deinit_datafifo(chn_num);

    sem_destroy(&g_vdec_chn_send_stream_sem[chn_num]);
    g_init = K_FALSE;
    return ret;
}

static k_s32 _vdec_datafifo_write_data(k_datafifo_handle data_hdl, void *data, k_u32 data_len)
{
    k_s32 s32Ret = K_FAILED;
    k_u32 availWriteLen = 0;

    // call write NULL to flush
    s32Ret = kd_datafifo_write(data_hdl, NULL);
    if (K_SUCCESS != s32Ret)
    {
        mapi_vdec_error_trace("%s write error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
    if (K_SUCCESS != s32Ret)
    {
        mapi_vdec_error_trace("%s get available write len error:%x\n", __FUNCTION__,s32Ret);
        return K_FAILED;
    }

    if (availWriteLen >= data_len)
    {
        s32Ret = kd_datafifo_write(data_hdl, data);
        if (K_SUCCESS != s32Ret)
        {
            mapi_vdec_error_trace("%s write error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }

        s32Ret = kd_datafifo_cmd(data_hdl, DATAFIFO_CMD_WRITE_DONE, NULL);
        if (K_SUCCESS != s32Ret)
        {
            mapi_vdec_error_trace("%s write done error:%x\n", __FUNCTION__,s32Ret);
            return K_FAILED;
        }
    }
    else
    {
        mapi_vdec_error_trace("%s no availWriteLen %d_%d\n",__FUNCTION__, availWriteLen, data_len);
        return K_FAILED;
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_vdec_start(k_u32 chn_num)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_START,
        &chn_num, sizeof(chn_num), NULL);

    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vdec_stop(k_u32 chn_num)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_STOP,
        &chn_num, sizeof(chn_num), NULL);

    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vdec_bind_vo(k_u32 chn_num,k_u32 vo_dev, k_u32 vo_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);

    k_msg_vdec_bind_vo_t vdec_vo_info;
    vdec_vo_info.vdec_chn = chn_num;
    vdec_vo_info.vo_dev = vo_dev;
    vdec_vo_info.vo_chn = vo_chn;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_BIND_VO,
        &vdec_vo_info, sizeof(vdec_vo_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vdec_unbind_vo(k_u32 chn_num,k_u32 vo_dev, k_u32 vo_chn)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);

    k_msg_vdec_bind_vo_t vdec_vo_info;
    vdec_vo_info.vdec_chn = chn_num;
    vdec_vo_info.vo_dev = vo_dev;
    vdec_vo_info.vo_chn = vo_chn;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_UNBIND_VO,
        &vdec_vo_info, sizeof(vdec_vo_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_vdec_send_stream(k_u32 chn_num, k_vdec_stream *stream, k_s32 milli_sec)
{
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);
    CHECK_MAPI_VDEC_NULL_PTR("stream",stream);

    g_vdec_chn_stream[chn_num].vdec_chn = chn_num;
    g_vdec_chn_stream[chn_num].stream = *stream;

    _vdec_datafifo_write_data(g_vdec_chn_datafifo[chn_num].data_hdl,&g_vdec_chn_stream[chn_num],K_VDEC_DATAFIFO_ITEM_SIZE);
    kd_datafifo_write(g_vdec_chn_datafifo[chn_num].data_hdl, NULL);

    k_s32 ret;
    k_u32 try_count = 50;
    struct timespec ts;
    while(1)
    {
        clock_gettime(CLOCK_REALTIME,&ts);
        ts.tv_nsec += 10*1000*1000;//10ms
        ts.tv_sec += ts.tv_nsec/(1000 * 1000 *1000);
		ts.tv_nsec %= (1000 * 1000 *1000);
        ret = sem_timedwait(&g_vdec_chn_send_stream_sem[chn_num],&ts);
        if (0 == ret)
        {
            break;
        }
        kd_datafifo_write(g_vdec_chn_datafifo[chn_num].data_hdl, NULL);
        #if 0
        if (try_count -- <= 0)
        {
            mapi_vdec_error_trace("sem_timedwait failed\n");
            return K_FAILED;
        }
        #endif
    }


    return K_SUCCESS;
}

k_s32 kd_mapi_vdec_query_status(k_u32 chn_num, k_vdec_chn_status *status)
{
    k_s32 ret;
    CHECK_MAPI_VDEC_CHANNEL_PTR(chn_num);
    CHECK_MAPI_VDEC_NULL_PTR("chn_status",status);

    k_msg_vdec_chn_status_t  chn_status;
    memset(&chn_status,0,sizeof(chn_status));
    chn_status.vdec_chn  = chn_num;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VDEC, 0, 0), MSG_CMD_MEDIA_VDEC_QUERY_STATUS,
        &chn_status, sizeof(chn_status), NULL);

    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("mapi_send_sync failed\n");
    }

    *status = chn_status.status;
    return ret;
}
