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

#include<stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/ioctl.h"
#include "k_type.h"
#include "mpi_fft_api.h"
#include <time.h>



//#include "fft.h"
#include <math.h>
#include <stdio.h>

#define PI				3.14159265358979323846264338327950288419716939937510	//圆周率
extern int soft_fft_ifft_calc(int fft_n);


//input data
short i_real[FFT_MAX_POINT];//实数
short i_imag[FFT_MAX_POINT];//虚数

// //soft fft
// short soft_real[FFT_MAX_POINT];
// short soft_imag[FFT_MAX_POINT];
// //soft ifft
// short o_r_soft_ifft[FFT_MAX_POINT];
// short o_i_soft_ifft[FFT_MAX_POINT];

//hard fft
short o_h_real[FFT_MAX_POINT];
short o_h_imag[FFT_MAX_POINT];
//hard ifft
short o_h_ifft_real[FFT_MAX_POINT];
short o_h_ifft_imag[FFT_MAX_POINT];

struct timespec begain_time;
struct timespec fft_end;
struct timespec ifft_end;
    

int g_log_verbs = 1;

#define k230_log(out_loglevel, fmt, ...) \
    if ( g_log_verbs >= out_loglevel) \
        printf(fmt, ##__VA_ARGS__)  

#define K230_dbg(fmt, ...) \
     k230_log(7, fmt, ##__VA_ARGS__)
     


static void test_build_fft_org_data(int fft_num, short *rx, short *iy)
{
    int i=0;
    float tempf1[5];
    for (i = 0; i < fft_num; i++) {                                       //init struct
        tempf1[0] = 10 * cosf(2 * PI * i / fft_num);
        tempf1[1] = 20 * cosf(2 * 2 * PI * i / fft_num);
        tempf1[2] = 30 * cosf(3 * 2 * PI * i / fft_num);
        tempf1[3] = 0.2 * cosf(4 * 2 * PI * i / fft_num);
        tempf1[4] = 1000 * cosf(5 * 2 * PI * i / fft_num);
        rx[i] = tempf1[0] + tempf1[1] + tempf1[2] + tempf1[3] + tempf1[4];
        iy[i] = 0;                                                //imag to 0
    }
    // printf("point %d original data:\n", fft_num);
    // for (i = 0; i < fft_num; i++) { 
    //     printf("i=%02d=0x%04hx 0x%04hx\n",i,rx[i], iy[i]);
    // }
    return ;
}

static int display_calc_result(int point)
{
    int i=0;
    k_s16 max_diff_real=0, max_diff_real_index = 0;
    k_s16 max_diff_imag=0, max_diff_imag_index = 0;
    k_s16 diff_r,diff_i;
    k_u64 fft_time,ifft_time;

    k230_log(0,"-----fft ifft point %04hd  -------\n", point);

    for(i=0;i<point;i++){

        diff_r = abs(o_h_ifft_real[i] - i_real[i]);
        diff_i = abs(o_h_ifft_imag[i] - i_imag[i]);

        k230_log(2,"i=%04d real  hf %04hx hif %04hx org %04hx dif %04hx\n", \
                        i,  o_h_real[i],  o_h_ifft_real[i], i_real[i], diff_r);
        k230_log(2,"i=%04d imag  hf %04hx hif %04hx org %04hx dif %04hx\n", \
                        i,  o_h_imag[i],  o_h_ifft_imag[i], i_imag[i], diff_i);

        if(max_diff_real < diff_r){
            max_diff_real = diff_r;
            max_diff_real_index = i;
        }
            

        if(max_diff_imag < diff_i){
            max_diff_imag = diff_i;
            max_diff_imag_index = i;
        }
   
    }
    k230_log(1,"\tmax diff %04hx %04hx \n", max_diff_real, max_diff_imag);
    i = max_diff_real_index;
    k230_log(1,"\ti=%04d real  hf %04hx  hif %04hx org %04hx dif %04hx\n", \
                        i,  o_h_real[i], o_h_ifft_real[i], i_real[i], max_diff_real);
    i = max_diff_imag_index;                        
    k230_log(1,"\ti=%04d imag  hf %04hx  hif %04hx org %04hx dif %04hx\n", \
                    i,  o_h_imag[i],o_h_ifft_imag[i], i_imag[i], max_diff_imag);

    //fft_time = (fft_end.tv_sec *1000 + write_end.tv_nsec/1000000) - (begain_time.tv_sec *1000 + begain_time.tv_nsec/1000000);
    fft_time = (fft_end.tv_sec-begain_time.tv_sec)*1000000 + (fft_end.tv_nsec - begain_time.tv_nsec)/1000;
    ifft_time = (ifft_end.tv_sec-fft_end.tv_sec)*1000000 + (ifft_end.tv_nsec - fft_end.tv_nsec)/1000;
    
    if( (max_diff_real > 5 ) || (max_diff_real > 5 ) ){
        k230_log(0,"-----fft ifft point %04hd use %ld us result: error \n\n\n", point, fft_time);
    }else
        k230_log(0,"-----fft ifft point %04hd use %ld us %ld us result: ok \n\n\n", point, fft_time,ifft_time);
    return 0;
}
static int fft_test(int point)
{

    test_build_fft_org_data(point, i_real, i_imag);	

    //soft_fft_ifft_calc(point);
    clock_gettime(CLOCK_MONOTONIC, &begain_time);
    kd_mpi_fft(point,   RIRI,RR_II_OUT, 0, 0x555,  i_real, i_imag, o_h_real, o_h_imag);
    clock_gettime(CLOCK_MONOTONIC, &fft_end);
    kd_mpi_ifft(point,  RIRI,RR_II_OUT, 0, 0xaaa,  o_h_real, o_h_imag, o_h_ifft_real, o_h_ifft_imag);
    clock_gettime(CLOCK_MONOTONIC, &ifft_end);
    display_calc_result(point);

    return 0;
}
//
//dma
// void usage()
// {



// }
int main(int argc, char *argv[])
{
    if(argc >=2 )
        g_log_verbs = atoi(argv[1]);



    //int fft_num=64;
    fft_test(64);
    fft_test(128);
    fft_test(256);
    fft_test(512);
    fft_test(1024);
    fft_test(2048);
    fft_test(4096);
	return 0;
}
