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

#include<stdio.h>
#include<pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi_sys_api.h"


#include "audio_sample.h"

static  void _help()
{
    printf("Please input:\n");
    printf("-type: test audio function[0-12]\n");
    printf("  type 0:sample ai i2s module\n");
    printf("  type 1:sample ai pdm module\n");
    printf("  type 2:sample ao i2s module\n");
    printf("  type 3:sample ai(i2s) to ao (api) module\n");
    printf("  type 4:sample ai(i2s) to ao (sysbind) module\n");
    printf("  type 5:sample ai(pdm) to ao (api) module\n");
    printf("  type 6:sample ai(pdm) bind ao (sysbind) module\n");
    printf("  type 7:sample aenc(ai->aenc->file) (sysbind) module\n");
    printf("  type 8:sample adec(file->adec->ao) (sysbind) module\n");
    printf("  type 9:sample aenc(ai->aenc->file) (api) module\n");
    printf("  type 10:sample adec(file->adec->ao) (api) module\n");
    printf("  type 11:sample overall test (ai->aenc->file file->adec->ao) module\n");
    printf("  type 12:sample overall test (ai->aenc  adec->ao loopback ) module\n");
    //printf("  type 9:overall audio test(ai->aenc adec->ao)\n");
    printf("-samplerate: set audio sample(8000 ~ 192000)\n");
    printf("-enablecodec: enable audio codec(0,1)\n");
    printf("-loglevel: show kernel log level[0,7]\n");
    printf("-bitwidth: set audio bit width(16,24,32)\n");
    printf("-channels: channel count\n");
    printf("-monochannel:0:mic input 1:headphone input\n");
    printf("-filename: load or save file name\n");
    printf("-audio3a: enable audio3a:enable_ans:0x01,enable_agc:0x02,enable_aec:0x04\n");
    //printf("-i2smode:set i2s mode(1:i2s  2:right justified  4:left justified)\n");
}
static pthread_t g_pthread_handle;
static int       g_type = -1;
static k_u32     g_sample_rate = 44100;//44100;
static k_audio_bit_width g_bit_width = KD_AUDIO_BIT_WIDTH_16;
static k_bool    g_enable_audio_codec = K_FALSE;
static k_i2s_work_mode g_i2s_work_mode = K_STANDARD_MODE;
static k_u32    g_enable_audio3a = 0;
static char g_wav_name[256];
static k_u32     g_channel_count = 2;
static k_i2s_in_mono_channel  g_mono_channel = KD_I2S_IN_MONO_RIGHT_CHANNEL;//mono channel use mic input
static void *sample_thread_fn(void *arg)
{
    switch (g_type)
    {
    case 0:
        printf("sample ai i2s module\n");
        audio_sample_get_ai_i2s_data(g_wav_name,g_bit_width, g_sample_rate,g_channel_count,g_mono_channel,g_i2s_work_mode, g_enable_audio3a); //采样精度设置为32时，不要启动2组io，因为无法区分数据是哪组
        break;
    case 1:
        printf("sample ai pdm module\n");
        audio_sample_get_ai_pdm_data(g_wav_name,g_bit_width, g_sample_rate,2);
        break;
    case 2:
        printf("sample ao i2s module\n");
        audio_sample_send_ao_data(g_wav_name,0, 0,g_sample_rate,g_bit_width, g_i2s_work_mode);
        break;
    case 3:
        printf("sample ai(i2s) to ao module\n");
        audio_sample_api_ai_to_ao(0, 0, 0, 0, g_sample_rate, g_bit_width,g_i2s_work_mode, g_enable_audio3a);
        break;
    case 4:
        printf("sample ai(i2s) bind ao module\n");
        audio_sample_bind_ai_to_ao(0, 0, 0, 0, g_sample_rate, g_bit_width,g_i2s_work_mode, g_enable_audio3a);
        break;
    case 5:
        printf("sample ai(pdm) to ao module\n");
        audio_sample_api_ai_to_ao(1,0,0,0,g_sample_rate, g_bit_width,g_i2s_work_mode, K_FALSE);
        break;
    case 6:
        printf("sample ai(pdm) bind ao module\n");
        audio_sample_bind_ai_to_ao(1,0,0,0,g_sample_rate, g_bit_width,g_i2s_work_mode, K_FALSE);
        break;
    case 7:
        printf("sample aenc module (sysbind)\n");
        audio_sample_ai_encode(K_TRUE,g_sample_rate,g_bit_width,0,K_PT_G711A,g_wav_name,g_enable_audio3a);
        break;
    case 8:
        printf("sample adec module (sysbind)\n");
        audio_sample_decode_ao(K_TRUE,g_sample_rate,g_bit_width,0,K_PT_G711A,g_wav_name);
        break;
    case 9:
        printf("sample aenc module (api)\n");
        audio_sample_ai_encode(K_FALSE,g_sample_rate,g_bit_width,0,K_PT_G711A,g_wav_name,g_enable_audio3a);
        break;
    case 10:
        printf("sample adec module (api)\n");
        audio_sample_decode_ao(K_FALSE,g_sample_rate,g_bit_width,0,K_PT_G711A,g_wav_name);
        break;
    case 11:
        printf("sample ai->aenc->file file->adec->ao module\n");
        audio_sample_ai_aenc_adec_ao(0,0,0,0,0,0,g_sample_rate,g_bit_width,K_PT_G711A,g_wav_name,g_enable_audio3a);
        break;
    case 12:
        printf("sample ai->aenc  adec->ao module (loopback)\n");
        audio_sample_ai_aenc_adec_ao_2(0,0,0,0,0,0,g_sample_rate,g_bit_width,K_PT_G711A,g_enable_audio3a);
        break;
    case 13:
        printf("sample  acodec\n");
        audio_sample_acodec();
        break;
    default :
        break;
    }

    return NULL;
}

static void _set_mod_log(k_mod_id mod_id,k_s32  level)
{
    k_log_level_conf level_conf;
    level_conf.level = level;
    level_conf.mod_id = mod_id;
    kd_mpi_log_set_level_conf(&level_conf);
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        _help();
        return -1;
    }
    int nbitwidth = 16;
    int nloglevel = 0;
    memcpy(g_wav_name,"test.wav",strlen("test.wav")+1);

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            _help();
            return 0;
        }
        else if (strcmp(argv[i], "-samplerate") == 0)
        {
            g_sample_rate = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-type") == 0)
        {
            g_type = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-bitwidth") == 0)
        {
            nbitwidth = atoi(argv[i + 1]);
            if (nbitwidth == 16)
            {
                g_bit_width = KD_AUDIO_BIT_WIDTH_16;
            }
            else if (nbitwidth == 32)
            {
                g_bit_width = KD_AUDIO_BIT_WIDTH_32;
            }
            else if (nbitwidth == 24)
            {
                g_bit_width = KD_AUDIO_BIT_WIDTH_24;
            }
            else
            {
                printf("bitwidth not support %s\n",argv[i+1]);
                return -1;
            }
        }
        else if (strcmp(argv[i], "-channels") == 0)
        {
            g_channel_count = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-monochannel") == 0)
        {
            g_mono_channel = (k_i2s_in_mono_channel)atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-loglevel") == 0)
        {
            nloglevel = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-enablecodec") == 0)
        {
            g_enable_audio_codec = (k_bool)atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-i2smode") == 0)
        {
            g_i2s_work_mode = (k_i2s_work_mode)atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-filename") == 0)
        {
            memcpy(g_wav_name,argv[i + 1],strlen(argv[i + 1])+1);
        }
        else if (strcmp(argv[i], "-audio3a") == 0)
        {
            g_enable_audio3a = atoi(argv[i + 1]);
        }

        else
        {
            printf("Error :Invalid arguments %s\n", argv[i]);
            _help();
            return -1;
        }
    }

    if (13 == g_type)
    {
        audio_sample_acodec();
        return 0;
    }


    printf("audio type:%d,sample rate:%d,bit width:%d,channels:%d,enablecodec:%d,monochannel:%d\n", g_type, g_sample_rate,nbitwidth,g_channel_count,g_enable_audio_codec,g_mono_channel);

    _set_mod_log(K_ID_AO,nloglevel);//K_DBG_EMERG
    _set_mod_log(K_ID_AI,nloglevel);//K_DBG_EMERG
    _set_mod_log(K_ID_AENC,nloglevel);//K_DBG_EMERG
    _set_mod_log(K_ID_ADEC,nloglevel);//K_DBG_EMERG


    audio_sample_enable_audio_codec(g_enable_audio_codec);

    audio_sample_vb_init(K_TRUE, g_sample_rate);
    pthread_create(&g_pthread_handle, NULL, sample_thread_fn, NULL);

    if (g_type >= 2 && g_type <= 12)
    {
        printf("enter q key to exit\n");
        while(getchar() != 'q')
        {
            usleep(100*1000);
        }
    }
    audio_sample_exit();
    pthread_join(g_pthread_handle, NULL);

    printf("destroy vb block \n");
    audio_sample_vb_destroy();

    printf("sample done\n");

    return 0;
}
