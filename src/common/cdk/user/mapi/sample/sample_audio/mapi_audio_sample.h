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
#ifndef __MAPI_AUDIO_SAMPLE_H__
#define __MAPI_AUDIO_SAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#include "k_type.h"
#include "k_payload_comm.h"

k_s32 audio_mapi_sample_ai_aenc(k_u32 sample_rate,k_u32 channels,k_bool enable_audio_codec, k_payload_type aenc_type,const char* filename);
k_s32 audio_mapi_sample_adec_ao(k_u32 sample_rate,k_u32 channels,k_bool enable_audio_codec,const char* filename);
k_s32 audio_mapi_sample_audio_loopback(k_u32 sample_rate,k_u32 channels);
k_s32 audio_mapi_sample_audio_double_loopback(k_u32 sample_rate,k_u32 channels,k_bool enable_audio_codec);
k_s32 audio_mapi_sample_play_wav(const char* filename);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif


