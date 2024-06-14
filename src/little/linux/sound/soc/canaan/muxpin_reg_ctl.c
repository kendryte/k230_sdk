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

#include "muxpin_reg_ctl.h"
#include <linux/io.h>


/* IOMUX */
#define IOMUX_BASE_ADDR             (0x91105000U)

static volatile muxpin_t *muxpin = NULL;

int muxpin_reg_init(void)
{
    if (muxpin == NULL)
    {
        void __iomem *base;
        base = ioremap(IOMUX_BASE_ADDR, sizeof(muxpin_t));
	    if (IS_ERR(base))
		    return -1;

        muxpin = base;
    }

    return 0;
}



/**
 * @brief set io configuretion
 *
 * @param io_num  0 ~ 63
 * @param config  be careful
 * @return int
 */
int muxpin_set_config(int io_num, muxpin_config_t config)
{
    if (io_num > IO_MAX_NUM || io_num < 0)
    {
        //  kendryte_logi("io num error.\n");
        return -1;
    }

#if 1
    muxpin->io[io_num].st       = config.st;
    muxpin->io[io_num].ds       = config.ds;
    muxpin->io[io_num].pd       = config.pd;
    muxpin->io[io_num].pu       = config.pu;
    muxpin->io[io_num].oe_en    = config.oe_en;
    muxpin->io[io_num].ie_en    = config.ie_en;
    muxpin->io[io_num].msc      = config.msc;
    muxpin->io[io_num].sl       = config.sl;
    muxpin->io[io_num].io_sel   = config.io_sel;
    muxpin->io[io_num].pad_di   = config.pad_di;
#else
    muxpin->io[io_num] = config;
#endif
    return 0;
}

/**
 * @brief get io configuretion
 *
 * @param io_num  0 ~ 63
 * @param config  return value
 */
void muxpin_get_config(int io_num, muxpin_config_t *config)
{
    if (io_num > IO_MAX_NUM || io_num < 0)
    {
//        kendryte_logi("io num error.\n");
        return;
    }

    *config = muxpin->io[io_num];
}


/**
 * @brief set io pull up/down or none
 *
 * @param io_num  0 ~ 63
 * @param pull  enum value
 * @return int
 */
int muxpin_set_io_pull(int io_num, muxpin_pull_t pull)
{
    muxpin_config_t cfg;
    if (io_num < 0 || io_num >= MUXPIN_NUM_IO || pull >= MUXPIN_PULL_MAX)
        return -1;

    cfg = muxpin->io[io_num];
    switch (pull)
    {
    case MUXPIN_PULL_NONE:
        cfg.pu = 0;
        cfg.pd = 0;
        break;
    case MUXPIN_PULL_DOWN:
        cfg.pu = 0;
        cfg.pd = 1;
        break;
    case MUXPIN_PULL_UP:
        cfg.pu = 1;
        cfg.pd = 0;
        break;
    default:
        break;
    }

    muxpin->io[io_num] = cfg;
    return 0;
}


/**
 * @brief set io drive current
 *
 * @param io_num  0 ~ 63
 * @param driving enum value
 * @return int
 */
int muxpin_set_io_driving(int io_num, muxpin_driving_t driving)
{
    muxpin_config_t cfg;
    if (io_num > 1 && driving >= MUXPIN_DRIVING_7)
    {
        //     kendryte_logi("io %d can't configured driving 0x%x.\n",io_num,driving);
        return -1;
    }

    cfg = muxpin->io[io_num];
    cfg.ds = driving;

    muxpin->io[io_num] = cfg;
    return 0;
}


/**
 * @brief set io function
 *
 * @param io_num  0 ~ 63
 * @param function  check table for this number
 * @return int
 */
int muxpin_set_function(int io_num, int function)
{
    muxpin_config_t cfg;
    if (io_num > IO_MAX_NUM)
    {
        //      kendryte_logi("io num error.\n");
        return -1;
    }

    if (function > 5)
    {
//        kendryte_logi("please check your function num.\n");
        return -1;
    }

    cfg = muxpin->io[io_num];
    cfg.io_sel = function;

    muxpin->io[io_num] = cfg;
    return 0;
}


/**
 * @brief set io voltage
 *
 * @param io_num  0 ~ 63
 * @param voltage 1:1.8v  0:3.3v
 * @return int
 */
int muxpin_set_io_voltage(int io_num, muxpin_io_voltage_t voltage)
{
    if (io_num > IO_MAX_NUM)
    {
        //      kendryte_logi("error: please check your io num and voltage.\n");
        return -1;
    }

    if (io_num < 2 && voltage == 0)
    {
//       kendryte_logi("error: io %d not suport 3.3v.\n",io_num);
        return -1;
    }

    muxpin->io[io_num].msc = (MUXPIN_IO_VOLTAGE_1V8 == voltage) ? 1 : 0;
    return 0;
}






