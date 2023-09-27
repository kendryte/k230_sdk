/**
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

#ifndef __K_FFT_IOCTL_H__
#define __K_FFT_IOCTL_H__


#include "k_ioctl.h"
#include "../k_type.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


#define FFT_MAX_POINT 4096

typedef enum {
    FFT_N64 = 0,
    FFT_N128,
    FFT_N256,
    FFT_N512,
    FFT_N1024,
    FFT_N2048,
    FFT_N4096,
}fft_point_e ;

typedef enum  {
    FFT_MODE = 0,
    IFFT_MODE,
}k_fft_mode_e;

typedef enum {
    RIRI = 0,
    RRRR,
    RR_II,
} k_fft_input_mode_e;

typedef enum {
    RIRI_OUT = 0,
    RR_II_OUT,
} k_fft_out_mode_e;

typedef union
{
    struct {
        volatile fft_point_e point:3; //2:0  0:64;1:128;2:256;3:512;4:1024;5:2048;6:4096
        volatile k_fft_mode_e mode:1;  //3 0:fft 1:ifft
        volatile k_fft_input_mode_e im:2; //5:4 0:RIRI....;1:RRRR....（纯实部）;2:RRRR...IIII..
        volatile k_fft_out_mode_e om:1; //6 0:RIRI....;1:RRRR...IIII...
        volatile k_u64 fft_intr_mask : 1;//7 0:not mask intr; 1:mask intr
        volatile k_u16 shift:12; //19:8  [11]第12级右移使能.....[0]第一级右移使能
        volatile k_u32 fft_disable_cg : 1;//20 clock gating disable使能信号，write 1 disable fft clock gating
        volatile k_u32 reserv : 11 ;//31:21
        volatile k_u32 time_out:32;//63:32 表示fft使能后FFT模块计算超时的门限；该值写0表示不存在FFT超时上报中断功能
    }__attribute__ ((packed));
    volatile k_u64 cfg_value;
} __attribute__ ((packed)) k_fft_cfg_reg_st;


typedef struct {
    k_fft_cfg_reg_st reg;
    k_char rsv[4];
    k_u64 data[FFT_MAX_POINT*4/8]; // input and output;
}k_fft_args_st;

#define	KD_IOC_CMD_FFT_IFFT                 _IOWR(K_IOC_TYPE_DMA, 203, k_fft_args_st)



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif