/*
  * sih688x.h
  *
  * code for vibrator
  *
  * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#ifndef _SIH6887_H_
#define _SIH6887_H_

#include "haptic.h"

#define SIH688X_CONT_CONFIG_MAX					7
#define SIH688X_SEQ_MUL_COEFFICIENT				64
#define SIH688X_SEQ_DIV_COEFFICIENT				10
#define SIH688X_SEQ_MUL_ASMOOTH_COE				4096
#define SIH688X_CLK_FS						6144
#define SIH688X_THALF_DIV_COEFFICIENT				1000
#define SIH688X_VERF_COEFFICIENT				78
#define SIH688X_VERF_DIV_COEFFICIENT				10000
#define SIH688X_VOLATE_COEFFICIENT				10
#define SIH688X_VBOOST_MUL_COE					4096
#define SIH688X_CLK_COEFFICIENT					1000
#define SIH688X_DETECT_TIME_COEFFICIENT				10000000
#define SIH688X_RINGING_COEFFICIENT				11
#define SIH688X_DETECT_FIFO_ARRAY_MAX				25
#define SIH688X_DETECT_FIFO_ARRAY_SIZE				5
#define SIH688X_DETECT_PERIOD_MAX				6
#define SIH688X_CHIPID_REG_VALUE				0x87
#define SIH688X_CHIPID_REG_ADDR					0x00
#define SIH688X_LPF_CAP_GAIN_1					82
#define SIH688X_LPF_CAP_GAIN_1_COE				100
#define SIH688X_LPF_CAP_GAIN_2					25
#define SIH688X_LPF_CAP_GAIN_2_COE				10
#define SIH688X_LPF_CAP_GAIN_3					4
#define SIH688X_LPF_CAP_GAIN_4					6
#define SIH688X_LPF_CAP_GAIN_5					8
#define SIH688X_DETECT_BEMF_COE					4096
#define SIH688X_DETECT_ADCV					16
#define SIH688X_BEMF_MV_COE					1000
#define SIH688X_DETECT_ADCV_COE					10
#define SIH688X_DETECT_F0_COE					64
#define SIH688X_DETECT_FOSC					6153
#define SIH688X_DETECT_FOSC_COE					1000
#define SIH688X_F0_CAL_COE					1000000
#define SIH688X_F0_DELTA					2880
#define SIH688X_RL_CALC_COE_DIV					2048
#define SIH688X_RL_CALC_COE_PLUS				10000
#define SIH688X_RL_CALC_COE_PLUS_COE				1000
#define SIH688X_RL_CALC_COE_DIV_COE				21
#define SIH688X_ADC_VREF					16
#define SIH688X_ADC_VREF_COE					10
#define SIH688X_RL_MODIFY					4
#define SIH688X_RL_MODIFY_COE					10
#define SIH688X_LPF_GAIN_COE					4
#define SIH688X_OSC_CALI_COE					1000
#define SIH688X_DRV_BOOST_BASE					35
#define SIH688X_DRV_BOOST_SETP_COE				1000
#define SIH688X_DRV_BOOST_SETP					625
#define SIH688X_STANDARD_VBAT					4200
#define SIH688X_PRE_GAIN_VALUE					128
#define SIH688X_VBAT_AMPLIFY_COE				1000
#define SIH688X_STANDBY_JUDGE_MAX_TIME				200
#define SIH688X_TRACKING_CONFIGE_SIZE				18

typedef enum sih688x_trig_index {
	SIH688X_TRIG_INDEX_TRIG0 = 0,
	SIH688X_TRIG_INDEX_TRIG1 = 1,
	SIH688X_TRIG_INDEX_TRIG2 = 2,
} sih688x_trig_index_e;

typedef enum sih688x_trig_level {
	SIH688X_TRIG_HIGH_ACTIVE = 0,
	SIH688X_TRIG_LOW_ACTIVE = 1,
} sih688x_trig_level_e;

typedef enum sih688x_trig_en {
	SIH688X_TRIG_DISABLE = 0,
	SIH688X_TRIG_ENABLE = 1,
} sih688x_trig_en_e;

typedef enum sih688x_trig_polor {
	SIH688X_TRIG_POSEDGE_TRIGGER = 0,
	SIH688X_TRIG_NEGEDGE_TRIGGER = 1,
	SIH688X_TRIG_BOTHEDGE_TRIGGER = 2,
	SIH688X_TRIG_LEVEL_TRIGGER = 3,
} sih688x_trig_polor_e;

typedef enum SIH688X_TRIG_BOOST {
	SIH688X_TRIG_BOOST_EN = 0,
	SIH688X_TRIG_BOOST_BYPASS = 1,
} sih688x_trig_boost_e;

typedef void (*cont_config_fp)(sih_haptic_t *sih_haptic, uint32_t value);

#endif

