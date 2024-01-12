/**
 * @file k_ipcmsg.h
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
#ifndef __K_IPCMSG_H__
#define __K_IPCMSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "k_comm_ipcmsg.h"
/** \addtogroup     IPCMSG*/
/** @{ */  /** <!-- [IPCMSG] */

/**
 * @brief Add services to global service table before use of IPCMSG.
 * This function should be called in two side arm with the same parameters.
 * @param[in] pszServiceName Service running on two side(a7/a17).
 * @param[in] u32Port The two service has unique port.
 * @param[in] u32Priority Priority of the communication.
 * @return K_SUCCESS Register success.
 * @return K_FAILED Register fail.
 * @return K_IPCMSG_EINVAL Parameter is invalid
 * @return K_IPCMSG_EINTER Open device fail or other internal error
 */
k_s32 kd_ipcmsg_add_service(const k_char* pszServiceName, const k_ipcmsg_connect_t* pstConnectAttr);

/**
 * @brief Delete service from global service table when don't need IPCMSG.
 * @param[in] pszServiceName Service running on two side(a7/a17).
 * @return K_IPCMSG_EINVAL Parameter is invalid
 * @return K_SUCCESS Unregister success.
 * @return K_FAILED Unregister fail.
 */
k_s32 kd_ipcmsg_del_service(const k_char* pszServiceName);

/**
 * @brief Establish the connection between master and slave.Attention: Message can send successfuly only the two side call this function. So ::kd_ipcmsg_is_connect should be called to ensure the connection is established before send or receive
 * @param[out] ps32Id Handle of IPCMSG. All operation in IPCMSG need this handle
 * @param[in] pszServiceName Service name running on the other side.
 * @param[in] pfnMessageHandle Callback function to receive message.
 * @return K_IPCMSG_EINVAL Parameter is invalid
 * @return K_IPCMSG_EINTER Open device fail or other internal error
 * @return K_SUCCESS Connect success.
 * @return K_FAILED Conectt fail.
 */
k_s32 kd_ipcmsg_try_connect(k_s32* ps32Id, const k_char* pszServiceName, k_ipcmsg_handle_fn_ptr pfnMessageHandle);

/**
 * @brief Establish the connection between master and slave. This function will block until two side connections established.
 * @param[out] ps32Id Handle of IPCMSG. All operation in IPCMSG need this handle
 * @param[in] pszServiceName Service name running on the other side.
 * @param[in] pfnMessageHandle Callback function to receive message.
 * @return K_IPCMSG_EINVAL Parameter is invalid
 * @return K_IPCMSG_EINTER Open device fail or other internal error
 * @return K_SUCCESS Connect success.
 * @return K_FAILED Conectt fail.
 */
k_s32 kd_ipcmsg_connect(k_s32* ps32Id, const k_char* pszServiceName, k_ipcmsg_handle_fn_ptr pfnMessageHandle);

/**
 * @brief Disconnect when don't want to send or receive message.
 * @param[in] s32Id Handle of IPCMSG.
 * @return K_IPCMSG_EINVAL Parameter is invalid
 * @return K_SUCCESS Disconnect success.
 * @return K_FAILED Disconnect fail.
 */
k_s32 kd_ipcmsg_disconnect(k_s32 s32Id);

/**
 * @brief Whether the communication can work.
 * @param[in] s32Id Handle of IPCMSG
 * @return K_TRUE connection is done. can send message.
 * @return K_FALSE connection is not finish yet. send message will return failure.
 */
k_bool kd_ipcmsg_is_connect(k_s32 s32Id);

/**
 * @brief Send message only, don't need response.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @return K_SUCCESS Send success.
 * @return K_FAILED Send fail.
 */
k_s32 kd_ipcmsg_send_only(k_s32 s32Id, k_ipcmsg_message_t *pstRequest);

/**
 * @brief Send message asynchronously. the function will return immediately.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @param[in] pfnRespHandle Callback function to receive response.
 * @return K_SUCCESS Send success.
 * @return K_FAILED Send fail.
 */
k_s32 kd_ipcmsg_send_async(k_s32 s32Id, k_ipcmsg_message_t* pstMsg, k_ipcmsg_resphandle_fn_ptr pfnRespHandle);

/**
 * @brief Send message synchronously. the function will block until response message received.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @param[out] ppstMsg Received response message.
 * @param[in] s32TimeoutMs When response not received in s32TimeoutMs, function will return ::K_IPCMSG_ETIMEOUT
 * @return K_SUCCESS Send success.
 * @return K_FAILED Send fail.
 * #return K_IPCMSG_ETIMEOUT Timeout to receive response
 */
k_s32 kd_ipcmsg_send_sync(k_s32 s32Id, k_ipcmsg_message_t* pstMsg, k_ipcmsg_message_t** ppstMsg, k_s32 s32TimeoutMs);

/**
 * @brief In this function, Message will be received and dispatched to message callback function.
 * User should create thread to run this function.
 * @param[in] s32Id Handle of IPCMSG.
 */
void kd_ipcmsg_run(k_s32 s32Id);

/**
 * @brief Create the message, used by K_IPCMSG_SendXXX and receive callback function.
 * @param[in] u32Module Module ID defined by user. user can use it to dispatch to different modules.
 * @param[in] u32CMD CMD ID, defined by user. user can use it to identify which command.
 * @param[in] pBody Message body, mustn't contain pointer because pointer will be useless in other side.
 * @param[in] u32BodyLen Length of pBody.
 * @return ::k_ipcmsg_message_t* Created message.
 */
k_ipcmsg_message_t* kd_ipcmsg_create_message(k_u32 u32Module, k_u32 u32CMD, const void* pBody, k_u32 u32BodyLen);

/**
 * @brief Create the response message.
 * @param[in] pstRequest Request message received by user.
 * @param[in] s32RetVal Integer return value.
 * @param[in] pBody Message body.
 * @param[in] u32BodyLen Length of pBody.
 * @return ::k_ipcmsg_message_t* Created message.
 */
k_ipcmsg_message_t* kd_ipcmsg_create_resp_message(k_ipcmsg_message_t* pstRequest, k_s32 s32RetVal, void* pBody, k_u32 u32BodyLen);

/**
 * @brief Destroy the message. Message must be destroyed when send and receive finish
 * @param[in] pstMsg Message to destroy.
 */
void kd_ipcmsg_destroy_message(k_ipcmsg_message_t* pstMsg);

#ifdef __cplusplus
}
#endif

#endif
