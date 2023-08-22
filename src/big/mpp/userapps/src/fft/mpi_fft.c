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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "k_type.h"
#include "k_dma_comm.h"
//#include "k_dma_ioctl.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/ioctl.h"
#include "k_type.h"
#include "mpi_fft_api.h"
#define DEV_NAME "/dev/fft_device"
#include <math.h>

#define ERET(A) {  if(A) \
    {printf("f=%s l=%d ret=%x \n", __func__, __LINE__, (A)); return (A); } }  



static void dump_buff(char *buff, int len)
{
    int i=0;
    printf("dump buff=%08lx len=%d", (k_u64)buff, len);
    for(i=0;i<len;i++)  {
        if(i%32==0) 
            printf("\n%08lx ",(k_u64)buff+i);

        printf("%02hx ",*(buff+i));    
    }    
    printf("\n");
}


/**
 * @brief 
 * 
 * @param point_num 
 * @param mode 
 * @param im 
 * @param om 
 * @param time_out 
 * @param shift 
 * @param real 
 * @param imag 
 * @return fft_cfg_reg_st* 
 */
static  fft_args_st * fft_prepare_data(int point_num , fft_mode_e mode, fft_input_mode_e im, \
                                    fft_out_mode_e om,  k_u32 time_out , k_u16 shift , k_u16 dma_ch, short *real, short *imag)
{
    k_u16 i;
    unsigned short *rx=(unsigned short *)real;
    unsigned short *iy=(unsigned short *)imag;
    k_u64 temp;
    k_u64 *buff = NULL;

    fft_args_st *fft_args = malloc(sizeof(fft_args_st));

    if(fft_args == NULL){
        printf("malloc  malloc_fft_info_and_build_data error \n");
        return NULL;
    }
    
    memset(fft_args, 0, sizeof(*fft_args));
    fft_args->reg.point=log2(point_num/64);
    fft_args->reg.mode=mode;
    fft_args->reg.im=im;
    fft_args->reg.om=om;
    fft_args->reg.time_out=time_out;
    fft_args->reg.shift=shift;
    fft_args->reg.fft_intr_mask = 0;
    fft_args->dma_channel = dma_ch;
    
    buff = &fft_args->data[0];

    //0x40 64bit---R[16]I[16]R[16]I[16]----小端模式；
    //I1R1I0R0
    if(RIRI == im)
    {
        for(i=0; i<point_num; i=i+2)
        {
            temp=((k_u64)iy[i+1] << 48)  | ((k_u64)rx[i+1] << 32) | ((k_u64)iy[i] << 16)  | (k_u64)rx[i];
            buff[i/2]=temp;           
        }
    }else if(RRRR == im)
    {//r3 r2 r1 r0
        for(i=0; i<point_num; i=i+4)
        {
            temp=((k_u64)rx[i+3] << 48)  | ((k_u64)rx[i+2] << 32) | ((k_u64)rx[i+1] << 16)  | (k_u64)rx[i];
            buff[i/4]=temp;
        }
    }
    else if(RR_II == im)
    {//r3 r2 r1 r0......i3 i2 i1 i0;
        int iy_start=point_num/4;
        for(i=0; i<point_num; i=i+4)
        {
            temp=((k_u64)rx[i+3] << 48)  | ((k_u64)rx[i+2] << 32) | ((k_u64)rx[i+1] << 16)  | (k_u64)rx[i];
            buff[i/4]=temp;
        }
        for(i=0; i<point_num; i=i+4)
        {
            temp=((k_u64)iy[i+3] << 48)  | ((k_u64)iy[i+2] << 32) | ((k_u64)iy[i+1] << 16)  | (k_u64)iy[i];
            buff[i/4 + iy_start ]=temp;
        }
    }

    //dump_buff((char*)fft_args, 64*4+16);
    return fft_args;
}
static int fft_post_proc(fft_args_st * fft_args, short *rx, short *iy)
{
    int i;
    k_u64 u64Data;
    fft_out_mode_e om = fft_args->reg.om;
    int point_num = 64 << fft_args->reg.point;
    k_u64 *fft_data_buff = fft_args->data;
    
    if(RIRI_OUT == om)
    {//r1 i1 r0 i0
        for (i = 0; i < point_num/2; i++) 
        {
            u64Data = fft_data_buff[i];
            rx[i*2]=u64Data & 0xffff;
            iy[i*2]=(u64Data >> 16) & 0xffff;
            rx[i*2+1]=u64Data >> 32 & 0xffff;
            iy[i*2+1]=(u64Data >> 48) & 0xffff;
        }
       
    }else if(RR_II_OUT == om)
    {//r3 r2 r1 r0......i3 i2 i1 i0
        for (i = 0; i < point_num/4; i++) 
        {
            u64Data = fft_data_buff[i];
            rx[i*4]=u64Data & 0xffff;
            rx[i*4+1]=(u64Data >> 16) & 0xffff;
            rx[i*4+2]=u64Data >> 32 & 0xffff;
            rx[i*4+3]=(u64Data >> 48) & 0xffff;
        }

        for (i = 0; i < point_num/4; i++) 
        {
            u64Data = fft_data_buff[i+point_num/4];
            iy[i*4]=u64Data & 0xffff;
            iy[i*4+1]=(u64Data >> 16) & 0xffff;
            iy[i*4+2]=u64Data >> 32 & 0xffff;
            iy[i*4+3]=(u64Data >> 48) & 0xffff;
        }
    }
    return 0;
}
//fft 信息，数据输入；数据输出；
static int hard_fft_or_ifft(fft_args_st * fft_args)
{

    int fd;
    int ret;	
    if(fft_args == NULL)
        return -1;
    fd = open(DEV_NAME, O_RDWR); ERET((fd<=0));
    ret = ioctl(fd, KD_IOC_CMD_FFT_IFFT, fft_args); ERET(ret);
    close(fd);
    return 0;
}


//64, IFFT_MODE, RIRI, RR_II_OUT, 0, 0x555
int k230_fft_ifft(int point_num , fft_mode_e mode, fft_input_mode_e im, 
           fft_out_mode_e om,  k_u32 time_out , k_u16 shift ,  k_u16 dma_ch,
           short *rx_input, short *iy_input, short *rx_out, short *iy_out)
{
    int ret = 0 ;
    fft_args_st * fft_args = NULL;   
    fft_args = fft_prepare_data( point_num, mode , \
                im, om, time_out, shift,dma_ch,  rx_input, iy_input); ERET((fft_args == NULL ));
                
    ret = hard_fft_or_ifft(fft_args); ERET(ret);
    ret = fft_post_proc(fft_args, rx_out, iy_out);ERET(ret);
    free(fft_args);
    return 0;
}
