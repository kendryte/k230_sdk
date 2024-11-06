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
#ifndef __AUDIO_SAMPLE_H__
#define __AUDIO_SAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#include "k_type.h"
#include "k_audio_comm.h"
#include "k_payload_comm.h"
#include "k_ai_comm.h"
#include "k_ao_comm.h"
#include "k_aenc_comm.h"
#include "k_adec_comm.h"

k_s32 audio_sample_vb_init(k_bool enable_cache, k_u32 sample_rate); //初始化vb
k_s32 audio_sample_vb_destroy(); //销毁vb
k_s32 audio_sample_enable_audio_codec(k_bool enable_audio_codec);
k_s32 audio_sample_get_ai_i2s_data(const char*filename, k_audio_bit_width bit_width, k_u32 sample_rate,k_u32 channel_count,k_i2s_in_mono_channel  mono_channel,k_i2s_work_mode i2s_work_mode, k_u32 enable_audio3a); //获取i2s输入数据
k_s32 audio_sample_get_ai_pdm_data(const char*filename, k_audio_bit_width bit_width, k_u32 sample_rate,k_u32 channel_count); //获取pdm输入数据
k_s32 audio_sample_send_ao_data(const char*filename,int nDev, int nChannel,  int samplerate,k_audio_bit_width bit_width,k_i2s_work_mode i2s_work_mode); //发送数据到ao
k_s32 audio_sample_api_ai_to_ao(int ai_dev_num, int ai_channel, int ao_dev_num, int ao_channel, k_u32 sample_rate, k_audio_bit_width bit_width,k_i2s_work_mode i2s_work_mode, k_u32 enable_audio3a); //通过api获取ai数据发送到ao
k_s32 audio_sample_bind_ai_to_ao(int ai_dev_num, int ai_channel, int ao_dev_num, int ao_channel, k_u32 sample_rate, k_audio_bit_width bit_width,k_i2s_work_mode i2s_work_mode, k_u32 enable_audio3a); //通过bind获取ai数据发送到ao
k_s32 audio_sample_ai_encode(k_bool use_sysbind, k_u32 samplerate,k_audio_bit_width bit_width,int encChannel,k_payload_type type,const char*filename, k_u32 enable_audio3a); //ai->编码,存文件
k_s32 audio_sample_decode_ao(k_bool use_sysbind,k_u32 samplerate,k_audio_bit_width bit_width,int decChannel,k_payload_type type,const char*filename); //解码文件->ao
k_s32 audio_sample_ai_aenc_adec_ao(k_audio_dev ai_dev,k_ai_chn ai_chn,k_audio_dev ao_dev,k_ao_chn ao_chn,k_aenc_chn aenc_chn,k_adec_chn adec_chn,k_u32 samplerate,k_audio_bit_width bit_width,k_payload_type type,const char *load_filename, k_u32 enable_audio3a);
k_s32 audio_sample_ai_aenc_adec_ao_2(k_audio_dev ai_dev,k_ai_chn ai_chn,k_audio_dev ao_dev,k_ao_chn ao_chn,k_aenc_chn aenc_chn,k_adec_chn adec_chn,k_u32 samplerate,k_audio_bit_width bit_width,k_payload_type type, k_u32 enable_audio3a);
k_s32 audio_sample_acodec();
k_s32 audio_sample_exit();
#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif


