/*
 * asp_codec_path.c -- codec driver
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

#include "asp_codec_store.h"
#include "asp_codec_type.h"
#include "audio_log.h"

#define LOG_TAG "Analog_less_path"

#define UPLINK_PLL \
	{ "AUDIO_UP_O_SWITCH",         NULL,   "PLL" }, \
	{ "VOICE_UP_O_SWITCH",         NULL,   "PLL" }, \
	{ "MIC34_UP_O_SWITCH",         NULL,   "PLL" }, \
	{ "MIC56_UP_O_SWITCH",         NULL,   "PLL" }, \
	{ "ULTRA_UP_O_SWITCH",         NULL,   "PLL" } \

#define AUDIO_VOICE_2MIC_ROUTE \
	{ "ADC1",                           NULL,                 "AUDIO_MIC12_INPUT" }, \
	{ "ADC2",                           NULL,                 "AUDIO_MIC12_INPUT" }, \
	{ "ADC1",                           NULL,                 "VOICE_MIC12_INPUT" }, \
	{ "ADC2",                           NULL,                 "VOICE_MIC12_INPUT" }, \
	{ "AUDIO_UP_L_MUX",                 "MIC1_ADC",           "ADC1" }, \
	{ "AUDIO_UP_L_MUX",                 "MIC2_ADC",           "ADC2" }, \
	{ "AUDIO_UP_R_MUX",                 "MIC1_ADC",           "ADC1" }, \
	{ "AUDIO_UP_R_MUX",                 "MIC2_ADC",           "ADC2" }, \
	{ "VOICE_UP_L_MUX",                 "MIC1_ADC",           "ADC1" }, \
	{ "VOICE_UP_L_MUX",                 "MIC2_ADC",           "ADC2" }, \
	{ "VOICE_UP_R_MUX",                 "MIC1_ADC",           "ADC1" }, \
	{ "VOICE_UP_R_MUX",                 "MIC2_ADC",           "ADC2" }, \
	{ "AUDIO_UP_O_SWITCH",              "ENABLE",             "AUDIO_UP_L_MUX" }, \
	{ "AUDIO_UP_O_SWITCH",              "ENABLE",             "AUDIO_UP_R_MUX" }, \
	{ "VOICE_UP_O_SWITCH",              "ENABLE",             "VOICE_UP_L_MUX" }, \
	{ "VOICE_UP_O_SWITCH",              "ENABLE",             "VOICE_UP_R_MUX" }, \
	{ "AUDIO_MIC12_OUTPUT",             NULL,                 "AUDIO_UP_O_SWITCH" }, \
	{ "VOICE_MIC12_OUTPUT",             NULL,                 "VOICE_UP_O_SWITCH" }, \
	{ "AUDIO_UP_2MIC_48K_O_SWITCH",     "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "VOICE_UP_2MIC_48K_O_SWITCH",     "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "VOICE_UP_2MIC_32K_O_SWITCH",     "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "UPLINK_MIC_OUTPUT",              NULL,                 "AUDIO_UP_2MIC_48K_O_SWITCH" }, \
	{ "UPLINK_MIC_OUTPUT",              NULL,                 "VOICE_UP_2MIC_48K_O_SWITCH" }, \
	{ "UPLINK_MIC_OUTPUT",              NULL,                 "VOICE_UP_2MIC_32K_O_SWITCH" } \

#define AUDIO_VOICE_4MIC_ROUTE \
	{ "ADC3",                       NULL,                 "AUDIO_MIC34_INPUT" }, \
	{ "ADC4",                       NULL,                 "AUDIO_MIC34_INPUT" }, \
	{ "MIC3_MUX",                   "MIC3_ADC",           "ADC3" }, \
	{ "MIC3_MUX",                   "MIC4_ADC",           "ADC4" }, \
	{ "MIC4_MUX",                   "MIC3_ADC",           "ADC3" }, \
	{ "MIC4_MUX",                   "MIC4_ADC",           "ADC4" }, \
	{ "MIC34_UP_O_SWITCH",          "ENABLE",             "MIC3_MUX" }, \
	{ "MIC34_UP_O_SWITCH",          "ENABLE",             "MIC4_MUX" }, \
	{ "AUDIO_MIC34_OUTPUT",         NULL,                 "MIC34_UP_O_SWITCH" }, \
	{ "AUDIO_UP_4MIC_48K_O_SWITCH", "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "VOICE_UP_4MIC_48K_O_SWITCH", "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "VOICE_UP_4MIC_32K_O_SWITCH", "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "VOICE_UP_4MIC_8K_O_SWITCH",  "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "UPLINK_MIC_OUTPUT",          NULL,                 "AUDIO_UP_4MIC_48K_O_SWITCH" }, \
	{ "UPLINK_MIC_OUTPUT",          NULL,                 "VOICE_UP_4MIC_48K_O_SWITCH" }, \
	{ "UPLINK_MIC_OUTPUT",          NULL,                 "VOICE_UP_4MIC_32K_O_SWITCH" }, \
	{ "UPLINK_MIC_OUTPUT",          NULL,                 "VOICE_UP_4MIC_8K_O_SWITCH" } \

#define MIC34_MIC56_UP_ROUTE \
	{ "MIC3_MUX",            "MIC1_ADC",           "ADC1" }, \
	{ "MIC4_MUX",            "MIC2_ADC",           "ADC2" }, \
	{ "MIC5_MUX",            "MIC1_ADC",           "ADC1" }, \
	{ "MIC6_MUX",            "MIC2_ADC",           "ADC2" } \

#define AUDIO_VOICE_6MIC_ROUTE \
	{ "ADC5",                         NULL,                 "AUDIO_MIC56_INPUT" }, \
	{ "ADC6",                         NULL,                 "AUDIO_MIC56_INPUT" }, \
	{ "MIC5_MUX",                     "MIC3_ADC",           "ADC3" }, \
	{ "MIC5_MUX",                     "MIC5_ADC",           "ADC5" }, \
	{ "MIC6_MUX",                     "MIC4_ADC",           "ADC4" }, \
	{ "MIC6_MUX",                     "MIC6_ADC",           "ADC6" }, \
	{ "MIC56_UP_O_SWITCH",            "ENABLE",             "MIC5_MUX" }, \
	{ "MIC56_UP_O_SWITCH",            "ENABLE",             "MIC6_MUX" }, \
	{ "AUDIO_MIC56_OUTPUT",           NULL,                 "MIC56_UP_O_SWITCH" }, \
	{ "AUDIO_UP_6MIC_48K_O_SWITCH",   "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "WAKEUP_MAIN_MIC_16K_O_SWITCH", "ENABLE",             "UPLINK_MIC_INPUT" }, \
	{ "UPLINK_MIC_OUTPUT",            NULL,                 "WAKEUP_MAIN_MIC_16K_O_SWITCH" }, \
	{ "UPLINK_MIC_OUTPUT",            NULL,                 "AUDIO_UP_6MIC_48K_O_SWITCH" } \

#define ULTRA_UP_ROUTE \
	{ "ADC2",                     NULL,                 "ULTRA_UP_INPUT" }, \
	{ "MDM_5G_UP_R_MUX",          "MIC2_ADC",           "ADC2" }, \
	{ "MDM5G_UP_R_48K_O_SWITCH",  "ENABLE",             "MDM_5G_UP_R_MUX" }, \
	{ "ULTRA_UP_OUTPUT",          NULL,                 "MDM5G_UP_R_48K_O_SWITCH" } \

#define AUDIO_VOICE_LOOP_ROUTE \
	{ "AUDIO_UP_L_MUX",  "MIXER4_L",     "DAC_L_MIXER4" }, \
	{ "AUDIO_UP_L_MUX",  "AUDIO_DN_L",   "AUDIO_DN_L_SRC3_EN" }, \
	{ "AUDIO_UP_R_MUX",  "MIXER4_R",     "DAC_R_MIXER4" }, \
	{ "AUDIO_UP_R_MUX",  "AUDIO_DN_R",   "AUDIO_DN_R_SRC4_EN" }, \
	{ "VOICE_UP_L_MUX",  "MIXER4_L",     "DAC_L_MIXER4" }, \
	{ "VOICE_UP_L_MUX",  "CODEC3_DN_L",  "CODEC3_DN_L_SRC1_EN" }, \
	{ "VOICE_UP_R_MUX",  "CODEC3_DN_R",  "CODEC3_DN_R_SRC2_EN" }, \
	{ "VOICE_UP_R_MUX",  "MIXER4_R",     "DAC_R_MIXER4" } \

#define DMIC12_ROUTE \
	{ "DMIC12_IN_SWITCH",     "ENABLE",              "DMIC12_INPUT" }, \
	{ "ADC1_CIC6_MUX",        "DMIC1_L",             "DMIC12_IN_SWITCH" }, \
	{ "ADC2_CIC6_MUX",        "DMIC1_R",             "DMIC12_IN_SWITCH" }, \
	{ "ADC1_CIC_OUT_DRV",     NULL,                  "ADC1_CIC6_MUX" }, \
	{ "ADC2_CIC_OUT_DRV",     NULL,                  "ADC2_CIC6_MUX" }, \
	{ "ADC1_DIN_MUX",         "ADC1_CIC",            "ADC1_CIC_OUT_DRV" }, \
	{ "ADC2_DIN_MUX",         "ADC2_CIC",            "ADC2_CIC_OUT_DRV" }, \
	{ "DMIC12_OUTPUT",        NULL,                  "ADC1_DIN_MUX" }, \
	{ "DMIC12_OUTPUT",        NULL,                  "ADC2_DIN_MUX" } \

#define DMIC34_ROUTE \
	{ "DMIC34_IN_SWITCH",     "ENABLE",             "DMIC34_INPUT" }, \
	{ "ADC3_CIC6_MUX",        "DMIC2_L",            "DMIC34_IN_SWITCH" }, \
	{ "ADC4_CIC6_MUX",        "DMIC2_R",            "DMIC34_IN_SWITCH" }, \
	{ "ADC3_CIC_OUT_DRV",     NULL,                 "ADC3_CIC6_MUX" }, \
	{ "ADC4_CIC_OUT_DRV",     NULL,                 "ADC4_CIC6_MUX" }, \
	{ "ADC3_DIN_MUX",         "ADC3_CIC",           "ADC3_CIC_OUT_DRV" }, \
	{ "ADC4_DIN_MUX",         "ADC4_CIC",           "ADC4_CIC_OUT_DRV" }, \
	{ "DMIC34_OUTPUT",        NULL,                 "ADC3_DIN_MUX" }, \
	{ "DMIC34_OUTPUT",        NULL,                 "ADC4_DIN_MUX" } \

#define DMIC56_ROUTE \
	{ "DMIC56_IN_SWITCH",     "ENABLE",             "DMIC56_INPUT" }, \
	{ "ADC5_CIC_OUT_DRV",     NULL,                 "DMIC56_IN_SWITCH" }, \
	{ "ADC6_CIC_OUT_DRV",     NULL,                 "DMIC56_IN_SWITCH" }, \
	{ "DMIC56_OUTPUT",        NULL,                 "ADC5_CIC_OUT_DRV" }, \
	{ "DMIC56_OUTPUT",        NULL,                 "ADC6_CIC_OUT_DRV" } \

#define DMIC78_ROUTE \
	{ "DMIC78_IN_SWITCH",     "ENABLE",             "DMIC78_INPUT" }, \
	{ "ADC7_CIC_OUT_DRV",     NULL,                 "DMIC78_IN_SWITCH" }, \
	{ "ADC8_CIC_OUT_DRV",     NULL,                 "DMIC78_IN_SWITCH" }, \
	{ "DMIC78_OUTPUT",        NULL,                 "ADC7_CIC_OUT_DRV" }, \
	{ "DMIC78_OUTPUT",        NULL,                 "ADC8_CIC_OUT_DRV" } \

#define MIC_CIC_PLL \
	{ "DMIC12_IN_SWITCH",         NULL,   "PLL" }, \
	{ "DMIC34_IN_SWITCH",         NULL,   "PLL" }, \
	{ "DMIC56_IN_SWITCH",         NULL,   "PLL" }, \
	{ "DMIC78_IN_SWITCH",         NULL,   "PLL" }, \
	{ "AU0_UP_L_O_SWITCH",        NULL,   "PLL" }, \
	{ "AU0_UP_R_O_SWITCH",        NULL,   "PLL" }, \
	{ "DMIC12_IN_SWITCH",         NULL,   "DMIC_CLK" }, \
	{ "DMIC34_IN_SWITCH",         NULL,   "DMIC_CLK" } \

#define AUDIO_PLAY_2PA \
	{ "I2S1_RX_INPUT",           NULL,           "I2S1_SUPPLY" }, \
	{ "I2S1_RX_INPUT",           NULL,           "I2S1_RX_SUPPLY" }, \
	{ "IV_2PA_48K_SWITCH",       "ENABLE",       "I2S1_RX_INPUT" }, \
	{ "ASP_CODEC_IV_4PA_SWITCH", "ENABLE",       "I2S1_RX_INPUT" }, \
	{ "SMARTPA_UP_FIFO_OUTPUT",  NULL,           "IV_2PA_48K_SWITCH" }, \
	{ "SMARTPA_UP_FIFO_OUTPUT",  NULL,           "ASP_CODEC_IV_4PA_SWITCH" }, \
	{ "AUDIO_2PA_DN_SWITCH",     "ENABLE",       "AUDIO_DN_FIFO_INPUT" },  \
	{ "AUDIO_DN_L_PGA",          NULL,           "AUDIO_2PA_DN_SWITCH" }, \
	{ "AUDIO_DN_R_PGA",          NULL,           "AUDIO_2PA_DN_SWITCH" }, \
	{ "AUDIO_DN_L_SRC3_EN",      NULL,           "AUDIO_DN_L_PGA" }, \
	{ "AUDIO_DN_R_SRC4_EN",      NULL,           "AUDIO_DN_R_PGA" }, \
	{ "DAC_L_MIXER4",            "AUDIO_DN_L",   "AUDIO_DN_L_SRC3_EN" }, \
	{ "DAC_R_MIXER4",            "AUDIO_DN_R",   "AUDIO_DN_R_SRC4_EN" }, \
	{ "I2S1_TX_TDM256_SWITCH",   "ENABLE",       "AUDIO_DN_L_SRC3_EN" }, \
	{ "I2S1_TX_TDM256_SWITCH",   "ENABLE",       "AUDIO_DN_R_SRC4_EN" }, \
	{ "DACL_MIXER4_SRC23_CLKEN", NULL,           "DAC_L_MIXER4"  }, \
	{ "DACR_MIXER4_SRC24_CLKEN", NULL,           "DAC_R_MIXER4"  }, \
	{ "I2S1_TX_L_MUX",           "MIXER4_SRC_L", "DACL_MIXER4_SRC23_CLKEN"  }, \
	{ "I2S1_TX_R_MUX",           "MIXER4_SRC_R", "DACR_MIXER4_SRC24_CLKEN"  }, \
	{ "I2S1_TX_I2S_SWITCH",      "ENABLE",       "I2S1_TX_L_MUX" }, \
	{ "I2S1_TX_I2S_SWITCH",      "ENABLE",       "I2S1_TX_R_MUX" }, \
	{ "I2S1_TX_DRV",             NULL,           "I2S1_TX_TDM256_SWITCH" }, \
	{ "I2S1_TX_DRV",             NULL,           "I2S1_TX_I2S_SWITCH" }, \
	{ "I2S1_TX_OUTPUT",          NULL,           "I2S1_TX_DRV" }, \
	{ "I2S1_TX_OUTPUT",          NULL,           "I2S1_SUPPLY" }, \
	{ "AUDIO_DN_FIFO_INPUT",     NULL,           "PLL" } \

#define AUDIO_PLAY_4PA \
	{ "AUDIO_4PA_DN_SWITCH",   "ENABLE",        "AUDIO_DN_FIFO_INPUT" }, \
	{ "AUDIO_4PA_DN_SWITCH",   "ENABLE",        "CODEC3_DN_FIFO_INPUT" }, \
	{ "CODEC3_DN_44K1_SWITCH", "ENABLE",        "CODEC3_DN_INPUT" }, \
	{ "CODEC3_DN_R_SWITCH",    "ENABLE",        "CODEC3_DN_FIFO_INPUT" }, \
	{ "AUDIO_DN_L_PGA",        NULL,            "AUDIO_4PA_DN_SWITCH" }, \
	{ "AUDIO_DN_R_PGA",        NULL,            "AUDIO_4PA_DN_SWITCH" }, \
	{ "CODEC3_DN_L_PGA",       NULL,            "AUDIO_4PA_DN_SWITCH" }, \
	{ "CODEC3_DN_R_PGA",       NULL,            "AUDIO_4PA_DN_SWITCH" }, \
	{ "CODEC3_DN_R_PGA",       NULL,            "CODEC3_DN_R_SWITCH" }, \
	{ "AUDIO_DN_L_SRC3_EN",    NULL,            "AUDIO_DN_L_PGA" }, \
	{ "AUDIO_DN_R_SRC4_EN",    NULL,            "AUDIO_DN_R_PGA" }, \
	{ "CODEC3_DN_L_SRC1_EN",   NULL,            "CODEC3_DN_L_PGA" }, \
	{ "CODEC3_DN_R_SRC2_EN",   NULL,            "CODEC3_DN_R_PGA" }, \
	{ "I2S1_TDM_TX_L_MUX",     "AUDIO_DN_L",    "AUDIO_DN_L_SRC3_EN" }, \
	{ "I2S1_TDM_TX_R_MUX",     "AUDIO_DN_R",    "AUDIO_DN_R_SRC4_EN" }, \
	{ "I2S1_TDM_TX_L_MUX",     "MIXER4_SRC_L",  "DACL_MIXER4_SRC23_CLKEN"  }, \
	{ "I2S1_TDM_TX_R_MUX",     "MIXER4_SRC_R",  "DACR_MIXER4_SRC24_CLKEN"  }, \
	{ "I2S1_TX_TDM_SWITCH",    "ENABLE",        "I2S1_TDM_TX_L_MUX" }, \
	{ "I2S1_TX_TDM_SWITCH",    "ENABLE",        "I2S1_TDM_TX_R_MUX" }, \
	{ "I2S1_TX_TDM_SWITCH",    "ENABLE",        "CODEC3_DN_L_SRC1_EN" }, \
	{ "I2S1_TX_TDM_SWITCH",    "ENABLE",        "CODEC3_DN_R_SRC2_EN" }, \
	{ "I2S1_TX_TDM256_SWITCH", "ENABLE",        "CODEC3_DN_L_SRC1_EN" }, \
	{ "I2S1_TX_TDM256_SWITCH", "ENABLE",        "CODEC3_DN_R_SRC2_EN" }, \
	{ "DAC_L_MIXER4",          "CODEC3_DN_L",   "CODEC3_DN_L_SRC1_EN" }, \
	{ "DAC_R_MIXER4",          "CODEC3_DN_R",   "CODEC3_DN_R_SRC2_EN" }, \
	{ "I2S1_TX_DRV",           NULL,            "I2S1_TX_TDM256_SWITCH" }, \
	{ "I2S1_TX_DRV",           NULL,            "I2S1_TX_TDM_SWITCH" }, \
	{ "CODEC3_OUTPUT",         NULL,            "CODEC3_DN_44K1_SWITCH" }, \
	{ "I2S1_TX_OUTPUT",        NULL,            "I2S1_TX_DRV" } \

#define AUDIO_CARTKIT \
	{ "CARKIT_48K_SWITCH",     "ENABLE", "CARKIT_INPUT" }, \
	{ "CARKIT_OUTPUT",         NULL,     "CARKIT_48K_SWITCH" } \

#define BT_DOWNLINK \
	{ "I2S2_TX_MIXER2",         "MIXER4_L",          "DAC_L_MIXER4" }, \
	{ "I2S2_TX_MIXER2",         "MIXER4_R",          "DAC_R_MIXER4" }, \
	{ "BT_TX_SRCDN_MUX",        "MIXER4_L",          "DAC_L_MIXER4" }, \
	{ "I2S2_TX_R_SRCDN_SWITCH", "ENABLE",            "DAC_R_MIXER4" }, \
	{ "BT_TX_SRCDN_MUX",        "I2S2_MIXER2",       "I2S2_TX_MIXER2" }, \
	{ "I2S2_TX_L_SRCDN_SWITCH", "ENABLE",            "BT_TX_SRCDN_MUX" }, \
	{ "I2S2_TX_L_SWITCH",       "ENABLE",            "I2S2_TX_L_SRCDN_SWITCH" }, \
	{ "I2S2_TX_R_SEL_MUX",      "I2S2_TX_L_BTMATCH", "I2S2_TX_L_SRCDN_SWITCH" }, \
	{ "I2S2_TX_R_SEL_MUX",      "I2S2_TX_R_BTMATCH", "I2S2_TX_R_SRCDN_SWITCH" }, \
	{ "I2S2_TX_R_SWITCH",       "ENABLE",            "I2S2_TX_R_SEL_MUX" }, \
	{ "I2S2_TX_DRV",            NULL,                "I2S2_TX_L_SWITCH" }, \
	{ "I2S2_TX_DRV",            NULL,                "I2S2_TX_R_SWITCH" }, \
	{ "I2S2_TX_OUTPUT",         NULL,                "I2S2_TX_DRV" }, \
	{ "I2S2_TX_OUTPUT",         NULL,                "I2S2_SUPPLY" } \

#define I2S2_TX_SWITCH \
	{ "I2S2_TX_SWITCH",         "ENABLE",            "I2S2_TX_L_SWITCH" }, \
	{ "I2S2_TX_SWITCH",         "ENABLE",            "I2S2_TX_R_SWITCH" }, \
	{ "I2S2_TX_SEL_MUX",        "I2S2_TX_BTMATCH",   "I2S2_TX_SWITCH" }, \
	{ "I2S2_TX_DRV",            NULL,                "I2S2_TX_SEL_MUX" } \

#define ULTRA_ROUTE \
	{ "ULTRA_DN_IN_48K_SWITCH",   "ENABLE",            "ULTRA_INPUT" }, \
	{ "ULTRA_DN_L_PGA",           NULL,                "ULTRA_DN_IN_48K_SWITCH" }, \
	{ "ULTRA_DN_R_PGA",           NULL,                "ULTRA_DN_IN_48K_SWITCH" }, \
	{ "I2S2_ULTRA_SWITCH",        "ENABLE",            "ULTRA_DN_L_PGA" }, \
	{ "I2S2_ULTRA_SWITCH",        "ENABLE",            "ULTRA_DN_R_PGA" }, \
	{ "I2S2_TX_SEL_MUX",          "I2S2_TX_ULTRA_DN",  "I2S2_ULTRA_SWITCH" } \

#define BT_UPLINK \
	{ "I2S2_RX_INPUT",         NULL,         "I2S2_SUPPLY" }, \
	{ "I2S2_RX_INPUT",         NULL,         "I2S2_RX_SUPPLY" }, \
	{ "I2S2_RX_L_SWITCH",      "ENABLE",     "I2S2_RX_INPUT" }, \
	{ "I2S2_RX_R_SWITCH",      "ENABLE",     "I2S2_RX_INPUT" }, \
	{ "I2S2_RX_L_PGA",         NULL,         "I2S2_RX_L_SWITCH" }, \
	{ "I2S2_RX_R_PGA",         NULL,         "I2S2_RX_R_SWITCH" }, \
	{ "I2S2_RX_L_SRC_SWITCH",  "ENABLE",     "I2S2_RX_L_PGA" }, \
	{ "I2S2_RX_R_SRC_SWITCH",  "ENABLE",     "I2S2_RX_R_PGA" }, \
	{ "VOICE_UP_L_MUX",        NULL,         "I2S2_RX_L_PGA" }, \
	{ "VOICE_UP_R_MUX",        NULL,         "I2S2_RX_R_PGA" }, \
	{ "VOICE_UP_L_MUX",        "I2S2_RX_L",  "I2S2_RX_L_SRC_SWITCH" }, \
	{ "VOICE_UP_R_MUX",        "I2S2_RX_R",  "I2S2_RX_R_SRC_SWITCH" }, \
	{ "AUDIO_UP_L_MUX",        "I2S2_RX_L",  "I2S2_RX_L_SRC_SWITCH" }, \
	{ "AUDIO_UP_R_MUX",        "I2S2_RX_R",  "I2S2_RX_R_SRC_SWITCH" } \

#define FM_DOWNLINK \
	{ "DAC_L_MIXER4",  "I2S2_RX_L",  "I2S2_RX_L_SRC_SWITCH" }, \
	{ "DAC_R_MIXER4",  "I2S2_RX_R",  "I2S2_RX_R_SRC_SWITCH" } \

#define MMI_ROUTE \
	{ "I2S2_BLUETOOTH_LOOP_OUTPUT",   NULL,              "I2S2_SUPPLY" }, \
	{ "I2S2_BLUETOOTH_LOOP_SWITCH",   "ENABLE",          "I2S2_BLUETOOTH_LOOP_INPUT" }, \
	{ "I2S2_BLUETOOTH_LOOP_OUTPUT",   NULL,              "I2S2_BLUETOOTH_LOOP_SWITCH" } \

#define AXI_2_TDM_ROUTE \
	{ "AUDIO_INTF1_SWITCH",     NULL,        "PLL"}, \
	{ "AUDIO_INTF2_SWITCH",     NULL,        "PLL"}, \
	{ "AUDIO_INTF3_SWITCH",     NULL,        "PLL"}, \
	{ "AUDIO_INTF1_SWITCH",     "ENABLE",    "INTF1_INPUT" }, \
	{ "AUDIO_INTF2_SWITCH",     "ENABLE",    "INTF2_INPUT" }, \
	{ "AUDIO_INTF3_SWITCH",     "ENABLE",    "INTF3_INPUT" }, \
	{ "INTF1_OUTPUT",           NULL,        "AUDIO_INTF1_SWITCH" }, \
	{ "INTF2_OUTPUT",           NULL,        "AUDIO_INTF2_SWITCH" }, \
	{ "INTF3_OUTPUT",           NULL,        "AUDIO_INTF3_SWITCH" } \

#define MAD_LP_SINGLE_MIC_PATH \
	{ "MAD_LP_SINGLE_MIC_SWITCH",   "ENABLE",    "MAD_DMIC_INPUT" }, \
	{ "MAD_DMIC_OUTPUT",            NULL,        "MAD_LP_SINGLE_MIC_SWITCH" } \

#define MAD_NORM_SINGLE_MIC_PATH \
	{ "MAD_NORM_SINGLE_MIC_SWITCH", "ENABLE",    "MAD_CODECLESS_INPUT" }, \
	{ "MAD_CODECLESS_OUTPUT",       NULL,        "MAD_NORM_SINGLE_MIC_SWITCH" } \

static int micbias_en_switch;
static const char * const micbias_en_switch_text[] = { "OFF", "ON" };
static const struct soc_enum micbias_en_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micbias_en_switch_text), micbias_en_switch_text),
};

static int micbias_gpio_en_switch_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EFAULT;
	}

	ucontrol->value.integer.value[0] = micbias_en_switch;

	return 0;
}

static int micbias_gpio_en_output_set(int cmd, unsigned int gpio)
{
	int ret;

	if (!gpio_is_valid(gpio)) {
		AUDIO_LOGE("Failed to get the hac gpio");
		return -EFAULT;
	}

	if (cmd) {
		AUDIO_LOGI("Enable micbias en gpio %u", gpio);
		ret = gpio_direction_output(gpio, 1);
	} else {
		AUDIO_LOGI("Disable micbias en gpio %u", gpio);
		ret = gpio_direction_output(gpio, 0);
	}

	return ret;
}

static int micbias_gpio_en_switch_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct snd_soc_component *codec = NULL;
	struct codec_platform_data *priv = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EFAULT;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	priv = snd_soc_component_get_drvdata(codec);
	if (priv->micbias_en_ctl.need_gpio_ctl == false)
		return 0;

	micbias_en_switch = ucontrol->value.integer.value[0];
	ret = micbias_gpio_en_output_set(micbias_en_switch,
		priv->micbias_en_ctl.gpio);

	return ret;
}

#define MIC_UP_CUSTOM_KCONTROLS \
	SOC_ENUM_EXT("MICBIAS_GPIO_EN", micbias_en_switch_enum[0], \
		micbias_gpio_en_switch_get, \
		micbias_gpio_en_switch_put)

static const struct snd_kcontrol_new g_kcontrols[] = {
	MIC_UP_CUSTOM_KCONTROLS
};

static const char * const i2s2_tx_sel_mux_texts[] = {
	"I2S2_TX_BTMATCH",
	"I2S2_TX_ULTRA_DN",
};

static const char * const mic3_mux_texts[] = {
	"MIC3_ADC",
	"MIC4_ADC",
	"MIC1_ADC",
};

static const char * const mic4_mux_texts[] = {
	"MIC3_ADC",
	"MIC4_ADC",
	"MIC2_ADC",
};

static const char * const mic5_mux_texts[] = {
	"MIC3_ADC",
	"MIC5_ADC",
	"MIC1_ADC",
};

static const char * const mic6_mux_texts[] = {
	"MIC4_ADC",
	"MIC6_ADC",
	"MIC2_ADC",
};

static const struct soc_enum i2s2_tx_sel_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, I2S2_TX_SEL_OFFSET,
		ARRAY_SIZE(i2s2_tx_sel_mux_texts), i2s2_tx_sel_mux_texts);

static const struct soc_enum mic3_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, MIC3_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic3_mux_texts), mic3_mux_texts);

static const struct soc_enum mic4_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX0_REG, MIC4_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic4_mux_texts), mic4_mux_texts);

static const struct soc_enum mic5_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, MIC5_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic5_mux_texts), mic5_mux_texts);

static const struct soc_enum mic6_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, MIC6_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic6_mux_texts), mic6_mux_texts);

static const struct snd_kcontrol_new dapm_mic3_mux_controls =
	SOC_DAPM_ENUM("Mux", mic3_mux_enum);
static const struct snd_kcontrol_new dapm_mic4_mux_controls =
	SOC_DAPM_ENUM("Mux", mic4_mux_enum);
static const struct snd_kcontrol_new dapm_mic5_mux_controls =
	SOC_DAPM_ENUM("Mux", mic5_mux_enum);
static const struct snd_kcontrol_new dapm_mic6_mux_controls =
	SOC_DAPM_ENUM("Mux", mic6_mux_enum);
static const struct snd_kcontrol_new dapm_i2s2_tx_sel_mux_controls =
	SOC_DAPM_ENUM("Mux", i2s2_tx_sel_mux_enum);
static const struct snd_kcontrol_new dapm_s2_tx_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, S2_TX_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_ultra_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, S2_ULTRA_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_s2_ultra_dn_48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", CODEC_VIR2_REG_ADDR, S2_ULTRA_DN_48K_BIT, 1, 0);

static int s2_ultra_dn_48k_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	AUDIO_LOGI("s2_ultra in, powen_event is %d", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, FS_CTRL0_REG,
			mask_on_bit(FS_ULTR_R_DN_AFIFO_LEN, FS_ULTR_L_DN_AFIFO_OFFSET) |
			mask_on_bit(FS_ULTR_L_DN_AFIFO_LEN, FS_ULTR_R_DN_AFIFO_OFFSET),
			SAMPLE_RATE_INDEX_48K << FS_ULTR_L_DN_AFIFO_OFFSET |
			SAMPLE_RATE_INDEX_48K << FS_ULTR_R_DN_AFIFO_OFFSET);
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(ULTR_L_DN_AFIFO_CLKEN_OFFSET) | BIT(ULTR_R_DN_AFIFO_CLKEN_OFFSET),
			BIT(ULTR_L_DN_AFIFO_CLKEN_OFFSET) | BIT(ULTR_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits_with_lock(codec, CODEC_CLK_EN0_REG,
			BIT(ULTR_L_DN_AFIFO_CLKEN_OFFSET) | BIT(ULTR_R_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, FS_CTRL0_REG,
			mask_on_bit(FS_ULTR_R_DN_AFIFO_LEN, FS_ULTR_L_DN_AFIFO_OFFSET) |
			mask_on_bit(FS_ULTR_L_DN_AFIFO_LEN, FS_ULTR_R_DN_AFIFO_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

#define ULTRA_WIDGET \
	SND_SOC_DAPM_MUX("I2S2_TX_SEL_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_i2s2_tx_sel_mux_controls), \
	SND_SOC_DAPM_SWITCH("I2S2_TX_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_tx_switch_controls), \
	SND_SOC_DAPM_SWITCH("I2S2_ULTRA_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_ultra_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("ULTRA_DN_IN_48K_SWITCH", SND_SOC_NOPM, 0, 0, \
		&dapm_s2_ultra_dn_48k_switch_controls, s2_ultra_dn_48k_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ULTRA_DN_L_PGA", 0, CODEC_CLK_EN0_REG, \
		ULTRA_DN_L_PGA_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_S("ULTRA_DN_R_PGA", 0, CODEC_CLK_EN0_REG, \
		ULTRA_DN_R_PGA_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_INPUT("ULTRA_INPUT") \

#define MIC_UP_MUX_WIDGET \
	SND_SOC_DAPM_MUX("MIC3_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic3_mux_controls), \
	SND_SOC_DAPM_MUX("MIC4_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic4_mux_controls), \
	SND_SOC_DAPM_MUX("MIC5_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic5_mux_controls), \
	SND_SOC_DAPM_MUX("MIC6_MUX", \
		SND_SOC_NOPM, 0, 0, &dapm_mic6_mux_controls) \

static const struct snd_soc_dapm_widget g_widgets[] = {
	ULTRA_WIDGET,
	MIC_UP_MUX_WIDGET,
};

static const struct snd_soc_dapm_route g_route_map[] = {
	AUDIO_VOICE_2MIC_ROUTE,
	AUDIO_VOICE_4MIC_ROUTE,
	AUDIO_VOICE_6MIC_ROUTE,
	ULTRA_UP_ROUTE,
	AUDIO_VOICE_LOOP_ROUTE,
	UPLINK_PLL,
	DMIC12_ROUTE,
	DMIC34_ROUTE,
	DMIC56_ROUTE,
	DMIC78_ROUTE,
	MIC_CIC_PLL,
	MIC34_MIC56_UP_ROUTE,
	AUDIO_PLAY_2PA,
	AUDIO_PLAY_4PA,
	AUDIO_CARTKIT,
	BT_DOWNLINK,
	I2S2_TX_SWITCH,
	BT_UPLINK,
	FM_DOWNLINK,
	ULTRA_ROUTE,
	MMI_ROUTE,
	AXI_2_TDM_ROUTE,
	MAD_LP_SINGLE_MIC_PATH,
	MAD_NORM_SINGLE_MIC_PATH
};

static const struct snd_codec_resource g_codec_source = {
	NULL,
	g_kcontrols,
	ARRAY_SIZE(g_kcontrols),
	g_widgets,
	ARRAY_SIZE(g_widgets)
};

static const struct snd_codec_route g_codec_route = {
	g_route_map,
	ARRAY_SIZE(g_route_map)
};

static int __init asp_codec_path_init(void)
{
	add_codec_platform_dapm_list(&g_codec_source);
	add_codec_add_route(&g_codec_route);
	return 0;
}

device_initcall(asp_codec_path_init);
