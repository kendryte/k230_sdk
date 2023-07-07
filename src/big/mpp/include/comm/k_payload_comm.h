/**
 * @file k_payload_comm.h
 * @author
 * @brief payload type
 * @version 1.0
 * @date 2023-04-18
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

#ifndef __K_PAYLOAD_COMM_H__
#define __K_PAYLOAD_COMM_H__

#include "k_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /**<  __cplusplus */

typedef enum
{
    K_PT_PCMU = 0,
    K_PT_1016 = 1,
    K_PT_G721 = 2,
    K_PT_GSM = 3,
    K_PT_G723 = 4,
    K_PT_DVI4_8K = 5,
    K_PT_DVI4_16K = 6,
    K_PT_LPC = 7,
    K_PT_PCMA = 8,
    K_PT_G722 = 9,
    K_PT_S16BE_STEREO = 10,
    K_PT_S16BE_MONO = 11,
    K_PT_QCELP = 12,
    K_PT_CN = 13,
    K_PT_MPEGAUDIO = 14,
    K_PT_G728 = 15,
    K_PT_DVI4_3 = 16,
    K_PT_DVI4_4 = 17,
    K_PT_G729 = 18,
    K_PT_G711A = 19,
    K_PT_G711U = 20,
    K_PT_G726 = 21,
    K_PT_G729A = 22,
    K_PT_LPCM = 23,
    K_PT_CelB = 25,
    K_PT_JPEG = 26,
    K_PT_CUSM = 27,
    K_PT_NV = 28,
    K_PT_PICW = 29,
    K_PT_CPV = 30,
    K_PT_H261 = 31,
    K_PT_MPEGVIDEO = 32,
    K_PT_MPEG2TS = 33,
    K_PT_H263 = 34,
    K_PT_SPEG = 35,
    K_PT_MPEG2VIDEO = 36,
    K_PT_AAC = 37,
    K_PT_WMA9STD = 38,
    K_PT_HEAAC = 39,
    K_PT_PCM_VOICE = 40,
    K_PT_PCM_AUDIO = 41,
    K_PT_MP3 = 43,
    K_PT_ADPCMA = 49,
    K_PT_AEC = 50,
    K_PT_X_LD = 95,
    K_PT_H264 = 96,
    K_PT_D_GSM_HR = 200,
    K_PT_D_GSM_EFR = 201,
    K_PT_D_L8 = 202,
    K_PT_D_RED = 203,
    K_PT_D_VDVI = 204,
    K_PT_D_BT656 = 220,
    K_PT_D_H263_1998 = 221,
    K_PT_D_MP1S = 222,
    K_PT_D_MP2P = 223,
    K_PT_D_BMPEG = 224,
    K_PT_MP4VIDEO = 230,
    K_PT_MP4AUDIO = 237,
    K_PT_VC1 = 238,
    K_PT_JVC_ASF = 255,
    K_PT_D_AVI = 256,
    K_PT_DIVX3 = 257,
    K_PT_AVS = 258,
    K_PT_REAL8 = 259,
    K_PT_REAL9 = 260,
    K_PT_VP6 = 261,
    K_PT_VP6F = 262,
    K_PT_VP6A = 263,
    K_PT_SORENSON = 264,
    K_PT_H265 = 265,
    K_PT_VP8 = 266,
    K_PT_MVC = 267,
    K_PT_PNG = 268,
    K_PT_AMR = 1001,
    K_PT_MJPEG = 1002,
    K_PT_AMRWB = 1003,
    K_PT_PRORES = 1006,
    K_PT_OPUS = 1007,
    K_PT_BUTT
} k_payload_type;

typedef enum
{
    K_VPU_ROTATION_0 = 0,       /*no rotation*/
    K_VPU_ROTATION_90 = 1,      /*90 degree rotation*/
    K_VPU_ROTATION_180 = 2,     /*180 degree rotation*/
    K_VPU_ROTATION_270 = 3,     /*270 degree rotation*/
    K_VPU_ROTATION_BUTT
} k_rotation;

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif