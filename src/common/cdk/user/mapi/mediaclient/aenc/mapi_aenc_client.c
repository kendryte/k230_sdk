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
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include "mapi_aenc_api.h"
#include "msg_client_dispatch.h"
#include "mapi_aenc_comm.h"
#include "mpi_aenc_api.h"
#include "msg_aenc.h"
#include "k_type.h"
#include "k_datafifo.h"

typedef struct
{
    pthread_t get_stream_tid;
    k_bool start;
    k_handle aenc_hdl;

} aenc_client_chn_ctl;

static k_aenc_datafifo g_aenc_chn_datafifo[AENC_MAX_CHN_NUMS];
static aenc_client_chn_ctl g_aenc_chn_ctl[AENC_MAX_CHN_NUMS];
static k_aenc_callback_s g_aenc_data_cb[AENC_MAX_CHN_NUMS];
static k_s32 g_mmap_fd_tmp = 0;

#define CHECK_MAPI_AENC_NULL_PTR(paraname, ptr)                      \
    do                                                               \
    {                                                                \
        if ((ptr) == NULL)                                           \
        {                                                            \
            mapi_aenc_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_AENC_NULL_PTR;                         \
        }                                                            \
    } while (0)

#define CHECK_MAPI_AENC_HANDLE_PTR(handle)                                   \
    do                                                                       \
    {                                                                        \
        if (handle < 0 || handle >= AENC_MAX_CHN_NUMS)                       \
        {                                                                    \
            mapi_aenc_error_trace("aenc handle(%d) is not valid\n", handle); \
            return K_MAPI_ERR_AENC_INVALID_HANDLE;                           \
        }                                                                    \
    } while (0)

static k_s32 _reset_aenc_ctl(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    g_aenc_chn_ctl[aenc_hdl].start = K_FALSE;
    g_aenc_chn_ctl[aenc_hdl].get_stream_tid = 0;
    return K_SUCCESS;
}

// client -> server,must client send mapi to server, server open datafifo and return phyaddr and client open datafifo by phyaddr
static k_s32 _aenc_init_datafifo(k_handle aenc_hdl, k_u64 *datafifo_phyaddr)
{
    k_s32 ret;
    k_msg_aenc_datafifo_t aenc_datafifo;
    aenc_datafifo.aenc_hdl = aenc_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_INIT_DATAFIFO,
                         &aenc_datafifo, sizeof(aenc_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    *datafifo_phyaddr = aenc_datafifo.phyAddr;

    return ret;
}

static k_s32 _aenc_deinit_datafifo(k_handle aenc_hdl)
{
    k_s32 ret;
    k_msg_aenc_datafifo_t aenc_datafifo;
    aenc_datafifo.aenc_hdl = aenc_hdl;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_DEINIT_DATAFIFO,
                         &aenc_datafifo, sizeof(aenc_datafifo), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

static k_s32 _aenc_datafifo_init_slave(k_aenc_datafifo *info, k_u64 phy_addr)
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

static k_s32 _init_datafifo(k_handle aenc_hdl, const k_aenc_chn_attr *attr)
{
    k_s32 ret;
    k_u64 datafifo_phyaddr;
    ret = _aenc_init_datafifo(aenc_hdl, &datafifo_phyaddr);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("_aenc_init_datafifo failed\n");
    }
    else
    {
        g_aenc_chn_datafifo[aenc_hdl].item_count = K_AENC_DATAFIFO_ITEM_COUNT;
        g_aenc_chn_datafifo[aenc_hdl].item_size = K_AENC_DATAFIFO_ITEM_SIZE;
        g_aenc_chn_datafifo[aenc_hdl].open_mode = DATAFIFO_READER;
        g_aenc_chn_datafifo[aenc_hdl].release_func = NULL;

        ret = _aenc_datafifo_init_slave(&g_aenc_chn_datafifo[aenc_hdl], datafifo_phyaddr);
        if (ret != K_SUCCESS)
        {
            mapi_aenc_error_trace("_aenc_datafifo_init_slave failed\n");
        }
        else
        {
            mapi_aenc_error_trace("_aenc_datafifo_init_slave ok,datafifo_phyaddr:0x%x,data_hdl:0x%x\n",
                                  datafifo_phyaddr, g_aenc_chn_datafifo[aenc_hdl].data_hdl);
        }
    }

    return ret;
}

k_s32 kd_mapi_aenc_init(k_handle aenc_hdl, const k_aenc_chn_attr *attr)
{
    k_s32 ret;
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    CHECK_MAPI_AENC_NULL_PTR("aenc_attr", attr);

    k_msg_aenc_pipe_attr_t aenc_attr;
    aenc_attr.aenc_hdl = aenc_hdl;
    aenc_attr.attr = *attr;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_INIT,
                         &aenc_attr, sizeof(aenc_attr), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    _reset_aenc_ctl(aenc_hdl);
    g_aenc_data_cb[aenc_hdl].pfn_data_cb = NULL;
    // init datafifo
    ret = _init_datafifo(aenc_hdl, attr);

    return ret;
}

k_s32 kd_mapi_aenc_deinit(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);

    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_DEINIT,
                         &aenc_hdl, sizeof(aenc_hdl), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    g_aenc_data_cb[aenc_hdl].pfn_data_cb = NULL;
    _reset_aenc_ctl(aenc_hdl);

    ret = _aenc_deinit_datafifo(aenc_hdl);
    return ret;
}

static void *_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);


    if (g_mmap_fd_tmp == 0)
    {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);

    if (mmap_addr != (void *)-1)
        virt_addr = (void*)((char*)mmap_addr + (phys_addr & page_mask));
    else
    {
        mapi_aenc_error_trace("mmap addr error: %d %s.\n", mmap_addr, strerror(errno));;
    }

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phy_addr, void *virt_addr,k_u32 size)
{
    if (g_mmap_fd_tmp == 0)
    {
        return -1;
    }
    k_u32 ret;

    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phy_addr & page_mask) + page_mask) & ~(page_mask);
    ret = munmap((void *)((k_u64)(virt_addr) & ~page_mask), mmap_size);
    if (ret == -1) {
        mapi_aenc_error_trace("munmap error.\n");
    }

    return 0;
}
static void _do_datafifo_stream(k_datafifo_handle data_hdl, void *ppData, k_u32 data_len)
{
    if (data_len != K_AENC_DATAFIFO_ITEM_SIZE)
    {
        mapi_aenc_error_trace("datafifo_read_func len error %d(%d)\n", data_len, sizeof(k_msg_aenc_stream_t));
        return;
    }

    k_msg_aenc_stream_t *msg_aenc_stream = (k_msg_aenc_stream_t *)ppData;
    k_handle aenc_hdl = msg_aenc_stream->aenc_hdl;

    if (aenc_hdl < 0 || aenc_hdl >= AENC_MAX_CHN_NUMS)
    {
        mapi_aenc_error_trace("aenc_hdl error %d\n", aenc_hdl);
        return;
    }

    k_aenc_callback_s aenc_data_cb = g_aenc_data_cb[msg_aenc_stream->aenc_hdl];

    if (aenc_data_cb.pfn_data_cb != NULL)
    {
        msg_aenc_stream->stream.stream = _sys_mmap(msg_aenc_stream->stream.phys_addr, msg_aenc_stream->stream.len);
        aenc_data_cb.pfn_data_cb(aenc_hdl, &msg_aenc_stream->stream, aenc_data_cb.p_private_data);
        _sys_munmap(msg_aenc_stream->stream.phys_addr,msg_aenc_stream->stream.stream, msg_aenc_stream->stream.len);
    }
}

static void *aenc_chn_get_stream_threads(void *arg)
{
    k_handle aenc_hdl = *((k_handle *)arg);

    k_s32 s32Ret = K_FAILED;
    k_u32 readLen = 0;
    void *pdata = NULL;

    while (g_aenc_chn_ctl[aenc_hdl].start)
    {
        s32Ret = kd_datafifo_cmd(g_aenc_chn_datafifo[aenc_hdl].data_hdl, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s get available read len error:%x\n", __FUNCTION__, s32Ret);
            break;
        }

        while (readLen >= K_AENC_DATAFIFO_ITEM_SIZE)
        {
            s32Ret = kd_datafifo_read(g_aenc_chn_datafifo[aenc_hdl].data_hdl, &pdata);
            //printf("========kd_datafifo_read end:%d,ret:%d\n", readLen, s32Ret);
            if (K_SUCCESS != s32Ret)
            {
                printf("%s read error:%x\n", __FUNCTION__, s32Ret);
                break;
            }

            _do_datafifo_stream(g_aenc_chn_datafifo[aenc_hdl].data_hdl, pdata, K_AENC_DATAFIFO_ITEM_SIZE);

            s32Ret = kd_datafifo_cmd(g_aenc_chn_datafifo[aenc_hdl].data_hdl, DATAFIFO_CMD_READ_DONE, pdata);
            if (K_SUCCESS != s32Ret)
            {
                printf("%s read done error:%x\n", __FUNCTION__, s32Ret);
                break;
            }
            readLen -= K_AENC_DATAFIFO_ITEM_SIZE;
        }
        // printf("%s get available read len error:%x(%d)\n", __FUNCTION__, s32Ret, readLen);
        usleep(10000);
    }
    return NULL;
}

k_s32 kd_mapi_aenc_start(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);

    _clean_all_datafifo_data();
    g_aenc_chn_ctl[aenc_hdl].aenc_hdl = aenc_hdl;
    g_aenc_chn_ctl[aenc_hdl].start = K_TRUE;
    pthread_create(&g_aenc_chn_ctl[aenc_hdl].get_stream_tid, NULL, aenc_chn_get_stream_threads, &g_aenc_chn_ctl[aenc_hdl].aenc_hdl);

    k_s32 ret;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_START,
                         &aenc_hdl, sizeof(aenc_hdl), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 _clean_all_datafifo_data(k_handle aenc_hdl)
{
    k_s32 s32Ret = K_FAILED;
    k_u32 readLen = 0;
    void *pdata = NULL;
    k_s32 ret;
    while (K_TRUE)
    {
        s32Ret = kd_datafifo_cmd(g_aenc_chn_datafifo[aenc_hdl].data_hdl, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s get available read len error:%x\n", __FUNCTION__, s32Ret);
            break;
        }
        if (readLen <= 0) {
            break;
        }

        while (readLen >= K_AENC_DATAFIFO_ITEM_SIZE)
        {
            s32Ret = kd_datafifo_read(g_aenc_chn_datafifo[aenc_hdl].data_hdl, &pdata);
            if (K_SUCCESS != s32Ret)
            {
                printf("%s read error:%x\n", __FUNCTION__, s32Ret);
                break;
            }

            s32Ret = kd_datafifo_cmd(g_aenc_chn_datafifo[aenc_hdl].data_hdl, DATAFIFO_CMD_READ_DONE, pdata);
            if (K_SUCCESS != s32Ret)
            {
                printf("%s read done error:%x\n", __FUNCTION__, s32Ret);
                break;
            }
            readLen -= K_AENC_DATAFIFO_ITEM_SIZE;
        }
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_aenc_stop(k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_STOP,
                         &aenc_hdl, sizeof(aenc_hdl), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    if (g_aenc_chn_ctl[aenc_hdl].get_stream_tid != 0)
    {
        g_aenc_chn_ctl[aenc_hdl].start = K_FALSE;
        pthread_join(g_aenc_chn_ctl[aenc_hdl].get_stream_tid, NULL);
        g_aenc_chn_ctl[aenc_hdl].get_stream_tid = 0;
    }

    _clean_all_datafifo_data(aenc_hdl);

    return ret;
}

k_s32 kd_mapi_aenc_registercallback(k_handle aenc_hdl, k_aenc_callback_s *aenc_cb)
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

k_s32 kd_mapi_aenc_bind_ai(k_handle ai_hdl, k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    k_msg_aenc_bind_ai_t aenc_ai_info;
    aenc_ai_info.ai_hdl = ai_hdl;
    aenc_ai_info.aenc_hdl = aenc_hdl;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_BIND_AI,
                         &aenc_ai_info, sizeof(aenc_ai_info), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_aenc_unbind_ai(k_handle ai_hdl, k_handle aenc_hdl)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    k_msg_aenc_bind_ai_t aenc_ai_info;
    aenc_ai_info.ai_hdl = ai_hdl;
    aenc_ai_info.aenc_hdl = aenc_hdl;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_UNBIND_AI,
                         &aenc_ai_info, sizeof(aenc_ai_info), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_register_ext_audio_encoder(const k_aenc_encoder *encoder, k_handle *encoder_hdl)
{
    CHECK_MAPI_AENC_NULL_PTR("encoder", encoder);

    k_msg_ext_audio_encoder_t encoder_info;
    encoder_info.encoder = *encoder;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_REGISTER_EXT_AUDIO_ENCODER,
                         &encoder_info, sizeof(encoder_info), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    *encoder_hdl = encoder_info.encoder_hdl;
    return ret;
}

k_s32 kd_mapi_unregister_ext_audio_encoder(k_handle encoder_hdl)
{
    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_UNREGISTER_EXT_AUDIO_ENCODER,
                         &encoder_hdl, sizeof(encoder_hdl), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_aenc_send_frame(k_handle aenc_hdl, const k_audio_frame *frame)
{
    CHECK_MAPI_AENC_HANDLE_PTR(aenc_hdl);
    CHECK_MAPI_AENC_NULL_PTR("frame", frame);

    k_msg_aenc_frame_t audio_frame_msg;
    audio_frame_msg.aenc_hdl = aenc_hdl;
    audio_frame_msg.frame = *frame;

    k_s32 ret;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_AENC, 0, 0), MSG_CMD_MEDIA_AENC_SEND_FRAME,
                         &audio_frame_msg, sizeof(audio_frame_msg), NULL);

    if (ret != K_SUCCESS)
    {
        mapi_aenc_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}
