#include "inno_k230_reg.h"
#include <linux/module.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/of.h>
#include <linux/delay.h>


typedef struct
{
    unsigned int gain_micl;
    unsigned int gain_micr;
    unsigned int adc_volumel;
    unsigned int adc_volumer;

    unsigned int gain_alcl;
    unsigned int gain_alcr;

    unsigned int gain_hpoutl;
    unsigned int gain_hpoutr;
    unsigned int dac_volumel;
    unsigned int dac_volumer;
}acodec_sound_values;

static acodec_sound_values g_snd_default_values;
static acodec_sound_values g_snd_current_values;

#define AUDIO_CODEC_BASE_ADDR (0x9140E000U)
#define AUDIO_ADC_MIX 0
#define AUDIO_ENABLE_BIST 0
#define AUDIO_MODIFY_ADC_MIC_DB 1 // 是否修改MIC db值(默认0db)
#define AUDIO_MODIFY_ADC_ALC_DB 1 // 是否修改ALC db值(默认0db)

#define AUDIO_MODIFY_DAC_DB 1

#define AUDIO_REG_CONFIG_NORMAL_DELAY 1 // 1ms
#define AUDIO_REG_CONFIG_LONG_DELAY  20 // 20ms
#define AUDIO_REG_CONFIG_LONG2_DELAY  10 // 10ms
//powerup 配置中sel_vref每次配置都要延迟20ms,adc的step1/step2，dac的step1延迟10ms，其他延迟1ms

static volatile audio_codec_reg_s *audio_codec_reg = NULL;
//tatic rt_thread_t                g_check_hp_thread = 0;
static bool g_adc_left_mute = false;
static bool g_adc_right_mute = false;
static bool g_dac_left_mute = false;
static bool g_dac_right_mute = false;

static int _reset_snd_values(acodec_sound_values* snd_values )
{
    snd_values->gain_micl = 0x03;
    snd_values->gain_micr = 0x03;
    snd_values->adc_volumel = 0xc3;
    snd_values->adc_volumer = 0xc3;

#if 0
    snd_values->gain_alcl = 0x1f;
    snd_values->gain_alcr = 0x1f;
#else
    snd_values->gain_alcl = 0x12;
    snd_values->gain_alcr = 0x12;
#endif

    snd_values->gain_hpoutl = 0x10;
    snd_values->gain_hpoutr = 0x10;
    snd_values->dac_volumel = 0xf1;
    snd_values->dac_volumer = 0xf1;

    return 0;
}

static int _reset_adc_sound(void)
{
    reg_24_t reg24;
    reg_27_t reg27;
    reg_8_t reg8;
    reg_9_t reg9;

    reg24.reg_data = readl(&audio_codec_reg->reg_24);
    reg24.reg_24.gain_micl = g_snd_default_values.gain_micl;
    writel(reg24.reg_data, &audio_codec_reg->reg_24);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg27.reg_data = readl(&audio_codec_reg->reg_27);
    reg27.reg_27.gain_micr = g_snd_default_values.gain_micr;
    writel(reg27.reg_data, &audio_codec_reg->reg_27);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg24.reg_data = readl(&audio_codec_reg->reg_24);
    reg24.reg_24.gain_alcl = g_snd_default_values.gain_alcl;
    writel(reg24.reg_data, &audio_codec_reg->reg_24);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg27.reg_data = readl(&audio_codec_reg->reg_27);
    reg27.reg_27.gain_alcr = g_snd_default_values.gain_alcr;
    writel(reg27.reg_data, &audio_codec_reg->reg_27);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg8.reg_data = readl(&audio_codec_reg->reg_08);
    reg8.reg_8.adcl_vol = g_snd_default_values.adc_volumel;
    writel(reg8.reg_data, &audio_codec_reg->reg_08);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg9.reg_data = readl(&audio_codec_reg->reg_09);
    reg9.reg_9.adcr_vol = g_snd_default_values.adc_volumer;
    writel(reg9.reg_data, &audio_codec_reg->reg_09);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);


    return 0;
}

static int _reset_dac_sound(void)
{
    reg_2b_t reg2b;
    reg_2e_t reg2e;
    reg_6_t reg6;

    reg2b.reg_data = readl(&audio_codec_reg->reg_2b);
    reg2b.reg_2b.gain_hpoutl = g_snd_default_values.gain_hpoutl;
    writel(reg2b.reg_data, &audio_codec_reg->reg_2b);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg2e.reg_data = readl(&audio_codec_reg->reg_2e);
    reg2e.reg_2e.gain_hpoutr = g_snd_default_values.gain_hpoutr;
    writel(reg2e.reg_data, &audio_codec_reg->reg_2e);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg6.reg_data = readl(&audio_codec_reg->reg_06);
    reg6.reg_6.dac_vol = g_snd_default_values.dac_volumel;
    reg6.reg_6.dac_vol = g_snd_default_values.dac_volumer;
    writel(reg6.reg_data, &audio_codec_reg->reg_06);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    return 0;
}

int audio_codec_reg_init(void* reg_base)
{
    if (audio_codec_reg == NULL)
    {
        audio_codec_reg = reg_base;
    }
    _reset_snd_values(&g_snd_default_values);
    _reset_snd_values(&g_snd_current_values);
    return 0;
}

static volatile bool g_powerup_init = false;
void audio_codec_powerup_init(void)
{
    reg_29_t reg29;
    reg_2c_t reg2c;
    reg_21_t reg21;
    reg_20_t reg20;
    reg_0_t reg0;
    int i = 0;

    if (g_powerup_init)
    {
        return;
    }
    g_powerup_init = true;

    // 0.reset the audio codec
    reg0.reg_data = readl(&audio_codec_reg->reg_00);
    reg0.reg_0.sys_bstn = 0;
    reg0.reg_0.digcore_bstn = 0;
#if AUDIO_ENABLE_BIST
    reg0.reg_0.bist_bstn = 0;
#endif
    writel(reg0.reg_data, &audio_codec_reg->reg_00);
    // printf("===========reset audio codec\n");
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg0.reg_data = readl(&audio_codec_reg->reg_00);
    reg0.reg_0.sys_bstn = 1;
// reg0.reg_0.digcore_bstn = 1;
#if AUDIO_ENABLE_BIST
    reg0.reg_0.bist_bstn = 1;
#endif
    writel(reg0.reg_data, &audio_codec_reg->reg_00);
    // printf("===========work audio codec\n");
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 1.Configure the register POP_CTRL_DACL[1:0] 0x29[5:4] to 2’b01, , to setup the output dc voltage of DAC left channel.
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.pop_ctrl_dacl = 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 1.Configure the register POP_CTRL_DACR[1:0] 0x2c[5:4] to 2’b01, , to setup the output dc voltage of DAC right channel.
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.pop_ctrl_dacr = 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 2.Configure the register SEL_VREF[7:0] reg0x21[7:0] to 8’b000_0001.
    reg21.reg_data = readl(&audio_codec_reg->reg_21);
    reg21.reg_21.sel_vref = 1;
    writel(reg21.reg_data, &audio_codec_reg->reg_21);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 3.Supply the power of the analog part .

    // 4.Configure the register EN_VREF reg0x20[6] to 1 to setup reference voltage
    reg20.reg_data = readl(&audio_codec_reg->reg_20);
    reg20.reg_20.en_vref = 1;
    writel(reg20.reg_data, &audio_codec_reg->reg_20);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 5.Change the register SEL_VREF[7:0] reg0x21[7:0] :configure the reg0x21[7:0] to 7’b1111_1111 directly.
#if 0
    reg21.reg_data = readl(&audio_codec_reg->reg_21);
    reg21.reg_21.sel_vref = 0xff;
    writel(reg21.reg_data,&audio_codec_reg->reg_21);
    msleep(20);
#else
    for (i = 0; i < 8; i++)
    {
        reg21.reg_data = readl(&audio_codec_reg->reg_21);
        if (0 == i)
        {
            reg21.reg_21.sel_vref = 0x1;
        }
        else if (1 == i)
        {

            reg21.reg_21.sel_vref = 0x3;
        }
        else if (2 == i)
        {

            reg21.reg_21.sel_vref = 0x7;
        }
        else if (3 == i)
        {

            reg21.reg_21.sel_vref = 0xf;
        }
        else if (4 == i)
        {

            reg21.reg_21.sel_vref = 0x1f;
        }
        else if (5 == i)
        {

            reg21.reg_21.sel_vref = 0x3f;
        }
        else if (6 == i)
        {

            reg21.reg_21.sel_vref = 0x7f;
        }
        else if (7 == i)
        {

            reg21.reg_21.sel_vref = 0xff;
        }
        writel(reg21.reg_data, &audio_codec_reg->reg_21);
        msleep(AUDIO_REG_CONFIG_LONG_DELAY);
    }

#endif

    // 6.Wait until the voltage of VCM keeps stable at the AVDD/2.

    // 7.Configure the register SEL_VREF[7:0] reg0x21[7:0] to the appropriate value(except 7’b0000_00000) for reducing power
    reg21.reg_data = readl(&audio_codec_reg->reg_21);
    reg21.reg_21.sel_vref = 0x02;
    writel(reg21.reg_data, &audio_codec_reg->reg_21);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    msleep(2000); //delay  to prevent ADC from generating noise
}


static int  audio_reset_micbias(void)
{
    int delay_milli_sec = 100;
    //acodec_err_trace("=========audio_reset_micbias:%d ms\n",delay_milli_sec);
    reg_20_t reg20;
    reg20.reg_data = readl(&audio_codec_reg->reg_20);
    reg20.reg_20.en_micbias = 0;
    writel(reg20.reg_data, &audio_codec_reg->reg_20);
    msleep(delay_milli_sec);

    reg20.reg_data = readl(&audio_codec_reg->reg_20);
    reg20.reg_20.en_micbias = 1;
    writel(reg20.reg_data, &audio_codec_reg->reg_20);

    return 0;
}

// #define ACODEC_ADC_CHANNEL 5
// static bool  g_hp_insert_det = false;
// static void _check_adc_hp_thread(void *parameter)
// {
//     rt_adc_device_t adc_dev;
//     rt_uint32_t reg_value = 0;

//     while(1)
//     {
//         adc_dev = (rt_adc_device_t)rt_device_find("adc");
//         if (adc_dev == RT_NULL)
//         {
//             acodec_err_trace("rt_device_find adc failed\n");
//             msleep(500);
//             continue;
//         }
//         rt_adc_enable(adc_dev, ACODEC_ADC_CHANNEL);
//         break;
//     }

//     bool  first_check = true;
//     while(1)
//     {
//         reg_value = rt_adc_read(adc_dev, ACODEC_ADC_CHANNEL);
//         if (reg_value > 100)
//         {
//             if (first_check)
//             {
//                 g_hp_insert_det = true;
//                 audio_codec_adc_hp_work(g_hp_insert_det);
//                 first_check = false;
//             }
//             else
//             {
//                 if (!g_hp_insert_det)
//                 {
//                     g_hp_insert_det = true;
//                     audio_codec_adc_hp_work(g_hp_insert_det);
//                 }
//             }
//         }
//         else
//         {
//             if (first_check)
//             {
//                 g_hp_insert_det = false;
//                 audio_codec_adc_hp_work(g_hp_insert_det);
//                 first_check = false;
//             }
//             else
//             {
//                 if (g_hp_insert_det)
//                 {
//                     g_hp_insert_det = false;
//                     audio_codec_adc_hp_work(g_hp_insert_det);
//                 }
//             }
//         }
//         msleep(500);
//     }
// }

static volatile bool g_adc_sound_init = false;
void audio_codec_adc_init(k_i2s_work_mode mode, unsigned int i2s_ws)
{
    reg_24_t reg24;
    reg_27_t reg27;
    reg_20_t reg20;
    reg_23_t reg23;
    reg_26_t reg26;
    reg_25_t reg25;
    reg_28_t reg28;
    reg_4_t reg4;
    //reg_8_t reg8;
    //reg_9_t reg9;
    //reg_2_t reg2;
    reg_0_t reg0;

    // 1.Configure the register MUTE_MICL 0x24[7] to 1, to end the mute station of the left ADC channel.
    reg24.reg_data = readl(&audio_codec_reg->reg_24);
    reg24.reg_24.mute_micl = 1;
    writel(reg24.reg_data, &audio_codec_reg->reg_24);
    msleep(AUDIO_REG_CONFIG_LONG2_DELAY);

    // 1.Configure the register MUTE_MICR 0x27[7] to 1, to end the mute station of the left ADC channel
    reg27.reg_data = readl(&audio_codec_reg->reg_27);
    reg27.reg_27.mute_micr = 1;
    writel(reg27.reg_data, &audio_codec_reg->reg_27);
    msleep(AUDIO_REG_CONFIG_LONG2_DELAY);

    // 2.Configure the register EN_IBIAS_ADC 0x20[5] to 1, to enable the current source of ADC.
    reg20.reg_data = readl(&audio_codec_reg->reg_20);
    reg20.reg_20.en_ibias_adc = 1;
    reg20.reg_20.en_micbias = 1;
    writel(reg20.reg_data, &audio_codec_reg->reg_20);
    msleep(AUDIO_REG_CONFIG_LONG2_DELAY);

    // 3.Configure the register EN_BUF_ADCL 0x23[7] to 1, to enable the reference voltage buffer in ADC left channel.
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.en_buf_adcl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 3.Configure the register EN_BUF_ADCR 0x26[7] to 1, to enable the reference voltage buffer in ADC right channel.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.en_buf_adcr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 4.Configure the register EN_MICL 0x23[6] to 1, to enable the MIC module in ADC left channel.
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.en_micl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 4.Configure the register EN_MICR 0x26[6] to 1, to enable the MIC module in ADC right channel.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.en_micr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 5.Configure the register EN_ALCL 0x23[5] to 1, to enable the ALC module in ADC left channel
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.en_alcl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 5.Configure the register EN_ALCR 0x26[5] to 1, to enable the ALC module in ADC right channel.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.en_alcr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 6.Configure the register EN_CLK_ADCL 0x23[4] to 1, to enable the clock module in ADC left channel.
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.en_clk_adcl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 6.Configure the register EN_CLK_ADCR 0x26[4] to 1, to enable the clock module in ADC right channel.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.en_clk_adcr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 7.Configure the register EN_ADCL 0x23[3] to 1, to enable the ADC module in ADC left channel.
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.en_adcl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 7.Configure the register EN_ADCR 0x26[3] to 1, to enable the ADC module in ADC right channel.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.en_adcr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // i2s rx word length

    reg4.reg_data = readl(&audio_codec_reg->reg_04);
    if (i2s_ws == 24)
    {
        // reg4.reg_4.i2s_rx_wl = 3;//32
        reg4.reg_4.i2s_rx_wl = 2; // 24
    }
    else if (i2s_ws == 16)
    {
        reg4.reg_4.i2s_rx_wl = 0; // 16
    }
    else if (i2s_ws == 32)
    {
        reg4.reg_4.i2s_rx_wl = 3;//32
    }

    writel(reg4.reg_data, &audio_codec_reg->reg_04);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // i2s rx fmt
    reg4.reg_data = readl(&audio_codec_reg->reg_04);
    if (K_STANDARD_MODE == mode)
    {
        reg4.reg_4.i2s_rx_fmt = 2;
    }
    else if (K_RIGHT_JUSTIFYING_MODE == mode)
    {
        reg4.reg_4.i2s_rx_fmt = 0;
    }
    else if (K_LEFT_JUSTIFYING_MODE == mode)
    {
        reg4.reg_4.i2s_rx_fmt = 1;
    }

    writel(reg4.reg_data, &audio_codec_reg->reg_04);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

#if AUDIO_ADC_MIX
    reg2.reg_data = readl(&audio_codec_reg->reg_02);

    // reg2.reg_2.i2s_tx_datsel = 2;//mix right data
    reg2.reg_2.i2s_tx_datsel = 1; // mix left data
    writel(reg2.reg_data, &audio_codec_reg->reg_02);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // 8.Configure the register INITIAL_ADCL 0x23[2] to 1, to end the initialization of the ADCL module.
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.initial_adcl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 8.Configure the register INITIAL_ADCR 0x26[2] to 1, to end the initialization of the ADCR module.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.initial_adcr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 9.Configure the register INITIAL_ALCL 0x23[1] to 1, to end the initialization of the left ALC module.
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.initial_alcl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 9.Configure the register INITIAL_ALCR 0x26[1] to 1, to end the initialization of the right ALC module.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.initial_alcr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 10.Configure the register INITIAL_MICL 0x23[0] to 1, to end the initialization of theleft MIC module.
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.initial_micl = 1;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 10.Configure the register INITIAL_MICR 0x26[0] to 1, to end the initialization of the right MIC module.
    reg26.reg_data = readl(&audio_codec_reg->reg_26);
    reg26.reg_26.initial_micr = 1;
    writel(reg26.reg_data, &audio_codec_reg->reg_26);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

#if 0
    //11.Configure the register MUTE_MICL 0x24[7] to 1, to end the mute station of the ADC left channel.
    reg24.reg_data = readl(&audio_codec_reg->reg_24);
    reg24.reg_24.mute_micl = 1;
    writel(reg24.reg_data,&audio_codec_reg->reg_24);

    //11.Configure the register MUTE_MICR 0x27[7] to 1, to end the mute station of the ADC right channel.
    reg27.reg_data = readl(&audio_codec_reg->reg_27);
    reg27.reg_27.mute_micr = 1;
    writel(reg27.reg_data,&audio_codec_reg->reg_27);
#endif

    if (!g_adc_sound_init)
    {
        _reset_adc_sound();
        g_adc_sound_init = true;
    }

    // 14.Configure the register EN_ZeroDET_ADCL 0x25[1] to 1, to enable the zerocrossing detection function in ADC left channel.
    reg25.reg_data = readl(&audio_codec_reg->reg_25);
    reg25.reg_25.en_zerodet_adcl = 1;
    writel(reg25.reg_data, &audio_codec_reg->reg_25);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 14.Configure the register EN_ZeroDET_ADCR 0x28[1] to 1, to enable the zerocrossing detection function in ADC right channel
    reg28.reg_data = readl(&audio_codec_reg->reg_28);
    reg28.reg_28.en_zerodet_adcr = 1;
    writel(reg28.reg_data, &audio_codec_reg->reg_28);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg0.reg_data = readl(&audio_codec_reg->reg_00);
    reg0.reg_0.digcore_bstn = 1;
    writel(reg0.reg_data, &audio_codec_reg->reg_00);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // if (0 == g_check_hp_thread)
    // {
    //     g_check_hp_thread = rt_thread_create("acodec_hp", _check_adc_hp_thread, RT_NULL, 1024*10, 10,10);
    //     rt_thread_startup(g_check_hp_thread);
    // }

    audio_reset_micbias();
}

static volatile bool g_dac_sound_init = false;
void audio_codec_dac_init(k_i2s_work_mode mode, unsigned int i2s_ws)
{
    reg_20_t reg20;
    reg_29_t reg29;
    reg_2c_t reg2c;
    reg_2a_t reg2a;
    reg_2d_t reg2d;
    //reg_2b_t reg2b;
    //reg_2e_t reg2e;
    reg_2_t reg2;
    reg_3_t reg3;
    //reg_1_t reg1;
    //reg_6_t reg6;
    //reg_7_t reg7;
    reg_0_t reg0;

    // 1.Configure the register EN_IBIAS_DAC 0x20[4] to 1, to enable the current source of DAC.
    reg20.reg_data = readl(&audio_codec_reg->reg_20);
    reg20.reg_20.en_ibias_dac = 1;
    writel(reg20.reg_data, &audio_codec_reg->reg_20);
    msleep(AUDIO_REG_CONFIG_LONG2_DELAY);

    // 2.Configure the register EN_BUF_DACL 0x29[6] to 1, to enable the reference voltage buffer of the DAC left channel
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.en_buf_dacl = 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 2.Configure the register EN_BUF_DACR 0x2c[6] to 1, to enable the reference voltage buffer of the DAC right channel.
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.en_buf_dacr = 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 3.Configure the register POP_CTRL_DACL<1:0> 0x29[5:4] to 2’b10, to enable POP sound in the DAC left channel.
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.pop_ctrl_dacl = 2;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 3.Configure the register POP_CTRL_DACR<1:0> 0x2c[5:4] to 2’b10, to enable POP sound in the DAC right channel
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.pop_ctrl_dacr = 2;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 4.Configure the register EN_HPOUTL 0x2a[5] to 1, to enable the HPDRV module in  the DAC left channel
    reg2a.reg_data = readl(&audio_codec_reg->reg_2a);
    reg2a.reg_2a.en_hpoutl = 1;
    writel(reg2a.reg_data, &audio_codec_reg->reg_2a);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 4.Configure the register EN_HPOUTR 0x2d[5] to 1, to enable the HPDRV module in the DAC right channel
    reg2d.reg_data = readl(&audio_codec_reg->reg_2d);
    reg2d.reg_2d.en_hpoutr = 1;
    writel(reg2d.reg_data, &audio_codec_reg->reg_2d);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

#if AUDIO_ENABLE_BIST
    reg7.reg_data = readl(&audio_codec_reg->reg_07);
    reg7.reg_7.dacl_bist_sel = 1;
    reg7.reg_7.dacr_bist_sel = 1;
    writel(reg7.reg_data, &audio_codec_reg->reg_07);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);
#endif

    // 5.Configure the register INITIAL_HPOUTL 0x2a[4] to 1, to end the initialization of the HPDRV module in the DAC left channel.
    reg2a.reg_data = readl(&audio_codec_reg->reg_2a);
    reg2a.reg_2a.initial_hpoutl = 1;
    writel(reg2a.reg_data, &audio_codec_reg->reg_2a);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 5.Configure the register INITIAL_HPOUTR 0x2d[4] to 1, to end the initialization of the HPDRV module in the DAC right channel.
    reg2d.reg_data = readl(&audio_codec_reg->reg_2d);
    reg2d.reg_2d.initial_hpoutr = 1;
    writel(reg2d.reg_data, &audio_codec_reg->reg_2d);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 6.Configure the register EN_VREF_DACL 0x29[3] to 1, to enable the reference voltage of DACL module
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.en_vref_dacl = 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 6.Configure the register EN_VREF_DACR 0x2c[3] to 1, to enable the reference voltage of DACR module.
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.en_vref_dacr = 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 7.Configure the register EN_CLK_DACL 0x29[2] to 1, to enable the clock module of DACL module.
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.en_clk_dacl = 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 7.Configure the register EN_CLK_DACR 0x2c[2] to 1, to enable the clock module of DACR module.
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.en_clk_dacr = 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 8.Configure the register EN_DACL 0x29[1] to 1, to enable the DACL module.
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.en_dacl = 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 8.Configure the register EN_DACR 0x2c[1] to 1, to enable the DACR module.
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.en_dacr = 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // i2s tx word length
    reg2.reg_data = readl(&audio_codec_reg->reg_02);

    if (i2s_ws == 24)
    {
        // reg2.reg_2.i2s_tx_wl = 3;//valid 32 bit
        reg2.reg_2.i2s_tx_wl = 2; // valid 24 bit
    }
    else if (i2s_ws == 16)
    {
        reg2.reg_2.i2s_tx_wl = 0; // valid 16 bit
    }
    else if (i2s_ws == 32)
    {
        reg2.reg_2.i2s_tx_wl = 3;//valid 32 bit
    }

    //reg2.reg_2.i2s_tx_datsel = 2;
    writel(reg2.reg_data, &audio_codec_reg->reg_02);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // i2s tx fmt
    reg2.reg_data = readl(&audio_codec_reg->reg_02);
    if (K_STANDARD_MODE == mode)
    {
        reg2.reg_2.i2s_tx_fmt = 2;
    }
    else if (K_RIGHT_JUSTIFYING_MODE == mode)
    {
        reg2.reg_2.i2s_tx_fmt = 0;
    }
    else if (K_LEFT_JUSTIFYING_MODE == mode)
    {
        reg2.reg_2.i2s_tx_fmt = 1;
    }

    writel(reg2.reg_data, &audio_codec_reg->reg_02);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    reg3.reg_data = readl(&audio_codec_reg->reg_03);
    reg3.reg_3.i2s_tx_len = 3; // 32 bit
    writel(reg3.reg_data, &audio_codec_reg->reg_03);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    /////////////////////////////////////////////////////////////////////////////////////////////////

    // 9.Configure the register INITIAL_DACL 0x29[0] to 1, to end the initialization of the DACL module.
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.initial_dacl = 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 9.Configure the register INITIAL_DACR 0x2c[0] to 1, to end the initialization of the DACR module.
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.initial_dacr = 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 10.Configure the register MUTE_HPOUTL 0x29[7] to 1, to end the mute station of the HPDRV module in the DAC left channel
    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.mute_hpoutl = 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // 10.Configure the register MUTE_HPOUTR 0x2c[7] to 1, to end the mute station of the HPDRV module in the DAC right channel.
    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.mute_hpoutr = 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    if (!g_dac_sound_init)
    {
        _reset_dac_sound();
        g_dac_sound_init = true;
    }

    reg0.reg_data = readl(&audio_codec_reg->reg_00);
    reg0.reg_0.digcore_bstn = 1;
    writel(reg0.reg_data, &audio_codec_reg->reg_00);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

}

static int convert_adc_gain(unsigned int gain, unsigned int *reg_val)
{
    unsigned int reg_value = 0;
    if (gain > 30 || gain < 0)
    {
        return -1;
    }

    if (0 == gain)
    {
        reg_value = 0;
    }
    else if (gain >0 && gain <= 10)
    {
        reg_value = 1;
    }
    else if (gain > 10 && gain <= 20)
    {
        reg_value = 2;
    }
    else if (gain > 20 && gain <= 30)
    {
        reg_value = 3;
    }

    *reg_val = reg_value;

    return 0;
}

static int convert_adc_gain_2(unsigned int reg_val, unsigned int *gain)
{
    unsigned int gain_value = 0;

    if (reg_val > 3 || reg_val < 0)
    {
        return -1;
    }

    if (0 == reg_val)
    {
        gain_value = 0;
    }
    else if (1 == reg_val)
    {
        gain_value = 6;
    }
    else if (2 == reg_val)
    {
        gain_value = 20;
    }
    else if (3 == reg_val)
    {
        gain_value = 30;
    }

    *gain = gain_value;

    return 0;
}

int audio_codec_adc_set_micl_gain(int gain)
{
    reg_24_t reg24;
    unsigned int reg_val = 0;

    printk("audio_codec_adc_set_micl_gain gain:%d\n", gain);
    if (0 != convert_adc_gain(gain, &reg_val))
    {
        return -1;
    }

    reg24.reg_data = readl(&audio_codec_reg->reg_24);
    reg24.reg_24.gain_micl = reg_val;
    writel(reg24.reg_data, &audio_codec_reg->reg_24);
    g_snd_current_values.gain_micl = reg_val;
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);
    return 0;
}

int audio_codec_adc_set_micr_gain(int gain)
{
    unsigned int reg_val = 0;
    reg_27_t reg27;

    printk("audio_codec_adc_set_micr_gain gain:%d\n", gain);
    if (0 != convert_adc_gain(gain, &reg_val))
    {
        return -1;
    }

    reg27.reg_data = readl(&audio_codec_reg->reg_27);
    reg27.reg_27.gain_micr = reg_val;
    writel(reg27.reg_data, &audio_codec_reg->reg_27);
    g_snd_current_values.gain_micr = reg_val;
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    return 0;
}

int  audio_codec_adc_get_micl_gain(int* gain)
{
    return convert_adc_gain_2(g_snd_current_values.gain_micl,gain);
}


int  audio_codec_adc_get_micr_gain( int* gain)
{
    return convert_adc_gain_2(g_snd_current_values.gain_micr,gain);
}


// static int convert_adc_volume(float volume, unsigned int *reg_val)
// {
//     unsigned int reg_value = 0;
//     float start_gain = -97;
//     bool bfind = false;
//     int i = 0;

//     if (volume < -97 || volume > 30)
//     {
//         return -1;
//     }

//     for (i = 0x1; i <= 0xff; i++)
//     {
//         if (volume == start_gain)
//         {
//             reg_value = i;
//             bfind = true;
//             break;
//         }
//         else
//         {
//             start_gain += 0.5;
//         }
//     }
//     if (!bfind)
//     {
//         return -1;
//     }

//     *reg_val = reg_value;

//     return 0;
// }

// static int convert_adc_volume2(unsigned int reg_val,float* volume)
// {
//     float start_gain = -97;
//     bool bfind = false;
//     int i =0;

//     if (reg_val < 1 || reg_val > 0xff)
//     {
//         return -1;
//     }

//     for (i = 0x1; i <= 0xff; i++)
//     {
//         if (reg_val == i)
//         {
//             bfind = true;
//             break;
//         }
//         else
//         {
//             start_gain += 0.5;
//         }
//     }
//     if (!bfind)
//     {
//         return -1;
//     }

//     *volume = start_gain;

//     return 0;
// }

// int audio_codec_adcl_set_volume(float volume)
// {
//     unsigned int reg_val = 0;
//     reg_8_t reg8;

//     if (0 != convert_adc_volume(volume, &reg_val))
//     {
//         return -1;
//     }

//     reg8.reg_data = readl(&audio_codec_reg->reg_08);
//     reg8.reg_8.adcl_vol = reg_val;
//     writel(reg8.reg_data, &audio_codec_reg->reg_08);
//     g_snd_current_values.adc_volumel = reg_val;
//     msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);
//     return 0;
// }
// int audio_codec_adcr_set_volume(float volume)
// {
//     unsigned int reg_val = 0;
//     reg_9_t reg9;

//     if (0 != convert_adc_volume(volume, &reg_val))
//     {
//         return -1;
//     }

//     reg9.reg_data = readl(&audio_codec_reg->reg_09);
//     reg9.reg_9.adcr_vol = reg_val;
//     writel(reg9.reg_data, &audio_codec_reg->reg_09);
//     g_snd_current_values.adc_volumer = reg_val;
//     msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);
//     return 0;
// }

// int  audio_codec_adcl_get_volume(float* volume)
// {
//     return convert_adc_volume2(g_snd_current_values.adc_volumel, volume);
// }
// int  audio_codec_adcr_get_volume(float* volume)
// {
//     return convert_adc_volume2(g_snd_current_values.adc_volumer, volume);
// }

int audio_codec_adc_micl_mute(bool mute)
{
    reg_24_t reg24;

    printk("audio_codec_adc_micl_mute mute:%d\n", mute);
    reg24.reg_data = readl(&audio_codec_reg->reg_24);
    reg24.reg_24.mute_micl = mute ? 0 : 1;
    writel(reg24.reg_data, &audio_codec_reg->reg_24);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    g_adc_left_mute = mute;
    return 0;
}

int audio_codec_adc_micr_mute(bool mute)
{
    reg_27_t reg27;
    printk("audio_codec_adc_micr_mute mute:%d\n", mute);
    reg27.reg_data = readl(&audio_codec_reg->reg_27);
    reg27.reg_27.mute_micr = mute ? 0 : 1;
    writel(reg27.reg_data, &audio_codec_reg->reg_27);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    g_adc_right_mute = mute;
    return 0;
}

int  audio_codec_adc_get_micl_mute(bool* mute)
{
    *mute = g_adc_left_mute;
    return 0;
}

int  audio_codec_adc_get_micr_mute(bool* mute)
{
    *mute = g_adc_right_mute;
    return 0;
}

// static int convert_alc_gain(float gain, unsigned int *reg_val)
// {
//     unsigned int reg_value = 0;
//     float start_gain = -18;
//     bool bfind = false;
//     int i = 0;

//     if (gain < -18 || gain > 28.5)
//     {
//         return -1;
//     }

//     for (i = 0; i <= 0x1f; i++)
//     {
//         if (gain == start_gain)
//         {
//             reg_value = i;
//             bfind = true;
//             break;
//         }
//         else
//         {
//             start_gain += 1.5;
//         }
//     }
//     if (!bfind)
//     {
//         return -1;
//     }

//     *reg_val = reg_value;

//     return 0;
// }

// static int convert_alc_gain2(unsigned int reg_val,float* gain)
// {
//     float start_gain = -18;
//     bool bfind = false;
//     int i =0;

//     if (reg_val < 0 || reg_val > 0x1f)
//     {
//         return -1;
//     }

//     for (i = 0; i <= 0x1f; i++)
//     {
//         if (reg_val == i)
//         {
//             bfind = true;
//             break;
//         }
//         else
//         {
//             start_gain += 1.5;
//         }
//     }
//     if (!bfind)
//     {
//         return -1;
//     }

//     *gain = start_gain;

//     return 0;
// }

// int audio_codec_alc_set_micl_gain(float gain)
// {
//     unsigned int reg_val = 0;
//     reg_24_t reg24;
//     //acodec_err_trace("audio_codec_alc_set_micl_gain gain:%.2f\n", gain);

//     if (0 != convert_alc_gain(gain, &reg_val))
//     {
//         return -1;
//     }

//     reg24.reg_data = readl(&audio_codec_reg->reg_24);
//     reg24.reg_24.gain_alcl = reg_val;
//     writel(reg24.reg_data, &audio_codec_reg->reg_24);
//     g_snd_current_values.gain_alcl = reg_val;
//     msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

//     return 0;
// }

// int audio_codec_alc_set_micr_gain(float gain)
// {
//     reg_27_t reg27;
//     int reg_val = 0;
//     //acodec_err_trace("audio_codec_alc_set_micr_gain gain:%.2f\n", gain);

//     if (0 != convert_alc_gain(gain, &reg_val))
//     {
//         return -1;
//     }

//     reg27.reg_data = readl(&audio_codec_reg->reg_27);
//     reg27.reg_27.gain_alcr = reg_val;
//     writel(reg27.reg_data, &audio_codec_reg->reg_27);
//     g_snd_current_values.gain_alcr = reg_val;
//     msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

//     return 0;
// }

// int  audio_codec_alc_get_micl_gain(float* gain)
// {
//     return convert_alc_gain2(g_snd_current_values.gain_alcl, gain);
// }

// int  audio_codec_alc_get_micr_gain(float* gain)
// {
//     return convert_alc_gain2(g_snd_current_values.gain_alcr, gain);
// }

static int convert_dac_gain(int gain, unsigned int *reg_val)
{
    unsigned int reg_value = 0;
    int start_gain = -39;
    bool bfind = false;
    int i = 0;

    if (gain < -39 || gain > 6)
    {
        return -1;
    }

    for (i = 0; i <= 0x1e; i+=2) // 11111/11110 both 6db
    {
        if (gain <= start_gain)
        {
            reg_value = i;
            bfind = true;
            break;
        }
        else
        {
            start_gain += 3;
        }
    }
    if (!bfind)
    {
        return -1;
    }

    *reg_val = reg_value;

    return 0;
}

static int convert_dac_gain2(unsigned int reg_val,int* gain)
{
    int start_gain = -39;
    bool bfind = false;
    int i = 0;

    if (reg_val < 0 || reg_val > 0x1e)
    {
        return -1;
    }

    for (i = 0; i <= 0x1e; i+=2) // 11111/11110 both 6db
    {
        if (reg_val == i)
        {
            bfind = true;
            break;
        }
        else
        {
            start_gain += 3;
        }
    }
    if (!bfind)
    {
        return -1;
    }

    *gain = start_gain;

    return 0;
}

int audio_codec_dac_set_hpoutl_gain(int gain)
{
    unsigned int reg_val = 0;
    reg_2b_t reg2b;

    if (0 != convert_dac_gain(gain, &reg_val))
    {
        return -1;
    }

    reg2b.reg_data = readl(&audio_codec_reg->reg_2b);

    reg2b.reg_2b.gain_hpoutl = reg_val;
    writel(reg2b.reg_data, &audio_codec_reg->reg_2b);
    g_snd_current_values.gain_hpoutl = reg_val;
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);
    return 0;
}

int audio_codec_dac_set_hpoutr_gain(int gain)
{
    unsigned int reg_val = 0;
    reg_2e_t reg2e;

    if (0 != convert_dac_gain(gain, &reg_val))
    {
        return -1;
    }

    reg2e.reg_data = readl(&audio_codec_reg->reg_2e);
    reg2e.reg_2e.gain_hpoutr = reg_val; // 最大db值
    writel(reg2e.reg_data, &audio_codec_reg->reg_2e);
    g_snd_current_values.gain_hpoutr = reg_val;
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);
    return 0;
}

int  audio_codec_dac_get_hpoutl_gain(int* gain)
{
    return convert_dac_gain2( g_snd_current_values.gain_hpoutl,gain);
}

int  audio_codec_dac_get_hpoutr_gain(int* gain)
{
    return convert_dac_gain2( g_snd_current_values.gain_hpoutr,gain);
}

int audio_codec_dac_hpoutl_mute(bool mute)
{
    reg_29_t reg29;
    reg_2a_t reg2a;

    reg29.reg_data = readl(&audio_codec_reg->reg_29);
    reg29.reg_29.mute_hpoutl = mute ? 0 : 1;
    writel(reg29.reg_data, &audio_codec_reg->reg_29);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // MUTE的同时将INITIAL_HPOUTL也置0即可（gain 无需修改）
    reg2a.reg_data = readl(&audio_codec_reg->reg_2a);
    reg2a.reg_2a.initial_hpoutl = mute ? 0 : 1;
    writel(reg2a.reg_data, &audio_codec_reg->reg_2a);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    g_dac_left_mute = mute;
    return 0;
}
int audio_codec_dac_hpoutr_mute(bool mute)
{
    reg_2c_t reg2c;
    reg_2d_t reg2d;

    reg2c.reg_data = readl(&audio_codec_reg->reg_2c);
    reg2c.reg_2c.mute_hpoutr = mute ? 0 : 1;
    writel(reg2c.reg_data, &audio_codec_reg->reg_2c);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    // MUTE的同时将INITIAL_HPOUTR也置0即可（gain 无需修改）
    reg2d.reg_data = readl(&audio_codec_reg->reg_2d);
    reg2d.reg_2d.initial_hpoutr = mute ? 0 : 1;
    writel(reg2d.reg_data, &audio_codec_reg->reg_2d);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    g_dac_right_mute = mute;
    return 0;
}

int  audio_codec_dac_get_hpoutl_mute(bool* mute)
{
    *mute = g_dac_left_mute;
    return 0;
}

int  audio_codec_dac_get_hpoutr_mute(bool* mute)
{
    *mute = g_dac_left_mute;
    return 0;
}

// static int convert_dac_volume(float gain, unsigned int *reg_val)
// {
//     unsigned int reg_value = 0;
//     float start_gain = -120;
//     bool bfind = false;
//     int i = 0 ;

//     if (gain < -120 || gain > 7)
//     {
//         return -1;
//     }

//     for (i = 1; i <= 0xff; i++)
//     {
//         if (gain == start_gain)
//         {
//             reg_value = i;
//             bfind = true;
//             break;
//         }
//         else
//         {
//             start_gain += 0.5;
//         }
//     }
//     if (!bfind)
//     {
//         return -1;
//     }

//     *reg_val = reg_value;

//     return 0;
// }

// static int convert_dac_volume2(unsigned int reg_val,float* gain)
// {
//     float start_gain = -120;
//     bool bfind = false;
//     int i = 0;

//     if (reg_val < 1 || reg_val > 0xff)
//     {
//         return -1;
//     }

//     for (i = 1; i <= 0xff; i++)
//     {
//         if (i == reg_val)
//         {
//             bfind = true;
//             break;
//         }
//         else
//         {
//             start_gain += 0.5;
//         }
//     }
//     if (!bfind)
//     {
//         return -1;
//     }

//     *gain = start_gain;

//     return 0;
// }

// int audio_codec_dacl_set_volume(float volume)
// {
//     unsigned int reg_val = 0;
//     reg_6_t reg6;

//     if (0 != convert_dac_volume(volume, &reg_val))
//     {
//         return -1;
//     }

//     reg6.reg_data = readl(&audio_codec_reg->reg_06);
//     reg6.reg_6.dac_vol = reg_val;
//     writel(reg6.reg_data, &audio_codec_reg->reg_06);
//     g_snd_current_values.dac_volumel = reg_val;
//     g_snd_current_values.dac_volumer = reg_val;
//     msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

//     return 0;
// }
// int audio_codec_dacr_set_volume(float volume)
// {
//     unsigned int reg_val = 0;
//     reg_6_t reg6;
//     if (0 != convert_dac_volume(volume, &reg_val))
//     {
//         return -1;
//     }

//     reg6.reg_data = readl(&audio_codec_reg->reg_06);
//     reg6.reg_6.dac_vol = reg_val;
//     writel(reg6.reg_data, &audio_codec_reg->reg_06);
//     g_snd_current_values.dac_volumel = reg_val;
//     g_snd_current_values.dac_volumer = reg_val;
//     msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

//     return 0;
// }

// int  audio_codec_dacl_get_volume(float* volume)
// {
//     return convert_dac_volume2(g_snd_current_values.dac_volumel, volume);
// }

// int  audio_codec_dacr_get_volume(float* volume)
// {
//     return convert_dac_volume2(g_snd_current_values.dac_volumer, volume);
// }

int  audio_codec_reset(void)
{
    _reset_adc_sound();
    _reset_dac_sound();
    g_snd_current_values = g_snd_default_values;
    return 0;
}

int  audio_codec_adc_hp_work(bool work)
{
    reg_23_t reg23;

    printk("audio_codec_adc_hp_work work:%d\n",work);
    reg23.reg_data = readl(&audio_codec_reg->reg_23);
    reg23.reg_23.initial_micl = work?1:0;
    writel(reg23.reg_data, &audio_codec_reg->reg_23);
    msleep(AUDIO_REG_CONFIG_NORMAL_DELAY);

    return 0;
}

