/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HDMIRX_REG_ADDR_H
#define HDMIRX_REG_ADDR_H

#define REG_CLK_DETECT0 0x1C000
#define REG_XTAL_TIMER_VAL 0xFF	   /* mask bit 7:0 */
#define REG_DVI_PRD_SEL   0x700	  /* mask bit 10:8 */
#define REG_CLK_STBEXT_SEL 0x7000	 /* mask bit 14:12 */
#define REG_TEST_SEL  0x78000    /* mask bit 18:15 */
#define HDMI_ZONE_CTRL 0x380000   /* mask bit 21:19 */
#define HDMI_CLK_STABLE   0x400000   /* mask bit 22 */
#define CLK_DVI_EXIST 0x800000   /* mask bit 23 */
#define HDMI_CLK_STABLE_LANE1 0x1000000  /* mask bit 24 */
#define CLK_DVI_EXIST_LANE1   0x2000000  /* mask bit 25 */
#define HDMI_CLK_STABLE_LANE2 0x4000000  /* mask bit 26 */
#define CLK_DVI_EXIST_LANE2   0x8000000  /* mask bit 27 */
#define HDMI_CLK_STABLE_LANE3 0x10000000 /* mask bit 28 */
#define CLK_DVI_EXIST_LANE3   0x20000000 /* mask bit 29 */

#define REG_TMDS_CONFIGURATION 0x8480
#define SCRAMBLING_ENABLE 0x1 /* mask bit 0 */
#define TMDS_BIT_CLOCK_RATIO 0x2 /* mask bit 1 */

#define REG_TMDS_SCRAMBLER_STATUS 0x8484
#define SCRAMBLER_STATUS 0x1 /* mask bit 0 */

#define REG_SINK_CONFIGURATION_B1 0x84C4
#define FRL_RATE   0xF  /* mask bit 3:0 */
#define FFE_LEVELS 0xF0 /* mask bit 7:4 */

#define REG_D2A12 0x1C1E0
#define REG_DA_RESET_VCO 0xF	   /* mask bit 3:0 */
#define REG_DA_RESET_VCO_EN	  0xF0	  /* mask bit 7:4 */
#define RG_EN_IDAC_CAL_IN_L3 0x3F00    /* mask bit 13:8 */
#define REG_DAMIX_CLK_DET_SEL 0x10000   /* mask bit 16 */
#define REG_DAMIX_CLK_DET_SEL_EN 0x20000   /* mask bit 17 */
#define REG_EN_EDFE  0x40000   /* mask bit 18 */
#define REG_DAMIX_FRL_RATE   0xF00000  /* mask bit 23:20 */
#define REG_DAMIX_FRL_RATE_EN 0x1000000 /* mask bit 24 */
#define RG_SEL_CAL   0xE000000 /* mask bit 27:25 */

#define REG_ANALOG33 0x1C1E4
#define RG_PD_LDO_NODIE	  0x1 /* mask bit 0 */
#define RG_EN_LDO_NODIE_TEST 0x2 /* mask bit 1 */
#define RG_SEL_LDO_NODIE 0xC /* mask bit 3:2 */

#define REG_ANALOG8 0x1C0C8
#define RG_ISEL_BIST_CH 0x3 /* mask bit 1:0 */
#define RG_ISEL_BIST_L0 0xC /* mask bit 3:2 */
#define RG_ISEL_BIST_L1 0x30   /* mask bit 5:4 */
#define RG_ISEL_BIST_L2 0xC0   /* mask bit 7:6 */
#define RG_ISEL_EQ_L0   0x300  /* mask bit 9:8 */
#define RG_ISEL_EQ_L1   0xC00  /* mask bit 11:10 */
#define RG_ISEL_EQ_L2   0x3000 /* mask bit 13:12 */
#define RG_PD_BG 0x4000 /* mask bit 14 */
#define RG_PD_CLK   0x8000 /* mask bit 15 */
#define RG_PD_LANE  0xF0000 /* mask bit 19:16 */
#define RG_PD_LDO   0x100000   /* mask bit 20 */
#define RG_PD_PHDAC 0x1E00000  /* mask bit 24:21 */
#define RG_PD_RT 0x1E000000 /* mask bit 28:25 */
#define RG_PD_PLL   0x20000000 /* mask bit 29 */
#define RG_ISEL_EQ_L3   0xC0000000 /* mask bit 31:30 */

#define REG_ANALOG24 0x1C1B0
#define RG_TEST_EQ_L2_HIGH   0xFFFFFFFF /* mask bit 31:0 */
#define RG_ENDISC_L2 0X20   /* mask bit 5 */
#define RG_EXTERNAL_DISCHARGE_L2 0X40   /* mask bit 6 */

#define REG_ANALOG25 0x1C1B4
#define RG_TEST_EQ_L1_HIGH   0xFFFFFFFF /* mask bit 31:0 */
#define RG_ENDISC_L1 0X20   /* mask bit 5 */
#define RG_EXTERNAL_DISCHARGE_L1 0X40   /* mask bit 6 */

#define REG_ANALOG26 0x1C1B8
#define RG_TEST_EQ_L0_HIGH   0xFFFFFFFF /* mask bit 31:0 */
#define RG_ENDISC_L0 0X20   /* mask bit 5 */
#define RG_EXTERNAL_DISCHARGE_L0 0X40   /* mask bit 6 */

#define REG_ANALOG28 0x1C1C0
#define RG_TEST_EQ_L3_HIGH   0xFFFFFFFF /* mask bit 31:0 */
#define RG_ENDISC_L3 0X20   /* mask bit 5 */
#define RG_EXTERNAL_DISCHARGE_L3 0X40   /* mask bit 6 */

#define REG_DFE_CFG0_LANE0 0x1C258
#define CFG_DTAP_SW_LANE0   0x3F    /* mask bit 5:0 */
#define CFG_DTAP_OVR_LANE0  0x40    /* mask bit 6 */
#define CFG_TTAP_SW_LANE0   0x3F00  /* mask bit 13:8 */
#define CFG_TTAP_OVR_LANE0  0x4000  /* mask bit 14 */
#define CFG_DFE_EN_SW_LANE0 0x10000 /* mask bit 16 */
#define CFG_DFE_EN_OVR_LANE0 0x20000 /* mask bit 17 */
#define CFG_DFE_JUDGE_MASK_LANE0 0x3C0000   /* mask bit 21:18 */
#define CFG_DFE_MODE_LANE0  0xC00000   /* mask bit 23:22 */
#define CFG_DFE_SW_START_LANE0  0x1000000  /* mask bit 24 */
#define CFG_DFE_DONE_MODE_LANE0 0x2000000  /* mask bit 25 */
#define CFG_DFE_UNSTABLE_MODE_LANE0 0x4000000  /* mask bit 26 */
#define CFG_DFE_ST_CLR_LANE0 0x8000000  /* mask bit 27 */
#define CFG_DFE_VALUE_CLR_LANE0  0x10000000 /* mask bit 28 */

#define REG_DFE_CFG0_LANE1 0x1C27C
#define CFG_DTAP_SW_LANE1   0x3F   /* mask bit 5:0 */
#define CFG_DTAP_OVR_LANE1  0x40   /* mask bit 6 */
#define CFG_TTAP_SW_LANE1   0x3F00 /* mask bit 13:8 */
#define CFG_TTAP_OVR_LANE1  0x4000 /* mask bit 14 */
#define CFG_DFE_EN_SW_LANE1 0x10000 /* mask bit 16 */
#define CFG_DFE_EN_OVR_LANE1 0x20000 /* mask bit 17 */
#define CFG_DFE_JUDGE_MASK_LANE1 0x3C0000   /* mask bit 21:18 */
#define CFG_DFE_MODE_LANE1   0xC00000   /* mask bit 23:22 */
#define CFG_DFE_SW_START_LANE1   0x1000000  /* mask bit 24 */
#define CFG_DFE_DONE_MODE_LANE1  0x2000000  /* mask bit 25 */
#define CFG_DFE_UNSTABLE_MODE_LANE1 0x4000000  /* mask bit 26 */
#define CFG_DFE_ST_CLR_LANE1 0x8000000  /* mask bit 27 */
#define CFG_DFE_VALUE_CLR_LANE1 0x10000000 /* mask bit 28 */

#define REG_DFE_CFG0_LANE2 0x1C2A0
#define CFG_DTAP_SW_LANE2   0x3F   /* mask bit 5:0 */
#define CFG_DTAP_OVR_LANE2  0x40   /* mask bit 6 */
#define CFG_TTAP_SW_LANE2   0x3F00 /* mask bit 13:8 */
#define CFG_TTAP_OVR_LANE2  0x4000 /* mask bit 14 */
#define CFG_DFE_EN_SW_LANE2 0x10000 /* mask bit 16 */
#define CFG_DFE_EN_OVR_LANE2 0x20000 /* mask bit 17 */
#define CFG_DFE_JUDGE_MASK_LANE2 0x3C0000   /* mask bit 21:18 */
#define CFG_DFE_MODE_LANE2   0xC00000   /* mask bit 23:22 */
#define CFG_DFE_SW_START_LANE2   0x1000000  /* mask bit 24 */
#define CFG_DFE_DONE_MODE_LANE2 0x2000000  /* mask bit 25 */
#define CFG_DFE_UNSTABLE_MODE_LANE2 0x4000000  /* mask bit 26 */
#define CFG_DFE_ST_CLR_LANE2 0x8000000  /* mask bit 27 */
#define CFG_DFE_VALUE_CLR_LANE2  0x10000000 /* mask bit 28 */

#define REG_DFE_CFG0_LANE3 0x1C2C4
#define CFG_DTAP_SW_LANE3    0x3F    /* mask bit 5:0 */
#define CFG_DTAP_OVR_LANE3   0x40    /* mask bit 6 */
#define CFG_TTAP_SW_LANE3    0x3F00	 /* mask bit 13:8 */
#define CFG_TTAP_OVR_LANE3   0x4000  /* mask bit 14 */
#define CFG_DFE_EN_SW_LANE3  0x10000 /* mask bit 16 */
#define CFG_DFE_EN_OVR_LANE3 0x20000 /* mask bit 17 */
#define CFG_DFE_JUDGE_MASK_LANE3 0x3C0000   /* mask bit 21:18 */
#define CFG_DFE_MODE_LANE3  0xC00000   /* mask bit 23:22 */
#define CFG_DFE_SW_START_LANE3  0x1000000  /* mask bit 24 */
#define CFG_DFE_DONE_MODE_LANE3 0x2000000  /* mask bit 25 */
#define CFG_DFE_UNSTABLE_MODE_LANE3 0x4000000  /* mask bit 26 */
#define CFG_DFE_ST_CLR_LANE3 0x8000000  /* mask bit 27 */
#define CFG_DFE_VALUE_CLR_LANE3 0x10000000 /* mask bit 28 */

#define REG_D2A5 0x1C094
#define REG_DA_ICTRL_PFD2 0xFF   /* mask bit 7:0 */
#define REG_DA_ICTRL_PFD_EN2 0x100  /* mask bit 8 */
#define REG_DA_ICTRL_PFD1 0x1FE00   /* mask bit 16:9 */
#define REG_DA_ICTRL_PFD_EN1 0x20000   /* mask bit 17 */
#define REG_DA_ICTRL_PFD0 0x3FC0000 /* mask bit 25:18 */
#define REG_DA_ICTRL_PFD_EN0 0x4000000 /* mask bit 26 */

#define REG_ANALOG29 0x1C1C4
#define RG_TEST_PLL_L3_LOW   0xFFFFFFFF /* mask bit 31:0 */
#define TEST_KVCO_L3 0x001f8000 /* mask bit 20:15 */
#define EN_LDO_L3    0x80000 /* mask bit 19 */
#define PD_STAFF_CALIBRATION_L3 0x80000000 /* mask bit 31 */

#define REG_ANALOG1 0x1C0AC
#define RG_TEST_PLL_L2_LOW   0xFFFFFFFF /* mask bit 31:0 */
#define TEST_KVCO_L2 0x001f8000 /* mask bit 20:15 */
#define EN_LDO_L2    0x80000 /* mask bit 19 */
#define PD_STAFF_CALIBRATION_L2 0x80000000 /* mask bit 31 */

#define REG_ANALOG2 0x1C0B0
#define RG_TEST_PLL_L1_LOW   0xFFFFFFFF /* mask bit 31:0 */
#define TEST_KVCO_L1 0x001f8000 /* mask bit 20:15 */
#define EN_LDO_L1    0x80000 /* mask bit 19 */
#define PD_STAFF_CALIBRATION_L1 0x80000000 /* mask bit 31 */

#define REG_ANALOG3 0x1C0B4
#define RG_TEST_PLL_L0_LOW    0xFFFFFFFF /* mask bit 31:0 */
#define TEST_KVCO_L0 0x001f8000 /* mask bit 20:15 */
#define EN_LDO_L0    0x80000 /* mask bit 19 */
#define PD_STAFF_CALIBRATION_L0 0x80000000 /* mask bit 31 */

#define REG_ANALOG9 0x1C0CC
#define RG_EN_DRVP 0xF /* mask bit 3:0 */
#define RG_EN_DRVN 0xF0    /* mask bit 7:4 */
#define RG_EN_CLKO_TMDS_2X 0x100   /* mask bit 8 */
#define RG_EN_CLKO_PIX_2X  0x200   /* mask bit 9 */
#define RG_EN_CLKO_PIX  0x400  /* mask bit 10 */
#define RG_DATI 0x3C000 /* mask bit 17:14 */
#define RG_EN_BIST  0x3C0000   /* mask bit 21:18 */
#define RG_SEL_LDO  0xC00000   /* mask bit 23:22 */
#define RG_RT_CTRL 0xF000000  /* mask bit 27:24 */
#define RG_EN_CAL_AMP  0xF0000000 /* mask bit 31:28 */

#define REG_ANALOG11 0x1C0D4
#define RG_RT_CTRL_CK   0xF0   /* mask bit 7:4 */
#define RG_BG_TEST  0x300  /* mask bit 9:8 */
#define RG_CAL_START 0x1000 /* mask bit 12 */
#define RG_CAL_OV   0x7E000 /* mask bit 18:13 */
#define RG_EN_CAL_OV 0x80000 /* mask bit 19 */
#define RG_EN_SHORT 0xF00000   /* mask bit 23:20 */
#define RG_EN_SCAN  0x7000000  /* mask bit 26:24 */
#define RG_EN_MHL_BIST  0x8000000  /* mask bit 27 */
#define RG_EN_MHL    0x10000000 /* mask bit 28 */
#define RG_EN_LPBK_TEST 0x20000000 /* mask bit 29 */

#define REG_D2A1 0x1C084
#define REG_DA_DIVSEL_POST2 0x7 /* mask bit 2:0 */
#define REG_DA_DIVSEL_POST_EN2 0x8 /* mask bit 3 */
#define REG_DA_DIVSEL_POST1 0x70    /* mask bit 6:4 */
#define REG_DA_DIVSEL_POST_EN1 0x80    /* mask bit 7 */
#define REG_DA_DIVSEL_POST0 0x700   /* mask bit 10:8 */
#define REG_DA_DIVSEL_POST_EN0 0x800  /* mask bit 11 */
#define REG_DA_DIVSEL_PLL2  0x1F000 /* mask bit 16:12 */
#define REG_DA_DIVSEL_PLL_EN2  0x20000 /* mask bit 17 */
#define REG_DA_DIVSEL_PLL1  0x7C0000   /* mask bit 22:18 */
#define REG_DA_DIVSEL_PLL_EN1  0x800000   /* mask bit 23 */
#define REG_DA_DIVSEL_PLL0  0x1F000000  /* mask bit 28:24 */
#define REG_DA_DIVSEL_PLL_EN0  0x20000000 /* mask bit 29 */

#define REG_D2A11 0x1C1DC
#define REG_DA_DIVSEL_POST3 0x7    /* mask bit 2:0 */
#define REG_DA_DIVSEL_POST_EN3 0x8    /* mask bit 3 */
#define REG_DA_DIVSEL_PLL3  0x1F0  /* mask bit 8:4 */
#define REG_DA_DIVSEL_PLL_EN3  0x200  /* mask bit 9 */
#define REG_DA_ICTRL_PD3    0x3FC00   /* mask bit 17:10 */
#define REG_DA_ICTRL_PD_EN3 0x40000   /* mask bit 18 */
#define REG_DA_ICTRL_PFD3   0x7F80000 /* mask bit 26:19 */
#define REG_DA_ICTRL_PFD_EN3   0x8000000 /* mask bit 27 */

#define REG_ANALOG4 0x1C0B8
#define RG_TEST_EQ_L2_LOW 0xFFFFFFFF /* mask bit 31:0 */

#define REG_ANALOG5 0x1C0BC
#define RG_TEST_EQ_L1_LOW 0xFFFFFFFF /* mask bit 31:0 */

#define REG_ANALOG6 0x1C0C0
#define RG_TEST_EQ_L0_LOW 0xFFFFFFFF /* mask bit 31:0 */

#define REG_ANALOG7 0x1C0C4
#define RG_TEST_ATOP_LOW 0xFFFFFFFF /* mask bit 31:0 */

#define REG_ANALOG0 0x1C0A8
#define RG_SEL_CLKIN_L0 0x3 /* mask bit 1:0 */
#define RG_SEL_CLKIN_L1 0xC /* mask bit 3:2 */
#define RG_SEL_CLKIN_L2 0x30    /* mask bit 5:4 */
#define RG_TEST_CLK_CH  0x3FC0  /* mask bit 13:6 */
#define RG_TST_SEL_BIST_CH 0x3C000 /* mask bit 17:14 */
#define RG_TST_SEL_BIST_L0 0x3C0000   /* mask bit 21:18 */
#define RG_TST_SEL_BIST_L1 0x3C00000  /* mask bit 25:22 */
#define RG_TST_SEL_BIST_L2 0x3C000000 /* mask bit 29:26 */
#define RG_SEL_CLKIN_L3 0xC0000000 /* mask bit 31:30 */

#define REG_PLL_LOCK1 0x1C018
#define PLL_CNT_LANE0    0x1FFFF /* mask bit 16:0 */
#define HDMI_CLK_LOCK_LANE2 0x20000 /* mask bit 17 */
#define HDMI_CLK_LOCK_LANE1 0x40000 /* mask bit 18 */
#define HDMI_CLK_LOCK_LANE0 0x80000 /* mask bit 19 */
#define PLL_LOCK            0x100000   /* mask bit 20 */
#define CLK_FB_EXIST_LANE2  0x200000   /* mask bit 21 */
#define CLK_FB_EXIST_LANE1  0x400000   /* mask bit 22 */
#define CLK_FB_EXIST_LANE0  0x800000   /* mask bit 23 */
#define CLK_FB_EXIST_LANE3  0x1000000  /* mask bit 24 */
#define HDMI_CLK_LOCK_LANE3 0x2000000  /* mask bit 25 */
#define HDMI_FRQ_MODE_HIGH  0x1C000000 /* mask bit 28:26 */

#define REG_AUTOEQ11 0x1C074
#define AUTO_EQ_ST_LANE2 0x1FF  /* mask bit 8:0 */
#define AUTO_EQ_ST_LANE1 0x3FE00   /* mask bit 17:9 */
#define AUTO_EQ_ST_LANE0 0x7FC0000 /* mask bit 26:18 */

#define REG_AUTOEQ20 0x1C130
#define AUTO_EQ_ST_LANE3 0x1FF /* mask bit 8:0 */

#define REG_DAMIX_INTR_STATE 0x1C194
#define DAMIX_INTR_STAT0  0x1    /* mask bit 0 */
#define DAMIX_INTR_STAT1  0x2    /* mask bit 1 */
#define DAMIX_INTR_STAT2  0x4    /* mask bit 2 */
#define DAMIX_INTR_STAT3  0x8    /* mask bit 3 */
#define DAMIX_INTR_STAT4  0x10   /* mask bit 4 */
#define DAMIX_INTR_STAT5  0x20   /* mask bit 5 */
#define DAMIX_INTR_STAT6  0x40   /* mask bit 6 */
#define DAMIX_INTR_STAT7  0x80   /* mask bit 7 */
#define DAMIX_INTR_STAT8  0x100  /* mask bit 8 */
#define DAMIX_INTR_STAT9  0x200  /* mask bit 9 */
#define DAMIX_INTR_STAT10 0x400  /* mask bit 10 */
#define DAMIX_INTR_STAT11 0x800  /* mask bit 11 */
#define DAMIX_INTR_STAT12 0x1000 /* mask bit 12 */
#define DAMIX_INTR_STAT13 0x2000 /* mask bit 13 */
#define DAMIX_INTR_STAT14 0x4000 /* mask bit 14 */
#define DAMIX_INTR_STAT15 0x8000  /* mask bit 15 */
#define DAMIX_INTR_STAT16 0x10000   /* mask bit 16 */
#define DAMIX_INTR_STAT17 0x20000   /* mask bit 17 */
#define DAMIX_INTR_STAT18 0x40000   /* mask bit 18 */
#define DAMIX_INTR_STAT19 0x80000   /* mask bit 19 */
#define DAMIX_INTR_STAT20 0x100000  /* mask bit 20 */
#define DAMIX_INTR_STAT21 0x200000  /* mask bit 21 */
#define DAMIX_INTR_STAT22 0x400000  /* mask bit 22 */
#define DAMIX_INTR_STAT23 0x800000  /* mask bit 23 */
#define DAMIX_INTR_STAT24 0x1000000 /* mask bit 24 */
#define DAMIX_INTR_STAT25 0x2000000 /* mask bit 25 */

#define REG_D2A4 0x1C090
#define REG_DA_ICTRL_PD2 0xFF   /* mask bit 7:0 */
#define REG_DA_ICTRL_PD_EN2 0x100 /* mask bit 8 */
#define REG_DA_ICTRL_PD1 0x1FE00   /* mask bit 16:9 */
#define REG_DA_ICTRL_PD_EN1 0x20000   /* mask bit 17 */
#define REG_DA_ICTRL_PD0 0x3FC0000 /* mask bit 25:18 */
#define REG_DA_ICTRL_PD_EN0 0x4000000 /* mask bit 26 */

#define REG_D2A7 0x1C09C
#define REG_DA_SEL_PHD_REG2 0x3 /* mask bit 1:0 */
#define REG_DA_SEL_PHD_REG_EN2 0x4 /* mask bit 2 */
#define REG_DA_SEL_PHD_REG1 0x18    /* mask bit 4:3 */
#define REG_DA_SEL_PHD_REG_EN1 0x20	   /* mask bit 5 */
#define REG_DA_SEL_PHD_REG0 0xC0    /* mask bit 7:6 */
#define REG_DA_SEL_PHD_REG_EN0 0x100   /* mask bit 8 */
#define REG_DA_SEL_PHD_REG3 0x600  /* mask bit 10:9 */
#define REG_DA_SEL_PHD_REG_EN3 0x800  /* mask bit 11 */
#define REG_DA_RCTRL_PLL2  0x78000 /* mask bit 18:15 */
#define REG_DA_RCTRL_PLL_EN2   0x80000 /* mask bit 19 */
#define REG_DA_RCTRL_PLL1  0xF00000   /* mask bit 23:20 */
#define REG_DA_RCTRL_PLL_EN1   0x1000000  /* mask bit 24 */
#define REG_DA_RCTRL_PLL0  0x1E000000 /* mask bit 28:25 */
#define REG_DA_RCTRL_PLL_EN0   0x20000000 /* mask bit 29 */

#define REG_D2A10 0x1C1D8
#define REG_DA_CR_LOCK    0xF /* mask bit 3:0 */
#define REG_DA_CR_LOCK_EN 0xF0    /* mask bit 7:4 */
#define REG_DA_BW_SEL3    0x700   /* mask bit 10:8 */
#define REG_DA_BW_SEL_EN3 0x800   /* mask bit 11 */
#define REG_DA_BW_SEL2    0x7000  /* mask bit 14:12 */
#define REG_DA_BW_SEL_EN2 0x8000  /* mask bit 15 */
#define REG_DA_BW_SEL1   0x70000 /* mask bit 18:16 */
#define REG_DA_BW_SEL_EN1 0x80000 /* mask bit 19 */
#define REG_DA_BW_SEL0    0x700000   /* mask bit 22:20 */
#define REG_DA_BW_SEL_EN0 0x800000   /* mask bit 23 */
#define REG_DA_RCTRL_PLL3 0xF000000  /* mask bit 27:24 */
#define REG_DA_RCTRL_PLL_EN3 0x10000000 /* mask bit 28 */

#define REG_FIXED_EQ_CFG0 0x1C15C
#define REG_EQ_FIXED0 0x1F    /* mask bit 4:0 */
#define REG_EQ_FIXED1 0x3E0	  /* mask bit 9:5 */
#define REG_EQ_FIXED2 0x7C00  /* mask bit 14:10 */
#define REG_EQ_FIXED3 0xF8000 /* mask bit 19:15 */
#define REG_EQ_FIXED4 0x1F00000  /* mask bit 24:20 */
#define REG_EQ_FIXED5 0x3E000000 /* mask bit 29:25 */

#define REG_FIXED_EQ_CFG1 0x1C160
#define REG_EQ_FIXED6 0x1F   /* mask bit 4:0 */
#define REG_EQ_FIXED7 0x3E0  /* mask bit 9:5 */
#define REG_EQ_SEL     0x1C00 /* mask bit 12:10 */
#define REG_FIXED_SCAN_EQ_EN 0x2000 /* mask bit 13 */

#define REG_D2A2 0x1C088
#define REG_DA_EQ_CODE2 0x1F    /* mask bit 4:0 */
#define REG_DA_EQ_CODE_EN2  0x20    /* mask bit 5 */
#define REG_DA_EQ_CODE1  0x7C0	  /* mask bit 10:6 */
#define REG_DA_EQ_CODE_EN1  0x800   /* mask bit 11 */
#define REG_DA_EQ_CODE0  0x1F000 /* mask bit 16:12 */
#define REG_DA_EQ_CODE_EN0  0x20000 /* mask bit 17 */
#define REG_DA_EN_PHD_FB 0x3C0000   /* mask bit 21:18 */
#define REG_DA_EN_PHD_FB_EN 0x3C00000  /* mask bit 25:22 */
#define REG_DA_EQ_CODE3  0x7C000000 /* mask bit 30:26 */
#define REG_DA_EQ_CODE_EN3  0x80000000 /* mask bit 31 */

#define REG_D2A3 0x1C08C
#define REG_DA_FIXED_CODE2 0x1F   /* mask bit 4:0 */
#define REG_DA_HDMI14_EQ_SEL2 0x20   /* mask bit 5 */
#define REG_DA_FIXED_CODE1 0x7C0  /* mask bit 10:6 */
#define REG_DA_HDMI14_EQ_SEL1 0x800 /* mask bit 11 */
#define REG_DA_FIXED_CODE0 0x1F000   /* mask bit 16:12 */
#define REG_DA_HDMI14_EQ_SEL0 0x20000   /* mask bit 17 */
#define REG_DA_DIVSEL_MHL2 0xC0000   /* mask bit 19:18 */
#define REG_DA_DIVSEL_MHL_EN2 0x100000  /* mask bit 20 */
#define REG_DA_DIVSEL_MHL1 0x600000  /* mask bit 22:21 */
#define REG_DA_DIVSEL_MHL_EN1 0x800000  /* mask bit 23 */
#define REG_DA_DIVSEL_MHL0 0x3000000 /* mask bit 25:24 */
#define REG_DA_DIVSEL_MHL_EN0 0x4000000 /* mask bit 26 */

#define REG_FRL_AUTO_EQ_CFG0_LANE0 0x1C210
#define CFG_BYPASS_COARSE_TUNE_LANE0  0x1  /* mask bit 0 */
#define CFG_BYPASS_PHASE_LOCK_LANE0   0x2  /* mask bit 1 */
#define CFG_COARSE_TUNE_CONDITIONS_LANE0 0xFC /* mask bit 7:2 */
#define CFG_SWEEP_EQ_CONDITIONS_LANE0 0x3F00  /* mask bit 13:8 */
#define CFG_FRL_EQ_MODE  0x4000  /* mask bit 14 */
#define CFG_EQ_CLK_LOCK_WAIT_SEL  0x8000  /* mask bit 15 */
#define CFG_SYMBOL_TIME_OUT_SEL   0x10000 /* mask bit 16 */

#define REG_AUTOEQ3 0x1C054
#define REG_PHASE_CHECK_DURATION 0xFFFF  /* mask bit 15:0 */
#define REG_SYMBOL_DET_DURATION  0xFFFF0000 /* mask bit 31:16 */

#define REG_AUTOEQ4 0x1C058
#define REG_EQ_UPDATE_DURATION    0xFFFF  /* mask bit 15:0 */
#define REG_EQ_UNDER_OVER_CHECK_DURATION 0xFFFF0000 /* mask bit 31:16 */

#define REG_AUTOEQ0 0x1C048
#define REG_PHASE_STABLE_THRESHOLD  0x3F   /* mask bit 5:0 */
#define REG_MHL_BBCDR_MODE_EN 0x40    /* mask bit 6 */
#define REG_MHL_AUTO_EQ_TRIGGER    0x80	   /* mask bit 7 */
#define REG_MHL_CLOCK_LOCK_SEL 0x100  /* mask bit 8 */
#define REG_FINE_SYMBOL_SEL    0x200  /* mask bit 9 */
#define REG_COARSE_SYMBOL_SEL  0x400  /* mask bit 10 */
#define REG_DP_EN 0x800  /* mask bit 11 */
#define REG_EN_SYMBOL_LOCK_TIMEOUT 0x1000 /* mask bit 12 */
#define REG_SYMBOL_DET_MODE    0x2000 /* mask bit 13 */
#define REG_SYMBOL_TIME_OUT_MODE   0x4000  /* mask bit 14 */
#define REG_HDMI2_ACDR_MODE   0x8000  /* mask bit 15 */
#define REG_BYPASS_SYMBOL_LOCK_DET_COARSE 0x10000 /* mask bit 16 */
#define REG_BYPASS_CR_LOCK_CHK_COARSE  0x20000 /* mask bit 17 */
#define REG_HDMI_FINE_SYMBOL_DET_EN   0x40000 /* mask bit 18 */
#define REG_EN_CK_LOSE_LOCK_TRIGGER   0x80000 /* mask bit 19 */
#define REG_BACKGROUND_EQ_MODE 0x100000   /* mask bit 20 */
#define REG_MHL_AUTO_EQ_EN 0x200000   /* mask bit 21 */
#define REG_AUTO_EQ_RETUNE_EN 0x400000   /* mask bit 22 */
#define REG_EN_AUTO_EQ_SPDUP_BY_OVF   0x800000   /* mask bit 23 */
#define REG_OTP_MUX   0x1000000  /* mask bit 24 */
#define CFG_FRL_AUTO_EQ_EN_LANE0  0x2000000  /* mask bit 25 */
#define CFG_FRL_AUTO_EQ_EN_LANE1  0x4000000  /* mask bit 26 */
#define CFG_FRL_AUTO_EQ_EN_LANE2  0x8000000  /* mask bit 27 */
#define CFG_FRL_AUTO_EQ_EN_LANE3  0x10000000 /* mask bit 28 */

#define REG_SWEEP_EQ_CFG 0x1C154
#define REG_EQ_ERROR_START_DELAY 0xFFFF /* mask bit 15:0 */
#define REG_EQ_ERR_CHECK_MODE 0x10000 /* mask bit 16 */
#define REG_EQ_OFFSET 0x1F00000  /* mask bit 24:20 */
#define REG_EQ_UP_DOWN    0x2000000  /* mask bit 25 */
#define REG_EQ_SWEEP_MODE 0x4000000  /* mask bit 26 */
#define REG_EQ_RANGE_THRESHOLD   0xF8000000 /* mask bit 31:27 */

#define REG_AUTOEQ10 0x1C070
#define REG_EQ_ERROR_STEP    0x3    /* mask bit 1:0 */
#define REG_EQ_ERROR_CHECK_DURATION 0x3FFFC00 /* mask bit 25:10 */
#define REG_HDMI1_DCDR_MODE  0x4000000 /* mask bit 26 */

#define REG_AUTOEQ8 0x1C068
#define REG_EQ_STRENGTH_COARSE_MODE 0x1   /* mask bit 0 */
#define REG_EQ_END_LIMIT 0x3E  /* mask bit 5:1 */
#define REG_EQ_START_LIMIT   0x7C0 /* mask bit 10:6 */
#define REG_EN_BYPASS_LOW_EQ 0x800 /* mask bit 11 */
#define REG_START_EQ_STRENGTH    0x3F000  /* mask bit 17:12 */
#define REG_EQ_STRENGTH_STEP 0x1C0000 /* mask bit 20:18 */

#define REG_FRL_AUTO_EQ_CFG0_LANE1 0x1C218
#define CFG_BYPASS_COARSE_TUNE_LANE1  0x1 /* mask bit 0 */
#define CFG_BYPASS_PHASE_LOCK_LANE1   0x2 /* mask bit 1 */
#define CFG_COARSE_TUNE_CONDITIONS_LANE1 0xFC   /* mask bit 7:2 */
#define CFG_SWEEP_EQ_CONDITIONS_LANE1 0x3F00 /* mask bit 13:8 */

#define REG_FRL_AUTO_EQ_CFG0_LANE2 0x1C220
#define CFG_BYPASS_COARSE_TUNE_LANE2 0x1 /* mask bit 0 */
#define CFG_BYPASS_PHASE_LOCK_LANE2   0x2 /* mask bit 1 */
#define CFG_COARSE_TUNE_CONDITIONS_LANE2 0xFC   /* mask bit 7:2 */
#define CFG_SWEEP_EQ_CONDITIONS_LANE2 0x3F00 /* mask bit 13:8 */

#define REG_FRL_AUTO_EQ_CFG0_LANE3 0x1C228
#define CFG_BYPASS_COARSE_TUNE_LANE3  0x1 /* mask bit 0 */
#define CFG_BYPASS_PHASE_LOCK_LANE3   0x2 /* mask bit 1 */
#define CFG_COARSE_TUNE_CONDITIONS_LANE3 0xFC   /* mask bit 7:2 */
#define CFG_SWEEP_EQ_CONDITIONS_LANE3 0x3F00 /* mask bit 13:8 */

#define REG_SHADOW_CED_CLR 0x1B070
#define CFG_CNT_SW_CLR 0x1 /* mask bit 0 */

#define REG_DFE_CFG1_LANE0 0x1C25C
#define CFG_DTAP_THRESHOLD_LANE0   0xFFF  /* mask bit 11:0 */
#define CFG_TTAP_THRESHOLD_LANE0   0xFFF000   /* mask bit 23:12 */
#define CFG_DFE_STABLE_THRESHOD_LANE0 0x3F000000 /* mask bit 29:24 */

#define REG_DFE_CFG2_LANE0 0x1C260
#define CFG_DFE_WAIT_CNT_LANE0 0x3FF   /* mask bit 9:0 */
#define CFG_DFE_TIMEOUT_CNT_LANE0 0xFFC00 /* mask bit 19:10 */

#define REG_DFE_CFG1_LANE1 0x1C280
#define CFG_DTAP_THRESHOLD_LANE1   0xFFF   /* mask bit 11:0 */
#define CFG_TTAP_THRESHOLD_LANE1   0xFFF000   /* mask bit 23:12 */
#define CFG_DFE_STABLE_THRESHOD_LANE1 0x3F000000 /* mask bit 29:24 */

#define REG_DFE_CFG2_LANE1 0x1C284
#define CFG_DFE_WAIT_CNT_LANE1 0x3FF   /* mask bit 9:0 */
#define CFG_DFE_TIMEOUT_CNT_LANE1 0xFFC00 /* mask bit 19:10 */

#define REG_DFE_CFG1_LANE2 0x1C2A4
#define CFG_DTAP_THRESHOLD_LANE2   0xFFF   /* mask bit 11:0 */
#define CFG_TTAP_THRESHOLD_LANE2   0xFFF000   /* mask bit 23:12 */
#define CFG_DFE_STABLE_THRESHOD_LANE2 0x3F000000 /* mask bit 29:24 */

#define REG_DFE_CFG2_LANE2 0x1C2A8
#define CFG_DFE_WAIT_CNT_LANE2 0x3FF   /* mask bit 9:0 */
#define CFG_DFE_TIMEOUT_CNT_LANE2 0xFFC00 /* mask bit 19:10 */

#define REG_DFE_CFG1_LANE3 0x1C2C8
#define CFG_DTAP_THRESHOLD_LANE3  0xFFF   /* mask bit 11:0 */
#define CFG_TTAP_THRESHOLD_LANE3  0xFFF000   /* mask bit 23:12 */
#define CFG_DFE_STABLE_THRESHOD_LANE3 0x3F000000 /* mask bit 29:24 */

#define REG_DFE_CFG2_LANE3 0x1C2CC
#define CFG_DFE_WAIT_CNT_LANE3 0x3FF   /* mask bit 9:0 */
#define CFG_DFE_TIMEOUT_CNT_LANE3 0xFFC00 /* mask bit 19:10 */

#define REG_FIXED_EQ_STATE 0x1C18C
#define FIXED_EQ_STATE 0x7 /* mask bit 2:0 */

#define REG_AUTOEQ1 0x1C04C
#define REG_EQ_FINE_SYMBOL_DET_TIMES   0xFFFF     /* mask bit 15:0 */
#define REG_EQ_COARSE_SYMBOL_DET_TIMES 0xFFFF0000 /* mask bit 31:16 */

#define REG_AUTOEQ7 0x1C064
#define REG_HDMI_PAT_DET_TIMES2 0xFFFF     /* mask bit 15:0 */
#define REG_HDMI_PAT_DET_TIMES1 0xFFFF0000 /* mask bit 31:16 */

#define REG_CLK_DETECT1 0x1C004
#define REG_CLK_JITT_VAL       0xFF       /* mask bit 7:0 */
#define REG_CLK_JITT_SEL       0x100      /* mask bit 8 */
#define REG_CLK_JITT_MODE      0x600      /* mask bit 10:9 */
#define REG_CLK_MEAN_SEL       0x800      /* mask bit 11 */
#define REG_CLK_UNSTABLE_TIMES 0xFF000    /* mask bit 19:12 */
#define REG_CLK_STABLE_TIMES   0xFF00000  /* mask bit 27:20 */
#define REG_DVI_DIV_SEL        0x70000000 /* mask bit 30:28 */

#define REG_PLL_LOCK0 0x1C014
#define REG_PLL_LOCK_TIMES   0x1        /* mask bit 0 */
#define REG_PLL_UNLOCK_SEL   0x2        /* mask bit 1 */
#define REG_PLL_LOCK_SEL     0xC        /* mask bit 3:2 */
#define REG_REF_UNLOCK_TIMER 0xFF0      /* mask bit 11:4 */
#define REG_REF_LOCK_TIMER   0xFF000    /* mask bit 19:12 */
#define REG_FB_PRD_SEL       0x700000   /* mask bit 22:20 */
#define REG_FB_DIV_SEL       0x3800000  /* mask bit 25:23 */
#define REG_PLL_LOCKEXT_SEL  0x1C000000 /* mask bit 28:26 */

#define REG_CHANNEL_FULL_CLR 0x1B048
#define CFG_CHANNEL_FULL_CLR 0x1 /* mask bit 0 */

#define REG_CH_DESKEW_STATE 0x1B050
#define DESKEW_LOCK      0x1  /* mask bit 0 */
#define DESKEW_FIFO_FULL 0x1E /* mask bit 4:1 */

#define REG_RX_PWD_SEL 0x28
#define CFG_FRL_MODE_EN                   0x1      /* mask bit 0 */
#define CFG_FRL_DYN_CKGT_EN               0x2      /* mask bit 1 */
#define CFG_FRL_4LN_EN                    0x4      /* mask bit 2 */
#define CFG_SCDC_FRL_EN                   0x8      /* mask bit 3 */
#define CFG_SCDC_CED_CLR_SEL              0x10     /* mask bit 4 */
#define CKSEL_AUD_MCLK                    0x100    /* mask bit 8 */
#define CFG_HDCP_MODE_SW_EN               0x1000   /* mask bit 12 */
#define CFG_HDCP_MODE_SW_VAL              0x2000   /* mask bit 13 */
#define CFG_SYS_MUTE_SW_EN                0x4000   /* mask bit 14 */
#define CFG_SYS_MUTE_SW_VAL               0x8000   /* mask bit 15 */
#define CFG_DPHY_CHANNEL_FIFO_RST_LANE_EN 0x10000  /* mask bit 16 */
#define CFG_DMIX_PLL_UNLOCK_RST_DPHY_EN   0x20000  /* mask bit 17 */
#define CFG_FRL_RATE_SW_EN                0x40000  /* mask bit 18 */
#define CFG_FRL_RATE_SW_VAL               0x780000 /* mask bit 22:19 */
// aon
#define REG_DDC_FILT_CTL 0x10A8
#define DDC_FILT_SEL 0x3 /* mask bit 1:0 */

#define REG_DDC_DLY_CTL 0x10AC
#define SDA_IN_DLY_CNT  0xF  /* mask bit 3:0 */
#define SDA_OUT_DLY_CNT 0xF0 /* mask bit 7:4 */
#define SCL_IN_DLY_CNT 0xF00 /* mask bit 11:8 */
#define CFG_KEEP_OUT_CNT 0xF000 /* mask bit 15:12 */
#define CFG_DDC_DLY_MODE 0xF0000 /* mask bit 19:16 */

#define REG_DDC_TOUT_RES 0x10B4
#define DDC_TOUT_RESHOLD 0xFF /* mask bit 7:0 */

#define REG_DDC_STRETCH_CTL 0x10B8
#define SCL_HOLD_RESH 0xFF /* mask bit 7:0 */

#endif