#ifndef __CANAAN_I2S_WS2812_H__
#define __CANAAN_I2S_WS2812_H__


typedef struct _i2s_channel
{
    /* Left  Receive or Left Transmit Register      (0x20) */
    volatile unsigned int left_rxtx;
    /* Right Receive or Right Transmit Register     (0x24) */
    volatile unsigned int right_rxtx;
    /* Receive Enable Register                      (0x28) */
    volatile unsigned int rer;
    /* Transmit Enable Register                     (0x2c) */
    volatile unsigned int ter;
    /* Receive Configuration Register               (0x30) */
    volatile unsigned int rcr;
    /* Transmit Configuration Register              (0x34) */
    volatile unsigned int tcr;
    /* Interrupt Status Register                    (0x38) */
    volatile unsigned int isr;
    /* Interrupt Mask Register                      (0x3c) */
    volatile unsigned int imr;
    /* Receive Overrun Register                     (0x40) */
    volatile unsigned int ror;
    /* Transmit Overrun Register                    (0x44) */
    volatile unsigned int tor;
    /* Receive FIFO Configuration Register          (0x48) */
    volatile unsigned int rfcr;
    /* Transmit FIFO Configuration Register         (0x4c) */
    volatile unsigned int tfcr;
    /* Receive FIFO Flush Register                  (0x50) */
    volatile unsigned int rff;
    /* Transmit FIFO Flush Register                 (0x54) */
    volatile unsigned int tff;
    /* reserved                                (0x58-0x5c) */
    volatile unsigned int reserved1[2];
} __attribute__((packed, aligned(4))) i2s_channel_t;

typedef struct _i2s
{
    /* I2S Enable Register                          (0x00) */
    volatile unsigned int ier;
    /* I2S Receiver Block Enable Register           (0x04) */
    volatile unsigned int irer;
    /* I2S Transmitter Block Enable Register        (0x08) */
    volatile unsigned int iter;
    /* Clock Enable Register                        (0x0c) */
    volatile unsigned int cer;
    /* Clock Configuration Register                 (0x10) */
    volatile unsigned int ccr;
    /* Receiver Block FIFO Reset Register           (0x04) */
    volatile unsigned int rxffr;
    /* Transmitter Block FIFO Reset Register        (0x18) */
    volatile unsigned int txffr;
    /* reserved                                     (0x1c) */
    volatile unsigned int reserved1;
    volatile i2s_channel_t channel[4];
    /* reserved                               (0x118-0x1bc) */
    volatile unsigned int reserved2[40];
    /*  Receiver Block DMA Register                 (0x1c0) */
    volatile unsigned int rxdma;
    /* Reset Receiver Block DMA Register            (0x1c4) */
    volatile unsigned int rrxdma;
    /* Transmitter Block DMA Register               (0x1c8) */
    volatile unsigned int txdma;
    /* Reset Transmitter Block DMA Register         (0x1cc) */
    volatile unsigned int rtxdma;
    /* reserved                               (0x1d0-0x1ec) */
    volatile unsigned int reserved3[8];
    /* Component Parameter Register 2               (0x1f0) */
    volatile unsigned int i2s_comp_param_2;
    /* Component Parameter Register 1               (0x1f4) */
    volatile unsigned int i2s_comp_param_1;
    /* I2S Component Version Register               (0x1f8) */
    volatile unsigned int i2s_comp_version_1;
    /* I2S Component Type Register                  (0x1fc) */
    volatile unsigned int i2s_comp_type;
} __attribute__((packed, aligned(4))) i2s_t;

typedef enum _fifo_threshold
{
    /* Interrupt trigger when FIFO level is 1 */
    TRIGGER_LEVEL_1 = 0x0,
    /* Interrupt trigger when FIFO level is 2 */
    TRIGGER_LEVEL_2 = 0x1,
    /* Interrupt trigger when FIFO level is 3 */
    TRIGGER_LEVEL_3 = 0x2,
    /* Interrupt trigger when FIFO level is 4 */
    TRIGGER_LEVEL_4 = 0x3,
    /* Interrupt trigger when FIFO level is 5 */
    TRIGGER_LEVEL_5 = 0x4,
    /* Interrupt trigger when FIFO level is 6 */
    TRIGGER_LEVEL_6 = 0x5,
    /* Interrupt trigger when FIFO level is 7 */
    TRIGGER_LEVEL_7 = 0x6,
    /* Interrupt trigger when FIFO level is 8 */
    TRIGGER_LEVEL_8 = 0x7,
} i2s_fifo_threshold_t;

typedef enum _i2s_work_mode
{
    STANDARD_MODE = 1,
    RIGHT_JUSTIFYING_MODE = 2,
    LEFT_JUSTIFYING_MODE = 4
} i2s_work_mode_t;

typedef enum _word_length
{
    /* Ignore the word length */
    IGNORE_WORD_LENGTH = 0x0,
    /* 12-bit data resolution of the receiver */
    RESOLUTION_12_BIT = 0x1,
    /* 16-bit data resolution of the receiver */
    RESOLUTION_16_BIT = 0x2,
    /* 20-bit data resolution of the receiver */
    RESOLUTION_20_BIT = 0x3,
    /* 24-bit data resolution of the receiver */
    RESOLUTION_24_BIT = 0x4,
    /* 32-bit data resolution of the receiver */
    RESOLUTION_32_BIT = 0x5
} i2s_word_length_t;

typedef enum _word_select_cycles
{
    /* 16 sclk cycles */
    SCLK_CYCLES_16 = 0x0,
    /* 24 sclk cycles */
    SCLK_CYCLES_24 = 0x1,
    /* 32 sclk cycles */
    SCLK_CYCLES_32 = 0x2
} i2s_word_select_cycles_t;

typedef enum _sclk_gating_cycles
{
    /* Clock gating is diable */
    NO_CLOCK_GATING = 0x0,
    /* Gating after 12 sclk cycles */
    CLOCK_CYCLES_12 = 0x1,
    /* Gating after 16 sclk cycles */
    CLOCK_CYCLES_16 = 0x2,
    /* Gating after 20 sclk cycles */
    CLOCK_CYCLES_20 = 0x3,
    /* Gating after 24 sclk cycles */
    CLOCK_CYCLES_24 = 0x4
} i2s_sclk_gating_cycles_t;

typedef enum _i2s_transmit
{
    I2S_TRANSMITTER = 0,
    I2S_RECEIVER = 1
} i2s_transmit_t;


typedef enum
{
    AUDIO_OUT_TYPE_32BIT        = 0,
    AUDIO_OUT_TYPE_24BIT        = 1,
    AUDIO_OUT_TYPE_16BIT        = 2,
} audio_out_data_width_e;

typedef enum
{
    AUDIO_OUT_MODE_TDM          = 0,
    AUDIO_OUT_MODE_PDM          = 1,
    AUDIO_OUT_MODE_I2S          = 2,
} audio_out_mode_e;

#endif