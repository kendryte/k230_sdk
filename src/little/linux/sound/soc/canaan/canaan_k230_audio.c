#include <linux/module.h>
#include <linux/platform_device.h>
#include "canaan_k230_audio.h"
#include <asm/io.h>
#include <linux/of.h>

struct canaan_audio_data {
	struct platform_device *pdev;
	void __iomem *base;
};

static struct canaan_audio_data sai = {0};

void audio_i2s_in_init(void)
{
    if(sai.base)
    {
        volatile audio_in_reg_s  *audio_in_reg = sai.base;
		audio_in_reg->audio_in_pdm_conf_0.audio_in_mode          = AUDIO_IO_OUT_MODE_I2S;//默认使用i2s
    	audio_in_reg->audio_in_agc_para_4.agc_bypass             = AUDIO_ENABLE;
    }
}

EXPORT_SYMBOL_GPL(audio_i2s_in_init);

void audio_i2s_out_init(bool enable, uint32_t word_len)
{
	audio_out_data_width_e out_word_len = AUDIO_OUT_TYPE_32BIT;
    if (32 == word_len)
    {
        out_word_len = AUDIO_OUT_TYPE_32BIT;
    }
    else if (24 == word_len)
    {
        out_word_len = AUDIO_OUT_TYPE_24BIT;
    }
    else if (16 == word_len)
    {
        out_word_len = AUDIO_OUT_TYPE_16BIT;
    }
    if(sai.base)
    {
        volatile audio_out_reg_s *audio_out_reg = sai.base + 0x800;
		audio_out_reg->audio_out_ctl.data_type              = out_word_len;
    	audio_out_reg->audio_out_ctl.mode                   = AUDIO_OUT_MODE_I2S;      /* i2s/pdm/tdm mode */
    	audio_out_reg->audio_out_ctl.enable                 = enable ? AUDIO_ENABLE : AUDIO_DISABLE;        /* enable audio out */
    }
}

EXPORT_SYMBOL_GPL(audio_i2s_out_init);

void audio_i2s_enable_audio_codec(bool use_audio_codec)
{
	if(sai.base){
		volatile audio_in_reg_s  *audio_in_reg = sai.base;
    	audio_in_reg->audio_in_pdm_conf_0.audio_codec_bypass     = !use_audio_codec;//是否启用内置codec
	}
}

EXPORT_SYMBOL_GPL(audio_i2s_enable_audio_codec);

static int canaan_audio_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret = 0;

 	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	sai.base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(sai.base))
		return PTR_ERR(sai.base);

    platform_set_drvdata(pdev, &sai);

	return ret;
}

static int canaan_audio_remove(struct platform_device *pdev)
{
	struct canaan_audio_data *priv = platform_get_drvdata(pdev);

	return 0;
}

static const struct of_device_id canaan_audio_ids[] = {
	{ .compatible = "canaan,k230-audio" },
	{ /* sentinel */ },
};

static struct platform_driver canaan_audio_driver = {
	.driver = {
		.name = "k230-audio",
		.of_match_table = canaan_audio_ids,
	},
	.probe = canaan_audio_probe,
	.remove = canaan_audio_remove,
};

module_platform_driver(canaan_audio_driver);

MODULE_DESCRIPTION("CANAAN AUDIO Interface");
MODULE_LICENSE("GPL");
