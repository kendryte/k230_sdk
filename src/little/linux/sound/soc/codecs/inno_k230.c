#include <sound/soc.h>
#include <sound/tlv.h>
#include <sound/soc-dapm.h>
#include <sound/soc-dai.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/regmap.h>
#include <linux/device.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/io.h>
#include "inno_k230_reg.h"

#define INNO_VOLUME_INVERT_VALUE 39

struct k230_inno_codec_priv {
	void __iomem *base;
	struct clk *pclk_adc;
	struct clk *pclk_dac;
	struct regmap *regmap;
	struct device *dev;
};

enum snd_inno_k230_ctrl {
	INNO_PCM_PLAYBACK_VOLUME,
	INNO_PCM_PLAYBACK_MUTE,
	INNO_PCM_CAPTURE_VOLUME,
	INNO_PCM_CAPTURE_MUTE,
};

static int inno_capture_info_vol(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_info *uinfo)
{
	//printk("====================inno_info_vol\n");
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 4;
	return 0;
}

static int inno_capture_get_vol(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{

	return 0;
}

static int inno_capture_put_vol(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{

	return 0;
}

static const struct snd_kcontrol_new inno_snd_capture_volume_control =
{
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, //表示snd_kcontrol结构体用于哪一类设备（表示进行参数设置）
    .name = "Capture Volume",  //音量控制，每个声卡驱动程序的snd_kcontrol各不相同，为什么应用程序都可以调整它的音量，对于某些常用的属性，它们都有固定的名字。应用程序根据名字找到它的snd_kcontrol项，调用里面的put函数。
	.info = inno_capture_info_vol,  //获得一些信息，如音量范围是多少
	.get  = inno_capture_get_vol,//获得当前的音量值
	.put  = inno_capture_put_vol, //设置音量
};

static int inno_playback_info_vol(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_info *uinfo)
{
	//printk("====================inno_info_vol\n");
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = -39+INNO_VOLUME_INVERT_VALUE;
	uinfo->value.integer.max = 6+INNO_VOLUME_INVERT_VALUE;
	return 0;
}

static int inno_playback_get_vol(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{
	int value;

	audio_codec_dac_get_hpoutl_gain(&value);
	//printk("=========inno_get_vol:%d\n",value+INNO_VOLUME_INVERT_VALUE);
	ucontrol->value.integer.value[1] = ucontrol->value.integer.value[0] = value+INNO_VOLUME_INVERT_VALUE;
	return 0;
}

static int inno_playback_put_vol(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{
	//printk("=========inno_put_vol:%d\n",ucontrol->value.integer.value[0]);

	audio_codec_dac_set_hpoutl_gain(ucontrol->value.integer.value[0]-INNO_VOLUME_INVERT_VALUE);
	audio_codec_dac_set_hpoutr_gain(ucontrol->value.integer.value[0]-INNO_VOLUME_INVERT_VALUE);
	return 0;
}

static const struct snd_kcontrol_new inno_snd_playback_volume_control =
{
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, //表示snd_kcontrol结构体用于哪一类设备（表示进行参数设置）
    .name = "Master Playback Volume",  //音量控制，每个声卡驱动程序的snd_kcontrol各不相同，为什么应用程序都可以调整它的音量，对于某些常用的属性，它们都有固定的名字。应用程序根据名字找到它的snd_kcontrol项，调用里面的put函数。
	.info = inno_playback_info_vol,  //获得一些信息，如音量范围是多少
	.get  = inno_playback_get_vol,//获得当前的音量值
	.put  = inno_playback_put_vol, //设置音量
};

static int snd_inno_ctl_vol(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_info *uinfo)
{
	if (kcontrol->private_value == INNO_PCM_PLAYBACK_VOLUME)
	{
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = -39+INNO_VOLUME_INVERT_VALUE;
		uinfo->value.integer.max = 6+INNO_VOLUME_INVERT_VALUE;
		uinfo->value.integer.step = 3;
	}
	else if (kcontrol->private_value == INNO_PCM_PLAYBACK_MUTE)
	{
		uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;
	}
	else if (kcontrol->private_value == INNO_PCM_CAPTURE_VOLUME)
	{
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 30;
		uinfo->value.integer.step = 10;
	}
	else if (kcontrol->private_value == INNO_PCM_CAPTURE_MUTE)
	{
		uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;
	}
	return 0;
}

static int snd_inno_ctl_get(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{
	int value;
	bool mute;

	if (kcontrol->private_value == INNO_PCM_PLAYBACK_VOLUME)
	{
		audio_codec_dac_get_hpoutl_gain(&value);
		ucontrol->value.integer.value[0] = value+INNO_VOLUME_INVERT_VALUE;
	}
	else if (kcontrol->private_value == INNO_PCM_PLAYBACK_MUTE)
	{
		audio_codec_dac_get_hpoutl_mute(&mute);
		ucontrol->value.integer.value[0] = !mute;
	}
	else if (kcontrol->private_value == INNO_PCM_CAPTURE_VOLUME)
	{
		audio_codec_adc_get_micl_gain(&value);
		if (value == 6)
		{
			value = 10;
		}
		ucontrol->value.integer.value[0] = value;
	}
	else if (kcontrol->private_value == INNO_PCM_CAPTURE_MUTE)
	{
		audio_codec_adc_get_micl_mute(&mute);
		ucontrol->value.integer.value[0] = !mute;
	}
	return 0;
}

static int snd_inno_ctl_put(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{
	if (kcontrol->private_value == INNO_PCM_PLAYBACK_VOLUME)
	{
		audio_codec_dac_set_hpoutl_gain(ucontrol->value.integer.value[0]-INNO_VOLUME_INVERT_VALUE);
		audio_codec_dac_set_hpoutr_gain(ucontrol->value.integer.value[0]-INNO_VOLUME_INVERT_VALUE);
	}
	else if (kcontrol->private_value == INNO_PCM_PLAYBACK_MUTE)
	{
		audio_codec_dac_hpoutl_mute(!ucontrol->value.integer.value[0]);
		audio_codec_dac_hpoutr_mute(!ucontrol->value.integer.value[0]);
	}
	else if (kcontrol->private_value == INNO_PCM_CAPTURE_VOLUME)
	{
		audio_codec_adc_set_micl_gain(ucontrol->value.integer.value[0]);
		audio_codec_adc_set_micl_gain(ucontrol->value.integer.value[0]);

	}
	else if (kcontrol->private_value == INNO_PCM_CAPTURE_MUTE)
	{
		audio_codec_adc_micl_mute(!ucontrol->value.integer.value[0]);
		audio_codec_adc_micr_mute(!ucontrol->value.integer.value[0]);
	}
	return 0;
}

static const struct snd_kcontrol_new inno_snd_control[] =
{
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
    	.name = "PCM Playback Volume",
		.info = snd_inno_ctl_vol,
		.get  = snd_inno_ctl_get,
		.put  = snd_inno_ctl_put,
		.private_value = INNO_PCM_PLAYBACK_VOLUME,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
    	.name = "PCM Playback Switch",
		.info = snd_inno_ctl_vol,
		.get  = snd_inno_ctl_get,
		.put  = snd_inno_ctl_put,
		.private_value = INNO_PCM_PLAYBACK_MUTE,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
    	.name = "Mic Capture Volume",
		.info = snd_inno_ctl_vol,
		.get  = snd_inno_ctl_get,
		.put  = snd_inno_ctl_put,
		.private_value = INNO_PCM_CAPTURE_VOLUME,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
    	.name = "Mic Capture Switch",
		.info = snd_inno_ctl_vol,
		.get  = snd_inno_ctl_get,
		.put  = snd_inno_ctl_put,
		.private_value = INNO_PCM_CAPTURE_MUTE,
	},

};


static int k230_inno_codec_probe(struct snd_soc_component *component)
{
	snd_soc_add_component_controls(component, inno_snd_control,ARRAY_SIZE(inno_snd_control));
	return 0;
}

static void k230_inno_codec_remove(struct snd_soc_component *component)
{

}

static int k230_inno_codec_set_bias_level(struct snd_soc_component *component,
				       enum snd_soc_bias_level level)
{
	return 0;
}

static const struct snd_soc_dapm_route k230_inno_codec_dapm_routes[] = {
	// {"DACL VREF", NULL, "DAC PWR"},
	// {"DACR VREF", NULL, "DAC PWR"},
	// {"DACL HiLo VREF", NULL, "DAC PWR"},
	// {"DACR HiLo VREF", NULL, "DAC PWR"},
	// {"DACL CLK", NULL, "DAC PWR"},
	// {"DACR CLK", NULL, "DAC PWR"},
};

static const struct snd_soc_dapm_widget k230_inno_codec_dapm_widgets[] = {
};

static int k230_inno_codec_open(struct snd_soc_component *component,
		    struct snd_pcm_substream *substream)
{
	//printk("=========k230_inno_codec_open\n");
	return 0;
}

static const struct snd_soc_component_driver k230_inno_codec_driver = {
	.open           = k230_inno_codec_open,
	.probe			= k230_inno_codec_probe,
	.remove			= k230_inno_codec_remove,
	.set_bias_level		= k230_inno_codec_set_bias_level,
	.dapm_routes		= k230_inno_codec_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(k230_inno_codec_dapm_routes),
	.dapm_widgets		= k230_inno_codec_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(k230_inno_codec_dapm_widgets),
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
	.non_legacy_dai_naming	= 1,
};

static int k230_inno_codec_dai_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	return 0;
}

static int k230_inno_codec_dai_hw_params(struct snd_pcm_substream *substream,
				      struct snd_pcm_hw_params *hw_params,
				      struct snd_soc_dai *dai)
{
	struct k230_inno_codec_priv *priv = snd_soc_dai_get_drvdata(dai);
	uint32_t i2s_ws = 16;
	uint32_t sample_rate = 0;
	uint32_t chn_cnt = 0;
	int ret = 0;
	struct snd_soc_component *component = dai->component;

	//clock init
	sample_rate = params_rate(hw_params);
	//adc or dac init
	switch (params_format(hw_params))
	{
		case SNDRV_PCM_FORMAT_S16_LE:
			i2s_ws = 16;
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
			i2s_ws = 24;
			break;
		case SNDRV_PCM_FORMAT_S32_LE:
			i2s_ws = 32;
			break;
		default:
			dev_err(component->dev,  "k230_inno_codec: unsupported PCM fmt");
			return -EINVAL;
	}

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
	{
		//adc clock init
		ret = clk_set_rate(priv->pclk_adc, sample_rate*256);
		//printk("=====inno codec adc clock(%d) init,ret:%d\n",sample_rate*256,ret);
		if (ret) {
			dev_err(priv->dev, "Can't set inno codec clock rate: %d\n",ret);
			return ret;
		}

		audio_codec_adc_init(K_STANDARD_MODE,i2s_ws);
	}
	else
	{
		//dac clock init
		ret = clk_set_rate(priv->pclk_dac, sample_rate*256);
		//printk("=====inno codec dac clock(%d) init,ret:%d,i2s_ws:%d\n",sample_rate*256,ret,i2s_ws);
		if (ret) {
			dev_err(priv->dev, "Can't set inno codec clock rate: %d\n",ret);
			return ret;
		}

		audio_codec_dac_init(K_STANDARD_MODE,i2s_ws);
	}

	//channel count
	chn_cnt = params_channels(hw_params);

	//printk("=====k230_inno_codec_dai_hw_params capture:%d,i2s_ws:%d,samplerate:%d,chn_cnt:%d",\
	substream->stream == SNDRV_PCM_STREAM_CAPTURE,i2s_ws,sample_rate,chn_cnt);
	return 0;
}

static int k230_inno_codec_dai_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *cpu_dai)
{
	return 0;
}

#define K230_INNO_CODEC_RATES (SNDRV_PCM_RATE_8000  | \
			    SNDRV_PCM_RATE_16000 | \
			    SNDRV_PCM_RATE_32000 | \
			    SNDRV_PCM_RATE_44100 | \
			    SNDRV_PCM_RATE_48000 )


#define K230_INNO_CODEC_FMTS (SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S32_LE)

static const struct snd_soc_dai_ops k230_inno_codec_dai_ops = {
	.startup	= k230_inno_codec_dai_startup,
	.set_fmt	= k230_inno_codec_dai_set_fmt,
	.hw_params	= k230_inno_codec_dai_hw_params,
};

static struct snd_soc_dai_driver k230_inno_codec_dai_driver[] = {
	{
		.name = "k230-inno-codec-dai",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = K230_INNO_CODEC_RATES,
			.formats = K230_INNO_CODEC_FMTS,
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = K230_INNO_CODEC_RATES,
			.formats = K230_INNO_CODEC_FMTS,
		},
		.ops = &k230_inno_codec_dai_ops,
		.symmetric_rates = 1,
	},
};

static int inno_k230_codec_platform_probe(struct platform_device *pdev)
{
	struct k230_inno_codec_priv *priv;
	//struct device_node *of_node = pdev->dev.of_node;
	void __iomem *base;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(base))
		return PTR_ERR(base);

	priv->base = base;

	priv->pclk_adc = devm_clk_get(&pdev->dev, "adc");
	if (IS_ERR(priv->pclk_adc))
		return PTR_ERR(priv->pclk_adc);

	ret = clk_prepare_enable(priv->pclk_adc);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to enable clk_adc\n");
		return ret;
	}

	priv->pclk_dac = devm_clk_get(&pdev->dev, "dac");
	if (IS_ERR(priv->pclk_dac))
		return PTR_ERR(priv->pclk_dac);

	ret = clk_prepare_enable(priv->pclk_dac);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to enable clk_dac\n");
		return ret;
	}

	priv->dev = &pdev->dev;
	dev_set_drvdata(&pdev->dev, priv);

	ret = devm_snd_soc_register_component(&pdev->dev, &k230_inno_codec_driver,
				k230_inno_codec_dai_driver,
				ARRAY_SIZE(k230_inno_codec_dai_driver));

	if (ret) {
		clk_disable_unprepare(priv->pclk_adc);
		clk_disable_unprepare(priv->pclk_dac);
		dev_set_drvdata(&pdev->dev, NULL);
	}

	audio_codec_reg_init(priv->base);
	audio_codec_powerup_init();

	return 0;
}

static int inno_k230_codec_platform_remove(struct platform_device *pdev)
{
	return 0;
}


static const struct of_device_id inno_k230_codec_of_match[] = {
	{ .compatible = "k230,inno-codec", },
	{}
};
MODULE_DEVICE_TABLE(of, inno_k230_codec_of_match);

static struct platform_driver inno_k230_platform_driver = {
	.driver = {
		.name = "k230,inno-codec",
		.of_match_table = of_match_ptr(inno_k230_codec_of_match),
	},
	.probe = inno_k230_codec_platform_probe,
	.remove = inno_k230_codec_platform_remove,
};

module_platform_driver(inno_k230_platform_driver);

MODULE_DESCRIPTION("ASoC inno codec driver");
MODULE_LICENSE("GPL v2");
