/*
 * asp_codec_btfm_path.c -- codec driver.
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

#define LOG_TAG "asp_codec_btfm_path"

#define FS_I2S2_SRC_KCONTROLS \
	SOC_SINGLE("I2S2_TX_L_SRCDN_CLKEN", \
		CODEC_CLK_EN2_REG, I2S2_TX_L_SRCDN_CLKEN_OFFSET, \
		max_val_on_bit(I2S2_TX_L_SRCDN_CLKEN_LEN), 0), \
	SOC_SINGLE("I2S2_TX_L_SRCDN_DIN", \
		FS_CTRL6_REG, FS_I2S2_TX_L_SRCDN_DIN_OFFSET, \
		max_val_on_bit(FS_I2S2_TX_L_SRCDN_DIN_LEN), 0), \
	SOC_SINGLE("I2S2_TX_L_SRCDN_DOUT", \
		FS_CTRL6_REG, FS_I2S2_TX_L_SRCDN_DOUT_OFFSET, \
		max_val_on_bit(FS_I2S2_TX_L_SRCDN_DOUT_LEN), 0), \
	SOC_SINGLE("I2S2_TX_L_SRCDN_MODE", \
		SRCDN_CTRL1_REG, I2S2_TX_L_SRCDN_SRC_MODE_OFFSET, \
		max_val_on_bit(I2S2_TX_L_SRCDN_SRC_MODE_LEN), 0), \
	SOC_SINGLE("I2S2_TX_R_SRCDN_CLKEN", \
		CODEC_CLK_EN2_REG, I2S2_TX_R_SRCDN_CLKEN_OFFSET, \
		max_val_on_bit(I2S2_TX_R_SRCDN_CLKEN_LEN), 0), \
	SOC_SINGLE("I2S2_TX_R_SRCDN_DIN", \
		FS_CTRL6_REG, FS_I2S2_TX_R_SRCDN_DIN_OFFSET, \
		max_val_on_bit(FS_I2S2_TX_R_SRCDN_DIN_LEN), 0), \
	SOC_SINGLE("I2S2_TX_R_SRCDN_DOUT", \
		FS_CTRL6_REG, FS_I2S2_TX_R_SRCDN_DOUT_OFFSET, \
		max_val_on_bit(FS_I2S2_TX_R_SRCDN_DOUT_LEN), 0), \
	SOC_SINGLE("I2S2_TX_R_SRCDN_MODE", \
		SRCDN_CTRL1_REG, I2S2_TX_R_SRCDN_SRC_MODE_OFFSET, \
		max_val_on_bit(I2S2_TX_R_SRCDN_SRC_MODE_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCUP_CLKEN", \
		I2S2_RX_L_SRCUP_CLKEN_REG, I2S2_RX_L_SRCUP_CLKEN_OFFSET, \
		max_val_on_bit(I2S2_RX_L_SRCUP_CLKEN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCUP_DIN", \
		FS_I2S2_RX_L_SRCUP_DIN_REG, FS_I2S2_RX_L_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_L_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCUP_DOUT", \
		FS_I2S2_RX_L_SRCUP_DOUT_REG, FS_I2S2_RX_L_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_L_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCUP_MODE", \
		I2S2_RX_L_SRCUP_SRC_MODE_REG, I2S2_RX_L_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(I2S2_RX_L_SRCUP_SRC_MODE_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCUP_CLKEN", \
		I2S2_RX_R_SRCUP_CLKEN_REG, I2S2_RX_R_SRCUP_CLKEN_OFFSET, \
		max_val_on_bit(I2S2_RX_R_SRCUP_CLKEN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCUP_DIN", \
		FS_I2S2_RX_R_SRCUP_DIN_REG, FS_I2S2_RX_R_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_R_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCUP_DOUT", \
		FS_I2S2_RX_R_SRCUP_DOUT_REG, FS_I2S2_RX_R_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_R_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCUP_MODE", \
		I2S2_RX_R_SRCUP_SRC_MODE_REG, I2S2_RX_R_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(I2S2_RX_R_SRCUP_SRC_MODE_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCDN_CLKEN", \
		I2S2_RX_L_SRCDN_CLKEN_REG, I2S2_RX_L_SRCDN_CLKEN_OFFSET, \
		max_val_on_bit(I2S2_RX_L_SRCDN_CLKEN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCDN_DIN", \
		FS_CTRL5_REG, FS_I2S2_RX_L_SRCDN_DIN_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_L_SRCDN_DIN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCDN_DOUT", \
		FS_CTRL5_REG, FS_I2S2_RX_L_SRCDN_DOUT_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_L_SRCDN_DOUT_LEN), 0), \
	SOC_SINGLE("I2S2_RX_L_SRCDN_MODE", \
		SRCDN_CTRL1_REG, I2S2_RX_L_SRCDN_SRC_MODE_OFFSET, \
		max_val_on_bit(I2S2_RX_L_SRCDN_SRC_MODE_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCDN_CLKEN", \
		I2S2_RX_R_SRCDN_CLKEN_REG, I2S2_RX_R_SRCDN_CLKEN_OFFSET, \
		max_val_on_bit(I2S2_RX_R_SRCDN_CLKEN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCDN_DIN", \
		FS_CTRL6_REG, FS_I2S2_RX_R_SRCDN_DIN_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_R_SRCDN_DIN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCDN_DOUT", \
		FS_CTRL6_REG, FS_I2S2_RX_R_SRCDN_DOUT_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_R_SRCDN_DOUT_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_SRCDN_MODE", \
		SRCDN_CTRL1_REG, I2S2_RX_R_SRCDN_SRC_MODE_OFFSET, \
		max_val_on_bit(I2S2_RX_R_SRCDN_SRC_MODE_LEN), 0) \

#define FS_MIXER_KCONTROLS \
	SOC_SINGLE("FS_I2S2_TX_MIXER2", \
		FS_I2S2_TX_MIXER2_REG, FS_I2S2_TX_MIXER2_OFFSET, \
		max_val_on_bit(FS_I2S2_TX_MIXER2_LEN), 0) \

#define FS_I2S2_PGA_KCONTYOLS \
	SOC_SINGLE("FS_I2S2_RX_L_PGA", \
		FS_CTRL3_REG, FS_I2S2_RX_L_PGA_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_L_PGA_LEN), 0), \
	SOC_SINGLE("FS_I2S2_RX_R_PGA", \
		FS_CTRL3_REG, FS_I2S2_RX_R_PGA_OFFSET, \
		max_val_on_bit(FS_I2S2_RX_R_PGA_LEN), 0) \

#define I2S2_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("I2S2_RX_L_PGA_GAIN", \
		I2S2_L_RX_PGA_CTRL_REG, I2S2_L_RX_PGA_GAIN_OFFSET, \
		max_val_on_bit(I2S2_L_RX_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("I2S2_RX_R_PGA_GAIN", \
		I2S2_R_RX_PGA_CTRL_REG, I2S2_R_RX_PGA_GAIN_OFFSET, \
		max_val_on_bit(I2S2_R_RX_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("I2S2_TX_MIXER2_S1_GAIN", \
		I2S2_TX_MIXER2_CTRL_REG, I2S2_TX_MIXER2_GAIN1_OFFSET, \
		max_val_on_bit(I2S2_TX_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("I2S2_TX_MIXER2_S2_GAIN", \
		I2S2_TX_MIXER2_CTRL_REG, I2S2_TX_MIXER2_GAIN2_OFFSET, \
		max_val_on_bit(I2S2_TX_MIXER2_GAIN2_LEN), 0) \

#define I2S2_PGA_BYPASS_KCONTROLS \
	SOC_SINGLE("I2S2_R_RX_PGA_BYPASS", \
		I2S2_R_RX_PGA_CTRL_REG, I2S2_R_RX_PGA_BYPASS_OFFSET, \
		max_val_on_bit(I2S2_R_RX_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("I2S2_L_RX_PGA_BYPASS", \
		I2S2_L_RX_PGA_CTRL_REG, I2S2_L_RX_PGA_BYPASS_OFFSET, \
		max_val_on_bit(I2S2_L_RX_PGA_BYPASS_LEN), 0) \

#define S2_CFG_KCONTROLS \
	SOC_SINGLE("FS_S2_IF_I2S", \
		I2S2_PCM_CTRL_REG, FS_I2S2_OFFSET, \
		max_val_on_bit(FS_I2S2_LEN), 0), \
	SOC_SINGLE("I2S2_FUNC_MODE", \
		I2S2_PCM_CTRL_REG, I2S2_FUNC_MODE_OFFSET, \
		max_val_on_bit(I2S2_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("I2S2_I2S_DIRECT_MODE", \
		I2S2_PCM_CTRL_REG, I2S2_DIRECT_LOOP_OFFSET, \
		max_val_on_bit(I2S2_DIRECT_LOOP_LEN), 0), \
	SOC_SINGLE("I2S2_FRAME_MODE", \
		I2S2_PCM_CTRL_REG, I2S2_FRAME_MODE_OFFSET, \
		max_val_on_bit(I2S2_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("I2S2_RX_CLK_SEL", \
		I2S2_PCM_CTRL_REG, I2S2_RX_CLK_SEL_OFFSET, \
		max_val_on_bit(I2S2_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("I2S2_TX_CLK_SEL", \
		I2S2_PCM_CTRL_REG, I2S2_TX_CLK_SEL_OFFSET, \
		max_val_on_bit(I2S2_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("I2S2_CODEC_IO_WORDLENGTH", \
		I2S2_PCM_CTRL_REG, I2S2_CODEC_IO_WORDLENGTH_OFFSET, \
		max_val_on_bit(I2S2_CODEC_IO_WORDLENGTH_LEN), 0), \
	SOC_SINGLE("I2S2_LRCLK_MODE", \
		I2S2_PCM_CTRL_REG, I2S2_LRCLK_MODE_OFFSET, \
		max_val_on_bit(I2S2_LRCLK_MODE_LEN), 0) \

static const struct snd_kcontrol_new btfm_snd_kcontrols[] = {
	FS_I2S2_SRC_KCONTROLS,
	FS_MIXER_KCONTROLS,
	FS_I2S2_PGA_KCONTYOLS,
	I2S2_PGA_GAIN_KCONTROLS,
	I2S2_PGA_BYPASS_KCONTROLS,
	S2_CFG_KCONTROLS,
};

static int s2_tx_l_srcdn_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(I2S2_TX_L_SRCDN_CLKEN_OFFSET),
			BIT(I2S2_TX_L_SRCDN_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(I2S2_TX_L_SRCDN_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s2_tx_r_srcdn_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(I2S2_TX_R_SRCDN_CLKEN_OFFSET),
			BIT(I2S2_TX_R_SRCDN_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(I2S2_TX_R_SRCDN_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s2_rx_l_src_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, I2S2_RX_L_SRCUP_CLKEN_REG, BIT(I2S2_RX_L_SRCUP_CLKEN_OFFSET),
			BIT(I2S2_RX_L_SRCUP_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, I2S2_RX_L_SRCUP_CLKEN_REG,
			BIT(I2S2_RX_L_SRCUP_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s2_rx_r_src_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, I2S2_RX_R_SRCUP_CLKEN_REG, BIT(I2S2_RX_R_SRCUP_CLKEN_OFFSET),
			BIT(I2S2_RX_R_SRCUP_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, I2S2_RX_R_SRCUP_CLKEN_REG,
			BIT(I2S2_RX_R_SRCUP_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int i2s2_bluetooth_loop_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, I2S2_PCM_CTRL_REG,
			BIT(I2S2_IF_RX_ENA_OFFSET) | BIT(I2S2_IF_TX_ENA_OFFSET),
			BIT(I2S2_IF_RX_ENA_OFFSET) | BIT(I2S2_IF_TX_ENA_OFFSET));
		snd_soc_component_update_bits(codec, I2S2_PCM_CLKEN_REG,
			BIT(I2S2_PCM_CLKEN_OFFSET), BIT(I2S2_PCM_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, I2S2_PCM_CLKEN_REG,
			BIT(I2S2_PCM_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, I2S2_PCM_CTRL_REG,
			BIT(I2S2_IF_RX_ENA_OFFSET) | BIT(I2S2_IF_TX_ENA_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static const struct snd_kcontrol_new i2s2_tx_mixer2_controls[] = {
	SOC_DAPM_SINGLE("MIXER4_L", I2S2_TX_MIXER2_CTRL_REG,
		I2S2_TX_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("MIXER4_R", I2S2_TX_MIXER2_CTRL_REG,
		I2S2_TX_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const char * const bt_tx_srcdn_mux_texts[] = {
	"I2S2_MIXER2",
	"MIXER4_L",
};

static const struct soc_enum bt_tx_srcdn_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, BT_TX_SRCDN_DIN_SEL_OFFSET,
		ARRAY_SIZE(bt_tx_srcdn_mux_texts), bt_tx_srcdn_mux_texts);

static const struct snd_kcontrol_new dapm_bt_tx_srcdn_mux_controls =
	SOC_DAPM_ENUM("Mux", bt_tx_srcdn_mux_enum);

static const struct snd_kcontrol_new dapm_i2s2_tx_l_srcdn_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, S2_TX_L_SRCDN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_i2s2_tx_r_srcdn_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, S2_TX_R_SRCDN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_tx_l_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, S2_TX_L_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_tx_r_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, S2_TX_R_BIT, 1, 0);
/* I2S2 bluetooth LOOP ENABLE */
static const struct snd_kcontrol_new dapm_i2s2_bluetooth_loop_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, I2S2_BLUETOOTH_LOOP_BIT, 1, 0);

static const char * const i2s2_tx_r_sel_mux_texts[] = {
	"I2S2_TX_L_BTMATCH",
	"I2S2_TX_R_BTMATCH",
};

static const struct soc_enum i2s2_tx_r_sel_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, I2S2_TX_R_SEL_OFFSET,
		ARRAY_SIZE(i2s2_tx_r_sel_mux_texts), i2s2_tx_r_sel_mux_texts);

static const struct snd_kcontrol_new dapm_i2s2_tx_r_sel_mux_controls =
	SOC_DAPM_ENUM("Mux", i2s2_tx_r_sel_mux_enum);

static const struct snd_kcontrol_new dapm_s2_rx_l_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, S2_RX_L_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_rx_r_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, S2_RX_R_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_rx_l_src_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, S2_RX_L_SRC_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_rx_r_src_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, S2_RX_R_SRC_BIT, 1, 0);

#define BTFM_INPUT_OUTPUT_WIDGET \
	SND_SOC_DAPM_INPUT("I2S2_RX_INPUT"), \
	SND_SOC_DAPM_OUTPUT("I2S2_TX_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("I2S2_ULTRA_PLL_TRK_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("I2S2_BLUETOOTH_LOOP_OUTPUT"), \
	SND_SOC_DAPM_INPUT("I2S2_BLUETOOTH_LOOP_INPUT") \

#define I2S2_MIXER_WIDGET \
	SND_SOC_DAPM_MIXER("I2S2_TX_MIXER2", I2S2_TX_MIXER2_CLKEN_REG, \
		I2S2_TX_MIXER2_CLKEN_OFFSET, 0, i2s2_tx_mixer2_controls, \
		ARRAY_SIZE(i2s2_tx_mixer2_controls)) \

#define I2S2_MUX_WIDGET \
	SND_SOC_DAPM_MUX("BT_TX_SRCDN_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_bt_tx_srcdn_mux_controls), \
	SND_SOC_DAPM_MUX("I2S2_TX_R_SEL_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_i2s2_tx_r_sel_mux_controls) \

#define I2S2_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("I2S2_TX_L_SRCDN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_i2s2_tx_l_srcdn_switch_controls, s2_tx_l_srcdn_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("I2S2_TX_R_SRCDN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_i2s2_tx_r_srcdn_switch_controls, s2_tx_r_srcdn_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("I2S2_TX_L_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_tx_l_switch_controls), \
	SND_SOC_DAPM_SWITCH("I2S2_TX_R_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_tx_r_switch_controls), \
	SND_SOC_DAPM_SWITCH("I2S2_RX_L_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_rx_l_switch_controls), \
	SND_SOC_DAPM_SWITCH("I2S2_RX_R_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_rx_r_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("I2S2_RX_L_SRC_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_rx_l_src_switch_controls, s2_rx_l_src_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("I2S2_RX_R_SRC_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_rx_r_src_switch_controls, s2_rx_r_src_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)) \

#define I2S2_PGA_SUPPLY_WIDGET \
	SND_SOC_DAPM_PGA_S("I2S2_RX_L_PGA", 0, CODEC_CLK_EN1_REG, \
		I2S2_RX_L_PGA_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("I2S2_RX_R_PGA", 0, CODEC_CLK_EN1_REG, \
		I2S2_RX_R_PGA_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("I2S2_TX_DRV", \
		I2S2_PCM_CTRL_REG, I2S2_IF_TX_ENA_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_SUPPLY_S("I2S2_SUPPLY", \
		0, I2S2_PCM_CLKEN_REG, I2S2_PCM_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_SUPPLY_S("I2S2_RX_SUPPLY", \
		0, I2S2_PCM_CTRL_REG, I2S2_IF_RX_ENA_OFFSET, 0, NULL, 0) \

#define I2S2_BT_LOOP_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("I2S2_BLUETOOTH_LOOP_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_i2s2_bluetooth_loop_switch_controls,\
		i2s2_bluetooth_loop_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)) \

static const struct snd_soc_dapm_widget btfm_dapm_widgets[] = {
	BTFM_INPUT_OUTPUT_WIDGET,
	I2S2_MIXER_WIDGET,
	I2S2_MUX_WIDGET,
	I2S2_SWITCH_WIDGET,
	I2S2_PGA_SUPPLY_WIDGET,
	I2S2_BT_LOOP_SWITCH_WIDGET,
};

static const struct snd_codec_resource btfm_resource = {
	"ASP_CODEC_BTFM",
	btfm_snd_kcontrols,
	ARRAY_SIZE(btfm_snd_kcontrols),
	btfm_dapm_widgets,
	ARRAY_SIZE(btfm_dapm_widgets),
};

int __init add_btfm_path_in_list(void)
{
	return add_codec_base_dapm_list(&btfm_resource);
}
device_initcall(add_btfm_path_in_list);
