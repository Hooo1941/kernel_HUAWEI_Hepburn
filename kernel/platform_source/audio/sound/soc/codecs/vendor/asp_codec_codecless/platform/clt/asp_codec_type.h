/*
 * asp_codec_type.h -- codec driver
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

#ifndef __ASP_CODEC_TYPE_H__
#define __ASP_CODEC_TYPE_H__

#include "asp_codec_reg.h"
#include "asp_codec_reg_adapter.h"
#include "asp_codec_comm.h"

enum virtual_reg_offset {
	VIR0_REG_OFFSET = 0x0,
	VIR1_REG_OFFSET,
	VIR2_REG_OFFSET,
	VIR_REG_CNT,
};

/* virtual reg */
#define CODEC_VIR0_REG_ADDR ((PAGE_VIRCODEC) + (VIR0_REG_OFFSET))
#define AUDIO_UP_O_BIT 0
#define VOICE_UP_O_BIT 1
#define MIC34_UP_O_BIT 2
#define MIC56_UP_O_BIT 3
#define MIC78_UP_O_BIT 4
#define AUDIO_UP_2MIC_48K_BIT 5
#define VOICE_UP_2MIC_48K_BIT 6
#define VOICE_UP_2MIC_32K_BIT 7
#define AUDIO_UP_4MIC_48K_BIT 8
#define VOICE_UP_4MIC_48K_BIT 9
#define VOICE_UP_4MIC_32K_BIT 10
#define AUDIO_UP_6MIC_48K_BIT 11
#define AMIC1_O_BIT 17
#define AMIC2_O_BIT 18
#define HSAMIC1_O_BIT 19
#define HSAMIC2_O_BIT 20
#define ANA_MIC3_PGA_BIT 21
#define ANA_MIC3PGA_BOOST_BIT 22
#define ANA_DACL_BIT 23
#define ANA_DACR_BIT 24
#define AU_HP_DACL_BIT 25
#define AU_HP_DACR_BIT 26
#define ANA_AUL_EN_BIT 27
#define ANA_AUR_EN_BIT 28
#define ANA_MUX3_EN_BIT 29
#define ANA_MUX4_EN_BIT 30
#define ANA_MUX5_EN_BIT 31

#define CODEC_VIR1_REG_ADDR   ((PAGE_VIRCODEC) + (VIR1_REG_OFFSET))
#define AUDIO_DN_4PA_IN_BIT 0
#define CODEC3_DN_44K1_IN_BIT 2
#define AUDIO_DN_2PA_IN_BIT 6
#define S1_TX_TDM_BIT 9
#define S1_TX_I2S_BIT  10
#define IV_2PA_BIT 11
#define IV_4PA_BIT 12
#define CARKIT_BIT 13
#define S1_TX_TDM256_BIT 14
#define CODEC3_DN_R_IN_BIT 15
#define MAD_LP_SINGLE_MIC_BIT 16
#define MAD_NORM_SINGLE_MIC_BIT 18
#define S2_TX_L_BIT 20
#define S2_TX_R_BIT 21
#define S2_RX_L_BIT 22
#define S2_RX_R_BIT 23
#define HP_AU_BIT 24
#define ANA_AU_ULTR_EN_BIT 25
#define ANA_AU_MAD_EN_BIT 26
#define AU0_L_BIT 27
#define AU0_R_BIT 28
#define SIDETONE_SWITCH_CLKEN_BIT 29

#define CODEC_VIR2_REG_ADDR   ((PAGE_VIRCODEC) + (VIR2_REG_OFFSET))
#define S2_TX_L_SRCDN_BIT 1
#define S2_TX_R_SRCDN_BIT 2
#define S2_RX_L_SRC_BIT 3
#define S2_RX_R_SRC_BIT 4
#define S2_PLL_TRK_BIT 5
#define S2_TX_BIT 6
#define S2_ULTRA_BIT 7
#define S2_ULTRA_DN_48K_BIT 8
#define AUDIO_AXI2TDM_INTF1_BIT 9
#define AUDIO_AXI2TDM_INTF2_BIT 10
#define AUDIO_AXI2TDM_INTF3_BIT 11
#define WAKEUP_MAIN_MIC_SWITCH_BIT 12
#define MDM5G_UP_48K_SWITCH_BIT 13
#define VOICE_UP_4MIC_8K_BIT 14
#define I2S2_BLUETOOTH_LOOP_BIT 15
#define BT_VOICE_8K_BIT         16
#define BT_VOICE_16K_BIT        17
#define BT_VOICE_32K_BIT        18

struct micbias_en_ctl_resource {
	bool need_gpio_ctl;
	int gpio;
};

struct codec_platform_data {
	struct codec_data base;
	struct micbias_en_ctl_resource micbias_en_ctl;
};

#endif

