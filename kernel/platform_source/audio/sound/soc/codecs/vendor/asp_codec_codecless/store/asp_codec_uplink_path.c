/*
 * asp_codec_uplink_path.c -- codec driver
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
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
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include "soc_mad.h"
#include "audio_log.h"
#include "asp_codec_type.h"
#include "asp_codec_store.h"

#define LOG_TAG "asp_codec_uplink_path"

#define MIC_UP_PGA_BYPASS_KCONTROLS \
	SOC_SINGLE("ADC1_UP_PGA_BYPASS", \
		ADC1_UP_PGA_CTRL_REG, ADC1_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC1_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC2_UP_PGA_BYPASS", \
		ADC2_UP_PGA_CTRL_REG, ADC2_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC2_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC3_UP_PGA_BYPASS", \
		ADC3_UP_PGA_CTRL_REG, ADC3_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC3_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC4_UP_PGA_BYPASS", \
		ADC4_UP_PGA_CTRL_REG, ADC4_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC4_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC5_UP_PGA_BYPASS", \
		ADC5_UP_PGA_CTRL_REG, ADC5_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC5_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC6_UP_PGA_BYPASS", \
		ADC6_UP_PGA_CTRL_REG, ADC6_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC6_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC7_UP_PGA_BYPASS", \
		ADC7_UP_PGA_CTRL_REG, ADC7_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC7_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC8_UP_PGA_BYPASS", \
		ADC8_UP_PGA_CTRL_REG, ADC8_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(ADC8_UP_PGA_BYPASS_LEN), 0) \

#define MIC_UP_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("AUDIO_L_UP_PGA_GAIN", \
		AUDIO_L_UP_PGA_CTRL_REG, AUDIO_L_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(AUDIO_L_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("AUDIO_L_UP_PGA_BYPASS", \
		AUDIO_L_UP_PGA_CTRL_REG, AUDIO_L_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(AUDIO_L_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("AUDIO_R_UP_PGA_GAIN", \
		AUDIO_R_UP_PGA_CTRL_REG, AUDIO_R_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(AUDIO_R_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("AUDIO_R_UP_PGA_BYPASS", \
		AUDIO_R_UP_PGA_CTRL_REG, AUDIO_R_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(AUDIO_R_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("VOICE_L_UP_PGA_GAIN", \
		VOICE_L_UP_PGA_CTRL_REG, VOICE_L_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(VOICE_L_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("VOICE_L_UP_PGA_BYPASS", \
		VOICE_L_UP_PGA_CTRL_REG, VOICE_L_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(VOICE_L_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("VOICE_R_UP_PGA_GAIN", \
		VOICE_R_UP_PGA_CTRL_REG, VOICE_R_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(VOICE_R_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("VOICE_R_UP_PGA_BYPASS", \
		VOICE_R_UP_PGA_CTRL_REG, VOICE_R_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(VOICE_R_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("MIC3_UP_PGA_GAIN", \
		MIC3_UP_PGA_CTRL_REG, MIC3_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(MIC3_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("MIC3_UP_PGA_BYPASS", \
		MIC3_UP_PGA_CTRL_REG, MIC3_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(MIC3_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("MIC4_UP_PGA_GAIN", \
		MIC4_UP_PGA_CTRL_REG, MIC4_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(MIC4_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("MIC4_UP_PGA_BYPASS", \
		MIC4_UP_PGA_CTRL_REG, MIC4_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(MIC4_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("MIC5_UP_PGA_GAIN", \
		MIC5_UP_PGA_CTRL_REG, MIC5_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(MIC5_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("MIC5_UP_PGA_BYPASS", \
		MIC5_UP_PGA_CTRL_REG, MIC5_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(MIC5_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("MIC6_UP_PGA_GAIN", \
		MIC6_UP_PGA_CTRL_REG, MIC6_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(MIC6_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("MIC6_UP_PGA_BYPASS", \
		MIC6_UP_PGA_CTRL_REG, MIC6_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(MIC6_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("MDM_5G_L_UP_PGA_GAIN", \
		MDM_5G_L_UP_PGA_CTRL_REG, MDM_5G_L_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(MDM_5G_L_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("MDM_5G_L_UP_PGA_BYPASS", \
		MDM_5G_L_UP_PGA_CTRL_REG, MDM_5G_L_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(MDM_5G_L_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("MDM_5G_R_UP_PGA_GAIN", \
		MDM_5G_R_UP_PGA_CTRL_REG, MDM_5G_R_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(MDM_5G_R_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("MDM_5G_R_UP_PGA_BYPASS", \
		MDM_5G_R_UP_PGA_CTRL_REG, MDM_5G_R_UP_PGA_BYPASS_OFFSET, \
		max_val_on_bit(MDM_5G_R_UP_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("ADC1_PGA_GAIN", \
		ADC1_UP_PGA_CTRL_REG, ADC1_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC1_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("ADC2_PGA_GAIN", \
		ADC2_UP_PGA_CTRL_REG, ADC2_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC2_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("ADC3_PGA_GAIN", \
		ADC3_UP_PGA_CTRL_REG, ADC3_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC3_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("ADC4_PGA_GAIN", \
		ADC4_UP_PGA_CTRL_REG, ADC4_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC4_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("ADC5_PGA_GAIN", \
		ADC5_UP_PGA_CTRL_REG, ADC5_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC5_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("ADC6_PGA_GAIN", \
		ADC6_UP_PGA_CTRL_REG, ADC6_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC6_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("ADC7_PGA_GAIN", \
		ADC7_UP_PGA_CTRL_REG, ADC7_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC7_UP_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("ADC8_PGA_GAIN", \
		ADC8_UP_PGA_CTRL_REG, ADC8_UP_PGA_GAIN_OFFSET, \
		max_val_on_bit(ADC8_UP_PGA_GAIN_LEN), 0) \

#define MIC_UP_SRC_KCONTROLS \
	SOC_SINGLE("VOICE_L_UP_SRC_CLKEN", \
		CODEC_CLK_EN1_REG, VOICE_UP_L_SRCDN_CLKEN_OFFSET, \
		max_val_on_bit(VOICE_UP_L_SRCDN_CLKEN_LEN), 0), \
	SOC_SINGLE("VOICE_R_UP_SRC_CLKEN", \
		CODEC_CLK_EN1_REG, VOICE_UP_R_SRCDN_CLKEN_OFFSET, \
		max_val_on_bit(VOICE_UP_R_SRCDN_CLKEN_LEN), 0) \

static const struct snd_kcontrol_new uplink_snd_kcontrols[] = {
	MIC_UP_PGA_BYPASS_KCONTROLS,
	MIC_UP_PGA_GAIN_KCONTROLS,
	MIC_UP_SRC_KCONTROLS,
};

#define MIC_INPUT_OUTPUT_WIDGET \
	SND_SOC_DAPM_INPUT("AUDIO_MIC12_INPUT"), \
	SND_SOC_DAPM_INPUT("VOICE_MIC12_INPUT"), \
	SND_SOC_DAPM_INPUT("AUDIO_MIC34_INPUT"), \
	SND_SOC_DAPM_INPUT("AUDIO_MIC56_INPUT"), \
	SND_SOC_DAPM_INPUT("ULTRA_UP_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC12_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC34_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC56_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC78_INPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_MIC12_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("VOICE_MIC12_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_MIC34_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_MIC56_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("ULTRA_UP_OUTPUT"), \
	SND_SOC_DAPM_INPUT("UPLINK_MIC_INPUT"), \
	SND_SOC_DAPM_OUTPUT("UPLINK_MIC_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DMIC12_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DMIC34_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DMIC56_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DMIC78_OUTPUT"), \
	SND_SOC_DAPM_INPUT("AU0_AMIC_INPUT"), \
	SND_SOC_DAPM_OUTPUT("AU0_AMIC_OUTPUT") \

#define ADC_DRV_WIDGET \
	SND_SOC_DAPM_OUT_DRV("ADC1_CIC_OUT_DRV", \
		ADC1_CIC_D16_CLKEN_REG, ADC1_CIC_D16_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("ADC2_CIC_OUT_DRV", \
		ADC2_CIC_D16_CLKEN_REG, ADC2_CIC_D16_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("ADC3_CIC_OUT_DRV", \
		ADC3_CIC_D16_CLKEN_REG, ADC3_CIC_D16_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("ADC4_CIC_OUT_DRV", \
		ADC4_CIC_D16_CLKEN_REG, ADC4_CIC_D16_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("ADC5_CIC_OUT_DRV", \
		ADC5_CIC_D16_CLKEN_REG, ADC5_CIC_D16_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("ADC6_CIC_OUT_DRV", \
		ADC6_CIC_D16_CLKEN_REG, ADC6_CIC_D16_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("ADC7_CIC_OUT_DRV", \
		ADC7_CIC_D16_CLKEN_REG, ADC7_CIC_D16_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("ADC8_CIC_OUT_DRV", \
		ADC8_CIC_D16_CLKEN_REG, ADC8_CIC_D16_CLKEN_OFFSET, 0, NULL, 0) \

/* mux widget */
static const char * const au_up_l_mux_texts[] = {
	"MIC1_ADC",
	"MIC2_ADC",
	"I2S2_RX_L",
	"MIXER4_L",
	"AUDIO_DN_L",
};

static const struct soc_enum au_up_l_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, AU_UP_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(au_up_l_mux_texts), au_up_l_mux_texts);
static const struct snd_kcontrol_new dapm_au_up_l_mux_controls =
	SOC_DAPM_ENUM("Mux", au_up_l_mux_enum);

static const char * const au_up_r_mux_texts[] = {
	"MIC1_ADC",
	"MIC2_ADC",
	"I2S2_RX_R",
	"MIXER4_R",
	"AUDIO_DN_R",
};

static const struct soc_enum au_up_r_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, AU_UP_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(au_up_r_mux_texts), au_up_r_mux_texts);

static const struct snd_kcontrol_new dapm_au_up_r_mux_controls =
	SOC_DAPM_ENUM("Mux", au_up_r_mux_enum);

static const char * const vo_up_l_mux_texts[] = {
	"MIC1_ADC",
	"MIC2_ADC",
	"I2S2_RX_L",
	"MIXER4_L",
	"CODEC3_DN_L",
};

static const struct soc_enum vo_up_l_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, VO_UP_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(vo_up_l_mux_texts), vo_up_l_mux_texts);

static const struct snd_kcontrol_new dapm_vo_up_l_mux_controls =
	SOC_DAPM_ENUM("Mux", vo_up_l_mux_enum);

static const char * const vo_up_r_mux_texts[] = {
	"MIC1_ADC",
	"MIC2_ADC",
	"I2S2_RX_R",
	"MIXER4_R",
	"CODEC3_DN_R",
};

static const struct soc_enum vo_up_r_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, VO_UP_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(vo_up_r_mux_texts), vo_up_r_mux_texts);

static const struct snd_kcontrol_new dapm_vo_up_r_mux_controls =
	SOC_DAPM_ENUM("Mux", vo_up_r_mux_enum);

static const char * const mic3_mux_texts[] = {
	"MIC3_ADC",
	"MIC4_ADC",
};

static const struct soc_enum mic3_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, MIC3_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic3_mux_texts), mic3_mux_texts);

static const struct snd_kcontrol_new dapm_mic3_mux_controls =
	SOC_DAPM_ENUM("Mux", mic3_mux_enum);

static const char * const mic4_mux_texts[] = {
	"MIC3_ADC",
	"MIC4_ADC",
};

static const struct soc_enum mic4_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, MIC4_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic4_mux_texts), mic4_mux_texts);

static const struct snd_kcontrol_new dapm_mic4_mux_controls =
	SOC_DAPM_ENUM("Mux", mic4_mux_enum);

static const char * const mic5_mux_texts[] = {
	"MIC3_ADC",
	"MIC5_ADC",
};

static const struct soc_enum mic5_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, MIC5_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic5_mux_texts), mic5_mux_texts);

static const struct snd_kcontrol_new dapm_mic5_mux_controls =
	SOC_DAPM_ENUM("Mux", mic5_mux_enum);

static const char * const mic6_mux_texts[] = {
	"MIC4_ADC",
	"MIC6_ADC",
};

static const struct soc_enum mic6_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, MIC6_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic6_mux_texts), mic6_mux_texts);

static const struct snd_kcontrol_new dapm_mic6_mux_controls =
	SOC_DAPM_ENUM("Mux", mic6_mux_enum);

static const char * const mdm_5g_up_l_mux_texts[] = {
	"MIXER4_L",
	"CODEC3_DN_L",
	"RESERVED",
	"ULTRA_DN_L",
	"MIC1_ADC",
};

static const struct soc_enum mdm_5g_up_l_mux_enum =
	SOC_ENUM_SINGLE(MDM_5G_UP_L_DIN_SEL_REG, MDM_5G_UP_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(mdm_5g_up_l_mux_texts), mdm_5g_up_l_mux_texts);

static const struct snd_kcontrol_new dapm_mdm_5g_up_l_mux_controls =
	SOC_DAPM_ENUM("Mux", mdm_5g_up_l_mux_enum);

static const char * const mdm_5g_up_r_mux_texts[] = {
	"MIXER4_R",
	"CODEC3_DN_R",
	"RESERVED",
	"ULTRA_DN_R",
	"MIC2_ADC",
};

static const struct soc_enum mdm_5g_up_r_mux_enum =
	SOC_ENUM_SINGLE(MDM_5G_UP_R_DIN_SEL_REG, MDM_5G_UP_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(mdm_5g_up_r_mux_texts), mdm_5g_up_r_mux_texts);

static const struct snd_kcontrol_new dapm_mdm_5g_up_r_mux_controls =
	SOC_DAPM_ENUM("Mux", mdm_5g_up_r_mux_enum);

static const char * const adc1_din_mux_texts[] = {
	"AU",
	"ADC1_CIC",
};

static const struct soc_enum adc1_din_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, ADC1_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc1_din_mux_texts), adc1_din_mux_texts);

static const struct snd_kcontrol_new dapm_adc1_din_mux_controls =
	SOC_DAPM_ENUM("Mux", adc1_din_mux_enum);

static const char * const adc2_din_mux_texts[] = {
	"AU",
	"ADC2_CIC",
};

static const struct soc_enum adc2_din_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, ADC2_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc2_din_mux_texts), adc2_din_mux_texts);

static const struct snd_kcontrol_new dapm_adc2_din_mux_controls =
	SOC_DAPM_ENUM("Mux", adc2_din_mux_enum);

static const char * const adc3_din_mux_texts[] = {
	"AU",
	"ADC3_CIC",
};

static const struct soc_enum adc3_din_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, ADC3_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc3_din_mux_texts), adc3_din_mux_texts);

static const struct snd_kcontrol_new dapm_adc3_din_mux_controls =
	SOC_DAPM_ENUM("Mux", adc3_din_mux_enum);

static const char * const adc4_din_mux_texts[] = {
	"AU",
	"ADC4_CIC",
};

static const struct soc_enum adc4_din_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, ADC4_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc4_din_mux_texts), adc4_din_mux_texts);

static const struct snd_kcontrol_new dapm_adc4_din_mux_controls =
	SOC_DAPM_ENUM("Mux", adc4_din_mux_enum);

static const char * const adc1_cic6_mux_texts[] = {
	"AU0_L",
	"AU0_R",
	"AU0_MIC3",
	"DMIC1_L",
};

static const struct soc_enum adc1_cic6_mux_enum =
	SOC_ENUM_SINGLE(ADC_DIN_SEL_CTRL_REG, ADC1_CIC_6STAGE_AU_SEL_OFFSET,
		ARRAY_SIZE(adc1_cic6_mux_texts), adc1_cic6_mux_texts);

static const struct snd_kcontrol_new adc1_cic6_mux_controls =
	SOC_DAPM_ENUM("Mux", adc1_cic6_mux_enum);

static const char * const adc2_cic6_mux_texts[] = {
	"AU0_L",
	"AU0_R",
	"AU0_MIC3",
	"DMIC1_R",
};

static const struct soc_enum adc2_cic6_mux_enum =
	SOC_ENUM_SINGLE(ADC_DIN_SEL_CTRL_REG, ADC2_CIC_6STAGE_AU_SEL_OFFSET,
		ARRAY_SIZE(adc2_cic6_mux_texts), adc2_cic6_mux_texts);

static const struct snd_kcontrol_new adc2_cic6_mux_controls =
	SOC_DAPM_ENUM("Mux", adc2_cic6_mux_enum);

static const char * const adc3_cic6_mux_texts[] = {
	"AU0_L",
	"AU0_R",
	"AU0_MIC3",
	"DMIC2_L",
};

static const struct soc_enum adc3_cic6_mux_enum =
	SOC_ENUM_SINGLE(ADC_DIN_SEL_CTRL_REG, ADC3_CIC_6STAGE_AU_SEL_OFFSET,
		ARRAY_SIZE(adc3_cic6_mux_texts), adc3_cic6_mux_texts);

static const struct snd_kcontrol_new adc3_cic6_mux_controls =
	SOC_DAPM_ENUM("Mux", adc3_cic6_mux_enum);

static const char * const adc4_cic6_mux_texts[] = {
	"AU0_L",
	"AU0_R",
	"AU0_MIC3",
	"DMIC2_R",
};

static const struct soc_enum adc4_cic6_mux_enum =
	SOC_ENUM_SINGLE(ADC_DIN_SEL_CTRL_REG, ADC4_CIC_6STAGE_AU_SEL_OFFSET,
		ARRAY_SIZE(adc4_cic6_mux_texts), adc4_cic6_mux_texts);

static const struct snd_kcontrol_new adc4_cic6_mux_controls =
	SOC_DAPM_ENUM("Mux", adc4_cic6_mux_enum);

#define MIC_UP_MUX_WIDGET \
	SND_SOC_DAPM_MUX("AUDIO_UP_L_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_au_up_l_mux_controls), \
	SND_SOC_DAPM_MUX("AUDIO_UP_R_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_au_up_r_mux_controls), \
	SND_SOC_DAPM_MUX("VOICE_UP_L_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_vo_up_l_mux_controls), \
	SND_SOC_DAPM_MUX("VOICE_UP_R_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_vo_up_r_mux_controls), \
	SND_SOC_DAPM_MUX("MIC3_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic3_mux_controls), \
	SND_SOC_DAPM_MUX("MIC4_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic4_mux_controls), \
	SND_SOC_DAPM_MUX("MIC5_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic5_mux_controls), \
	SND_SOC_DAPM_MUX("MIC6_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic6_mux_controls), \
	SND_SOC_DAPM_MUX("MDM_5G_UP_L_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mdm_5g_up_l_mux_controls), \
	SND_SOC_DAPM_MUX("MDM_5G_UP_R_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mdm_5g_up_r_mux_controls), \
	SND_SOC_DAPM_MUX("ADC1_DIN_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_adc1_din_mux_controls), \
	SND_SOC_DAPM_MUX("ADC2_DIN_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_adc2_din_mux_controls), \
	SND_SOC_DAPM_MUX("ADC3_DIN_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_adc3_din_mux_controls), \
	SND_SOC_DAPM_MUX("ADC4_DIN_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_adc4_din_mux_controls), \
	SND_SOC_DAPM_MUX("ADC1_CIC6_MUX", \
		SND_SOC_NOPM, 0, 0, &adc1_cic6_mux_controls), \
	SND_SOC_DAPM_MUX("ADC2_CIC6_MUX", \
		SND_SOC_NOPM, 0, 0, &adc2_cic6_mux_controls), \
	SND_SOC_DAPM_MUX("ADC3_CIC6_MUX", \
		SND_SOC_NOPM, 0, 0, &adc3_cic6_mux_controls), \
	SND_SOC_DAPM_MUX("ADC4_CIC6_MUX", \
		SND_SOC_NOPM, 0, 0, &adc4_cic6_mux_controls) \

static int adc1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC1_0P5_PGA_CLKEN_REG, BIT(ADC1_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC1_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC1_CLKEN_REG, BIT(ADC1_CLKEN_OFFSET),
			BIT(ADC1_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, ADC1_CLKEN_REG,
			BIT(ADC1_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, ADC1_0P5_PGA_CLKEN_REG,
			BIT(ADC1_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int adc2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC2_0P5_PGA_CLKEN_REG, BIT(ADC2_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC2_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC2_CLKEN_REG, BIT(ADC2_CLKEN_OFFSET),
			BIT(ADC2_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, ADC2_CLKEN_REG,
			BIT(ADC2_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, ADC2_0P5_PGA_CLKEN_REG,
			BIT(ADC2_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int adc3_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC3_0P5_PGA_CLKEN_REG, BIT(ADC3_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC3_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC3_CLKEN_REG, BIT(ADC3_CLKEN_OFFSET),
			BIT(ADC3_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, ADC3_CLKEN_REG,
			BIT(ADC3_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, ADC3_0P5_PGA_CLKEN_REG,
			BIT(ADC3_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int adc4_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC4_0P5_PGA_CLKEN_REG, BIT(ADC4_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC4_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC4_CLKEN_REG, BIT(ADC4_CLKEN_OFFSET),
			BIT(ADC4_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, ADC4_CLKEN_REG,
			BIT(ADC4_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, ADC4_0P5_PGA_CLKEN_REG,
			BIT(ADC4_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int adc5_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC5_0P5_PGA_CLKEN_REG, BIT(ADC5_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC5_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC5_CLKEN_REG, BIT(ADC5_CLKEN_OFFSET),
			BIT(ADC5_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, ADC5_CLKEN_REG,
			BIT(ADC5_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, ADC5_0P5_PGA_CLKEN_REG,
			BIT(ADC5_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int adc6_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC6_0P5_PGA_CLKEN_REG, BIT(ADC6_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC6_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC6_CLKEN_REG, BIT(ADC6_CLKEN_OFFSET),
			BIT(ADC6_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, ADC6_CLKEN_REG,
			BIT(ADC6_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, ADC6_0P5_PGA_CLKEN_REG,
			BIT(ADC6_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int adc7_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC7_0P5_PGA_CLKEN_REG, BIT(ADC7_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC7_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC7_CLKEN_REG, BIT(ADC7_CLKEN_OFFSET),
			BIT(ADC7_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, ADC7_CLKEN_REG,
			BIT(ADC7_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, ADC7_0P5_PGA_CLKEN_REG,
			BIT(ADC7_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int adc8_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, ADC8_0P5_PGA_CLKEN_REG, BIT(ADC8_0P5_PGA_CLKEN_OFFSET),
			BIT(ADC8_0P5_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, ADC8_CLKEN_REG, BIT(ADC8_CLKEN_OFFSET),
			BIT(ADC8_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, ADC8_CLKEN_REG,
			BIT(ADC8_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, ADC8_0P5_PGA_CLKEN_REG,
			BIT(ADC8_0P5_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

#define MIC_UP_ADC_PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("ADC1", 1, SND_SOC_NOPM, 0, 0, adc1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC2", 1, SND_SOC_NOPM, 0, 0, adc2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC3", 1, SND_SOC_NOPM, 0, 0, adc3_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC4", 1, SND_SOC_NOPM, 0, 0, adc4_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC5", 1, SND_SOC_NOPM, 0, 0, adc5_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC6", 1, SND_SOC_NOPM, 0, 0, adc6_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC7", 1, SND_SOC_NOPM, 0, 0, adc7_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC8", 1, SND_SOC_NOPM, 0, 0, adc8_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)) \

static const struct snd_kcontrol_new dapm_audio_up_o_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, AUDIO_UP_O_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice_up_o_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, VOICE_UP_O_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_mic34_up_o_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, MIC34_UP_O_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_mic56_up_o_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, MIC56_UP_O_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_dmic12_in_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", DMIC1_CLKEN_REG, DMIC1_CLKEN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_dmic34_in_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", DMIC2_CLKEN_REG, DMIC2_CLKEN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_dmic56_in_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_CLK_EN3_REG, DMIC3_CLKEN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_dmic78_in_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_CLK_EN3_REG, DMIC4_CLKEN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_au0_up_l_in_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", AU0_CTRL_REG, AU0_ADCL_EN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_au0_up_r_in_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", AU0_CTRL_REG, AU0_ADCR_EN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_cic_adc1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", ADC1_CIC_D16_CLKEN_REG, ADC1_CIC_D16_CLKEN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_cic_adc2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", ADC2_CIC_D16_CLKEN_REG, ADC2_CIC_D16_CLKEN_OFFSET, 1, 0);
static const struct snd_kcontrol_new dapm_mic78_up_o_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, MIC78_UP_O_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audio_up_2mic_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, AUDIO_UP_2MIC_48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice_up_2mic_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, VOICE_UP_2MIC_48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice_up_2mic_32k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, VOICE_UP_2MIC_32K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audio_up_4mic_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, AUDIO_UP_4MIC_48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice_up_4mic_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, VOICE_UP_4MIC_48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice_up_4mic_32k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, VOICE_UP_4MIC_32K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audio_up_6mic_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR0_REG_ADDR, AUDIO_UP_6MIC_48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_wakeup_main_mic_16k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, WAKEUP_MAIN_MIC_SWITCH_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_mdm5g_up_r_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, MDM5G_UP_48K_SWITCH_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_voice_up_4mic_8k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, VOICE_UP_4MIC_8K_BIT, 1, 0);

static void audio_src_and_pga_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, AUDIO_UP_PGA_CLKEN_REG,
		BIT(AUDIO_UP_L_PGA_CLKEN_OFFSET) | BIT(AUDIO_UP_R_PGA_CLKEN_OFFSET),
		BIT(AUDIO_UP_L_PGA_CLKEN_OFFSET) | BIT(AUDIO_UP_R_PGA_CLKEN_OFFSET));
	snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
		BIT(AUDIO_UP_L_SRCDN_CLKEN_OFFSET) | BIT(AUDIO_UP_R_SRCDN_CLKEN_OFFSET),
		BIT(AUDIO_UP_L_SRCDN_CLKEN_OFFSET) | BIT(AUDIO_UP_R_SRCDN_CLKEN_OFFSET));
}

static void audio_src_and_pga_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, AUDIO_UP_PGA_CLKEN_REG,
		BIT(AUDIO_UP_L_PGA_CLKEN_OFFSET) | BIT(AUDIO_UP_R_PGA_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
		BIT(AUDIO_UP_L_SRCDN_CLKEN_OFFSET) | BIT(AUDIO_UP_R_SRCDN_CLKEN_OFFSET), 0);
}

static void voice_src_and_pga_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, VOICE_UP_PGA_CLKEN_REG,
		BIT(VOICE_UP_L_PGA_CLKEN_OFFSET) | BIT(VOICE_UP_R_PGA_CLKEN_OFFSET),
		BIT(VOICE_UP_L_PGA_CLKEN_OFFSET) | BIT(VOICE_UP_R_PGA_CLKEN_OFFSET));
	snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
		BIT(VOICE_UP_L_SRCDN_CLKEN_OFFSET) | BIT(VOICE_UP_R_SRCDN_CLKEN_OFFSET),
		BIT(VOICE_UP_L_SRCDN_CLKEN_OFFSET) | BIT(VOICE_UP_R_SRCDN_CLKEN_OFFSET));
}

static void voice_src_and_pga_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, VOICE_UP_PGA_CLKEN_REG,
		BIT(VOICE_UP_L_PGA_CLKEN_OFFSET) | BIT(VOICE_UP_R_PGA_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
		BIT(VOICE_UP_L_SRCDN_CLKEN_OFFSET) | BIT(VOICE_UP_R_SRCDN_CLKEN_OFFSET), 0);
}

static void mic34_src_and_pga_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, MIC3_UP_PGA_CLKEN_REG, BIT(MIC3_UP_PGA_CLKEN_OFFSET),
		BIT(MIC3_UP_PGA_CLKEN_OFFSET));
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN1_REG, BIT(MIC4_UP_PGA_CLKEN_OFFSET),
		BIT(MIC4_UP_PGA_CLKEN_OFFSET));
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN1_REG, BIT(MIC3_UP_SRCDN_CLKEN_OFFSET),
		BIT(MIC3_UP_SRCDN_CLKEN_OFFSET));
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN1_REG, BIT(MIC4_UP_SRCDN_CLKEN_OFFSET),
		BIT(MIC4_UP_SRCDN_CLKEN_OFFSET));
}

static void mic34_src_and_pga_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, MIC3_UP_PGA_CLKEN_REG, BIT(MIC3_UP_PGA_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN1_REG, BIT(MIC4_UP_PGA_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN1_REG, BIT(MIC3_UP_SRCDN_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN1_REG, BIT(MIC4_UP_SRCDN_CLKEN_OFFSET), 0);
}

static void mic56_src_and_pga_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, MIC56_UP_PGA_CLKEN_REG,
		BIT(MIC5_UP_PGA_CLKEN_OFFSET) | BIT(MIC6_UP_PGA_CLKEN_OFFSET),
		BIT(MIC5_UP_PGA_CLKEN_OFFSET) | BIT(MIC6_UP_PGA_CLKEN_OFFSET));
	snd_soc_component_update_bits(codec, MIC56_UP_SRCDN_CLKEN_REG,
		BIT(MIC5_UP_SRCDN_CLKEN_OFFSET) | BIT(MIC6_UP_SRCDN_CLKEN_OFFSET),
		BIT(MIC5_UP_SRCDN_CLKEN_OFFSET) | BIT(MIC6_UP_SRCDN_CLKEN_OFFSET));
}

static void mic56_src_and_pga_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, MIC56_UP_PGA_CLKEN_REG,
		BIT(MIC6_UP_PGA_CLKEN_OFFSET) | BIT(MIC5_UP_PGA_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits(codec, MIC56_UP_SRCDN_CLKEN_REG,
		BIT(MIC5_UP_SRCDN_CLKEN_OFFSET) | BIT(MIC6_UP_SRCDN_CLKEN_OFFSET), 0);
}

static void audio_fifo_clk_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET) | BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET), 0);
}

static void audio_fifo_clk_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET) | BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET),
		BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET) | BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET));
}

static void voice_fifo_clk_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET) | BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET), 0);
}

static void voice_fifo_clk_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET) | BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET),
		BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET) | BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET));
}

static void mic34_fifo_clk_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(MIC3_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC4_UP_AFIFO_CLKEN_OFFSET), 0);
}

static void mic34_fifo_clk_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(MIC3_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC4_UP_AFIFO_CLKEN_OFFSET),
		BIT(MIC3_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC4_UP_AFIFO_CLKEN_OFFSET));
}

static void mic56_fifo_clk_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, MIC56_UP_AFIFO_CLKEN_REG,
		BIT(MIC5_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC6_UP_AFIFO_CLKEN_OFFSET), 0);
}

static void mic56_fifo_clk_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, MIC56_UP_AFIFO_CLKEN_REG,
		BIT(MIC5_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC6_UP_AFIFO_CLKEN_OFFSET),
		BIT(MIC5_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC6_UP_AFIFO_CLKEN_OFFSET));
}

static void voice_up_8k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, SRCDN_CTRL0_REG,
		mask_on_bit(VOICE_L_UP_SRCDN_SRC_MODE_LEN, VOICE_L_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(VOICE_R_UP_SRCDN_SRC_MODE_LEN, VOICE_R_UP_SRCDN_SRC_MODE_OFFSET),
		0x1 << VOICE_L_UP_SRCDN_SRC_MODE_OFFSET | 0x1 << VOICE_R_UP_SRCDN_SRC_MODE_OFFSET);
	snd_soc_component_update_bits(codec, FS_CTRL1_REG,
		mask_on_bit(FS_VOICE_L_UP_AFIFO_LEN, FS_VOICE_L_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_VOICE_R_UP_AFIFO_LEN, FS_VOICE_R_UP_AFIFO_OFFSET), 0);
	snd_soc_component_update_bits(codec, FS_VOICE_UP_PGA_REG,
		mask_on_bit(FS_VOICE_L_UP_PGA_LEN, FS_VOICE_L_UP_PGA_OFFSET) |
		mask_on_bit(FS_VOICE_R_UP_PGA_LEN, FS_VOICE_R_UP_PGA_OFFSET), 0);
	snd_soc_component_update_bits(codec, FS_CTRL4_REG,
		mask_on_bit(FS_VOICE_UP_L_SRCDN_DOUT_LEN, FS_VOICE_UP_L_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_VOICE_UP_R_SRCDN_DOUT_LEN, FS_VOICE_UP_R_SRCDN_DOUT_OFFSET), 0);
}

static void voice_up_32k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, SRCDN_CTRL0_REG,
		mask_on_bit(VOICE_L_UP_SRCDN_SRC_MODE_LEN, VOICE_L_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(VOICE_R_UP_SRCDN_SRC_MODE_LEN, VOICE_R_UP_SRCDN_SRC_MODE_OFFSET), 0);
	snd_soc_component_update_bits(codec, FS_CTRL1_REG,
		mask_on_bit(FS_VOICE_L_UP_AFIFO_LEN, FS_VOICE_L_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_VOICE_R_UP_AFIFO_LEN, FS_VOICE_R_UP_AFIFO_OFFSET),
		0x2 << FS_VOICE_L_UP_AFIFO_OFFSET | 0x2 << FS_VOICE_R_UP_AFIFO_OFFSET);
	snd_soc_component_update_bits(codec, FS_VOICE_UP_PGA_REG,
		mask_on_bit(FS_VOICE_L_UP_PGA_LEN, FS_VOICE_L_UP_PGA_OFFSET) |
		mask_on_bit(FS_VOICE_R_UP_PGA_LEN, FS_VOICE_R_UP_PGA_OFFSET),
		0x2 << FS_VOICE_L_UP_PGA_OFFSET | 0x2 << FS_VOICE_R_UP_PGA_OFFSET);
	snd_soc_component_update_bits(codec, FS_CTRL4_REG,
		mask_on_bit(FS_VOICE_UP_L_SRCDN_DOUT_LEN, FS_VOICE_UP_L_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_VOICE_UP_R_SRCDN_DOUT_LEN, FS_VOICE_UP_R_SRCDN_DOUT_OFFSET),
		0x2 << FS_VOICE_UP_L_SRCDN_DOUT_OFFSET | 0x2 << FS_VOICE_UP_R_SRCDN_DOUT_OFFSET);
}

static void voice_up_48k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, SRCDN_CTRL0_REG,
		mask_on_bit(VOICE_L_UP_SRCDN_SRC_MODE_LEN, VOICE_L_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(VOICE_R_UP_SRCDN_SRC_MODE_LEN, VOICE_R_UP_SRCDN_SRC_MODE_OFFSET),
		0x3 << VOICE_L_UP_SRCDN_SRC_MODE_OFFSET | 0x3 << VOICE_R_UP_SRCDN_SRC_MODE_OFFSET);
	snd_soc_component_update_bits(codec, FS_CTRL1_REG,
		mask_on_bit(FS_VOICE_L_UP_AFIFO_LEN, FS_VOICE_L_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_VOICE_R_UP_AFIFO_LEN, FS_VOICE_R_UP_AFIFO_OFFSET),
		0x3 << FS_VOICE_L_UP_AFIFO_OFFSET | 0x3 << FS_VOICE_R_UP_AFIFO_OFFSET);
	snd_soc_component_update_bits(codec, FS_VOICE_UP_PGA_REG,
		mask_on_bit(FS_VOICE_L_UP_PGA_LEN, FS_VOICE_L_UP_PGA_OFFSET) |
		mask_on_bit(FS_VOICE_R_UP_PGA_LEN, FS_VOICE_R_UP_PGA_OFFSET),
		0x3 << FS_VOICE_L_UP_PGA_OFFSET | 0x3 << FS_VOICE_R_UP_PGA_OFFSET);
	snd_soc_component_update_bits(codec, FS_CTRL4_REG,
		mask_on_bit(FS_VOICE_UP_L_SRCDN_DOUT_LEN, FS_VOICE_UP_L_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_VOICE_UP_R_SRCDN_DOUT_LEN, FS_VOICE_UP_R_SRCDN_DOUT_OFFSET),
		0x3 << FS_VOICE_UP_L_SRCDN_DOUT_OFFSET | 0x3 << FS_VOICE_UP_R_SRCDN_DOUT_OFFSET);
}

static void voice_up_power_on(struct snd_soc_component *codec)
{
	voice_src_and_pga_set(codec);
	voice_fifo_clk_clear(codec);
	udelay(CLEAR_FIFO_DELAY_LEN_US);
	voice_fifo_clk_set(codec);
}

static void voice_up_power_off(struct snd_soc_component *codec)
{
	voice_src_and_pga_clear(codec);
	voice_fifo_clk_clear(codec);
}

static void mic34_up_48k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, SRCDN_CTRL0_REG,
		mask_on_bit(MIC3_UP_SRCDN_SRC_MODE_LEN, MIC3_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(MIC4_UP_SRCDN_SRC_MODE_LEN, MIC4_UP_SRCDN_SRC_MODE_OFFSET),
		0x3 << MIC3_UP_SRCDN_SRC_MODE_OFFSET | 0x3 << MIC4_UP_SRCDN_SRC_MODE_OFFSET);
	snd_soc_component_update_bits(codec, FS_CTRL1_REG,
		mask_on_bit(FS_MIC3_UP_AFIFO_LEN, FS_MIC3_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_MIC4_UP_AFIFO_LEN, FS_MIC4_UP_AFIFO_OFFSET),
		0x3 << FS_MIC3_UP_AFIFO_OFFSET | 0x3 << FS_MIC4_UP_AFIFO_OFFSET);
	snd_soc_component_update_bits(codec, FS_CTRL2_REG,
		mask_on_bit(FS_MIC3_UP_PGA_LEN, FS_MIC3_UP_PGA_OFFSET) |
		mask_on_bit(FS_MIC4_UP_PGA_LEN, FS_MIC4_UP_PGA_OFFSET),
		0x3 << FS_MIC3_UP_PGA_OFFSET | 0x3 << FS_MIC4_UP_PGA_OFFSET);
	snd_soc_component_update_bits(codec, FS_CTRL5_REG,
		mask_on_bit(FS_MIC3_SRCDN_DOUT_LEN, FS_MIC3_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_MIC4_SRCDN_DOUT_LEN, FS_MIC4_SRCDN_DOUT_OFFSET),
		0x3 << FS_MIC3_SRCDN_DOUT_OFFSET | 0x3 << FS_MIC4_SRCDN_DOUT_OFFSET);
}

static void mic56_up_8k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, MIC56_UP_SRCDN_SRC_MODE_REG,
		mask_on_bit(MIC5_UP_SRCDN_SRC_MODE_LEN, MIC5_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(MIC6_UP_SRCDN_SRC_MODE_LEN, MIC6_UP_SRCDN_SRC_MODE_OFFSET),
		0x1 << MIC5_UP_SRCDN_SRC_MODE_OFFSET | 0x1 << MIC6_UP_SRCDN_SRC_MODE_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_AFIFO_REG,
		mask_on_bit(FS_MIC5_UP_AFIFO_LEN, FS_MIC5_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_MIC6_UP_AFIFO_LEN, FS_MIC6_UP_AFIFO_OFFSET), 0);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_PGA_REG,
		mask_on_bit(FS_MIC5_UP_PGA_LEN, FS_MIC5_UP_PGA_OFFSET) |
		mask_on_bit(FS_MIC6_UP_PGA_LEN, FS_MIC6_UP_PGA_OFFSET), 0);
	snd_soc_component_update_bits(codec, FS_MIC56_SRCDN_DOUT_REG,
		mask_on_bit(FS_MIC5_SRCDN_DOUT_LEN, FS_MIC5_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_MIC6_SRCDN_DOUT_LEN, FS_MIC6_SRCDN_DOUT_OFFSET), 0);
}

static void mic56_up_16k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, MIC56_UP_SRCDN_SRC_MODE_REG,
		mask_on_bit(MIC5_UP_SRCDN_SRC_MODE_LEN, MIC5_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(MIC6_UP_SRCDN_SRC_MODE_LEN, MIC6_UP_SRCDN_SRC_MODE_OFFSET),
		0x2 << MIC5_UP_SRCDN_SRC_MODE_OFFSET | 0x2 << MIC6_UP_SRCDN_SRC_MODE_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_AFIFO_REG,
		mask_on_bit(FS_MIC5_UP_AFIFO_LEN, FS_MIC5_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_MIC6_UP_AFIFO_LEN, FS_MIC6_UP_AFIFO_OFFSET),
		0x1 << FS_MIC5_UP_AFIFO_OFFSET | 0x1 << FS_MIC6_UP_AFIFO_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_PGA_REG,
		mask_on_bit(FS_MIC5_UP_PGA_LEN, FS_MIC5_UP_PGA_OFFSET) |
		mask_on_bit(FS_MIC6_UP_PGA_LEN, FS_MIC6_UP_PGA_OFFSET),
		0x1 << FS_MIC5_UP_PGA_OFFSET | 0x1 << FS_MIC6_UP_PGA_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_SRCDN_DOUT_REG,
		mask_on_bit(FS_MIC5_SRCDN_DOUT_LEN, FS_MIC5_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_MIC6_SRCDN_DOUT_LEN, FS_MIC6_SRCDN_DOUT_OFFSET),
		0x1 << FS_MIC5_SRCDN_DOUT_OFFSET | 0x1 << FS_MIC6_SRCDN_DOUT_OFFSET);
}

static void mic56_up_32k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, MIC56_UP_SRCDN_SRC_MODE_REG,
		mask_on_bit(MIC5_UP_SRCDN_SRC_MODE_LEN, MIC5_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(MIC6_UP_SRCDN_SRC_MODE_LEN, MIC6_UP_SRCDN_SRC_MODE_OFFSET), 0);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_AFIFO_REG,
		mask_on_bit(FS_MIC5_UP_AFIFO_LEN, FS_MIC5_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_MIC6_UP_AFIFO_LEN, FS_MIC6_UP_AFIFO_OFFSET),
		0x2 << FS_MIC5_UP_AFIFO_OFFSET | 0x2 << FS_MIC6_UP_AFIFO_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_PGA_REG,
		mask_on_bit(FS_MIC5_UP_PGA_LEN, FS_MIC5_UP_PGA_OFFSET) |
		mask_on_bit(FS_MIC6_UP_PGA_LEN, FS_MIC6_UP_PGA_OFFSET),
		0x2 << FS_MIC5_UP_PGA_OFFSET | 0x2 << FS_MIC6_UP_PGA_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_SRCDN_DOUT_REG,
		mask_on_bit(FS_MIC5_SRCDN_DOUT_LEN, FS_MIC5_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_MIC6_SRCDN_DOUT_LEN, FS_MIC6_SRCDN_DOUT_OFFSET),
		0x2 << FS_MIC5_SRCDN_DOUT_OFFSET | 0x2 << FS_MIC6_SRCDN_DOUT_OFFSET);
}

static void mic56_up_48k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, MIC56_UP_SRCDN_SRC_MODE_REG,
		mask_on_bit(MIC5_UP_SRCDN_SRC_MODE_LEN, MIC5_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(MIC6_UP_SRCDN_SRC_MODE_LEN, MIC6_UP_SRCDN_SRC_MODE_OFFSET),
		0x3 << MIC5_UP_SRCDN_SRC_MODE_OFFSET | 0x3 << MIC6_UP_SRCDN_SRC_MODE_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_AFIFO_REG,
		mask_on_bit(FS_MIC5_UP_AFIFO_LEN, FS_MIC5_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_MIC6_UP_AFIFO_LEN, FS_MIC6_UP_AFIFO_OFFSET),
		0x3 << FS_MIC5_UP_AFIFO_OFFSET | 0x3 << FS_MIC6_UP_AFIFO_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_UP_PGA_REG,
		mask_on_bit(FS_MIC5_UP_PGA_LEN, FS_MIC5_UP_PGA_OFFSET) |
		mask_on_bit(FS_MIC6_UP_PGA_LEN, FS_MIC6_UP_PGA_OFFSET),
		0x3 << FS_MIC5_UP_PGA_OFFSET | 0x3 << FS_MIC6_UP_PGA_OFFSET);
	snd_soc_component_update_bits(codec, FS_MIC56_SRCDN_DOUT_REG,
		mask_on_bit(FS_MIC5_SRCDN_DOUT_LEN, FS_MIC5_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_MIC6_SRCDN_DOUT_LEN, FS_MIC6_SRCDN_DOUT_OFFSET),
		0x3 << FS_MIC5_SRCDN_DOUT_OFFSET | 0x3 << FS_MIC6_SRCDN_DOUT_OFFSET);
}

static void mdm5g_up_48k_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits(codec, MDM_5G_UP_SRCDN_SRC_MODE_REG,
		mask_on_bit(MDM_5G_L_UP_SRCDN_SRC_MODE_LEN, MDM_5G_L_UP_SRCDN_SRC_MODE_OFFSET) |
		mask_on_bit(MDM_5G_R_UP_SRCDN_SRC_MODE_LEN, MDM_5G_R_UP_SRCDN_SRC_MODE_OFFSET),
		0x3 << MDM_5G_L_UP_SRCDN_SRC_MODE_OFFSET | 0x3 << MDM_5G_R_UP_SRCDN_SRC_MODE_OFFSET);
	snd_soc_component_update_bits(codec, FS_MDM_5G_SRCDN_OUT_REG,
		mask_on_bit(FS_MDM_5G_L_SRCDN_DOUT_LEN, FS_MDM_5G_L_SRCDN_DOUT_OFFSET) |
		mask_on_bit(FS_MDM_5G_R_SRCDN_DOUT_LEN, FS_MDM_5G_R_SRCDN_DOUT_OFFSET),
		0x3 << FS_MDM_5G_L_SRCDN_DOUT_OFFSET | 0x3 << FS_MDM_5G_R_SRCDN_DOUT_OFFSET);
	snd_soc_component_update_bits(codec, FS_MDM_5G_UP_PGA_REG,
		mask_on_bit(FS_MDM_5G_L_UP_PGA_LEN, FS_MDM_5G_L_UP_PGA_OFFSET) |
		mask_on_bit(FS_MDM_5G_R_UP_PGA_LEN, FS_MDM_5G_R_UP_PGA_OFFSET),
		0x3 << FS_MDM_5G_L_UP_PGA_OFFSET | 0x3 << FS_MDM_5G_R_UP_PGA_OFFSET);
	snd_soc_component_update_bits(codec, FS_MDM_5G_UP_AFIFO_REG,
		mask_on_bit(FS_MDM_5G_L_UP_AFIFO_LEN, FS_MDM_5G_L_UP_AFIFO_OFFSET) |
		mask_on_bit(FS_MDM_5G_R_UP_AFIFO_LEN, FS_MDM_5G_R_UP_AFIFO_OFFSET),
		0x3 << FS_MDM_5G_L_UP_AFIFO_OFFSET | 0x3 << FS_MDM_5G_R_UP_AFIFO_OFFSET);
}

static int audio_up_2mic_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		audio_src_and_pga_set(codec);
		audio_fifo_clk_clear(codec);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		audio_fifo_clk_set(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		audio_src_and_pga_clear(codec);
		audio_fifo_clk_clear(codec);
		break;
	default:
		AUDIO_LOGW("audio 2mic 48k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int audio_up_4mic_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;
	AUDIO_LOGI("4mic 48k power event %d", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, CODEC_SW_RST_N_REG,
			BIT(RST_4MIC_AUDIO_ACCESS_IRQ_OFFSET), BIT(RST_4MIC_AUDIO_ACCESS_IRQ_OFFSET));
		mic34_up_48k_set(codec);
		audio_src_and_pga_set(codec);
		mic34_src_and_pga_set(codec);
		audio_fifo_clk_clear(codec);
		mic34_fifo_clk_clear(codec);
		snd_soc_component_update_bits(codec, CODEC_SW_RST_N_REG, BIT(RST_4MIC_AUDIO_ACCESS_IRQ_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_US);
		audio_fifo_clk_set(codec);
		mic34_fifo_clk_set(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		audio_fifo_clk_clear(codec);
		mic34_fifo_clk_clear(codec);
		audio_src_and_pga_clear(codec);
		mic34_src_and_pga_clear(codec);
		break;
	default:
		AUDIO_LOGW("4mic 48k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int audio_up_6mic_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, CODEC_SW_RST_N_REG,
			BIT(RST_8MIC_UP_ACCESS_IRQ_OFFSET), BIT(RST_8MIC_UP_ACCESS_IRQ_OFFSET));

		mic34_up_48k_set(codec);
		mic56_up_48k_set(codec);
		audio_src_and_pga_set(codec);
		mic34_src_and_pga_set(codec);
		mic56_src_and_pga_set(codec);
		audio_fifo_clk_clear(codec);
		mic34_fifo_clk_clear(codec);
		mic56_fifo_clk_clear(codec);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		audio_fifo_clk_set(codec);
		mic34_fifo_clk_set(codec);
		mic56_fifo_clk_set(codec);

		snd_soc_component_update_bits(codec, CODEC_SW_RST_N_REG, BIT(RST_8MIC_UP_ACCESS_IRQ_OFFSET), 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		audio_src_and_pga_clear(codec);
		mic34_src_and_pga_clear(codec);
		mic56_src_and_pga_clear(codec);
		audio_fifo_clk_clear(codec);
		mic34_fifo_clk_clear(codec);
		mic56_fifo_clk_clear(codec);
		break;
	default:
		AUDIO_LOGW("4mic power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int voice_up_2mic_32k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		voice_up_32k_set(codec);
		voice_up_power_on(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		voice_up_power_off(codec);
		break;
	default:
		AUDIO_LOGW("voice 2mic 32k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int voice_up_2mic_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("voice up 2mic 48k power event: %d", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		voice_up_48k_set(codec);
		snd_soc_component_update_bits_with_lock(codec, VOICE_UP_PGA_CLKEN_REG,
			BIT(VOICE_UP_L_PGA_CLKEN_OFFSET) | BIT(VOICE_UP_R_PGA_CLKEN_OFFSET),
			BIT(VOICE_UP_L_PGA_CLKEN_OFFSET) | BIT(VOICE_UP_R_PGA_CLKEN_OFFSET));
		voice_fifo_clk_clear(codec);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		voice_fifo_clk_set(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, VOICE_UP_PGA_CLKEN_REG,
			BIT(VOICE_UP_L_PGA_CLKEN_OFFSET) | BIT(VOICE_UP_R_PGA_CLKEN_OFFSET), 0);
		voice_fifo_clk_clear(codec);
		break;
	default:
		AUDIO_LOGW("voice 2mic 48k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static void voice_4mic_up_power_on(struct snd_soc_component *codec)
{
	IN_FUNCTION;

	voice_src_and_pga_set(codec);
	mic56_src_and_pga_set(codec);
	voice_fifo_clk_clear(codec);
	mic56_fifo_clk_clear(codec);
	udelay(CLEAR_FIFO_DELAY_LEN_US);
	voice_fifo_clk_set(codec);
	mic56_fifo_clk_set(codec);

	OUT_FUNCTION;
}

static void voice_4mic_up_power_off(struct snd_soc_component *codec)
{
	voice_fifo_clk_clear(codec);
	mic56_fifo_clk_clear(codec);
	mic56_src_and_pga_clear(codec);
	voice_src_and_pga_clear(codec);
}

static int voice_up_4mic_8k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		voice_up_8k_set(codec);
		mic56_up_8k_set(codec);
		voice_4mic_up_power_on(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		voice_4mic_up_power_off(codec);
		break;
	default:
		AUDIO_LOGW("voice up 4mic 8k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int voice_up_4mic_32k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		voice_up_32k_set(codec);
		mic56_up_32k_set(codec);
		voice_4mic_up_power_on(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		voice_4mic_up_power_off(codec);
		break;
	default:
		AUDIO_LOGW("voice up 4mic 32k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int voice_up_4mic_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		voice_up_48k_set(codec);
		mic56_up_48k_set(codec);
		voice_4mic_up_power_on(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		voice_4mic_up_power_off(codec);
		break;
	default:
		AUDIO_LOGW("voice up 4mic 48k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int wakeup_main_mic_16k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		mic56_fifo_clk_clear(codec);
		mic56_up_16k_set(codec);
		mic56_src_and_pga_set(codec);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		mic56_fifo_clk_set(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		mic56_fifo_clk_clear(codec);
		mic56_src_and_pga_clear(codec);
		break;
	default:
		AUDIO_LOGW("wakeup main mic 16k power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int ultra_up_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, MIC78_UP_AFIFO_CLKEN_REG,
			BIT(MIC7_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC8_UP_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, MIC78_UP_PGA_CLKEN_REG,
			BIT(MIC7_UP_PGA_CLKEN_OFFSET) | BIT(MIC8_UP_PGA_CLKEN_OFFSET),
			BIT(MIC7_UP_PGA_CLKEN_OFFSET) | BIT(MIC8_UP_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits(codec, MIC78_UP_SRCDN_CLKEN_REG,
			BIT(MIC7_UP_SRCDN_CLKEN_OFFSET) | BIT(MIC8_UP_SRCDN_CLKEN_OFFSET),
			BIT(MIC7_UP_SRCDN_CLKEN_OFFSET) | BIT(MIC8_UP_SRCDN_CLKEN_OFFSET));

		snd_soc_component_update_bits_with_lock(codec, MIC78_UP_AFIFO_CLKEN_REG,
			BIT(MIC7_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC8_UP_AFIFO_CLKEN_OFFSET),
			BIT(MIC7_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC8_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, MIC78_UP_AFIFO_CLKEN_REG,
			BIT(MIC7_UP_AFIFO_CLKEN_OFFSET) | BIT(MIC8_UP_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, MIC78_UP_SRCDN_CLKEN_REG,
			BIT(MIC7_UP_SRCDN_CLKEN_OFFSET) | BIT(MIC8_UP_SRCDN_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, MIC78_UP_PGA_CLKEN_REG,
			BIT(MIC7_UP_PGA_CLKEN_OFFSET) | BIT(MIC8_UP_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("ultra up power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int mdm5g_up_r_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		mdm5g_up_48k_set(codec);
		snd_soc_component_update_bits_with_lock(codec, MDM_5G_R_UP_AFIFO_CLKEN_REG,
			BIT(MDM_5G_R_UP_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, MDM_5G_R_UP_PGA_CLKEN_REG,
			BIT(MDM_5G_R_UP_PGA_CLKEN_OFFSET), BIT(MDM_5G_R_UP_PGA_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, MDM_5G_R_UP_AFIFO_CLKEN_REG,
			BIT(MDM_5G_R_UP_AFIFO_CLKEN_OFFSET), BIT(MDM_5G_R_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, MDM_5G_R_UP_AFIFO_CLKEN_REG,
			BIT(MDM_5G_R_UP_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, MDM_5G_R_UP_PGA_CLKEN_REG,
			BIT(MDM_5G_R_UP_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("mdm 5g up r power event err: %d", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

#define MIC_UP_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH("AUDIO_UP_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_audio_up_o_switch_controls), \
	SND_SOC_DAPM_SWITCH("VOICE_UP_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_voice_up_o_switch_controls), \
	SND_SOC_DAPM_SWITCH("MIC34_UP_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_mic34_up_o_switch_controls), \
	SND_SOC_DAPM_SWITCH("MIC56_UP_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_mic56_up_o_switch_controls), \
	SND_SOC_DAPM_SWITCH("DMIC12_IN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_dmic12_in_switch_controls), \
	SND_SOC_DAPM_SWITCH("DMIC34_IN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_dmic34_in_switch_controls), \
	SND_SOC_DAPM_SWITCH("DMIC56_IN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_dmic56_in_switch_controls), \
	SND_SOC_DAPM_SWITCH("DMIC78_IN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_dmic78_in_switch_controls), \
	SND_SOC_DAPM_SWITCH("CIC_ADC1_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_cic_adc1_switch_controls), \
	SND_SOC_DAPM_SWITCH("CIC_ADC2_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_cic_adc2_switch_controls), \
	SND_SOC_DAPM_SWITCH("AU0_UP_L_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_au0_up_l_in_switch_controls), \
	SND_SOC_DAPM_SWITCH("AU0_UP_R_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_au0_up_r_in_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("ULTRA_UP_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_mic78_up_o_switch_controls, ultra_up_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIO_UP_2MIC_48K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_audio_up_2mic_48k_switch_controls, audio_up_2mic_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE_UP_2MIC_48K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_voice_up_2mic_48k_switch_controls, voice_up_2mic_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE_UP_2MIC_32K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_voice_up_2mic_32k_switch_controls, voice_up_2mic_32k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIO_UP_4MIC_48K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_audio_up_4mic_48k_switch_controls, audio_up_4mic_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE_UP_4MIC_48K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_voice_up_4mic_48k_switch_controls, voice_up_4mic_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE_UP_4MIC_32K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_voice_up_4mic_32k_switch_controls, voice_up_4mic_32k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE_UP_4MIC_8K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_voice_up_4mic_8k_switch_controls, voice_up_4mic_8k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIO_UP_6MIC_48K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_audio_up_6mic_48k_switch_controls, audio_up_6mic_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("WAKEUP_MAIN_MIC_16K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_wakeup_main_mic_16k_switch_controls, wakeup_main_mic_16k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MDM5G_UP_R_48K_O_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_mdm5g_up_r_48k_switch_controls, mdm5g_up_r_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)) \

#ifdef CONFIG_SOC_MAD
static int dmic_clk_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, DMIC_DIV_REG,
			mask_on_bit(FS_DMIC1_LEN, FS_DMIC1_OFFSET) |
			mask_on_bit(FS_DMIC2_LEN, FS_DMIC2_OFFSET),
			0x1 << FS_DMIC1_OFFSET | 0x1 << FS_DMIC2_OFFSET);
		soc_mad_request_pinctrl_state(HIGH_FREQ_MODE);
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, DMIC_DIV_REG,
			mask_on_bit(FS_DMIC1_LEN, FS_DMIC1_OFFSET) |
			mask_on_bit(FS_DMIC2_LEN, FS_DMIC2_OFFSET),
			0x7 << FS_DMIC1_OFFSET | 0x7 << FS_DMIC2_OFFSET);
		soc_mad_release_pinctrl_state(HIGH_FREQ_MODE);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}
#else
static int dmic_clk_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return 0;
}
#endif

#define SUPPLY_WIDGET \
	SND_SOC_DAPM_SUPPLY("DMIC_CLK", SND_SOC_NOPM, 0, 0, \
		dmic_clk_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)) \

static const struct snd_soc_dapm_widget uplink_dapm_widgets[] = {
	MIC_INPUT_OUTPUT_WIDGET,
	ADC_DRV_WIDGET,
	MIC_UP_SWITCH_WIDGET,
	MIC_UP_MUX_WIDGET,
	MIC_UP_ADC_PGA_WIDGET,
	SUPPLY_WIDGET,
};

static const struct snd_codec_resource uplink_resource = {
	"ASP_CODEC_UPLINK",
	uplink_snd_kcontrols,
	ARRAY_SIZE(uplink_snd_kcontrols),
	uplink_dapm_widgets,
	ARRAY_SIZE(uplink_dapm_widgets),
};

int __init add_uplink_path_in_list(void)
{
	return add_codec_base_dapm_list(&uplink_resource);
}
device_initcall(add_uplink_path_in_list);
