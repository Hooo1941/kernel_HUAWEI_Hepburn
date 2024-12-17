/*
 * asp_codec_sidetone_path.c -- codec driver
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

#include <sound/soc.h>
#include "audio_log.h"
#include "asp_codec_type.h"
#include "asp_codec_store.h"

#define LOG_TAG "Analog_less_v2_sidetone_path"

#define SIDETONE_PGA_BYPASS_KCONTROLS \
	SOC_SINGLE("ASP_CODEC_SIDETONE_PGA_BYPASS", \
		SIDETONE_PGA_CTRL_REG, SIDETONE_PGA_BYPASS_OFFSET, \
		max_val_on_bit(SIDETONE_PGA_BYPASS_LEN), 0) \

#define SIDETONE_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("ASP_CODEC_SIDETONE_PGA_GAIN", \
		SIDETONE_PGA_CTRL_REG, SIDETONE_PGA_GAIN_OFFSET, \
		max_val_on_bit(SIDETONE_PGA_GAIN_LEN), 0) \

static const struct snd_kcontrol_new sidetone_snd_kcontrols[] = {
	SIDETONE_PGA_BYPASS_KCONTROLS,
	SIDETONE_PGA_GAIN_KCONTROLS,
};

#define SIDETONE_INPUT_OUTPUT_WIDGET \
	SND_SOC_DAPM_INPUT("SIDETONE_INPUT"), \
	SND_SOC_DAPM_OUTPUT("SIDETONE_OUTPUT") \

static const struct snd_kcontrol_new dapm_sidetone_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, SIDETONE_SWITCH_CLKEN_BIT, 1, 0);

static int sidetone_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG, BIT(SIDETONE_PGA_CLKEN_OFFSET),
			BIT(SIDETONE_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(SIDETONE_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

#define SIDETONE_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("SIDETONE_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_sidetone_switch_controls, sidetone_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)) \

static const char * const sidetone_pag_din_mux_texts[] = {
	"MIC1_ADC",
	"MIC2_ADC",
	"MIC3_ADC",
	"MIC4_ADC",
	"MIC5_ADC",
	"MIC6_ADC",
	"MIC7_ADC",
	"MIC8_ADC",
};

static const struct soc_enum sidetone_pag_din_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, SIDETONE_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(sidetone_pag_din_mux_texts),
		sidetone_pag_din_mux_texts);

static const struct snd_kcontrol_new dapm_sidetone_pag_din_mux_controls =
	SOC_DAPM_ENUM("Mux", sidetone_pag_din_mux_enum);

#define SIDETONE_MUX_WIDGET \
	SND_SOC_DAPM_MUX("SIDETONE_PAG_DIN_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_sidetone_pag_din_mux_controls) \

static const struct snd_soc_dapm_widget sidetone_dapm_widgets[] = {
	SIDETONE_INPUT_OUTPUT_WIDGET,
	SIDETONE_SWITCH_WIDGET,
	SIDETONE_MUX_WIDGET,
};

static const struct snd_codec_resource sidetone_resource = {
	"ASP_CODEC_SIDETONE",
	sidetone_snd_kcontrols,
	ARRAY_SIZE(sidetone_snd_kcontrols),
	sidetone_dapm_widgets,
	ARRAY_SIZE(sidetone_dapm_widgets),
};

int __init add_sidetone_path_in_list(void)
{
	return add_codec_base_dapm_list(&sidetone_resource);
}
device_initcall(add_sidetone_path_in_list);
