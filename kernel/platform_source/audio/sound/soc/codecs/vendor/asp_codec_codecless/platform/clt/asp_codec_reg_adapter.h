/*
 * asp_codec_reg_adapter.h -- codec driver
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

#ifndef __ASP_CODEC_ADAPTER_REG_H__
#define __ASP_CODEC_ADAPTER_REG_H__

/* clk reg adapter begin */
#define I2S2_RX_L_SRCUP_CLKEN_REG           CODEC_CLK_EN1_REG
#define I2S2_RX_R_SRCUP_CLKEN_REG           CODEC_CLK_EN1_REG
#define I2S2_RX_L_SRCDN_CLKEN_REG           CODEC_CLK_EN1_REG
#define I2S2_RX_R_SRCDN_CLKEN_REG           CODEC_CLK_EN1_REG
#define I2S2_PCM_CLKEN_REG                  CODEC_CLK_EN2_REG
#define I2S2_TX_MIXER2_CLKEN_REG            CODEC_CLK_EN2_REG

#define ADC1_CIC_D16_CLKEN_REG              CODEC_CLK_EN2_REG
#define ADC2_CIC_D16_CLKEN_REG              CODEC_CLK_EN2_REG
#define ADC3_CIC_D16_CLKEN_REG              CODEC_CLK_EN2_REG
#define ADC4_CIC_D16_CLKEN_REG              CODEC_CLK_EN2_REG
#define ADC5_CIC_D16_CLKEN_REG              CODEC_CLK_EN2_REG
#define ADC6_CIC_D16_CLKEN_REG              CODEC_CLK_EN3_REG
#define ADC7_CIC_D16_CLKEN_REG              CODEC_CLK_EN3_REG
#define ADC8_CIC_D16_CLKEN_REG              CODEC_CLK_EN3_REG

#define ADC1_0P5_PGA_CLKEN_REG              CODEC_CLK_EN1_REG
#define ADC1_CLKEN_REG                      CODEC_CLK_EN0_REG
#define ADC2_0P5_PGA_CLKEN_REG              CODEC_CLK_EN1_REG
#define ADC2_CLKEN_REG                      CODEC_CLK_EN0_REG
#define ADC3_0P5_PGA_CLKEN_REG              CODEC_CLK_EN1_REG
#define ADC3_CLKEN_REG                      CODEC_CLK_EN0_REG
#define ADC4_0P5_PGA_CLKEN_REG              CODEC_CLK_EN1_REG
#define ADC4_CLKEN_REG                      CODEC_CLK_EN0_REG
#define ADC5_0P5_PGA_CLKEN_REG              CODEC_CLK_EN1_REG
#define ADC5_CLKEN_REG                      CODEC_CLK_EN2_REG
#define ADC6_0P5_PGA_CLKEN_REG              CODEC_CLK_EN3_REG
#define ADC6_CLKEN_REG                      CODEC_CLK_EN3_REG
#define ADC7_0P5_PGA_CLKEN_REG              CODEC_CLK_EN3_REG
#define ADC7_CLKEN_REG                      CODEC_CLK_EN3_REG
#define ADC8_0P5_PGA_CLKEN_REG              CODEC_CLK_EN3_REG
#define ADC8_CLKEN_REG                      CODEC_CLK_EN3_REG
#define MIC3_UP_PGA_CLKEN_REG               CODEC_CLK_EN0_REG

#define DMIC1_CLKEN_REG                     CODEC_CLK_EN2_REG
#define DMIC2_CLKEN_REG                     CODEC_CLK_EN2_REG

#define AUDIO_UP_PGA_CLKEN_REG              CODEC_CLK_EN0_REG
#define VOICE_UP_PGA_CLKEN_REG              CODEC_CLK_EN0_REG
#define MIC56_UP_PGA_CLKEN_REG              CODEC_CLK_EN4_REG
#define MIC56_UP_SRCDN_CLKEN_REG            CODEC_CLK_EN2_REG
#define MIC56_UP_AFIFO_CLKEN_REG            CODEC_CLK_EN3_REG
#define MIC78_UP_PGA_CLKEN_REG              CODEC_CLK_EN4_REG
#define MIC78_UP_SRCDN_CLKEN_REG            CODEC_CLK_EN2_REG
#define MIC78_UP_AFIFO_CLKEN_REG            CODEC_CLK_EN3_REG

#define MDM_5G_R_UP_AFIFO_CLKEN_REG         CODEC_CLK_EN0_REG
#define MDM_5G_R_UP_PGA_CLKEN_REG           CODEC_CLK_EN1_REG

/* fs reg adapter */
#define FS_I2S2_RX_L_SRCUP_DIN_REG          FS_CTRL3_REG
#define FS_I2S2_RX_L_SRCUP_DOUT_REG         FS_CTRL3_REG
#define FS_I2S2_RX_R_SRCUP_DIN_REG          FS_CTRL4_REG
#define FS_I2S2_RX_R_SRCUP_DOUT_REG         FS_CTRL4_REG
#define FS_I2S2_TX_MIXER2_REG               FS_CTRL4_REG

#define FS_VOICE_UP_PGA_REG                 FS_CTRL2_REG
#define FS_MIC56_UP_AFIFO_REG               FS_CTRL8_REG
#define FS_MIC56_UP_PGA_REG                 FS_CTRL9_REG
#define FS_MIC56_SRCDN_DOUT_REG             FS_CTRL6_REG

#define FS_MDM_5G_SRCDN_OUT_REG             FS_CTRL5_REG
#define FS_MDM_5G_UP_PGA_REG                FS_CTRL2_REG
#define FS_MDM_5G_UP_AFIFO_REG              FS_CTRL1_REG

/* src reg adapter */
#define I2S2_RX_L_SRCUP_SRC_MODE_REG        SRCUP_CTRL_REG
#define I2S2_RX_R_SRCUP_SRC_MODE_REG        SRCUP_CTRL_REG
#define MIC56_UP_SRCDN_SRC_MODE_REG         SRCDN_CTRL1_REG

#define MDM_5G_UP_SRCDN_SRC_MODE_REG        SRCDN_CTRL0_REG

/* mux reg adapter */
#define MDM_5G_UP_L_DIN_SEL_REG             CODEC_DIN_MUX1_REG
#define MDM_5G_UP_R_DIN_SEL_REG             CODEC_DIN_MUX1_REG

#endif
