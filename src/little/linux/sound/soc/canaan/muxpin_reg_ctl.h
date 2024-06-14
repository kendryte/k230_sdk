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
#ifndef _MUXPIN_REG_CTL_H
#define _MUXPIN_REG_CTL_H

#include <linux/types.h>

#define IO_MAX_NUM 64

typedef enum muxpin_pull_e
{
    MUXPIN_PULL_NONE,      /*!< No Pull */
    MUXPIN_PULL_DOWN,      /*!< Pull Down */
    MUXPIN_PULL_UP,        /*!< Pull Up */
    MUXPIN_PULL_MAX        /*!< Count of pull settings */
} muxpin_pull_t;

typedef enum muxpin_driving_e
{
    MUXPIN_DRIVING_0,      /*!<  000 */
    MUXPIN_DRIVING_1,      /*!<  001 */
    MUXPIN_DRIVING_2,      /*!<  010 */
    MUXPIN_DRIVING_3,      /*!<  011 */
    MUXPIN_DRIVING_4,      /*!<  100 */
    MUXPIN_DRIVING_5,      /*!<  101 */
    MUXPIN_DRIVING_6,      /*!<  110 */
    MUXPIN_DRIVING_7,      /*!<  111 */
    MUXPIN_DRIVING_8,      /*!< 1000 */
    MUXPIN_DRIVING_9,      /*!< 1001 */
    MUXPIN_DRIVING_10,     /*!< 1010 */
    MUXPIN_DRIVING_11,     /*!< 1100 */
    MUXPIN_DRIVING_12,     /*!< 1101 */
    MUXPIN_DRIVING_13,     /*!< 1110 */
    MUXPIN_DRIVING_14,     /*!< 1111 */
} muxpin_driving_t;

typedef enum muxpin_io_voltage_e
{
    MUXPIN_IO_VOLTAGE_1V8 = 1,
    MUXPIN_IO_VOLTAGE_3V3 = 0,
} muxpin_io_voltage_t;


typedef struct mux_config
{
    uint32_t st : 1;
    /*!< Schmitt trigger. */
    uint32_t ds : 4;
    /*!< Driving selector. */
    uint32_t pd : 1;
    /*!< Pull down enable. 0 for nothing, 1 for pull down. */
    uint32_t pu : 1;
    /*!< Pull up enable. 0 for nothing, 1 for pull up. */
    uint32_t oe_en : 1;
    /*!< Static output enable. */
    uint32_t ie_en : 1;
    /*!< Static output enable. */
    uint32_t msc : 1;
    /*!< msc control bit. */
    uint32_t sl : 1;
    /*!< Slew rate control enable. */
    /*!< IO config setting. */
    uint32_t io_sel : 3;
    /*!< set io function mode. */
    uint32_t resv0 : 17;
    /*!< Reserved bits. */
    uint32_t pad_di : 1;
    /*!< Read current IO's data input. */
}  __attribute__((aligned(4)))  muxpin_config_t;

typedef enum muxpin_io_function
{
    MUXPIN_FUNCTION1 = 0x0,
    MUXPIN_FUNCTION2 = 0x1,
    MUXPIN_FUNCTION3 = 0x2,
    MUXPIN_FUNCTION4 = 0x3,
    MUXPIN_FUNCTION5 = 0x4,
} muxpin_io_function_t;

#define MUXPIN_NUM_IO    (64)
typedef struct _muxpin_t
{
    muxpin_config_t io[MUXPIN_NUM_IO];
    /*!< FPIOA GPIO multiplexer io array */
} __attribute__((aligned(4)))   muxpin_t ;



int muxpin_reg_init(void);
int muxpin_set_config(int io_num, muxpin_config_t config);
void muxpin_get_config(int io_num, muxpin_config_t *config);
int muxpin_set_io_pull(int io_num, muxpin_pull_t pull);
int muxpin_set_io_driving(int io_num, muxpin_driving_t driving);
int muxpin_set_function(int io_num, int function);
int muxpin_set_io_voltage(int io_num, muxpin_io_voltage_t voltage);

#endif


