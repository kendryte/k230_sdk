/**
 * @file k_audio_comm.h
 * @author
 * @sxp
 * @version 1.0
 * @date 2022-10-21
 *
 * @copyright Copyright (C) hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 *
 */

#ifndef __K_AUDIO_COMM_H__
#define __K_AUDIO_COMM_H__

#include "k_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /**<  __cplusplus */

#define AUDIO_BASE_ADDR                 (0x9140f000U)

#define       AI_DEV_I2S      0
#define       AI_DEV_PDM      1
#define       AO_DEV_I2S      0


typedef k_u32 k_audio_dev;

#define K_AUDIO_FRAME_CHN_NUM 2      //定义存放音频数据的最大声道数
#define K_MAX_AUDIO_FRAME_NUM 50     //定义最大音频解码缓存帧数(最多可以申请多少个block)


typedef enum
{
    KD_AUDIO_BIT_WIDTH_16 = 0, /* 16bit width */
    KD_AUDIO_BIT_WIDTH_24 = 1, /* 24bit width */
    KD_AUDIO_BIT_WIDTH_32 = 2, /* 32bit width */
} k_audio_bit_width;

typedef enum
{
    KD_AUDIO_SOUND_MODE_UNKNOWN = 0,
    KD_AUDIO_SOUND_MODE_MONO = 1, /* mono */
    KD_AUDIO_SOUND_MODE_STEREO = 2, /* stereo */
} k_audio_snd_mode;

typedef enum
{
    KD_I2S_IN_MONO_RIGHT_CHANNEL = 0,  //mic input
    KD_I2S_IN_MONO_LEFT_CHANNEL = 1,   //hp input
} k_i2s_in_mono_channel;

typedef enum
{
    KD_AUDIO_INPUT_TYPE_I2S  = 0,//i2s in
    KD_AUDIO_INPUT_TYPE_PDM  = 1,//pdm in
    KD_AUDIO_OUTPUT_TYPE_I2S = 2,//i2s out
} k_audio_type;

typedef enum
{
    KD_AUDIO_PDM_INPUT_OVERSAMPLE_32 = 0,
    KD_AUDIO_PDM_INPUT_OVERSAMPLE_64,
    KD_AUDIO_PDM_INPUT_OVERSAMPLE_128,
} k_audio_pdm_oversample;

typedef enum
{
    K_STANDARD_MODE = 1,
    K_RIGHT_JUSTIFYING_MODE = 2,
    K_LEFT_JUSTIFYING_MODE = 4
} k_i2s_work_mode;

typedef enum
{
    KD_AUDIO_PDM_IO_35_37 = 0,
    KD_AUDIO_PDM_IO_27_28 = 1,
} k_audio_pdm0_pdm1_io;

typedef enum
{
    K_AIO_I2STYPE_INNERCODEC = 0, /* AIO I2S connect inner audio CODEC */
    K_AIO_I2STYPE_EXTERN,/* AIO I2S connect extern hardware */
} k_aio_i2s_type;

typedef struct
{
    k_u32 chn_cnt; /* channle number on FS,i2s valid value:1/2,pdm valid value:1/2/3/4*/
    k_u32 sample_rate; /* sample rate */
    k_audio_bit_width bit_width; /* bit_width */
    k_audio_snd_mode snd_mode; /* momo or stereo */
    k_audio_pdm_oversample    pdm_oversample;
    k_u32 frame_num; /* frame num in buf[2,K_MAX_AUDIO_FRAME_NUM] */
    k_u32 point_num_per_frame;
} k_audio_pdm_attr;

typedef struct
{
    k_u32 chn_cnt; /* channle number on FS,i2s valid value:1/2,pdm valid value:1/2/3/4*/
    k_u32 sample_rate; /* sample rate */
    k_audio_bit_width bit_width; /* bit_width */
    k_audio_snd_mode snd_mode; /* momo or stereo */
    k_i2s_in_mono_channel  mono_channel;/* use mic input or headphone input */
    k_i2s_work_mode   i2s_mode;  /*i2s work mode*/
    k_u32 frame_num; /* frame num in buf[2,K_MAX_AUDIO_FRAME_NUM] */
    k_u32 point_num_per_frame;
    k_aio_i2s_type  i2s_type;/* i2s type */
} k_audio_i2s_attr;




typedef struct
{
    k_audio_type audio_type;/* audio type */
    k_bool avsync;
    union
    {
        k_audio_pdm_attr pdm_attr;
        k_audio_i2s_attr i2s_attr;
    } kd_audio_attr;
} k_aio_dev_attr;


typedef struct
{
    k_audio_bit_width bit_width; /* audio frame bit_width */
    k_audio_snd_mode snd_mode; /* audio frame momo or stereo mode */
    void  *virt_addr;
    k_u64  phys_addr;
    k_u64 time_stamp; /* audio frame time stamp */
    k_u32 seq; /* audio frame seq */
    k_u32 len; /* data lenth per channel in frame */
    k_u32 pool_id;
} k_audio_frame;

typedef struct
{
    void *stream; /* the virtual address of stream */
    k_u64 phys_addr; /* the physics address of stream */
    k_u32 len; /* stream lenth, by bytes */
    k_u64 time_stamp; /* frame time stamp */
    k_u32 seq; /* frame seq, if stream is not a valid frame,seq is 0 */
} k_audio_stream;

typedef struct
{
    k_bool aec_enable;
    k_u32  aec_echo_delay_ms;//speaker播出时间到mic录到的时间差(100-500ms)
    k_bool agc_enable;
    k_bool ans_enable;
}k_ai_vqe_enable;

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus */

#endif /*  __K_AUDIO_COMM_H__ */

