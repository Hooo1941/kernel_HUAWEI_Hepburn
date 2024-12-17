/*
 * asp_codec_machine.c
 *
 * asp_codec_machine driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/of.h>
#include <sound/soc.h>
#include <securec.h>
#include "audio_log.h"

#define LOG_TAG                              "asp_codec_machine"

#define DAI_LINK_CODECLESS_NAME              "asp-codec-codecless"
#define DAI_LINK_CODECLESS_DAI_NAME          "asp-codec-codecless-dai"
#define ASP_CODEC_MACHINE_DRIVER_NAME        "analog-less-machine"
#define ASP_CODEC_MACHINE_CARD_NAME          "analog-less-machine-card"

#define DAI_LINK_ANA_CODEC_NAME              "ana-codec"
#define DAI_LINK_ANA_CODEC_DAI_NAME          "ana-codec-dai"
#define CODEC_ANA_MACHINE_CARD_NAME          "codec-ana-machine-card"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
SND_SOC_DAILINK_DEFS(analog_less_machine_pb_normal,
	DAILINK_COMP_ARRAY(COMP_CPU("asp-pcm-mm")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("asp-pcm-hifi")));

SND_SOC_DAILINK_DEFS(analog_less_machine_voice,
	DAILINK_COMP_ARRAY(COMP_CPU("asp-pcm-modem")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(analog_less_machine_fm1,
	DAILINK_COMP_ARRAY(COMP_CPU("asp-pcm-fm")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(analog_less_machine_pb_dsp,
	DAILINK_COMP_ARRAY(COMP_CPU("asp-pcm-lpp")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("asp-pcm-hifi")));

SND_SOC_DAILINK_DEFS(analog_less_machine_pb_direct,
	DAILINK_COMP_ARRAY(COMP_CPU("asp-pcm-direct")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("asp-pcm-hifi")));

SND_SOC_DAILINK_DEFS(analog_less_machine_lowlatency,
	DAILINK_COMP_ARRAY(COMP_CPU("asp-pcm-fast")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("asp-pcm-hifi")));

SND_SOC_DAILINK_DEFS(analog_less_machine_mmap,
	DAILINK_COMP_ARRAY(COMP_CPU("audio-pcm-mmap")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("asp-pcm-hifi")));

SND_SOC_DAILINK_DEFS(analog_less_machine_spatial_audio,
	DAILINK_COMP_ARRAY(COMP_CPU("asp-pcm-spatial-audio")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_CODECLESS_NAME, DAI_LINK_CODECLESS_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("asp-pcm-hifi")));

SND_SOC_DAILINK_DEFS(codec_ana_machine_normal,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC(DAI_LINK_ANA_CODEC_NAME, DAI_LINK_ANA_CODEC_DAI_NAME)),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

#endif

static struct snd_soc_dai_link g_ana_dai_link[] = {
	{
		.name           = "codec_ana_machine_normal",
		.stream_name    = "codec_ana_machine_normal",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(codec_ana_machine_normal),
#else
		.codec_name     = DAI_LINK_ANA_CODEC_NAME,
		.cpu_dai_name   = "snd-soc-dummy-dai",
		.codec_dai_name = DAI_LINK_ANA_CODEC_DAI_NAME,
		.platform_name  = "snd-soc-dummy",
#endif
	},
};


static struct snd_soc_dai_link g_analog_less_dai_link[] = {
	{
		.name           = "analog_less_machine_pb_normal",
		.stream_name    = "analog_less_machine_pb_normal",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_pb_normal),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "asp-pcm-mm",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "asp-pcm-hifi",
#endif
	},
	{
		.name           = "analog_less_machine_voice",
		.stream_name    = "analog_less_machine_voice",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_voice),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "asp-pcm-modem",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "snd-soc-dummy",
#endif
	},
	{
		.name           = "analog_less_machine_fm1",
		.stream_name    = "analog_less_machine_fm1",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_fm1),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "asp-pcm-fm",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "snd-soc-dummy",
#endif
	},
	{
		.name           = "analog_less_machine_pb_dsp",
		.stream_name    = "analog_less_machine_pb_dsp",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_pb_dsp),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "asp-pcm-lpp",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "asp-pcm-hifi",
#endif
	},
	{
		.name           = "analog_less_machine_pb_direct",
		.stream_name    = "analog_less_machine_pb_direct",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_pb_direct),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "asp-pcm-direct",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "asp-pcm-hifi",
#endif
	},
	{
		.name           = "analog_less_machine_lowlatency",
		.stream_name    = "analog_less_machine_lowlatency",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_lowlatency),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "asp-pcm-fast",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "asp-pcm-hifi",
#endif
	},
	{
		.name           = "analog_less_machine_mmap",
		.stream_name    = "analog_less_machine_mmap",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_mmap),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "audio-pcm-mmap",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "asp-pcm-hifi",
#endif
	},
	{
		.name           = "analog_less_machine_spatial_audio",
		.stream_name    = "analog_less_machine_spatial_audio",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		SND_SOC_DAILINK_REG(analog_less_machine_spatial_audio),
#else
		.codec_name     = DAI_LINK_CODECLESS_NAME,
		.cpu_dai_name   = "asp-pcm-spatial-audio",
		.codec_dai_name = DAI_LINK_CODECLESS_DAI_NAME,
		.platform_name  = "asp-pcm-hifi",
#endif
	},
};

static struct snd_soc_dai_link g_codecless_ana_dai_links[
	ARRAY_SIZE(g_analog_less_dai_link) + ARRAY_SIZE(g_ana_dai_link)];

static void populate_extern_snd_ana_card_dailinks(struct platform_device *pdev)
{
	memcpy_s(g_codecless_ana_dai_links, sizeof(g_codecless_ana_dai_links),
		g_analog_less_dai_link, sizeof(g_analog_less_dai_link));
	memcpy_s(g_codecless_ana_dai_links + ARRAY_SIZE(g_analog_less_dai_link),
		sizeof(g_codecless_ana_dai_links) - sizeof(g_analog_less_dai_link),
		g_ana_dai_link, sizeof(g_ana_dai_link));
}

static struct snd_soc_card g_analog_less_card = {
	.name      = ASP_CODEC_MACHINE_CARD_NAME,
	.owner     = THIS_MODULE,
	.dai_link  = g_analog_less_dai_link,
	.num_links = ARRAY_SIZE(g_analog_less_dai_link),
};

static struct snd_soc_card g_codeless_ana_card = {
	.name      = CODEC_ANA_MACHINE_CARD_NAME,
	.owner     = THIS_MODULE,
	.dai_link  = g_codecless_ana_dai_links,
	.num_links = ARRAY_SIZE(g_codecless_ana_dai_links),
};


static int analog_less_probe(struct platform_device *pdev)
{
	int ret;
	struct snd_soc_card *card = &g_analog_less_card;

	if (!pdev) {
		AUDIO_LOGE("pdev is null, fail");
		return -EINVAL;
	}

	AUDIO_LOGI("analog less probe");

	if (of_property_read_bool(pdev->dev.of_node, "is_support_ana")) {
		AUDIO_LOGI("This platform has ana mode\n");
		populate_extern_snd_ana_card_dailinks(pdev);
		card = &g_codeless_ana_card;
	}


	card->dev = &pdev->dev;

	ret = snd_soc_register_card(card);
	if (ret != 0)
		AUDIO_LOGE("sound card register failed %d", ret);

	return ret;
}

static int analog_less_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	if (card)
		snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id g_analog_less_of_match[] = {
	{ .compatible = "hisilicon,analog-less-machine", },
	{ },
};
MODULE_DEVICE_TABLE(of, g_analog_less_of_match);

static struct platform_driver g_analog_less_driver = {
	.driver  = {
		.name           = ASP_CODEC_MACHINE_DRIVER_NAME,
		.owner          = THIS_MODULE,
		.of_match_table = g_analog_less_of_match,
	},
	.probe  = analog_less_probe,
	.remove = analog_less_remove,
};

static int __init analog_less_init(void)
{
	AUDIO_LOGI("analog less init");

	return platform_driver_register(&g_analog_less_driver);
}
late_initcall(analog_less_init);

static void __exit analog_less_exit(void)
{
	platform_driver_unregister(&g_analog_less_driver);
}
module_exit(analog_less_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ALSA SoC for analog less machine driver");
