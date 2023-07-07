/**
 * @file msg_dispatch.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2022-10-10
 *
* @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
 *
 */
#ifndef __MSG_DISPATCH_H__
#define __MSG_DISPATCH_H__

#include "k_mapi_module.h"
#include "k_comm_ipcmsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MSG_SERVER_MODULE_NAME_LEN (16)

/******************************************************************************
|----------------------------------------------------------------|
| |   MOD_ID    |   DEV_ID    |   CHN_ID    |      reserve         |
|----------------------------------------------------------------|
|<--8bits----><--8bits --><--8bits --><-----8bits---->|
******************************************************************************/

#define MODFD(MOD, DevID, ChnID) \
    ((k_u32)((MOD & 0xFF) << 24 | ((DevID & 0xFF) << 16) | ((ChnID & 0xFF) << 8)))

#define GET_MOD_ID(ModFd) \
    (((ModFd) >> 24) & 0xFF)

#define GET_DEV_ID(ModFd) \
    (((ModFd) >> 16) & 0xFF)

#define GET_CHN_ID(ModFd) \
    (((ModFd) >> 8) & 0xFF)

#define MediaMsgId k_s32 mapi_media_msg_get_id()

#define K_IPCMSG_SEND_SYNC_TIMEOUT 50000  // ms

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_DISPATCH_H_ */

