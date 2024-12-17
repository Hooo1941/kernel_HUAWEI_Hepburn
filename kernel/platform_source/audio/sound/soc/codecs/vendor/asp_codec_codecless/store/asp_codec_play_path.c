/*
 * asp_codec_play_path.c -- codec driver
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

#define LOG_TAG "asp_codec_play_path"

#define S1_I2S_CFG_KCONTROLS \
	SOC_SINGLE("FS_S1_IF_I2S", \
		I2S1_CTRL_REG, FS_I2S1_OFFSET, \
		max_val_on_bit(FS_I2S1_LEN), 0), \
	SOC_SINGLE("I2S1_FUNC_MODE", \
		I2S1_CTRL_REG, I2S1_FUNC_MODE_OFFSET, \
		max_val_on_bit(I2S1_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("I2S1_I2S_DIRECT_MODE", \
		I2S1_CTRL_REG, I2S1_DIRECT_LOOP_OFFSET, \
		max_val_on_bit(I2S1_DIRECT_LOOP_LEN), 0), \
	SOC_SINGLE("I2S1_FRAME_MODE", \
		I2S1_CTRL_REG, I2S1_FRAME_MODE_OFFSET, \
		max_val_on_bit(I2S1_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("I2S1_RX_CLK_SEL", \
		I2S1_CTRL_REG, I2S1_RX_CLK_SEL_OFFSET, \
		max_val_on_bit(I2S1_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("I2S1_TX_CLK_SEL", \
		I2S1_CTRL_REG, I2S1_TX_CLK_SEL_OFFSET, \
		max_val_on_bit(I2S1_TX_CLK_SEL_LEN), 0) \

#define S1_TDM_CFG_KCONTROLS \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_I0", \
		I2S1_TDM_CTRL0_REG, S1_TDM_RX_SLOT_SEL_I0_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_I0_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_V0", \
		I2S1_TDM_CTRL0_REG, S1_TDM_RX_SLOT_SEL_V0_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_V0_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_I1", \
		I2S1_TDM_CTRL0_REG, S1_TDM_RX_SLOT_SEL_I1_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_I1_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_V1", \
		I2S1_TDM_CTRL0_REG, S1_TDM_RX_SLOT_SEL_V1_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_V1_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_I2", \
		I2S1_TDM_CTRL1_REG, S1_TDM_RX_SLOT_SEL_I2_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_I2_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_V2", \
		I2S1_TDM_CTRL1_REG, S1_TDM_RX_SLOT_SEL_V2_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_V2_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_I3", \
		I2S1_TDM_CTRL1_REG, S1_TDM_RX_SLOT_SEL_I3_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_I3_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_SLOT_SEL_V3", \
		I2S1_TDM_CTRL1_REG, S1_TDM_RX_SLOT_SEL_V3_OFFSET, \
		max_val_on_bit(S1_TDM_RX_SLOT_SEL_V3_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_RX_CLK_SEL", \
		I2S1_TDM_CTRL0_REG, S1_TDM_RX_CLK_SEL_OFFSET, \
		max_val_on_bit(S1_TDM_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_TX_CLK_SEL", \
		I2S1_TDM_CTRL0_REG, S1_TDM_TX_CLK_SEL_OFFSET, \
		max_val_on_bit(S1_TDM_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_DIRECT_MODE", \
		I2S1_TDM_CTRL0_REG, S1_TDM_DIRECT_LOOP_OFFSET, \
		max_val_on_bit(S1_TDM_DIRECT_LOOP_LEN), 0), \
	SOC_SINGLE("I2S1_TDM_FRAME_MODE", \
		I2S1_TDM_CTRL0_REG, S1_TDM_FRAME_MODE_OFFSET, \
		max_val_on_bit(S1_TDM_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("FS_S1_TDM", \
		FS_CTRL7_REG, FS_S1_TDM_OFFSET, \
		max_val_on_bit(FS_S1_TDM_LEN), 0) \

#define MIXER4_GAIN_KCONTROLS \
	SOC_SINGLE("DACL_MIXER4_S1_GAIN", \
		DACL_MIXER4_CTRL0_REG, DACL_MIXER4_GAIN1_OFFSET, \
		max_val_on_bit(DACL_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACL_MIXER4_S2_GAIN", \
		DACL_MIXER4_CTRL0_REG, DACL_MIXER4_GAIN2_OFFSET, \
		max_val_on_bit(DACL_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACL_MIXER4_S3_GAIN", \
		DACL_MIXER4_CTRL0_REG, DACL_MIXER4_GAIN3_OFFSET, \
		max_val_on_bit(DACL_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACL_MIXER4_S4_GAIN", \
		DACL_MIXER4_CTRL0_REG, DACL_MIXER4_GAIN4_OFFSET, \
		max_val_on_bit(DACL_MIXER4_GAIN4_LEN), 0), \
	SOC_SINGLE("DACR_MIXER4_S1_GAIN", \
		DACR_MIXER4_CTRL0_REG, DACR_MIXER4_GAIN1_OFFSET, \
		max_val_on_bit(DACR_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACR_MIXER4_S2_GAIN", \
		DACR_MIXER4_CTRL0_REG, DACR_MIXER4_GAIN2_OFFSET, \
		max_val_on_bit(DACR_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACR_MIXER4_S3_GAIN", \
		DACR_MIXER4_CTRL0_REG, DACR_MIXER4_GAIN3_OFFSET, \
		max_val_on_bit(DACR_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACR_MIXER4_S4_GAIN", \
		DACR_MIXER4_CTRL0_REG, DACR_MIXER4_GAIN4_OFFSET, \
		max_val_on_bit(DACR_MIXER4_GAIN4_LEN), 0) \

#define MIXER4_SRC_KCONTROLS \
	SOC_SINGLE("DACL_MIXER4_SRCUP_DIN", \
		FS_CTRL4_REG, FS_DACL_MIXER4_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_DACL_MIXER4_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("DACL_MIXER4_SRCUP_DOUT", \
		FS_CTRL4_REG, FS_DACL_MIXER4_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_DACL_MIXER4_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("DACL_MIXER4_SRCUP_MODE", \
		SRCUP_CTRL_REG, DACL_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(DACL_SRCUP_SRC_MODE_LEN), 0), \
	SOC_SINGLE("DACR_MIXER4_SRCUP_DIN", \
		FS_CTRL4_REG, FS_DACR_MIXER4_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_DACR_MIXER4_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("DACR_MIXER4_SRCUP_DOUT", \
		FS_CTRL4_REG, FS_DACR_MIXER4_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_DACR_MIXER4_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("DACR_MIXER4_SRCUP_MODE", \
		SRCUP_CTRL_REG, DACR_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(DACR_SRCUP_SRC_MODE_LEN), 0) \

#define MIXER4_FS_KCONTROLS \
	SOC_SINGLE("FS_DACL_MIXER4", \
		FS_CTRL6_REG, FS_DACL_MIXER4_OFFSET, \
		max_val_on_bit(FS_DACL_MIXER4_LEN), 0), \
	SOC_SINGLE("FS_DACR_MIXER4", \
		FS_CTRL6_REG, FS_DACR_MIXER4_OFFSET, \
		max_val_on_bit(FS_DACR_MIXER4_LEN), 0) \

#define FS_DN_FIFO_KCONTROLS \
	SOC_SINGLE("FS_SPA_1_UP_AFIFO", \
		FS_CTRL0_REG, FS_SPA_1_UP_AFIFO_OFFSET, \
		max_val_on_bit(FS_SPA_1_UP_AFIFO_LEN), 0), \
	SOC_SINGLE("FS_SPA_2_UP_AFIFO", \
		FS_CTRL0_REG, FS_SPA_2_UP_AFIFO_OFFSET, \
		max_val_on_bit(FS_SPA_2_UP_AFIFO_LEN), 0), \
	SOC_SINGLE("FS_SPA_3_UP_AFIFO", \
		FS_CTRL0_REG, FS_SPA_3_UP_AFIFO_OFFSET, \
		max_val_on_bit(FS_SPA_3_UP_AFIFO_LEN), 0), \
	SOC_SINGLE("FS_SPA_4_UP_AFIFO", \
		FS_CTRL0_REG, FS_SPA_4_UP_AFIFO_OFFSET, \
		max_val_on_bit(FS_SPA_4_UP_AFIFO_LEN), 0), \
	SOC_SINGLE("FS_AUDIO_L_DN_AFIFO", \
		FS_CTRL0_REG, FS_AUDIO_L_DN_AFIFO_OFFSET, \
		max_val_on_bit(FS_AUDIO_L_DN_AFIFO_LEN), 0), \
	SOC_SINGLE("FS_AUDIO_R_DN_AFIFO", \
		FS_CTRL0_REG, FS_AUDIO_R_DN_AFIFO_OFFSET, \
		max_val_on_bit(FS_AUDIO_R_DN_AFIFO_LEN), 0), \
	SOC_SINGLE("FS_CODEC3_L_DN_AFIFO", \
		FS_CTRL0_REG, FS_CODEC3_L_DN_AFIFO_OFFSET, \
		max_val_on_bit(FS_CODEC3_L_DN_AFIFO_LEN), 0), \
	SOC_SINGLE("FS_CODEC3_R_DN_AFIFO", \
		FS_CTRL0_REG, FS_CODEC3_R_DN_AFIFO_OFFSET, \
		max_val_on_bit(FS_CODEC3_R_DN_AFIFO_LEN), 0) \

#define FS_DN_PGA_KCONTROLS \
	SOC_SINGLE("FS_AUDIO_DN_L_PGA", \
		FS_CTRL2_REG, FS_AUDIO_L_DN_PGA_OFFSET, \
		max_val_on_bit(FS_AUDIO_L_DN_PGA_LEN), 0), \
	SOC_SINGLE("FS_AUDIO_DN_R_PGA", \
		FS_CTRL2_REG, FS_AUDIO_R_DN_PGA_OFFSET, \
		max_val_on_bit(FS_AUDIO_R_DN_PGA_LEN), 0), \
	SOC_SINGLE("FS_CODEC3_DN_L_PGA", \
		FS_CTRL2_REG, FS_CODEC3_L_DN_PGA_OFFSET, \
		max_val_on_bit(FS_CODEC3_L_DN_PGA_LEN), 0), \
	SOC_SINGLE("FS_CODEC3_DN_R_PGA", \
		FS_CTRL2_REG, FS_CODEC3_R_DN_PGA_OFFSET, \
		max_val_on_bit(FS_CODEC3_R_DN_PGA_LEN), 0) \

#define DN_PGA_BYPASS_KCONTROLS \
	SOC_SINGLE("AUDIO_DN_L_PGA_BYPASS", \
		AUDIO_L_DN_PGA_CTRL_REG, AUDIO_L_DN_PGA_BYPASS_OFFSET, \
		max_val_on_bit(AUDIO_L_DN_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("AUDIO_DN_R_PGA_BYPASS", \
		AUDIO_R_DN_PGA_CTRL_REG, AUDIO_R_DN_PGA_BYPASS_OFFSET, \
		max_val_on_bit(AUDIO_R_DN_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_L_PGA_BYPASS", \
		CODEC3_L_DN_PGA_CTRL_REG, CODEC3_L_DN_PGA_BYPASS_OFFSET, \
		max_val_on_bit(CODEC3_L_DN_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_R_PGA_BYPASS", \
		CODEC3_R_DN_PGA_CTRL_REG, CODEC3_R_DN_PGA_BYPASS_OFFSET, \
		max_val_on_bit(CODEC3_R_DN_PGA_BYPASS_LEN), 0) \

#define DN_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("AUDIO_DN_L_PGA_GAIN", \
		AUDIO_L_DN_PGA_CTRL_REG, AUDIO_L_DN_PGA_GAIN_OFFSET, \
		max_val_on_bit(AUDIO_L_DN_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("AUDIO_DN_R_PGA_GAIN", \
		AUDIO_R_DN_PGA_CTRL_REG, AUDIO_R_DN_PGA_GAIN_OFFSET, \
		max_val_on_bit(AUDIO_R_DN_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_L_PGA_GAIN", \
		CODEC3_L_DN_PGA_CTRL_REG, CODEC3_L_DN_PGA_GAIN_OFFSET, \
		max_val_on_bit(CODEC3_L_DN_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_R_PGA_GAIN", \
		CODEC3_R_DN_PGA_CTRL_REG, CODEC3_R_DN_PGA_GAIN_OFFSET, \
		max_val_on_bit(CODEC3_R_DN_PGA_GAIN_LEN), 0) \

#define DN_SRC_KCONTROLS \
	SOC_SINGLE("AUDIO_DN_L_SRCUP_DOUT", \
		FS_CTRL3_REG, FS_AUDIO_L_DN_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_AUDIO_L_DN_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("AUDIO_DN_L_SRCUP_DIN", \
		FS_CTRL3_REG, FS_AUDIO_L_DN_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_AUDIO_L_DN_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("AUDIO_DN_L_SRCUP_MODE", \
		SRCUP_CTRL_REG, AUDIO_L_DN_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(AUDIO_L_DN_SRCUP_SRC_MODE_LEN), 0), \
	SOC_SINGLE("AUDIO_DN_R_SRCUP_DOUT", \
		FS_CTRL3_REG, FS_AUDIO_R_DN_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_AUDIO_R_DN_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("AUDIO_DN_R_SRCUP_DIN", \
		FS_CTRL3_REG, FS_AUDIO_R_DN_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_AUDIO_R_DN_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("AUDIO_DN_R_SRCUP_MODE", \
		SRCUP_CTRL_REG, AUDIO_R_DN_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(AUDIO_R_DN_SRCUP_SRC_MODE_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_L_SRCUP_DOUT", \
		FS_CTRL3_REG, FS_CODEC3_L_DN_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_CODEC3_L_DN_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_L_SRCUP_DIN", \
		FS_CTRL3_REG, FS_CODEC3_L_DN_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_CODEC3_L_DN_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_L_SRCUP_MODE", \
		SRCUP_CTRL_REG, CODEC3_L_DN_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(CODEC3_L_DN_SRCUP_SRC_MODE_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_R_SRCUP_DOUT", \
		FS_CTRL3_REG, FS_CODEC3_R_DN_SRCUP_DOUT_OFFSET, \
		max_val_on_bit(FS_CODEC3_R_DN_SRCUP_DOUT_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_R_SRCUP_DIN", \
		FS_CTRL3_REG, FS_CODEC3_R_DN_SRCUP_DIN_OFFSET, \
		max_val_on_bit(FS_CODEC3_R_DN_SRCUP_DIN_LEN), 0), \
	SOC_SINGLE("CODEC3_DN_R_SRCUP_MODE", \
		SRCUP_CTRL_REG, CODEC3_R_DN_SRCUP_SRC_MODE_OFFSET, \
		max_val_on_bit(CODEC3_R_DN_SRCUP_SRC_MODE_LEN), 0) \


static const struct snd_kcontrol_new audio_play_kcontrols[] = {
	S1_I2S_CFG_KCONTROLS,
	S1_TDM_CFG_KCONTROLS,
	MIXER4_GAIN_KCONTROLS,
	MIXER4_SRC_KCONTROLS,
	MIXER4_FS_KCONTROLS,
	FS_DN_FIFO_KCONTROLS,
	FS_DN_PGA_KCONTROLS,
	DN_PGA_BYPASS_KCONTROLS,
	DN_PGA_GAIN_KCONTROLS,
	DN_SRC_KCONTROLS,
};

/* mixer widget */
static const struct snd_kcontrol_new dac_l_mixer4_controls[] = {
	SOC_DAPM_SINGLE("I2S2_RX_L", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("CODEC3_DN_L", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("AUDIO_DN_L", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE", DACL_MIXER4_CTRL0_REG,
		DACL_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new dac_r_mixer4_controls[] = {
	SOC_DAPM_SINGLE("I2S2_RX_R", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("CODEC3_DN_R", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("AUDIO_DN_R", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE", DACR_MIXER4_CTRL0_REG,
		DACR_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const char * const i2s1_tx_l_mux_texts[] = {
	"AU_UP_L",
	"NULL",
	"MIXER4_SRC_L",
};

static const struct soc_enum i2s1_tx_l_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, I2S1_TX_L_SEL_OFFSET,
		ARRAY_SIZE(i2s1_tx_l_mux_texts), i2s1_tx_l_mux_texts);

static const struct snd_kcontrol_new dapm_i2s1_tx_l_mux_controls =
	SOC_DAPM_ENUM("Mux", i2s1_tx_l_mux_enum);

static const char * const i2s1_tx_r_mux_texts[] = {
	"AU_UP_R",
	"NULL",
	"MIXER4_SRC_R",
};

static const struct soc_enum i2s1_tx_r_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, I2S1_TX_R_SEL_OFFSET,
		ARRAY_SIZE(i2s1_tx_r_mux_texts), i2s1_tx_r_mux_texts);

static const struct snd_kcontrol_new dapm_i2s1_tx_r_mux_controls =
	SOC_DAPM_ENUM("Mux", i2s1_tx_r_mux_enum);

static const char * const s1_tdm_tx_l_mux_texts[] = {
	"AUDIO_DN_L",
	"MIXER4_SRC_L",
};

static const struct soc_enum s1_tdm_tx_l_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, BM_I2S1_TDM_TX_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(s1_tdm_tx_l_mux_texts), s1_tdm_tx_l_mux_texts);

static const struct snd_kcontrol_new dapm_s1_tdm_tx_l_mux_controls =
	SOC_DAPM_ENUM("Mux", s1_tdm_tx_l_mux_enum);

static const char * const s1_tdm_tx_r_mux_texts[] = {
	"AUDIO_DN_R",
	"MIXER4_SRC_R",
};

static const struct soc_enum s1_tdm_tx_r_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, BM_I2S1_TDM_TX_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(s1_tdm_tx_r_mux_texts), s1_tdm_tx_r_mux_texts);

static const struct snd_kcontrol_new dapm_s1_tdm_tx_r_mux_controls =
	SOC_DAPM_ENUM("Mux", s1_tdm_tx_r_mux_enum);

static const struct snd_kcontrol_new dapm_s1_tx_tdm_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, S1_TX_TDM_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s1_tx_i2s_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, S1_TX_I2S_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_codec3_dn_44k1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, CODEC3_DN_44K1_IN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audio_2pa_dn_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, AUDIO_DN_2PA_IN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_audio_4pa_dn_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, AUDIO_DN_4PA_IN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_iv_2pa_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, IV_2PA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_iv_4pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, IV_4PA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_carkit_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, CARKIT_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_codec3_dn_r_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, CODEC3_DN_R_IN_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s1_tx_tdm256_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR1_REG_ADDR, S1_TX_TDM256_BIT, 1, 0);

static int s1_tx_tdm_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL1_REG,
			BIT(S1_I2S_TDM_MODE_OFFSET), BIT(S1_I2S_TDM_MODE_OFFSET));
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL0_REG,
			BIT(S1_TDM_IF_EN_OFFSET), BIT(S1_TDM_IF_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL1_REG, BIT(S1_I2S_TDM_MODE_OFFSET), 0);
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL0_REG, BIT(S1_TDM_IF_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s1_tx_i2s_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL1_REG,
			BIT(S1_I2S_TDM_MODE_OFFSET), 0);
		
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL1_REG, BIT(S1_I2S_TDM_MODE_OFFSET),
			BIT(S1_I2S_TDM_MODE_OFFSET));
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s1_tx_tdm256_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL1_REG, BIT(S1_I2S_TDM_MODE_OFFSET),
			BIT(S1_I2S_TDM_MODE_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, I2S1_TDM_CTRL1_REG, BIT(S1_I2S_TDM_MODE_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int codec3_dn_44k1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	return 0;
}

static int audio_2pa_dn_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET), 0);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		break;
	}
	AUDIO_LOGI("power event: %d", event);

	return 0;
}

static void spa4_dn_fifo_clk_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET), 0);
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
		BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET), 0);
}

static void spa4_dn_fifo_clk_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG, BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET),
		BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET));
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG, BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET),
		BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET));
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG, BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET),
		BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET));
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG, BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET),
		BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET));
}

static int audio_4pa_dn_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		spa4_dn_fifo_clk_clear(codec);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		spa4_dn_fifo_clk_set(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		spa4_dn_fifo_clk_clear(codec);
		break;
	default:
		break;
	}
	AUDIO_LOGI("power event: %d", event);

	return 0;
}

static int codec3_dn_r_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET), 0);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG, BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET),
			BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void spa4_up_fifo_clk_clear(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
		BIT(SPA_1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA_2_UP_AFIFO_CLKEN_OFFSET) |
		BIT(SPA_3_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA_4_UP_AFIFO_CLKEN_OFFSET), 0);
}

static void spa4_up_fifo_clk_set(struct snd_soc_component *codec)
{
	snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
		BIT(SPA_1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA_2_UP_AFIFO_CLKEN_OFFSET) |
		BIT(SPA_3_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA_4_UP_AFIFO_CLKEN_OFFSET),
		BIT(SPA_1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA_2_UP_AFIFO_CLKEN_OFFSET) |
		BIT(SPA_3_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA_4_UP_AFIFO_CLKEN_OFFSET));
}

static int iv_2pa_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(SPA_1_UP_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(SPA_3_UP_AFIFO_CLKEN_OFFSET), 0);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG, BIT(SPA_1_UP_AFIFO_CLKEN_OFFSET),
			BIT(SPA_1_UP_AFIFO_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG, BIT(SPA_3_UP_AFIFO_CLKEN_OFFSET),
			BIT(SPA_3_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(SPA_1_UP_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(SPA_3_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		break;
	}
	AUDIO_LOGI("power event: %d", event);

	return 0;
}

static int iv_4pa_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, CODEC_SW_RST_N_REG,
			BIT(RST_4MIC_DN_ACCESS_IRQ_OFFSET) | BIT(CODEC_SW_RST_N_OFFSET),
			BIT(RST_4MIC_DN_ACCESS_IRQ_OFFSET) | BIT(CODEC_SW_RST_N_OFFSET));
		spa4_up_fifo_clk_clear(codec);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(I2S1_TDM_CLKEN_OFFSET), 0);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		spa4_up_fifo_clk_set(codec);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN2_REG,
			BIT(I2S1_TDM_CLKEN_OFFSET), BIT(I2S1_TDM_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, CODEC_SW_RST_N_REG,
			BIT(RST_4MIC_DN_ACCESS_IRQ_OFFSET), 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		spa4_up_fifo_clk_clear(codec);
		break;
	default:
		break;
	}
	AUDIO_LOGI("power event: %d", event);

	return 0;
}

static int carkit_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET), 0);
		udelay(CLEAR_FIFO_DELAY_LEN_US);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET));
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err:%d", event);
		break;
	}

	return 0;
}

#define AUDIO_INPUT_OUTPUT_WIDGET \
	SND_SOC_DAPM_INPUT("CODEC3_DN_INPUT"), \
	SND_SOC_DAPM_OUTPUT("CODEC3_OUTPUT"), \
	SND_SOC_DAPM_INPUT("I2S1_RX_INPUT"), \
	SND_SOC_DAPM_OUTPUT("SMARTPA_UP_FIFO_OUTPUT"), \
	SND_SOC_DAPM_INPUT("AUDIO_DN_FIFO_INPUT"), \
	SND_SOC_DAPM_OUTPUT("I2S1_TX_OUTPUT"), \
	SND_SOC_DAPM_INPUT("CODEC3_DN_FIFO_INPUT"), \
	SND_SOC_DAPM_INPUT("CARKIT_INPUT"), \
	SND_SOC_DAPM_OUTPUT("CARKIT_OUTPUT") \

#define AUDIO_MIXER_WIDGET \
	SND_SOC_DAPM_MIXER("DAC_L_MIXER4", CODEC_CLK_EN2_REG, \
		DACL_MIXER4_CLKEN_OFFSET, 0, dac_l_mixer4_controls, \
		ARRAY_SIZE(dac_l_mixer4_controls)), \
	SND_SOC_DAPM_MIXER("DAC_R_MIXER4", CODEC_CLK_EN2_REG, \
		DACR_MIXER4_CLKEN_OFFSET, 0, dac_r_mixer4_controls, \
		ARRAY_SIZE(dac_r_mixer4_controls)) \

#define AUDIO_MUX_WIDGET \
	SND_SOC_DAPM_MUX("I2S1_TX_L_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_i2s1_tx_l_mux_controls), \
	SND_SOC_DAPM_MUX("I2S1_TX_R_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_i2s1_tx_r_mux_controls), \
	SND_SOC_DAPM_MUX("I2S1_TDM_TX_L_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_s1_tdm_tx_l_mux_controls), \
	SND_SOC_DAPM_MUX("I2S1_TDM_TX_R_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_s1_tdm_tx_r_mux_controls) \

#define AUDIO_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("I2S1_TX_TDM_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s1_tx_tdm_switch_controls, s1_tx_tdm_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("I2S1_TX_I2S_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s1_tx_i2s_switch_controls, s1_tx_i2s_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("CODEC3_DN_44K1_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_codec3_dn_44k1_switch_controls, \
		codec3_dn_44k1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("I2S1_TX_TDM256_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s1_tx_tdm256_switch_controls, s1_tx_tdm256_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIO_2PA_DN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_audio_2pa_dn_switch_controls, audio_2pa_dn_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIO_4PA_DN_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_audio_4pa_dn_switch_controls, audio_4pa_dn_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("CODEC3_DN_R_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_codec3_dn_r_switch_controls, codec3_dn_r_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("IV_2PA_48K_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_iv_2pa_48k_switch_controls, iv_2pa_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("ASP_CODEC_IV_4PA_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_iv_4pa_switch_controls, iv_4pa_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("CARKIT_48K_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_carkit_48k_switch_controls, carkit_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)) \

#define PA_PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("AUDIO_DN_L_PGA", 0, CODEC_CLK_EN0_REG, \
		AUDIO_DN_L_PGA_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("AUDIO_DN_R_PGA", 0, CODEC_CLK_EN0_REG, \
		AUDIO_DN_R_PGA_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("CODEC3_DN_L_PGA", 0, CODEC_CLK_EN0_REG, \
		CODEC3_DN_L_PGA_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("CODEC3_DN_R_PGA", 0, CODEC_CLK_EN0_REG, \
		CODEC3_DN_R_PGA_CLKEN_OFFSET, 0, NULL, 0) \

#define AUDIO_DRV_WIDGET \
	SND_SOC_DAPM_OUT_DRV("I2S1_TX_DRV", \
		I2S1_CTRL_REG, I2S1_IF_TX_ENA_OFFSET, 0, NULL, 0) \

#define SRC_WIDGET \
	SND_SOC_DAPM_PGA_S("DACL_MIXER4_SRC23_CLKEN", 0, CODEC_CLK_EN1_REG, \
		DACL_MIXER4_SRCUP_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("DACR_MIXER4_SRC24_CLKEN", 0, CODEC_CLK_EN1_REG, \
		DACR_MIXER4_SRCUP_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("AUDIO_DN_L_SRC3_EN", 0, CODEC_CLK_EN1_REG, \
		AUDIO_DN_L_SRCUP_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("AUDIO_DN_R_SRC4_EN", 0, CODEC_CLK_EN1_REG, \
		AUDIO_DN_R_SRCUP_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("CODEC3_DN_L_SRC1_EN", 0, CODEC_CLK_EN1_REG, \
		CODEC3_DN_L_SRCUP_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("CODEC3_DN_R_SRC2_EN", 0, CODEC_CLK_EN1_REG, \
		CODEC3_DN_R_SRCUP_CLKEN_OFFSET, 0, NULL, 0) \

static int pll_supply_power_mode_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = NULL;
	struct codec_data *priv = NULL;

	codec = snd_soc_dapm_to_component(w->dapm);
	priv = snd_soc_component_get_drvdata(codec);
	if (priv->platform_type == UDP_PLATFORM) {
		switch (event) {
		case SND_SOC_DAPM_PRE_PMU:
			priv->pm.have_dapm = true;
			break;
		case SND_SOC_DAPM_POST_PMD:
			priv->pm.have_dapm = false;
			break;
		default:
			AUDIO_LOGE("power mode event err: %d", event);
			break;
		}
	} else {
		AUDIO_LOGI("fpga clk is always on\n");
	}

	return 0;
}

#define SUPPLY_WIDGET \
	SND_SOC_DAPM_SUPPLY_S("I2S1_SUPPLY", \
		0, CODEC_CLK_EN2_REG, I2S1_TDM_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_SUPPLY_S("I2S1_RX_SUPPLY", \
		0, I2S1_CTRL_REG, I2S1_IF_RX_ENA_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_SUPPLY_S("PLL", \
		0, SND_SOC_NOPM, 0, 0, pll_supply_power_mode_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD))

static const struct snd_soc_dapm_widget audio_dapm_widgets[] = {
	AUDIO_INPUT_OUTPUT_WIDGET,
	AUDIO_MIXER_WIDGET,
	AUDIO_MUX_WIDGET,
	AUDIO_SWITCH_WIDGET,
	PA_PGA_WIDGET,
	AUDIO_DRV_WIDGET,
	SRC_WIDGET,
	SUPPLY_WIDGET,
};

static const struct snd_codec_resource audio_resource = {
	"ASP_CODEC_PA",
	audio_play_kcontrols,
	ARRAY_SIZE(audio_play_kcontrols),
	audio_dapm_widgets,
	ARRAY_SIZE(audio_dapm_widgets),
};

int __init add_pa_path_in_list(void)
{
	return add_codec_base_dapm_list(&audio_resource);
}

device_initcall(add_pa_path_in_list);
