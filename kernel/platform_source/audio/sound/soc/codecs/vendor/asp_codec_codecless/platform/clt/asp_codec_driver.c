/*
 * asp_codec.c -- codec driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include "audio_log.h"
#include "asp_codec_utils.h"
#include "asp_codec_type.h"
#include "asp_codec_control.h"

#define LOG_TAG "asp_codec"

static struct snd_soc_component *soc_codec;

static struct asp_codec_reg_page g_reg_array[] = {
	{NULL, PAGE_SOCCODEC, \
	 PAGE_SOCCODEC + SOCCODEC_END_OFFSET, false, true},
	{NULL, PAGE_ASPCFG, \
	 PAGE_ASPCFG + ASPCFG_END_OFFSET, false, true},
	{NULL, PAGE_AO_IOC, \
	 PAGE_AO_IOC + AOIOC_END_OFFSET, false, false},
	{NULL, PAGE_IOC, \
	 PAGE_IOC + IOC_END_OFFSET, false, false},
	{NULL, PAGE_AXI2TDM, \
	 PAGE_AXI2TDM + AXI2TDM_END, false, true},
	{NULL, PAGE_MAD, \
	 PAGE_MAD + MAD_END, false, false},
	{NULL, PAGE_VIRCODEC, \
	 PAGE_VIRCODEC + VIRCODEC_END_OFFSET, true, false},
};

static int micbias_en_ctl_gpio_init(int gpio)
{
	if (!gpio_is_valid(gpio)) {
		AUDIO_LOGE("gpio is not valid");
		return -EINVAL;
	}

	if (gpio_request(gpio, "micbias_en_ctl")) {
		AUDIO_LOGE("micbias_en_ctl failed");
		return -EINVAL;
	}

	if (gpio_direction_output(gpio, 0)) {
		gpio_free(gpio);
		AUDIO_LOGE("micbias en gpio set output failed");
		return -EINVAL;
	}

	return 0;
}

static int get_micbias_en_ctl_resource(struct snd_soc_component *codec)
{
	int gpio;
	int ret;
	const char *micbias_en_gpio_str = "hisilicon,micbias_en_gpio";
	struct device_node *node = NULL;
	struct codec_platform_data *priv = snd_soc_component_get_drvdata(codec);

	if (priv == NULL)
		return 0;

	node = codec->dev->of_node;
	if (node == NULL) {
		priv->micbias_en_ctl.need_gpio_ctl = false;
		return 0;
	}

	if (of_property_read_bool(node, micbias_en_gpio_str)) {
		priv->micbias_en_ctl.need_gpio_ctl = true;
		AUDIO_LOGI("need config micbias en ctl");
	} else {
		priv->micbias_en_ctl.need_gpio_ctl = false;
		return 0;
	}

	gpio = of_get_named_gpio(node, micbias_en_gpio_str, 0);
	if (gpio < 0) {
		AUDIO_LOGI("%s of_get_named_gpio failed, %d\n",
			micbias_en_gpio_str, gpio);
		ret = of_property_read_u32(node, micbias_en_gpio_str,
			(u32 *)&(gpio));
		if (ret < 0) {
			priv->micbias_en_ctl.need_gpio_ctl = false;
			AUDIO_LOGE("%s read gpio prop failed, %d\n",
				micbias_en_gpio_str, ret);
			return ret;
		}
	}
	AUDIO_LOGI("gpio:%d", gpio);
	ret = micbias_en_ctl_gpio_init(gpio);
	if (ret < 0) {
		priv->micbias_en_ctl.need_gpio_ctl = false;
		AUDIO_LOGE("gpio resource init fail, ret = %d", ret);
		return ret;
	}
	priv->micbias_en_ctl.gpio = gpio;
	return 0;
}

static void asp_codec_shutdown(struct platform_device *device)
{
	if (soc_codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	/* set audio down pga -120db */
	snd_soc_component_update_bits(soc_codec, AUDIO_L_DN_PGA_CTRL_REG,
		max_val_on_bit(AUDIO_L_DN_PGA_GAIN_LEN) <<
		AUDIO_L_DN_PGA_GAIN_OFFSET, 0x88 <<
		AUDIO_L_DN_PGA_GAIN_OFFSET);
	snd_soc_component_update_bits(soc_codec, AUDIO_R_DN_PGA_CTRL_REG,
		max_val_on_bit(AUDIO_R_DN_PGA_GAIN_LEN) <<
		AUDIO_R_DN_PGA_GAIN_OFFSET, 0x88 <<
		AUDIO_R_DN_PGA_GAIN_OFFSET);

	/* set codec3 down pga -120db */
	snd_soc_component_update_bits(soc_codec, CODEC3_L_DN_PGA_CTRL_REG,
		max_val_on_bit(CODEC3_L_DN_PGA_GAIN_LEN) <<
		CODEC3_L_DN_PGA_GAIN_OFFSET, 0x88 <<
		CODEC3_L_DN_PGA_GAIN_OFFSET);
	snd_soc_component_update_bits(soc_codec, CODEC3_R_DN_PGA_CTRL_REG,
		max_val_on_bit(CODEC3_R_DN_PGA_GAIN_LEN) <<
		CODEC3_R_DN_PGA_GAIN_OFFSET, 0x88 <<
		CODEC3_R_DN_PGA_GAIN_OFFSET);

	/* set dacl mixer4 pga -12db */
	snd_soc_component_update_bits(soc_codec, DACL_MIXER4_CTRL0_REG,
		max_val_on_bit(DACL_MIXER4_GAIN1_LEN) <<
		DACL_MIXER4_GAIN1_OFFSET, 0);
	snd_soc_component_update_bits(soc_codec, DACL_MIXER4_CTRL0_REG,
		max_val_on_bit(DACL_MIXER4_GAIN2_LEN) <<
		DACL_MIXER4_GAIN2_OFFSET, 0);
	snd_soc_component_update_bits(soc_codec, DACL_MIXER4_CTRL0_REG,
		max_val_on_bit(DACL_MIXER4_GAIN3_LEN) <<
		DACL_MIXER4_GAIN3_OFFSET, 0);
	snd_soc_component_update_bits(soc_codec, DACL_MIXER4_CTRL0_REG,
		max_val_on_bit(DACL_MIXER4_GAIN4_LEN) <<
		DACL_MIXER4_GAIN4_OFFSET, 0);

	/* set dacr mixer4 pga -12db */
	snd_soc_component_update_bits(soc_codec, DACR_MIXER4_CTRL0_REG,
		max_val_on_bit(DACR_MIXER4_GAIN1_LEN) <<
		DACR_MIXER4_GAIN1_OFFSET, 0);
	snd_soc_component_update_bits(soc_codec, DACR_MIXER4_CTRL0_REG,
		max_val_on_bit(DACR_MIXER4_GAIN2_LEN) <<
		DACR_MIXER4_GAIN2_OFFSET, 0);
	snd_soc_component_update_bits(soc_codec, DACR_MIXER4_CTRL0_REG,
		max_val_on_bit(DACR_MIXER4_GAIN3_LEN) <<
		DACR_MIXER4_GAIN3_OFFSET, 0);
	snd_soc_component_update_bits(soc_codec, DACR_MIXER4_CTRL0_REG,
		max_val_on_bit(DACR_MIXER4_GAIN4_LEN) <<
		DACR_MIXER4_GAIN4_OFFSET, 0);
}

static void asp_codec_priv_data_init(struct snd_soc_component *codec)
{
	int ret;
	soc_codec = codec;

	ret = get_micbias_en_ctl_resource(codec);
	if (ret < 0)
		AUDIO_LOGE("get micbias en ctl failed");
}

static void asp_codec_priv_data_deinit(struct snd_soc_component *codec)
{
	soc_codec = NULL;
}

static void asp_codec_reg_init(struct snd_soc_component *codec)
{
	/* adc pga bypass */
	snd_soc_component_update_bits(codec, ADC1_UP_PGA_CTRL_REG, BIT(ADC1_UP_PGA_BYPASS_OFFSET),
		BIT(ADC1_UP_PGA_BYPASS_OFFSET));
	snd_soc_component_update_bits(codec, ADC2_UP_PGA_CTRL_REG, BIT(ADC2_UP_PGA_BYPASS_OFFSET),
		BIT(ADC2_UP_PGA_BYPASS_OFFSET));
	snd_soc_component_update_bits(codec, ADC3_UP_PGA_CTRL_REG, BIT(ADC3_UP_PGA_BYPASS_OFFSET),
		BIT(ADC3_UP_PGA_BYPASS_OFFSET));
	snd_soc_component_update_bits(codec, ADC4_UP_PGA_CTRL_REG, BIT(ADC4_UP_PGA_BYPASS_OFFSET),
		BIT(ADC4_UP_PGA_BYPASS_OFFSET));
	snd_soc_component_update_bits(codec, ADC5_UP_PGA_CTRL_REG, BIT(ADC5_UP_PGA_BYPASS_OFFSET),
		BIT(ADC5_UP_PGA_BYPASS_OFFSET));
	snd_soc_component_update_bits(codec, ADC6_UP_PGA_CTRL_REG, BIT(ADC6_UP_PGA_BYPASS_OFFSET),
		BIT(ADC6_UP_PGA_BYPASS_OFFSET));
	snd_soc_component_update_bits(codec, ADC7_UP_PGA_CTRL_REG, BIT(ADC7_UP_PGA_BYPASS_OFFSET),
		BIT(ADC7_UP_PGA_BYPASS_OFFSET));
	snd_soc_component_update_bits(codec, ADC8_UP_PGA_CTRL_REG, BIT(ADC8_UP_PGA_BYPASS_OFFSET),
		BIT(ADC8_UP_PGA_BYPASS_OFFSET));
	/* set i2s2 frq 8k */
	snd_soc_component_update_bits(codec, I2S2_PCM_CTRL_REG, max_val_on_bit(FS_I2S2_LEN) << FS_I2S2_OFFSET, 0);
	snd_soc_component_update_bits(codec, I2S1_CTRL_REG,
		max_val_on_bit(I2S1_TX_CLK_SEL_LEN) << I2S1_TX_CLK_SEL_OFFSET,
		BIT(I2S1_TX_CLK_SEL_OFFSET));

	/* dma select */
	snd_soc_component_write(codec, R_DMA_SEL, 0x8A008A0);

	/* dmic 1/2 768k */
	snd_soc_component_update_bits(codec, DMIC_DIV_REG,
		mask_on_bit(FS_DMIC1_LEN, FS_DMIC1_OFFSET) | mask_on_bit(FS_DMIC2_LEN, FS_DMIC2_OFFSET),
		0x7 << FS_DMIC1_OFFSET | 0x7 << FS_DMIC2_OFFSET);

	/* mem ctrl */
	snd_soc_component_update_bits(codec, MEM_CTRL_S_REG,
		max_val_on_bit(MEM_CTRL_S_LEN) << MEM_CTRL_S_OFFSET, 0x15858);
	snd_soc_component_update_bits(codec, MEM_CTRL_1W2R_REG,
		max_val_on_bit(MEM_CTRL_1W2R_LEN) << MEM_CTRL_1W2R_OFFSET, 0x850);
}

static void asp_codec_reset(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, R_RST_CTRLEN, BIT(RST_EN_CODEC_N));
	snd_soc_component_write(codec, R_RST_CTRLDIS, BIT(RST_DISEN_CODEC_N));
	snd_soc_component_write(codec, R_GATE_EN, BIT(GT_CODEC_CLK));
	/* init AX2TDM */
	snd_soc_component_write(codec, R_RST_CTRLEN, BIT(RST_EN_AXI2TDM_N));
	snd_soc_component_write(codec, R_RST_CTRLDIS, BIT(RST_DISEN_AXI2TDM_N));
	snd_soc_component_write(codec, R_GATE_EN, BIT(GT_AXI2TDM_CLK));
}

static void asp_codec_fpga_pinctrl_init(struct snd_soc_component *codec)
{
	AUDIO_LOGI("set asp gpio pinctrl");
}

struct codec_platform_data g_codec_platform_data = {
	.base.ops = {
		.priv_data_init = asp_codec_priv_data_init,
		.priv_data_deinit = asp_codec_priv_data_deinit,
		.asp_reg_init = asp_codec_reg_init,
		.reset_asp_codec = asp_codec_reset,
		.shutdown = asp_codec_shutdown,
		.fpga_pinctrl_init = asp_codec_fpga_pinctrl_init,
	},
	.base.v_codec_num = VIR_REG_CNT,
	.base.platform_reg_array = &g_reg_array[0],
	.base.platform_reg_size = ARRAY_SIZE(g_reg_array),
};

static int __init asp_codec_data_init(void)
{
	asp_codec_driver_init(&(g_codec_platform_data.base));
	return 0;
}

arch_initcall(asp_codec_data_init);
