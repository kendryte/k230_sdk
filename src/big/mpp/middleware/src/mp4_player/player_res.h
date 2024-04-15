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

#ifndef __K_PLAYER_RES_H__
#define __K_PLAYER_RES_H__

#include "k_type.h"
#include "k_payload_comm.h"
#include "kplayer.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

k_s32 sys_init(k_bool init_vo);
k_s32 sys_deinit(k_bool deinit_vo);

k_s32 disp_open(k_payload_type video_dec_type,k_u32 width,k_u32 height,int type);//K_PT_H264/K_PT_H265
k_s32 disp_play(k_u8*pdata,k_u32 len,k_u64 timestamp,k_bool end_stream);
k_s32 disp_close();

k_s32 ao_open(k_s32 s32SampleRate, k_s32 s32ChanNum,k_payload_type audio_dec_type, k_bool avsync);//K_PT_G711A/K_PT_G711U
k_s32 ao_play(k_u8*pdata,k_u32 len,k_u64 timestamp);
k_s32 ao_close();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
