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

#ifndef _INNO_K230_REG_H_
#define _INNO_K230_REG_H_
#include <linux/types.h>

typedef enum
{
    K_STANDARD_MODE = 1,
    K_RIGHT_JUSTIFYING_MODE = 2,
    K_LEFT_JUSTIFYING_MODE = 4
} k_i2s_work_mode;

typedef struct _codec_reg_0
{
    uint32_t sys_bstn        : 1;
    uint32_t digcore_bstn    : 1;
    uint32_t reserved        : 5;
    uint32_t bist_bstn       : 1;


    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_0_t;

typedef union _reg_0u {
    codec_reg_0_t reg_0;
    uint32_t reg_data;
} reg_0_t;



typedef struct _codec_reg_1
{
    uint32_t reserved       : 4;
    uint32_t dac_mute_sr    : 3;
    uint32_t dac_mute_en    : 1;


    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_1_t;

typedef union _reg_1u {
    codec_reg_1_t reg_1;
    uint32_t reg_data;
} reg_1_t;




typedef struct _codec_reg_2
{
    uint32_t i2s_tx_datsel : 2;
    uint32_t reserved      : 1;
    uint32_t i2s_tx_fmt    : 2;
    uint32_t i2s_tx_wl     : 2;
    uint32_t i2s_tx_lrp    : 1;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_2_t;

typedef union _reg_2u {
    codec_reg_2_t reg_2;
    uint32_t reg_data;
} reg_2_t;


typedef struct _codec_reg_3
{
    uint32_t i2s_tx_bclkinv : 1;
    uint32_t i2s_tx_rstn : 1;
    uint32_t i2s_tx_len : 2;
    uint32_t i2s_tx_func_mst : 1;
    uint32_t i2s_tx_pin_mst : 1;
    uint32_t i2s_rx_func_mst : 1;
    uint32_t i2s_rx_pin_mst : 1;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_3_t;

typedef union _reg_3u {
    codec_reg_3_t reg_3;
    uint32_t reg_data;
} reg_3_t;


typedef struct _codec_reg_4
{
    uint32_t reserved : 2;
    uint32_t i2s_lr_swap : 1;
    uint32_t i2s_rx_fmt : 2;
    uint32_t i2s_rx_wl : 2;
    uint32_t i2s_rx_lrp : 1;
    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_4_t;

typedef union _reg_4u {
    codec_reg_4_t reg_4;
    uint32_t reg_data;
} reg_4_t;



typedef struct _codec_reg_6
{
    uint32_t dac_vol : 8;
    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_6_t;

typedef union _reg_6u {
    codec_reg_6_t reg_6;
    uint32_t reg_data;
} reg_6_t;

typedef struct _codec_reg_7
{

    uint32_t reserved      : 4;
    uint32_t dacl_bist_sel : 2;
    uint32_t dacr_bist_sel : 2;
    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_7_t;

typedef union _reg_7u {
    codec_reg_7_t reg_7;
    uint32_t reg_data;
} reg_7_t;



typedef struct _codec_reg_8
{
    uint32_t adcl_vol : 8;
    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_8_t;

typedef union _reg_8u {
    codec_reg_8_t reg_8;
    uint32_t reg_data;
} reg_8_t;

typedef struct _codec_reg_9
{
    uint32_t adcr_vol : 8;
    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_9_t;

typedef union _reg_9u {
    codec_reg_9_t reg_9;
    uint32_t reg_data;
} reg_9_t;





typedef struct _codec_reg_20
{
    uint32_t gain_micbias : 3;
    uint32_t en_micbias   : 1;
    uint32_t en_ibias_dac : 1;
    uint32_t en_ibias_adc : 1;
    uint32_t en_vref      : 1;
    uint32_t reserved     : 1;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_20_t;

typedef union _reg_20u {
    codec_reg_20_t reg_20;
    uint32_t reg_data;
} reg_20_t;



typedef struct _codec_reg_21
{
    uint32_t sel_vref : 8;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_21_t;

typedef union _reg_21u {
    codec_reg_21_t reg_21;
    uint32_t reg_data;
} reg_21_t;


typedef struct _codec_reg_23
{
    uint32_t initial_micl : 1;
    uint32_t initial_alcl : 1;
    uint32_t initial_adcl : 1;
    uint32_t en_adcl      : 1;
    uint32_t en_clk_adcl  : 1;
    uint32_t en_alcl      : 1;
    uint32_t en_micl      : 1;
    uint32_t en_buf_adcl  : 1;
    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_23_t;

typedef union _reg_23u {
    codec_reg_23_t reg_23;
    uint32_t reg_data;
} reg_23_t;



typedef struct _codec_reg_24
{
    uint32_t gain_alcl : 5;
    uint32_t gain_micl : 2;
    uint32_t mute_micl : 1;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_24_t;

typedef union _reg_24u {
    codec_reg_24_t reg_24;
    uint32_t reg_data;
} reg_24_t;


typedef struct _codec_reg_25
{
    uint32_t gain_alcl_det0_bp : 1;
    uint32_t en_zerodet_adcl   : 1;
    uint32_t reserved          : 6;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_25_t;

typedef union _reg_25u {
    codec_reg_25_t reg_25;
    uint32_t reg_data;
} reg_25_t;


typedef struct _codec_reg_26
{
    uint32_t initial_micr : 1;
    uint32_t initial_alcr : 1;
    uint32_t initial_adcr : 1;
    uint32_t en_adcr      : 1;
    uint32_t en_clk_adcr  : 1;
    uint32_t en_alcr      : 1;
    uint32_t en_micr      : 1;
    uint32_t en_buf_adcr  : 1;


    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_26_t;

typedef union _reg_26u {
    codec_reg_26_t reg_26;
    uint32_t reg_data;
} reg_26_t;


typedef struct _codec_reg_27
{
    uint32_t gain_alcr : 5;
    uint32_t gain_micr : 2;
    uint32_t mute_micr : 1;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_27_t;

typedef union _reg_27u {
    codec_reg_27_t reg_27;
    uint32_t reg_data;
} reg_27_t;


typedef struct _codec_reg_28
{
    uint32_t gain_alcr_det0_bp : 1;
    uint32_t en_zerodet_adcr   : 1;
    uint32_t reserved          : 6;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_28_t;

typedef union _reg_28u {
    codec_reg_28_t reg_28;
    uint32_t reg_data;
} reg_28_t;


typedef struct _codec_reg_29
{
    uint32_t initial_dacl      : 1;
    uint32_t en_dacl           : 1;
    uint32_t en_clk_dacl       : 1;
    uint32_t en_vref_dacl      : 1;
    uint32_t pop_ctrl_dacl     : 2;
    uint32_t en_buf_dacl       : 1;
    uint32_t mute_hpoutl       : 1;


    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_29_t;

typedef union _reg_29u {
    codec_reg_29_t reg_29;
    uint32_t reg_data;
} reg_29_t;

typedef struct _codec_reg_2a
{
    uint32_t sel_hpoutl       : 4;
    uint32_t initial_hpoutl   : 1;
    uint32_t en_hpoutl        : 1;
    uint32_t reserved         : 2;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_2a_t;

typedef union _reg_2au {
    codec_reg_2a_t reg_2a;
    uint32_t reg_data;
} reg_2a_t;


typedef struct _codec_reg_2b
{
    uint32_t gain_hpoutl       : 5;
    uint32_t reserverd         : 3;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_2b_t;

typedef union _reg_2bu {
    codec_reg_2b_t reg_2b;
    uint32_t reg_data;
} reg_2b_t;




typedef struct _codec_reg_2d
{
    uint32_t sel_hpoutr       : 4;
    uint32_t initial_hpoutr   : 1;
    uint32_t en_hpoutr        : 1;
    uint32_t reserved         : 2;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_2d_t;

typedef union _reg_2du {
    codec_reg_2d_t reg_2d;
    uint32_t reg_data;
} reg_2d_t;



typedef struct _codec_reg_2c
{
    uint32_t initial_dacr      : 1;
    uint32_t en_dacr           : 1;
    uint32_t en_clk_dacr       : 1;
    uint32_t en_vref_dacr      : 1;
    uint32_t pop_ctrl_dacr     : 2;
    uint32_t en_buf_dacr       : 1;
    uint32_t mute_hpoutr       : 1;


    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_2c_t;

typedef union _reg_2cu {
    codec_reg_2c_t reg_2c;
    uint32_t reg_data;
} reg_2c_t;


typedef struct _codec_reg_2e
{
    uint32_t gain_hpoutr       : 5;
    uint32_t reserverd         : 3;

    /* Bits [31:8] is reseved */
    uint32_t resv : 24;
}__attribute__((packed, aligned(4)))  codec_reg_2e_t;

typedef union _reg_2eu {
    codec_reg_2e_t reg_2e;
    uint32_t reg_data;
} reg_2e_t;

typedef struct _audio_codec_reg
{
   volatile uint32_t                             reg_00;/* address:0x00 */
   volatile uint32_t                             reg_01;/* address:0x04 */
   volatile uint32_t                             reg_02;/* address:0x08 */
   volatile uint32_t                             reg_03;/* address:0x0c */
   volatile uint32_t                             reg_04;/* address:0x10 */
   volatile uint32_t                             reg_05;/* address:0x14 */
   volatile uint32_t                             reg_06;/* address:0x18 */
   volatile uint32_t                             reg_07;/* address:0x1c */
   volatile uint32_t                             reg_08;/* address:0x20 */
   volatile uint32_t                             reg_09;/* address:0x24 */
   volatile uint32_t                             reg_0a;/* address:0x28 */

   volatile uint32_t                             reg_reserved_0b_1f[21];/* address:0x04 */

   volatile uint32_t                             reg_20;/* address:0x80 */
   volatile uint32_t                             reg_21;/* address:0x84 */
   volatile uint32_t                             reg_22;/* address:0x88 */
   volatile uint32_t                             reg_23;/* address:0x8c */
   volatile uint32_t                             reg_24;/* address:0x90 */
   volatile uint32_t                             reg_25;/* address:0x04 */
   volatile uint32_t                             reg_26;/* address:0x98 */
   volatile uint32_t                             reg_27;/* address:0x9c */
   volatile uint32_t                             reg_28;/* address:0xa0 */
   volatile uint32_t                             reg_29;/* address:0xa4 */
   volatile uint32_t                             reg_2a;/* address:0xa8 */
   volatile uint32_t                             reg_2b;/* address:0xac */
   volatile uint32_t                             reg_2c;/* address:0xb0 */
   volatile uint32_t                             reg_2d;/* address:0xb4 */
   volatile uint32_t                             reg_2e;/* address:0xb8 */

   volatile uint32_t                             reg_reserved_2f_3f[17];/* address:0x04 */

   //The register's address which related to the ALCL function arrange from 0x40~0x4c.
   volatile uint32_t                             reg_40;/* address:0x100 */
   volatile uint32_t                             reg_41;/* address:0x104 */
   volatile uint32_t                             reg_42;/* address:0x108 */
   volatile uint32_t                             reg_43;/* address:0x10c */
   volatile uint32_t                             reg_44;/* address:0x110*/
   volatile uint32_t                             reg_45;/* address:0x114 */
   volatile uint32_t                             reg_46;/* address:0x118 */
   volatile uint32_t                             reg_47;/* address:0x11c */
   volatile uint32_t                             reg_48;/* address:0x120 */
   volatile uint32_t                             reg_49;/* address:0x124 */
   volatile uint32_t                             reg_4a;/* address:0x128 */
   volatile uint32_t                             reg_4b;/* address:0x12c */
   volatile uint32_t                             reg_4c;/* address:0x130 */
   volatile uint32_t                             reg_4d;/* address:0x134 */
   volatile uint32_t                             reg_4e;/* address:0x138 */
   volatile uint32_t                             reg_4f;/* address:0x13c */

   //The register's address which related to the ALCR function arrange from 0x50~0x5c.
   volatile uint32_t                             reg_50;/* address:0x140 */
   volatile uint32_t                             reg_51;/* address:0x144 */
   volatile uint32_t                             reg_52;/* address:0x148 */
   volatile uint32_t                             reg_53;/* address:0x14c */
   volatile uint32_t                             reg_54;/* address:0x150 */
   volatile uint32_t                             reg_55;/* address:0x154 */
   volatile uint32_t                             reg_56;/* address:0x158 */
   volatile uint32_t                             reg_57;/* address:0x15c */
   volatile uint32_t                             reg_58;/* address:0x160 */
   volatile uint32_t                             reg_59;/* address:0x164 */
   volatile uint32_t                             reg_5a;/* address:0x168 */
   volatile uint32_t                             reg_5b;/* address:0x16c */
   volatile uint32_t                             reg_5c;/* address:0x170 */

}__attribute__((packed, aligned(4))) audio_codec_reg_s;

int    audio_codec_reg_init(void* reg_base);
void   audio_codec_powerup_init(void);
void   audio_codec_adc_init(k_i2s_work_mode mode,uint32_t i2s_ws);//i2s_ws最大24，设置32bit仍以24bit工作
void   audio_codec_dac_init(k_i2s_work_mode mode,uint32_t i2s_ws);//i2s_ws最大24，设置32bit仍以24bit工作

int  audio_codec_adc_set_micl_gain(int gain);
int  audio_codec_adc_set_micr_gain(int gain);
int  audio_codec_adc_get_micl_gain(int* gain);
int  audio_codec_adc_get_micr_gain(int* gain);

// int  audio_codec_adcl_set_volume(float volume);
// int  audio_codec_adcr_set_volume(float volume);
// int  audio_codec_adcl_get_volume(float* volume);
// int  audio_codec_adcr_get_volume(float* volume);

int  audio_codec_adc_micl_mute(bool mute);
int  audio_codec_adc_micr_mute(bool mute);

int  audio_codec_adc_get_micl_mute(bool* mute);
int  audio_codec_adc_get_micr_mute(bool* mute);

// int  audio_codec_alc_set_micl_gain(float gain);
// int  audio_codec_alc_set_micr_gain(float gain);
// int  audio_codec_alc_get_micl_gain(float* gain);
// int  audio_codec_alc_get_micr_gain(float* gain);

int  audio_codec_dac_set_hpoutl_gain(int gain);
int  audio_codec_dac_set_hpoutr_gain(int gain);
int  audio_codec_dac_get_hpoutl_gain(int* gain);
int  audio_codec_dac_get_hpoutr_gain(int* gain);

int  audio_codec_dac_hpoutl_mute(bool mute);
int  audio_codec_dac_hpoutr_mute(bool mute);

int  audio_codec_dac_get_hpoutl_mute(bool* mute);
int  audio_codec_dac_get_hpoutr_mute(bool* mute);

// int  audio_codec_dacl_set_volume(float volume);
// int  audio_codec_dacr_set_volume(float volume);
// int  audio_codec_dacl_get_volume(float* volume);
// int  audio_codec_dacr_get_volume(float* volume);

int  audio_codec_reset(void);

int  audio_codec_adc_hp_work(bool work);//Reset headphone input

#endif


