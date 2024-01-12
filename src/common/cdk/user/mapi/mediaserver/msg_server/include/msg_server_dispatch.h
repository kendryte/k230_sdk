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
#ifndef __MSG_SERVER_DISPATCH_H__
#define __MSG_SERVER_DISPATCH_H__

#include "k_mapi_module.h"
#include "k_comm_ipcmsg.h"
#include "msg_dispatch.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MSG_SERVER_MODULE_NAME_LEN (16)

typedef k_s32 (*cmd_proc_fn)(k_s32 id, k_ipcmsg_message_t *msg);

typedef struct {
    k_u32 cmd;
    cmd_proc_fn cmd_proc_fn_ptr;
} msg_module_cmd_t;

typedef struct {
    k_u32 mod_id;
    k_char module_name[MSG_SERVER_MODULE_NAME_LEN];
    k_u32 modile_cmd_amount;
    msg_module_cmd_t *module_cmd_table;
} msg_server_module_t;

typedef struct {
    msg_server_module_t *server_modules[K_MAPI_MOD_BUTT];
} msg_server_context_t;

msg_server_module_t *mapi_msg_get_sys_mod(void);
msg_server_module_t *mapi_msg_get_vi_mod(void);
msg_server_module_t *mapi_msg_get_vproc_mod(void);
msg_server_module_t *mapi_msg_get_venc_mod(void);
msg_server_module_t *mapi_msg_get_vdec_mod(void);
msg_server_module_t *mapi_msg_get_vrec_mod(void);
msg_server_module_t *mapi_msg_get_vo_mod(void);
msg_server_module_t *mapi_msg_get_ai_mod(void);
msg_server_module_t *mapi_msg_get_ao_mod(void);
msg_server_module_t *mapi_msg_get_aenc_mod(void);
msg_server_module_t *mapi_msg_get_adec_mod(void);
msg_server_module_t *mapi_msg_get_arec_mod(void);
msg_server_module_t *mapi_msg_get_vvi_mod(void);
msg_server_module_t *mapi_msg_get_vvo_mod(void);
msg_server_module_t *mapi_msg_get_dpu_mod(void);
msg_server_module_t *mapi_msg_get_vicap_mod(void);
msg_server_module_t *mapi_msg_get_sensor_mod(void);
msg_server_module_t *mapi_msg_get_isp_mod(void);

k_s32 media_msg_server_init(void);
k_s32 media_msg_server_deinit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_SERVER_DISPATCH_H_ */

