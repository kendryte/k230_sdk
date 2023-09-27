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

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "k_ipcmsg.h"
#include "k_comm_ipcmsg.h"
#include "mapi_sys_comm.h"
#include "mapi_sys_api.h"
#include "msg_server_dispatch.h"

#define IPCMSG_MEDIA_SERVER_NAME    "kd_mapi_msg"
#define IPCMSG_PORT_MPP             (1)

static msg_server_context_t g_server_context;
static k_s32 g_media_msg_id = -1;
static k_bool g_msg_start_flag = K_FALSE;
static pthread_t g_server_receive_thread = -1;

cmd_proc_fn msg_server_get_func(k_u32 mod_id, k_u32 cmd_id)
{
    k_u32 i;
    msg_server_module_t *temp_server_module =
            g_server_context.server_modules[mod_id];

    if(temp_server_module == NULL)
        return NULL;

    for(i = 0; i < temp_server_module->modile_cmd_amount; i++) {
        if(cmd_id == (temp_server_module->module_cmd_table + i)->cmd)
            return (temp_server_module->module_cmd_table + i)->cmd_proc_fn_ptr;
    }
    return NULL;
}
static void meida_msg_receive_proc(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_u32 mod_id;
    cmd_proc_fn func = NULL;

    mod_id = GET_MOD_ID(msg->u32Module);

    switch(mod_id) {
    case K_MAPI_MOD_SYS:
    case K_MAPI_MOD_VI:
    case K_MAPI_MOD_VPROC:
    case K_MAPI_MOD_VENC:
    case K_MAPI_MOD_VDEC:
    case K_MAPI_MOD_VREC:
    case K_MAPI_MOD_VO:
    case K_MAPI_MOD_AI:
    case K_MAPI_MOD_AENC:
    case K_MAPI_MOD_ADEC:
    case K_MAPI_MOD_AREC:
    case K_MAPI_MOD_AO:
    case K_MAPI_MOD_VVI:
    case K_MAPI_MOD_VVO:
    case K_MAPI_MOD_VICAP:
    case K_MAPI_MOD_SENSOR:
    case K_MAPI_MOD_ISP:
        func = msg_server_get_func(mod_id, msg->u32CMD);
        if(func != NULL) {
            k_s32 ret = func(id, msg);
            if(ret != K_SUCCESS) {
                mapi_sys_error_trace(
                    "mod_id:%u cmd:%u func return error:0x%x\n",
                    mod_id, msg->u32CMD, ret);
            }
        } else {
            mapi_sys_error_trace(
                    "mod_id:%u cmd:%u func is NULL\n",
                    mod_id, msg->u32CMD);
        }
        break;
    default:
        printf("receive mod_id:%d msg %d error.\n", mod_id, msg->u32CMD);
        break;
    }
}

void* media_msg_server_receive_thread(void *arg)
{
    k_s32 ret;

    ret = kd_ipcmsg_connect(&g_media_msg_id, IPCMSG_MEDIA_SERVER_NAME, meida_msg_receive_proc);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_ipcmsg_connect failed:0x%08x\n", ret);
        return NULL;
    }

    kd_ipcmsg_run(g_media_msg_id);

    /* reconnect detetc and process*/
    while(g_msg_start_flag) {
        if(kd_ipcmsg_is_connect(g_media_msg_id) != K_TRUE) {

            kd_ipcmsg_disconnect(g_media_msg_id);

            ret = kd_ipcmsg_connect(&g_media_msg_id, IPCMSG_MEDIA_SERVER_NAME, meida_msg_receive_proc);
            if(ret != K_SUCCESS) {
                mapi_sys_error_trace("kd_ipcmsg_connect failed:0x%08x\n", ret);
                return NULL;
            }
            kd_ipcmsg_run(g_media_msg_id);

        }
    }
    return NULL;
}

k_s32 media_msg_server_init(void)
{
    k_s32 ret = K_SUCCESS;
    k_ipcmsg_connect_t conn_attr ={ 0, IPCMSG_PORT_MPP, 1 };

    ret = kd_ipcmsg_add_service(IPCMSG_MEDIA_SERVER_NAME, &conn_attr);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_ipcmsg_add_service failed:0x%08x\n", ret);
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    g_server_context.server_modules[K_MAPI_MOD_SYS] = mapi_msg_get_sys_mod();
#if 0
    g_server_context.server_modules[K_MAPI_MOD_VI] = mapi_msg_get_vi_mod();
    g_server_context.server_modules[K_MAPI_MOD_VPROC] = mapi_msg_get_vproc_mod();
#endif
    g_server_context.server_modules[K_MAPI_MOD_VENC] = mapi_msg_get_venc_mod();
    g_server_context.server_modules[K_MAPI_MOD_VDEC] = mapi_msg_get_vdec_mod();
#if 0
    g_server_context.server_modules[K_MAPI_MOD_VDEC] = mapi_msg_get_vdec_mod();
    g_server_context.server_modules[K_MAPI_MOD_VREC] = mapi_msg_get_vrec_mod();
    g_server_context.server_modules[K_MAPI_MOD_VO] = mapi_msg_get_vo_mod();
    g_server_context.server_modules[K_MAPI_MOD_AI] = mapi_msg_get_ai_mod();
    g_server_context.server_modules[K_MAPI_MOD_AENC] = mapi_msg_get_aenc_mod();
    g_server_context.server_modules[K_MAPI_MOD_ADEC] = mapi_msg_get_adec_mod();
    g_server_context.server_modules[K_MAPI_MOD_AREC] = mapi_msg_get_arec_mod();
    g_server_context.server_modules[K_MAPI_MOD_AO] = mapi_msg_get_ao_mod();
#endif

    g_server_context.server_modules[K_MAPI_MOD_VO] = mapi_msg_get_vo_mod();

    g_server_context.server_modules[K_MAPI_MOD_VI] = mapi_msg_get_vi_mod();
    g_server_context.server_modules[K_MAPI_MOD_VVI] = mapi_msg_get_vvi_mod();
    g_server_context.server_modules[K_MAPI_MOD_VICAP] = mapi_msg_get_vicap_mod();
    g_server_context.server_modules[K_MAPI_MOD_SENSOR] = mapi_msg_get_sensor_mod();
    g_server_context.server_modules[K_MAPI_MOD_ISP] = mapi_msg_get_isp_mod();
//    g_server_context.server_modules[K_MAPI_MOD_VVO] = mapi_msg_get_vvo_mod();

    g_server_context.server_modules[K_MAPI_MOD_AI] = mapi_msg_get_ai_mod();
    g_server_context.server_modules[K_MAPI_MOD_AO] = mapi_msg_get_ao_mod();
    g_server_context.server_modules[K_MAPI_MOD_AENC] = mapi_msg_get_aenc_mod();
    g_server_context.server_modules[K_MAPI_MOD_ADEC] = mapi_msg_get_adec_mod();

    g_msg_start_flag = K_TRUE;
    ret = pthread_create(&g_server_receive_thread, NULL, media_msg_server_receive_thread, NULL);
    if(K_SUCCESS != ret) {
        mapi_sys_error_trace("kd_ipcmsg_add_service failed:0x%08x\n", ret);
    }

    mapi_sys_info_trace("msg init success!\n");
    return ret;
}

k_s32 media_msg_server_deinit(void)
{
    k_s32 ret;

    g_msg_start_flag = K_FALSE;
    ret = kd_ipcmsg_disconnect(g_media_msg_id);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_ipcmsg_disconnect fail:0x%x\n", ret);
//        return K_FAILED;
    }
    /*
     The detachstate attribute controls whether the thread is created in a detached state.
     If the thread is created detached, then use of the ID of the newly created thread by the
     pthread_detach() or pthread_join() function is an error.
     https://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread_attr_getdetachstate.html
    */
#if 1
    if(g_server_receive_thread != -1) {
        pthread_join(g_server_receive_thread, NULL);
        g_server_receive_thread = -1;
    }
#endif
    g_msg_start_flag = K_FALSE;
    kd_ipcmsg_del_service(IPCMSG_MEDIA_SERVER_NAME);
    return ret;
}

k_s32 mapi_media_msg_get_id(void)
{
    return g_media_msg_id;
}