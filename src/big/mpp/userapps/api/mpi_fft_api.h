/**
 * @file mpi_FFT_api.h
 * @author
 * @brief Defines APIs related to FFT device
 * @version 1.0
 * @date 2022-09-22
 *
 * @copyright
 * Copyright (c), Canaan Bright Sight Co., Ltd
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


#ifndef __MPI_FFT_API_H__
#define __MPI_FFT_API_H__

#include "k_type.h"
#include "k_fft_ioctl.h"

/**
 *  k230 硬件fft或者ifft接口，k230 fft模块特性如下
 *         支持64、128、256、512、1024、2048、4096点fft、ifft计算。
 *         支持int16计算精度，即输入输出的实部、虚部均为int16格式。
 *         支持标准的axi4 slave接口，参数配置与数据搬移均使用该接口。
 *         输入支持RIRI....、RRRR....（纯实部）、RRRR...IIII...格式排列，输出支持RIRI....、RRRR...IIII...格式排列。
 *         采用基2-时间抽取的计算方式，内部只有一个蝶形算子。
 *         采用单时钟域设计，总线时钟同时做为运算时钟，以节省跨时钟域的开销。
 *         4096点fft/ifft的计算时长控制在1ms以内，包括数据搬移、计算、中断交互的总开销。
 *         支持中断mask、原始中断查询
 */


/**
 * @brief fft或者ifft计算
 * 
 * @param fft_args 输出输出参数
 * @return int 0:ok other:failed
 */
int kd_mpi_fft_or_ifft(k_fft_args_st * fft_args);  

/**
 * @brief  fft_args初始化(辅助函数)
 * 
 * @param point  计算点数 64、128、256、512、1024、2048、4096
 * @param mode   FFT_MODE/IFFT_MODE 
 * @param im  RIRI/RRRR/RR_II
 * @param om   RIRI_OUT/RR_II_OUT
 * @param timeout 超时时间
 * @param shift  溢出偏移
 * @param dma_ch  使用的sdma通道号,0xff表示使用cpu搬移数据 0-3表示使用dma搬移的sdma通道号，其他值非法；
 * @param real  输入实部
 * @param imag  输入虚部
 * @param fft_args  要填充的 fft_args 变量
 * @return int  0：成功，other:失败
 */
int kd_mpi_fft_args_init( int point,           k_fft_mode_e mode,    k_fft_input_mode_e im, 
                          k_fft_out_mode_e om,    k_u32 timeout ,    k_u16 shift ,
                          short *real, short *imag, k_fft_args_st *fft_args );
/**
 * @brief 把fft输出转换成数组(辅助函数,方便打印)
 * 
 * @param fft_args fft输出结构体
 * @param rx  实部数据
 * @param iy  虚部数据
 * @return int   0:成功， other:失败
 */
int kd_mpi_fft_args_2_array(k_fft_args_st * fft_args, short *rx, short *iy);      



int kd_mpi_fft(int point , k_fft_input_mode_e im,  k_fft_out_mode_e om,
                k_u32 timeout , k_u16 shift ,         
                short *rx_in, short *iy_in, short *rx_out, short *iy_out);
int kd_mpi_ifft(int point , k_fft_input_mode_e im,  k_fft_out_mode_e om,
                k_u32 timeout , k_u16 shift ,         
                short *rx_in, short *iy_in, short *rx_out, short *iy_out);                
#endif 