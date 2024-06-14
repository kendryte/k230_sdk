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
#ifndef _CANAAN_AUDIO_H
#define _CANAAN_AUDIO_H
#include <linux/types.h>

typedef enum
{
    AUDIO_DISABLE               = 0,
    AUDIO_ENABLE                = 1,
} audio_enable_e;

typedef enum
{
    AUDIO_OUT_TYPE_32BIT        = 0,
    AUDIO_OUT_TYPE_24BIT        = 1,
    AUDIO_OUT_TYPE_16BIT        = 2,
} audio_out_data_width_e;

typedef enum
{
    AUDIO_OUT_24BIT_ALIGN_RIGTH = 0,
    AUDIO_OUT_24BIT_ALIGN_LEFT  = 1,
} audio_out_align_e;

typedef enum
{
    AUDIO_OUT_MODE_TDM          = 0,
    AUDIO_OUT_MODE_PDM          = 1,
    AUDIO_OUT_MODE_I2S          = 2,
} audio_out_mode_e;

typedef enum
{
    AUDIO_OUT_PDM_RISING        = 0,
    AUDIO_OUT_PDM_BOTH          = 1,
} audio_out_pdm_edge_e;

typedef enum
{
    AUDIO_PDM_OUT_CIC16         = 0,
    AUDIO_PDM_OUT_CIC32         = 1,
} audio_out_pdm_cic_e;

typedef enum
{
    AUDIO_PDM_OVERSAMPLE_32 = 0,
    AUDIO_PDM_OVERSAMPLE_64,
    AUDIO_PDM_OVERSAMPLE_128,
} audio_pdm_oversample_e;
typedef struct _audio_out_ctl
{
    uint32_t enable             : 1;        /* bit0:     0: disable 1:enable */
    uint32_t data_type          : 2;        /* bit1-2:   0: channel 32bit; 1:channel 24bit 2:channel 16bit */
    uint32_t align              : 1;        /* bit3:     0: 24bit right align;  1: 24bit left align */
    uint32_t loop_en            : 1;        /* bit4:     0: disable 1:enable */
    uint32_t mode               : 2;        /* bit5-6:   0: pcm 1:pdm 2:i2s */
    uint32_t fir2_out_bypass    : 1;        /* bit7:     0: enable  1:bypass  ENABLE for bypass*/
    uint32_t dma_enable         : 1;        /* bit8:     0: disable 1:enable */
    uint32_t channel_num        : 4;        /* bit9-12:  channel number */
    uint32_t dma_threshold      : 5;        /* bit13-17: dma threshold 16bit should be >= 2; 24/32bit should be >= 1 */
    uint32_t reserved           : 14;       /* bit18-31 */
} __attribute__((packed, aligned(4))) audio_out_ctl_s;

typedef struct _audio_out_rcv_fifo
{
    uint32_t audio_fifo_clear   : 1;
    uint32_t ref_fifo_th        : 5;
    uint32_t reserved           : 26;
} __attribute__((packed, aligned(4))) audio_out_rcv_fifo_s;

typedef struct _audio_out_tdm_conf
{
    uint32_t frame_sclk_delay   : 1;        /* bit0:    delay or not. 1: frame sync delay 1 cycle */
    uint32_t frame_sync_total   : 10;       /* bit1-10: how many sclks frame sync 32*channels */
    uint32_t frame_sync_high    : 10;       /* bit1-10: how many sclks frame sync high, 1~channel bit width */
    uint32_t reserved           : 11;       /* reserved */
} __attribute__((packed, aligned(4))) audio_out_tdm_conf_s;

typedef struct _audio_out_pmd_conf
{
    uint32_t reserved0                  : 4; /* bit0 */
    uint32_t pcm_sync_en                : 1; /* bit4:*/
    uint32_t pdm_spike_th               : 4; /* bit5-8:*/
    uint32_t pdm_ord_sel                : 1; /* bit9:    0 for 24bit * 44.1K cic16 = 2.8224MHz, 1for 24bit * 16K cic32
= 2.048MHz */
    uint32_t pdm_pn_sel                 : 1; /* bit10:   0 for rising edge , 1 for rising&failing edge*/
    uint32_t reserved1                   : 21; /* bit11-bit31: reserved*/
} __attribute__((packed, aligned(4))) audio_out_pmd_conf_s;

typedef struct _audio_out_condition
{
    uint32_t ref_out_fifo_rdy   : 1;        /* bit0: */
    uint32_t out_buf_full       : 1;        /* bit1: */
    uint32_t out_buf_empty      : 1;        /* bit2: */
    uint32_t out_rdy            : 1;        /* bit3: */
    uint32_t reserved           : 28;       /* bit4-bit32:*/
} __attribute__((packed, aligned(4))) audio_out_condition_s;

typedef struct _audio_filter_conf
{
    uint32_t loop_fir_bypass            : 1; /* bit0 */
    uint32_t pdm_cic_filter_bypass      : 1; /* bit1 bypass cic */
    uint32_t pdm_cic_mp_filter_bypass   : 1; /* bit2 bypass cic补偿 */
    uint32_t pdm_hbf_filter_bypass      : 1; /* bit3 bypass hbf */
    uint32_t reserved                   : 28;
} __attribute__((packed, aligned(4))) audio_filter_conf_s;


typedef struct _audio_iir_fir_conf
{
    uint32_t T_DST                      : 16;
    uint32_t resample_sel_bypass        : 3;
    uint32_t iir_scnt                   : 4;
    uint32_t iir_clr                    : 1;
    uint32_t reserved                   : 8;
} __attribute__((packed, aligned(4))) audio_iir_fir_conf_s;

typedef struct _audio_out_reg
{
    audio_out_ctl_s         audio_out_ctl;                  /* address:0x00 */
    audio_out_rcv_fifo_s    audio_out_fifo;                 /* address:0x04 */
    uint32_t                audio_out_reserved0[1];         /* address:0x08 */
    audio_out_tdm_conf_s    audio_out_tdm_conf;             /* address:0x0c */

    int                     audio_out_data;                 /* address:0x10 audio output register */
    uint32_t                audio_out_reserved1[5];         /* address:0x14 */
    uint32_t                audio_out_interrupt;            /* address:0x28 reserved for future use */
    uint32_t                audio_out_reserved2[1];
    audio_out_pmd_conf_s    audio_out_pdm_conf;             /* address:0x30 */
    uint32_t                audio_out_iir_matrix_a[4][4];   /* address:0x34 */
    uint32_t                audio_out_iir_matrix_b[4][2];   /* address:0x74 */
    uint32_t                audio_out_iir_matrix_c[4];      /* address:0x94 */
    uint32_t                audio_out_iir_matrix_d;         /* address:0xa4 */
    uint32_t                audio_out_ref2apb_data;         /* address:0xa8 loopback? echo & mix ?*/

    audio_out_condition_s   audio_out_condition;            /* address:0xac read only*/
    uint32_t                audio_out_reserved3[16];        /* address:0xb0 read only*/
    uint32_t                fir2_tap_coef[11];              /* address:0xf0 read only*/
    uint32_t                reserved1[2];                   /* address:0x11c-0x120*/
    uint32_t                fir2_tap_coef_11[9];            /* address:0x124-0x144*/
    uint32_t                reserved2;                      /* address:0x148*/
    uint32_t                fir2_tap_coef_20[12];           /* address:0x14c-0x178*/
    audio_filter_conf_s     audio_out_filter;               /* address:0x17c*/
    uint32_t                fir0_tap_coef[32];              /* address:0x180*/
    uint32_t                fir1_tap_coef[32];              /* address:0x260*/
    audio_iir_fir_conf_s    iir_fir_conf;                   /* address:0x340*/
    uint32_t                iir_x_parameter[3];             /* address:0x344*/
    uint32_t                pn_delay;                       /* address:0x350*/
} __attribute__((packed, aligned(4))) audio_out_reg_s;


/* --------------------------------------------------audio input
-----------------------------------------------------*/
/* pdm */
typedef enum
{
    AUDIO_IO_IN_MODE_PDM           = 0,//IO35、IO37:PDM in0和i2s out1复用的IO功能选择为i2s out1，PDM in1和i2s out0复用的IO功能选择为i2s out0；
    AUDIO_IO_OUT_MODE_I2S          = 2,//IO35、IO37:PDM in0和i2s out1复用的IO功能选择为PDM in0，PDM in1和i2s out0复用的IO功能选择为PDM in1。
} audio_io_select_mode_e;

typedef enum
{
    AUDIO_IN_LEFT_ALIGN         = 0,
    AUDIO_IN_RIGHT_ALIGN        = 1,
} audio_in_align_e;

typedef enum
{
    AUDIO_IN_PDM_BOTH           = 0,
    AUDIO_IN_PDM_RISING         = 2,
    AUDIO_IN_PDM_FAILING        = 3,
} audio_in_pdm_edge_e;

typedef enum
{
    AUDIO_IN_PDM_CIC16          = 0,
    AUDIO_IN_PDM_CIC32          = 1,
} audio_in_pdm_cic_e;

typedef struct _audio_in_pdm_conf_0
{
    uint32_t pdm_enable         : 1;    /* bit0: 1 enable, 0 disable */
    uint32_t mpf_enable         : 1;    /* bit2: cic 补偿滤波器 1 enable, 0 disable */
    uint32_t hbf_enable         : 1;    /* bit1: 1 enable, 0 disable */
    uint32_t pdm_in_endian      : 1;     /* bit3: pdm
input时读入的pdm转换后的pcm数据位于32bit数据的位置：0:高28bit(4bit通道号+24bit pcm数据) ,1:
低28bit(4bit通道号+24bit pcm数据) */
    uint32_t pdm_fifo_clear     : 1;    /* bit4: fifo clear */
    uint32_t audio_codec_bypass : 1;     /* bit5: bypass audio codec, use I2S directly to IO for digital I2S
microphone */
    uint32_t audio_in_mode      : 2;     /* bit76: 10: PDM in0和i2s out1复用的IO功能选择为i2s out1，PDM
in1和i2s out0复用的IO功能选择为i2s out0；
                                           其它：PDM in0和i2s out1复用的IO功能选择为PDM in0，PDM
in1和i2s out0复用的IO功能选择为PDM in1。*/
    uint32_t pdm_dma_fifo_th    : 5;    /* bit8-12: should as same as dma controller */
    uint32_t audio_dev_clk_sel  : 1;    /* bit13:audio_clk invert control signal */
    uint32_t pdm_clk_sel        : 1;    /* bit14:pdm_clk invert control signal*/
    uint32_t reserved2          : 1;    /* bit15: */
    uint32_t pdm_buf_rdy        : 1;    /* bit16: software check this bit and receive data */
    uint32_t reserved           : 15;   /* bit17 ~ bit31:reserved */
} __attribute__((packed, aligned(4))) audio_in_pdm_conf_0_s;

typedef struct _audio_in_pdm_conf_1
{
    uint32_t ord_select         : 1;    /* bit0: 0 for 16, 1 for 32 */
    uint32_t reserved           : 31;   /* bit1-31: */
} __attribute__((packed, aligned(4))) audio_in_pdm_conf_1_s;

typedef struct _audio_in_pdm_ch_conf
{
    uint32_t pdm_chl_num        : 4;    /* bit0-3: pdm input channel num*/
    uint32_t pdm_pn_sel         : 2;    /* bit4-5: 2'b10: posedge 2'b11: negedge others: double edge*/
    uint32_t reserved           : 26;
} __attribute__((packed, aligned(4))) audio_in_pdm_ch_conf_s;

/* TDM */
typedef struct _audio_in_tdm_conf_0
{
    uint32_t tdm_channel_mode   : 4;     /* bit0-3 : select tdm input channel number; 4'h0: 24 channel; 4'h1: 12; 4'h2:
8; 4'h3: 6; 4'h4: 4; 4'h5:3; 4'h6:2; 4'h7:1 */
    uint32_t tdm_width_mode     : 3;    /* 0: 32; 1:24；2:20; 3:16; 4:12 */
    uint32_t reserved1          : 4;    /* reserved1*/
    uint32_t tdm_clk_dly        : 1;     /* bit11, data delay one clock or not, frame_sync , 1表示在frame
sync后一拍出数据，0表示在frame sync当前拍出数据*/
    uint32_t tdm_in_endian      : 1;     /* bit12 0: 24/12 bit left align 1: 24/12 bit right align */
    uint32_t tdm_dma_fifo_th    : 5;    /* bit13-17:tdm fifo thershold set for making tdm dma request*/
    uint32_t reserved2          : 14;
} __attribute__((packed, aligned(4))) audio_in_tdm_conf_0_s;

typedef struct _audio_in_tdm_channel_cfg0
{
    uint32_t io0_channel_num    : 4; /* 2-24 channel */
    uint32_t io1_channel_num    : 4; /* 2-24 channel */
    uint32_t io2_channel_num    : 4; /* 2-24 channel */
    uint32_t io3_channel_num    : 4; /* 2-24 channel */
    uint32_t io4_channel_num    : 4; /* 2-24 channel */
    uint32_t io5_channel_num    : 4; /* 2-24 channel */
    uint32_t io6_channel_num    : 4; /* 2-24 channel */
    uint32_t io7_channel_num    : 4; /* 2-24 channel */
} __attribute__((packed, aligned(4))) audio_in_tdm_channel_cfg0_s;

typedef struct _audio_in_tdm_channel_cfg1
{
    uint32_t io8_channel_num    : 4; /* 2-24 channel */
    uint32_t io9_channel_num    : 4; /* 2-24 channel */
    uint32_t io10_channel_num   : 4; /* 2-24 channel */
    uint32_t io11_channel_num   : 4; /* 2-24 channel */
    uint32_t io12_channel_num   : 4; /* 2-24 channel */
    uint32_t io13_channel_num   : 4; /* 2-24 channel */
    uint32_t io14_channel_num   : 4; /* 2-24 channel */
    uint32_t io15_channel_num   : 4; /* 2-24 channel */
} __attribute__((packed, aligned(4))) audio_in_tdm_channel_cfg1_s;

typedef struct _audio_in_tdm_fsync_config
{
    uint32_t tdm_fsync_div_high : 10;
    uint32_t tdm_fsync_div_low  : 10;
    uint32_t reserved           : 12;
} __attribute__((packed, aligned(4)))  audio_in_tdm_fsync_config_s;

typedef struct _audio_in_tdm_ctl
{
    uint32_t tdm_intr           : 1; /* write 1 to clear */
    uint32_t audi_buf_full_n2   : 1; /* tdm fifo full */
    uint32_t audi_buf_rdy_n2    : 1; /* tdm fifo not empty, data could be read from fifo */
    uint32_t tdm_enable         : 1; /* enable tdm */
    uint32_t reserved           : 28;
} __attribute__((packed, aligned(4)))  audio_in_tdm_ctl_s;

/* audio in agc */
typedef struct _audio_in_agc_para_0
{
    uint32_t agc_gatt           : 16;
    uint32_t agc_headroom       : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para0_s;

typedef struct _audio_in_agc_para_1
{
    uint32_t agc_np_margin      : 16;
    uint32_t agc_gdth           : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para1_s;

typedef struct _audio_in_agc_para_2
{
    uint32_t agc_gainstep_slow  : 16;
    uint32_t agc_gainstep_fast  : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para2_s;

typedef struct _audio_in_agc_para_3
{
    uint32_t agc_frame_num      : 16;
    uint32_t agc_max_gainstep   : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para3_s;

typedef struct _audio_in_agc_para_4
{
    uint32_t agc_bypass         : 1;
    uint32_t agc_model2_en      : 1;
    uint32_t agc_model1_en      : 1;
    uint32_t agc_sd_sel         : 1;
    uint32_t agc_xpkth          : 16;
    uint32_t agc_nvak_num       : 6;
    uint32_t agc_npkobs         : 6;
} __attribute__((packed, aligned(4)))  auido_in_agc_para4_s;

typedef struct _audio_in_agc_para_5
{
    uint32_t agc_us_num         : 16;
    uint32_t agc_dpk            : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para5_s;

typedef struct _audio_in_agc_para_6
{
    uint32_t agc_ds_num         : 16;
    uint32_t agc_ub_num         : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para6_s;

typedef struct _audio_in_agc_para_7
{
    uint32_t agc_ku             : 16;
    uint32_t agc_db_num         : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para7_s;

typedef struct _audio_in_agc_para_8
{
    uint32_t agc_d              : 16;
    uint32_t agc_kd             : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para8_s;

typedef struct _audio_in_agc_para_9
{
    uint32_t agc_theta_1         : 16;
    uint32_t agc_theta_0         : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para9_s;

typedef struct _audio_in_agc_para_10
{
    uint32_t agc_d2b_num         : 16;
    uint32_t agc_d2s_num         : 16;
} __attribute__((packed, aligned(4)))  auido_in_agc_para10_s;

typedef struct _audio_in_agc_para_11
{
    uint32_t agc_d2b_num;
} __attribute__((packed, aligned(4)))  auido_in_agc_para11_s;

typedef struct _audio_in_iir_fir_ctl
{
    uint32_t    chl_num             : 3; /* 只能设置8个通道? */
    uint32_t    T_DST               : 16; /* I don't known */
    uint32_t    re_bypass           : 3; /* disable resample, fir, iir */
    uint32_t    iir_scnt            : 4; /* status move bit */
    uint32_t    iir_clear           : 1; /* iir clear */
    uint32_t    reserved            : 5;
} __attribute__((packed, aligned(4)))  audio_in_iir_fir_ctl_s;

typedef struct _audio_in_pdm_clk_spike
{
    /* debounce */
    uint32_t pdmin_clk_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_clk_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t reserved               : 28;
} __attribute__((packed, aligned(4)))  audio_in_pdm_clk_spike_s;

typedef struct _audio_in_pdm_ch_spike
{
    /* debounce */
    uint32_t pdmin_ch0_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch0_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t pdmin_ch1_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch1_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t pdmin_ch2_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch2_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t pdmin_ch3_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch3_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t pdmin_ch4_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch4_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t pdmin_ch5_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch5_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t pdmin_ch6_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch6_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
    uint32_t pdmin_ch7_spike_cnt    : 2; /* 0: 1 pclk cycle, 1: 2 pclk cycles, 2: 3 pclk cycles, 3: 4 pclk cycles*/
    uint32_t pdmin_ch7_spike_th     : 2; /* 0: 2 pclk cycle, 1: 3 pclk cycles, 2: 4 pclk cycles, 3: 5 pclk cycles */
} __attribute__((packed, aligned(4)))  audio_in_pdm_ch_spike_s;

typedef struct _audio_in_pdm_bypass
{
    uint32_t hbf_dsmpl_fir_bypass   : 1;
    uint32_t cic_mp_dsmpl_fir_bypass: 1;
    uint32_t cic_dsmple_fir_bypass  : 1;
    uint32_t reserved               : 29;
} __attribute__((packed, aligned(4))) audio_in_pdm_bypass_s;

typedef struct _audio_in_reg
{
    /* PDM reg */
    audio_in_pdm_conf_0_s       audio_in_pdm_conf_0;                /* address:0x00 */
    audio_in_pdm_conf_1_s       audio_in_pdm_conf_1;                /* address:0x04 0 for 24bit * 44.1K cic16= 2.
8224MHz, 1 for 24bit * 16K cic32= 2.048MHz */
    uint32_t                    audio_in_pdm_mp_para_tap_coef[16];  /* address:0x08 cic 补偿滤波系数 */
    uint32_t                    audio_in_pdm_hbf_para_tap_coef[29]; /* address:0x48 hbf 滤波器*/
    audio_in_pdm_ch_conf_s      audio_in_pdm_ch_conf;               /* address:0xbc */
    uint32_t                    audio_in_pdm2pcm_data;              /* address:0xc0 , read 24bit data
数据有效位28bit，其中[27:24]为数据对应的通道编号，[23:0]为实际pcm数据*/
    uint32_t                    audio_in_pdm_interrupt;             /* address:0xc4 , only bit0 valid,write 0 clear */

    /* TDM reg */
    audio_in_tdm_conf_0_s       audio_in_tdm_config;                /* address:0xc8 */
    audio_in_tdm_channel_cfg0_s audio_in_tdm_channel_cfg0;          /* address:0xcc */
    audio_in_tdm_channel_cfg1_s audio_in_tdm_channel_cfg1;          /* address:0xd0 */
    uint32_t                    reserved0;
    audio_in_tdm_fsync_config_s audio_in_tdm_fsync_config;          /* address:0xd8 */
    audio_in_tdm_ctl_s          audio_in_tdm_ctl;                   /* address:0xdc */
    uint32_t                    reserved1;                          /* address:0xe0 */
    uint32_t                    audio_in_tdm2apb_data;              /* address:0xe4 data read reg */
    uint32_t                    reserved2[2];                       /* address:0xe8 */

    /* For TDM/I2S/PDM */
    /* fir0 */
    uint32_t                    audio_in_fir0_tap_coef[11];         /* address:0xf0  fir before resample */
    uint32_t                    reserved3[2];                       /* address:0x11c-0x120 */
    uint32_t                    audio_in_fir0_tap_coef_11[9];       /* address:0x124 */
    uint32_t                    reserved4;                          /* address:0x148 */
    uint32_t                    audio_in_fir0_tap_coef_20[12];      /* address:0x14c */

    /* fir1 */
    uint32_t                    audio_in_fir1_tap_coef[3];          /* address:0x17c fir after resample */
    uint32_t                    reserved5;                          /* address:0x188 */
    uint32_t                    audio_in_fir1_tap_coef_3[29];       /* address:0x18c fir after resample */

    /* iir x parameter */
    uint32_t                    audio_in_iir_x_0_0_1;               /* address:0x200 , bit0-15 for 0, bit16-31 for 1 */
    uint32_t                    audio_in_iir_x_0_2;                 /* address:0x204 , bit0-15 valid */
    uint32_t                    audio_in_iir_y_0_0_1;               /* address:0x208 , bit0-15 for 0, bit16-31 for 1 */

    /* agc */
    auido_in_agc_para0_s        audio_in_agc_para_0;                /* address:0x20c*/
    auido_in_agc_para1_s        audio_in_agc_para_1;                /* address:0x210*/
    auido_in_agc_para2_s        audio_in_agc_para_2;                /* address:0x214*/
    auido_in_agc_para3_s        audio_in_agc_para_3;                /* address:0x218*/
    auido_in_agc_para4_s        audio_in_agc_para_4;                /* address:0x21c*/
    auido_in_agc_para5_s        audio_in_agc_para_5;                /* address:0x220*/
    auido_in_agc_para6_s        audio_in_agc_para_6;                /* address:0x224*/
    auido_in_agc_para7_s        audio_in_agc_para_7;                /* address:0x228*/
    auido_in_agc_para8_s        audio_in_agc_para_8;                /* address:0x22c*/
    auido_in_agc_para9_s        audio_in_agc_para_9;                /* address:0x230*/
    auido_in_agc_para10_s       audio_in_agc_para_10;               /* address:0x234*/
    auido_in_agc_para11_s       audio_in_agc_para_11;               /* address:0x238*/

    /* iir fir config */
    audio_in_iir_fir_ctl_s      audio_in_iir_fir_ctl;               /* address:0x23c*/

    /* Only for PDM */
    audio_in_pdm_clk_spike_s    audio_in_pdm_clk_spike;             /* address:0x240*/
    audio_in_pdm_ch_spike_s     audio_in_pdm_ch_spike;              /* address:0x244*/
    audio_in_pdm_bypass_s       audio_in_pdm_bypass;                /* address:0x248*/
} __attribute__((packed, aligned(4))) audio_in_reg_s;

typedef enum
{
    AUDIO_SAMPLE_8K         =   0,
    AUDIO_SAMPLE_16K        =   1,  /*PDM 16K(24bit)   --> HBF(高通+2上采样) --32K----> CIC 补偿(2上采样) --
64K-----> 4级CIC(32上采样)-->2.048MHz (24bit)(饱和处理)--->环路滤波-->2.048MHz (1bit) */
    AUDIO_SAMPLE_32K        =   2,
    AUDIO_SAMPLE_44D1K      =   3,  /*PDM 44.1K(24bit) --> HBF(高通+2上采样) --88.2K--> CIC 补偿(2上采样) --
176.4K--> 4级CIC(16上采样)-->2.8224MHz(24bit)(饱和处理)--->环路滤波-->2.8224MHz(1bit) */
    AUDIO_SAMPLE_48K        =   4,
} audio_sample_e;

//void audio_io3537_select_mode(audio_io_select_mode_e mode);//IO35,37引脚模式选择
void audio_i2s_in_init(void);
void audio_i2s_enable_audio_codec(bool use_audio_codec);//i2s选择使用内置audio codec

//pdm in操作寄存器
//void audio_pdm_in_init(audio_pdm_oversample_e pdm_oversample, uint32_t pdm_rx_num, audio_in_pdm_edge_e pdm_rx_edge, audio_in_align_e    pdm_rx_align);
//volatile uint32_t *audio_pdm_rx_dma_fifo();
//void  audio_pdm_fifo_overrun_clear();



void audio_i2s_out_init(bool enable, uint32_t word_len);

#endif

