/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <sound/core.h>
#include <sound/jack.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "canaan_k230_audio.h"
#include "audio_muxpin_ctrl.h"

#define DRV_NAME "canaan-k230-snd-inno"

struct k230_inno_info {
	struct clk *xtal;
	struct clk *pclk;
	struct mutex clk_lock;
	int clk_users;
};

static int k230_inno_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params)
{
	return 0;
}

static void k230_inno_shutdown(struct snd_pcm_substream *substream)
{

}

static int k230_inno_startup(struct snd_pcm_substream *substream)
{
	return 0;
}

static const struct snd_soc_ops canaan_k230_inno_ops = {
	.startup = k230_inno_startup,
	.shutdown = k230_inno_shutdown,
	.hw_params = k230_inno_hw_params,
};

SND_SOC_DAILINK_DEFS(k230_inno,
	DAILINK_COMP_ARRAY(COMP_EMPTY()),
	DAILINK_COMP_ARRAY(COMP_CODEC(NULL, "k230-inno-codec-dai")),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

static struct snd_soc_dai_link canaan_k230_dailink = {
	.name = "k230-inno-codec",
	.stream_name = "Audio",
	.ops = &canaan_k230_inno_ops,
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS,
	SND_SOC_DAILINK_REG(k230_inno),
};

static struct snd_soc_card snd_soc_card_k230 = {
	.name = "CANAAN-K230-I2S",
	.owner = THIS_MODULE,
	.dai_link = &canaan_k230_dailink,
	.num_links = 1,
};

static int canaan_k230_inno_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_card_k230;
	struct device_node *codec_np, *cpu_np;
	struct snd_soc_dai_link *dailink = &canaan_k230_dailink;
	struct device_node *np = pdev->dev.of_node;
	struct k230_inno_info *priv;
	int ret;

	if (!np) {
		dev_err(&pdev->dev, "only device tree supported\n");
		return -EINVAL;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	mutex_init(&priv->clk_lock);

	card->dev = &pdev->dev;
	snd_soc_card_set_drvdata(card, priv);

	codec_np = of_parse_phandle(np,
			"canaan,k230-audio-codec", 0);
	if (!codec_np) {
		dev_err(&pdev->dev,
			"Property 'canaan,k230-audio-codec' missing or invalid\n");
		return -EINVAL;
	}
	dailink->codecs->of_node = codec_np;
	of_node_put(codec_np);

	cpu_np = of_parse_phandle(np,
			"canaan,k230-i2s-controller", 0);
	if (!cpu_np) {
		dev_err(&pdev->dev,
			"Property 'canaan,k230-i2s-controller' missing or invalid\n");
		return -EINVAL;
	}

	dailink->cpus->of_node = cpu_np;
	dailink->platforms->of_node = cpu_np;
	of_node_put(cpu_np);

	ret = snd_soc_of_parse_card_name(card, "canaan,model");
	if (ret) {
		dev_err(&pdev->dev,
			"Soc parse card name failed %d\n", ret);
		return ret;
	}


	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret){
		dev_err(&pdev->dev, "failed to register card: %d\n", ret);
		return ret;
	}

	// printk("====audio muxpin \n");
	// ai_i2s_muxpin_config();
	// ao_i2s_muxpin_config();

	audio_i2s_in_init();
	audio_i2s_enable_audio_codec(true);
	audio_i2s_out_init(true,32);

	return ret;
}

static const struct of_device_id canaan_k230_inno_of_match[] = {
	{ .compatible = "canaan,k230-audio-inno", },
	{},
};

MODULE_DEVICE_TABLE(of, canaan_k230_inno_of_match);

static struct platform_driver canaan_k230_inno_driver = {
	.probe = canaan_k230_inno_probe,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = canaan_k230_inno_of_match,
	},
};

module_platform_driver(canaan_k230_inno_driver);

MODULE_DESCRIPTION("CANAAN k230 inno machine ASoC driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
