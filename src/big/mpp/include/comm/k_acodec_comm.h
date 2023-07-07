/**
* @file k_codec_comm.h
* @author
* @version 1.0
* @date 2023-5-5
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
#ifndef __K_ACODEC_COMM_H__
#define __K_ACODEC_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_audio_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

typedef enum
{   //cmd 3、4 unable to jump to kernel,so define start begin 10 */
    k_acodec_set_gain_micl          =10,//[0db,6db,20db,30db]    adc左声道输入的模拟增益控制
    k_acodec_set_gain_micr           ,//[0db,6db,20db,30db]    adc右声道输入的模拟增益控制
    k_acodec_set_adcl_volume         ,//[-97,30],step 0.5      adc左声道数字音量控制
    k_acodec_set_adcr_volume         ,//[-97,30],step 0.5      adc右声道数字音量控制

    k_acodec_set_alc_gain_micl       ,//[-18,28.5],step 1.5    alc左声道输入的模拟增益控制
    k_acodec_set_alc_gain_micr       ,//[-18,28.5],step 1.5    alc右声道输入的模拟增益控制


    k_acodec_set_gain_hpoutl         ,//[-39,6],step 1.5       dac左声道输出模拟音量控制
    k_acodec_set_gain_hpoutr         ,//[-39,6],step 1.5       dac右声道输出模拟音量控制
    k_acodec_set_dacl_volume         ,//[-120,7],step 0.5      dac左声道输出数字音量控制
    k_acodec_set_dacr_volume         ,//[-120,7],step 0.5      dac右声道输出数字音量控制

    k_acodec_set_micl_mute           ,//[0,1]                  左声道输入静音控制
    k_acodec_set_micr_mute           ,//[0,1]                  右声道输入静音控制
    k_acodec_set_dacl_mute           ,//[0,1]                  左声道输出静音控制
    k_acodec_set_dacr_mute           ,//[0,1]                  右声道输出静音控制

    k_acodec_get_gain_micl           ,//[0db,6db,20db,30db]    获取adc左声道输入的模拟增益控制
    k_acodec_get_gain_micr           ,//[0db,6db,20db,30db]    获取adc右声道输入的模拟增益控制
    k_acodec_get_adcl_volume         ,//[-97,30],step 0.5      获取adc左声道数字音量控制
    k_acodec_get_adcr_volume         ,//[-97,30],step 0.5      获取adc右声道数字音量控制

    k_acodec_get_alc_gain_micl       ,//[-18,28.5],step 1.5    获取alc左声道输入的模拟增益控制
    k_acodec_get_alc_gain_micr       ,//[-18,28.5],step 1.5    获取alc右声道输入的模拟增益控制


    k_acodec_get_gain_hpoutl         ,//[-39,6],step 1.5       获取dac左声道输出模拟音量控制
    k_acodec_get_gain_hpoutr         ,//[-39,6],step 1.5       获取dac右声道输出模拟音量控制
    k_acodec_get_dacl_volume         ,//[-120,7],step 0.5      获取dac左声道输出数字音量控制
    k_acodec_get_dacr_volume         ,//[-120,7],step 0.5      获取dac右声道输出数字音量控制

    k_acodec_reset                   ,//reset acodec
}k_audio_codec_cmd;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
