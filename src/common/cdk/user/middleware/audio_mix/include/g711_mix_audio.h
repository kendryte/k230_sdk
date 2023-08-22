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

#ifndef __G711_MIX_AUDIO_H__
#define __G711_MIX_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "k_type.h"

/**
 * @brief mix g711a audio
 *
 * @param [in] g711a source data1
 * @param [in] g711a source data2
 * @param [in] mix data length
 * @param [out] g711a mix out data
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" error
 */
k_s32 kd_mix_g711a_audio(k_char *src_data1, k_char *src_data2, k_u32 data_len, k_char *dst_mix_data);

/**
 * @brief mix g711u audio
 *
 * @param [in] g711u source data1
 * @param [in] g711u source data2
 * @param [in] mix data length
 * @param [out] g711u mix out data
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" error
 */
k_s32 kd_mix_g711u_audio(k_char *src_data1, k_char *src_data2, k_u32 data_len, k_char *dst_mix_data);

#ifdef __cplusplus
}
#endif

#endif

