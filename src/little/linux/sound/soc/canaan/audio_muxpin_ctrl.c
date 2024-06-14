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

#include "audio_muxpin_ctrl.h"
#include "muxpin_reg_ctl.h"


void  ai_i2s_muxpin_config(void)
{
    muxpin_reg_init();

#define AUDIO_I2S_ENABLE_CHANNEL_OUT0_IN1 0
#define AUDIO_I2S_CLK               32
#define AUDIO_I2S_WS                33
#define AUDIO_I2S_I2SIN1            36

#define AUDIO_I2S_I2SIN0            34

    muxpin_config_t config1 =
    {
        0x0, //st
        0x4, //ds
        0x0, //pd
        0x0, //pu
        0x1, //oe
        0x0, //ie
        0x1, //msc
        0x0, //sl
        0x2  //io_sel
    };

    muxpin_config_t config2 =
    {
        0x0, //st
        0x4, //ds
        0x0, //pd
        0x0, //pu
        0x0, //oe
        0x1, //ie
        0x1, //msc
        0x0, //sl
        0x2 //io_sel
    };


    //受限于硬件，I2S_0支持声音播放，I2S_1支持话音输入；
    //func3   out
    muxpin_set_config(AUDIO_I2S_CLK, config1);
    muxpin_set_config(AUDIO_I2S_WS, config1);

#if AUDIO_I2S_ENABLE_CHANNEL_OUT0_IN1
    //func3   in
    muxpin_set_config(AUDIO_I2S_I2SIN1, config2);

#else
    //func3     in
    muxpin_set_config(AUDIO_I2S_I2SIN0, config2);
#endif
}

void  ao_i2s_muxpin_config(void)
{
    muxpin_reg_init();
#define AUDIO_I2S_CLK               32
#define AUDIO_I2S_WS                33
#define AUDIO_I2S_I2SOUT0           35
#define AUDIO_I2S_I2SOUT1           37

    muxpin_config_t config1 =
    {
        0x0, //st
        0x4, //ds
        0x0, //pd
        0x0, //pu
        0x1, //oe
        0x0, //ie
        0x1, //msc
        0x0, //sl
        0x2  //io_sel
    };


    //受限于硬件，I2S_0支持声音播放，I2S_1支持话音输入；
    //func3   out
    muxpin_set_config(AUDIO_I2S_CLK, config1);
    muxpin_set_config(AUDIO_I2S_WS, config1);

    muxpin_set_config(AUDIO_I2S_I2SOUT0, config1);
    muxpin_set_config(AUDIO_I2S_I2SOUT1, config1);
}

