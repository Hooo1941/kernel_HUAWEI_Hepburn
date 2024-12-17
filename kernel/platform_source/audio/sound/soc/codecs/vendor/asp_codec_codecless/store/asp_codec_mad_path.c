/*
 * asp_codec_mad_path.c codec driver.
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
 *
 */

#include <sound/soc.h>
#include "audio_log.h"
#include "asp_codec_type.h"
#include "asp_codec_store.h"
#include "asp_codec_mad_regs.h"
#include "soc_mad.h"

#define LOG_TAG "asp_codec_mad_path"

static const struct snd_kcontrol_new mad_lp_single_mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, MAD_LP_SINGLE_MIC_BIT, 1, 0);

static const struct snd_kcontrol_new mad_norm_single_mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, MAD_NORM_SINGLE_MIC_BIT, 1, 0);

#ifdef CONFIG_SOC_MAD
static int mad_lp_single_mic_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		soc_mad_request_pinctrl_state(LOW_FREQ_MODE);
		soc_mad_select_din(LOW_FREQ_MODE);

		soc_mad_hpf_config(LOW_FREQ_MODE, HPF_ENABLE);
		AUDIO_LOGI("mad lp switch PMU: %d", event);
		break;
	case SND_SOC_DAPM_POST_PMD:
		soc_mad_select_din(HIGH_FREQ_MODE);
		soc_mad_release_pinctrl_state(LOW_FREQ_MODE);
		soc_mad_hpf_config(LOW_FREQ_MODE, HPF_BYPASS);

		AUDIO_LOGI("mad lp switch PMD: %d", event);
		break;
	default:
		AUDIO_LOGE("mad power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}
#else
static int mad_lp_single_mic_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return 0;
}
#endif

#ifdef CONFIG_SOC_MAD
static int mad_norm_single_mic_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* data src select codecless */
		soc_mad_select_din(HIGH_FREQ_MODE);
		soc_mad_hpf_config(HIGH_FREQ_MODE, HPF_ENABLE);

		AUDIO_LOGI("mad norm switch PMU: %d", event);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* data src select DMIC */
		soc_mad_select_din(LOW_FREQ_MODE);
		soc_mad_hpf_config(HIGH_FREQ_MODE, HPF_BYPASS);

		AUDIO_LOGI("mad norm switch PMD: %d", event);
		break;
	default:
		AUDIO_LOGE("mad power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}
#else
static int mad_norm_single_mic_switch_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return 0;
}
#endif

static const struct snd_soc_dapm_widget mad_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("MAD_DMIC_INPUT"), \
	SND_SOC_DAPM_INPUT("MAD_CODECLESS_INPUT"), \
	SND_SOC_DAPM_SWITCH_E("MAD_LP_SINGLE_MIC_SWITCH", SND_SOC_NOPM, 0, 0, \
		&mad_lp_single_mic_switch_controls, mad_lp_single_mic_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MAD_NORM_SINGLE_MIC_SWITCH", SND_SOC_NOPM, 0, 0, \
		&mad_norm_single_mic_switch_controls, mad_norm_single_mic_switch_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUTPUT("MAD_DMIC_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("MAD_CODECLESS_OUTPUT") \
};

static const struct snd_codec_resource mad_resource = {
	"ASP_CODEC_MAD",
	NULL,
	0,
	mad_dapm_widgets,
	ARRAY_SIZE(mad_dapm_widgets),
};

int __init add_mad_path_in_list(void)
{
	return add_codec_base_dapm_list(&mad_resource);
}

device_initcall(add_mad_path_in_list);
