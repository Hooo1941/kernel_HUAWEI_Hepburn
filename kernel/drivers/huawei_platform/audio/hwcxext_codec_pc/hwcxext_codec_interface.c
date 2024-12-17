/*
 * hwcxext_codec_interface.c
 *
 * analog headset interface
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <securec.h>
#include "hwcxext_log.h"

#define HWLOG_TAG hwcxext_interface
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define EXTERN_CODEC_NAME  "hwcxext_codec"
#define DEFAULT_MCLK_FERQ 19200000

static unsigned int g_mclk;

SND_SOC_DAILINK_DEFS(hwcxext_codec,
	DAILINK_COMP_ARRAY(COMP_CPU("hwcxextcard_dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("hwcxext_codec", "HWCXEXT HiFi")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("pcm-codec")));

static int hwcxext_codec_init(struct snd_soc_pcm_runtime *rtd)
{
	int ret;
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);

	IN_FUNCTION;

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, g_mclk, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		hwlog_err("%s : set codec system clock failed %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static int hwcxext_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	unsigned int fmt = 0;
	int ret = 0;

	IN_FUNCTION;
	switch (params_channels(params)) {
	case 2: /* Stereo I2S mode */
		fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS;
		break;
	default:
		return -EINVAL;
	}

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (ret < 0) {
		hwlog_err("%s : set codec DAI configuration failed %d\n",
			__func__, ret);
		return ret;
	}

	/* set the codec mclk */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, g_mclk, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		hwlog_err("%s : set codec system clock failed %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_ops hwcxext_ops = {
	.hw_params = hwcxext_hw_params,
};

static struct snd_soc_dai_link hwcxext_dai_link[] = {
	{
		.name = "hwcxext_codec",
		.stream_name = "hwcxext_codec",
		.init = &hwcxext_codec_init,
		.ops = &hwcxext_ops,
		.nonatomic = true,
		SND_SOC_DAILINK_REG(hwcxext_codec),
	},
};

int hwcxext_codec_set_dailink_in_machine_driver(struct platform_device *pdev,
	struct snd_soc_dai_link *dai_link)
{
	int ret;
	const char *extern_codec_type = "huawei,extern_codec_type";
	const char *ptr = NULL;
	hwlog_info("%s function comein hwcxext cx11880 register in\n", __func__);

	if (pdev == NULL || dai_link == NULL) {
		hwlog_err("%s: params is invaild\n", __func__);
		return -EINVAL;
	}

	ret = of_property_read_string(pdev->dev.of_node,
		extern_codec_type, &ptr);
	if (ret) {
		hwlog_err("%s: of_property_read_string \n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: extern_codec_type: %s in dt node\n", __func__, ptr);
	if (strncmp(ptr, EXTERN_CODEC_NAME, strlen(EXTERN_CODEC_NAME))) {
		hwlog_err("%s: strncmp \n", __func__);
		return -EINVAL;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "extern_codec_clk", &g_mclk);
	if (ret < 0)
		g_mclk = DEFAULT_MCLK_FERQ;

	hwlog_info("%s: externcodec g_mclk is %u\n", __func__, g_mclk);

	if (memcpy_s(dai_link, sizeof(hwcxext_dai_link),
		hwcxext_dai_link, sizeof(hwcxext_dai_link)) != EOK) {
		hwlog_err("%s: memcpy_s is failed\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s function comein hwcxext cx11880 register ok\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(hwcxext_codec_set_dailink_in_machine_driver);

