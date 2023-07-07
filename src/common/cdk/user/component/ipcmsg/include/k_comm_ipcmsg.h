/**
 * @file k_comm_ipcmsg.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-06-12
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
 */
#ifndef __K_COMM_IPCMSG_H__
#define __K_COMM_IPCMSG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

#include <stdlib.h>
#include <string.h>
#include "k_type.h"

/** \addtogroup     IPCMSG*/
/** @{ */  /** <!-- [IPCMSG] */

#define IPCMSG_MAX_PORT_NUM 512
#define K_IPCMSG_MAX_CONTENT_LEN (1024)
#define K_IPCMSG_PRIVDATA_NUM (8)
#define K_IPCMSG_INVALID_MSGID (0xFFFFFFFFFFFFFFFF)

/** @} */  /*! <!-- Macro Definition End */

typedef struct
{
    k_char aszServiceName[16];
    k_s32 s32Id;
    k_char pData[0];
} IPCMSG_TRANS_CONNECT_ATTR_S;

typedef struct IPCMSG_CONNECT_S
{
    k_u32 u32RemoteId;
    k_u32 u32Port;
    k_u32 u32Priority;
} k_ipcmsg_connect_t;

/**Message structure*/
typedef struct IPCMSG_MESSAGE_S
{
    k_bool bIsResp;    /**<Identify the response messgae*/
    k_u64 u64Id;       /**<Message ID*/
    k_u32 u32Module;   /**<Module ID, user-defined*/
    k_u32 u32CMD;      /**<CMD ID, user-defined*/
    k_s32 s32RetVal;   /**<Retrun Value in response message*/
    k_s32 as32PrivData[K_IPCMSG_PRIVDATA_NUM]; /**<Private data, can be modify directly after ::kd_ipcmsg_create_message or ::kd_ipcmsg_create_resp_message*/
    void* pBody;     /**<Message body*/
    k_u32 u32BodyLen;  /**<Length of pBody*/
} k_ipcmsg_message_t;


/** Error number base */
#define K_IPCMSG_ERRNO_BASE 0x1900
/** Parameter is invalid */
#define K_IPCMSG_EINVAL (K_IPCMSG_ERRNO_BASE+1)
/** The function run timeout */
#define K_IPCMSG_ETIMEOUT (K_IPCMSG_ERRNO_BASE+2)
/** IPC driver open fail */
#define K_IPCMSG_ENOOP (K_IPCMSG_ERRNO_BASE+3)
/** Internal error */
#define K_IPCMSG_EINTER (K_IPCMSG_ERRNO_BASE+4)
/** Null pointer*/
#define K_IPCMSG_ENULL_PTR (K_IPCMSG_ERRNO_BASE+5)


#define K_IPCMSG_MAX_SERVICENAME_LEN (16)

int IPCMSG_Log(unsigned int  level, const char *str, ...);

#define PRINT_LEVEL_ERROR   (1)
#define PRINT_LEVEL_WARN    (2)
#define PRINT_LEVEL_INFO    (3)
#define PRINT_LEVEL_DEBUG   (4)
#define PRINT_LEVEL         PRINT_LEVEL_ERROR

#define IPCMSG_LOGE(fmt, ...)   IPCMSG_Log(PRINT_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define IPCMSG_LOGW(fmt, ...)   IPCMSG_Log(PRINT_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define IPCMSG_LOGI(fmt, ...)   IPCMSG_Log(PRINT_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define IPCMSG_LOGD(fmt, ...)   IPCMSG_Log(PRINT_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

/**
 * @brief Callback of receiving message.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Received message.
 */
typedef void (*k_ipcmsg_handle_fn_ptr)(k_s32 s32Id, k_ipcmsg_message_t* pstMsg);

/**
 * @brief Callback of receiving response message. used by kd_ipcmsg_send_async
 * @param[in] pstMsg Response message.
 */
typedef void (*k_ipcmsg_resphandle_fn_ptr)(k_ipcmsg_message_t* pstMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif
