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

#ifndef __K_PLAYER_H__
#define __K_PLAYER_H__

#include "k_type.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


typedef enum KLAYER_EVENT_E
{
    K_PLAYER_EVENT_EOF,          /**< the player is playing the end*/
    K_LITEPLAYER_EVENT_BUTT
} K_PLAYER_EVENT_E;

typedef k_s32 (*K_PLAYER_EVENT_FN)(K_PLAYER_EVENT_E enEvent, void* pData);

/**
*   @brief init the player
*   @param[in] N/A
*   @retval  0 success,others failed
*/
k_s32 kd_player_init();

/**
*   @brief deinit of the player
*   @param[in] : N/A
*   @retval  0 success,others failed
*/
k_s32 kd_player_deinit();

/**
*   @brief register call back fun
*   @param[in] pfnCallback : K_PLAYER_EVENT_E: call back fun
*   @param[in] pData :  call back data
*   @retval  0 success,others failed
*/
k_s32 kd_player_regcallback( K_PLAYER_EVENT_FN pfnCallback,void* pData);

/**
*   @brief    set the file for playing
*   @param[in] filePath : k_char: media file path
*   @retval  0 success,others failed
*/
k_s32 kd_player_setdatasource(const k_char* filePath);

/**
*   @brief  do play of the stream
*   @param[in] : N/A
*   @retval  0 success,others failed
*/
k_s32 kd_player_start();

/**
*   @brief stop the stream playing, and release the resource
*   @retval  0 success,others failed
*/
k_s32 kd_player_stop();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
