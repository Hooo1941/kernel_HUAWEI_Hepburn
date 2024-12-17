/*
 * pcie-kport-phy-kphy.c
 *
 * kport PCIePHY Driver
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
 */

#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <linux/of_address.h>
#include <linux/delay.h>

#include <securec.h>
#ifdef CONFIG_PCIE_PHY_DIGITAL_GATHER
#include <linux/slab.h>
#endif

#include "pcie-kport-phy.h"
#include "pcie-kport-phy-kphy.h"

static u32 pcie_phy_init_tbl[][2] =
{
	{PHY_PLL_REFCLK_SEL_VAL, PHY_PLL_REFCLK_SEL_ADDR},           /* PLL ref clk select */
	{PHY_REG_DA_DFE_CTRL_EN_VAL, PHY_REG_DA_DFE_CTRL_EN_ADDR},   /* dfe_en = 0 */
	{PHY_REG_DA_DFE_CTRL_VAL_VAL, PHY_REG_DA_DFE_CTRL_VAL_ADDR},
	{PHY_RG_BG_VREF_SEL_VAL, PHY_RG_BG_VREF_SEL_ADDR},           /* bg mt en = 1 */
	{PHY_PLL_UPDATE_CTRL_VAL, PHY_PLL_UPDATE_CTRL_ADDR},         /* bias_stben/ldo_stben */
	{PHY_REG_DA_TX_EQ_CTRL_VAL_VAL, PHY_REG_DA_TX_EQ_CTRL_ADDR}, /* close tx ffe */
	{PHY_REG_DA_TX_EQ_FFE_VAL, PHY_REG_DA_TX_EQ_FFE},
	{PHY_RXUPDTCTRL_VAL, PHY_RXUPDTCTRL_EN_ADDR},                /* gs eq en */
	{PHY_DFE_MANUAL_EN_VAL, PHY_DFE_MANUAL_EN_ADDR},             /* sel_r and sel_gain manual controls */
	{PHY_RG_RX_CFG_ADDED4_VAL, PHY_RG_RX_CFG_ADDED4_ADDR},
	{PHY_DFE_EN_VAL, PHY_DFE_EN_ADDR},                           /* dfe tap3/2/1_en = 0 */
	{PHY_PMA_RX_ATT_RG_RX0_VAL, PHY_PMA_RX_ATT_RG_RX0_ADDR},     /* PMA RX ATT GEN1-GEN3     PMA RG_RX0_EQ_R GEN1-GEN3 */
	{PHY_RX_EQA_CTRL_SEL_C_VAL, PHY_RX_EQA_CTRL_ADDR},           /* rxctrl eqa sel c */
	{PHY_RX_EQA_TH_VAL, PHY_RX_EQA_CTRL_ADDR},                   /* rxctrl eqa th */
	{PHY_RG_RX0_GAIN_SEL_VAL, PHY_RG_RX0_GAIN_SEL_ADDR},         /* reg RX0 GAIN select */
	{PHY_RG_G1_KI_FINE_VAL, PHY_RG_G1_KI_FINE_ADDR},             /* GEN1 KI fine */
	{PHY_RG_G2_G3_KI_FINE_VAL, PHY_RG_G2_G3_KI_FINE_ADDR},       /* GEN2 GEN3 KI fine */
	{PHY_RG_RX0_LOS_HYS_VAL, PHY_RG_RX0_LOS_HYS_ADDR},           /* reg RX0 LOS HYS */
	{PHY_RG_RX0_GAIN_VREF_VAL, PHY_RG_RX0_GAIN_VREF_ADDR},       /* reg RX0 GAIN VREF */
	{PHY_RG_RX_CTRL_GSA_TH_VAL, PHY_RG_RX0_GAIN_VREF_ADDR},      /* rxctrl gsa th */
	{PHY_RG_RX0_EQ_PAT_MODE_CFG_VAL, PHY_RG_RX0_EQ_PAT_MODE_CFG_ADDR}, /* EQ pattern and EQ CFG */
	{PHY_DCC_WAITING_TIME_VAL, PHY_DCC_WAITING_TIME_ADDR},       /* DCC waiting time */
	{PHY_RXOFSTCTRL_VAL, PHY_RXOFSTCTRL_ADDR},
	{PHY_TX_TERMK_WATING_TIME_VAL, PHY_TX_TERMK_WATING_TIME_ADDR}, /* TX TERMK waiting time */
	{PHY_TX_TERMK_WATING_TIME1_2_VAL, PHY_TX_TERMK_WATING_TIME1_2_ADDR},
#ifdef CONFIG_PCIE_KPORT_JUN
	{PHY_TX_DRV_TRANSMIT_TIME_VAL,PHY_TX_DRV_TRANSMIT_TIME_ADDR},
#else
	{PHY_PLL_LOCK_WAIT_TIME_VAL, PHY_PLL_LOCK_WAIT_TIME_ADDR},  /* pll lock keep time */
	{PHY_PMA_CTRL_OTHER_TMR0_VAL, PHY_PMA_CTRL_OTHER_TMR0_ADDR},
#endif
	{PHY_TERMKCTRL0_VAL, PHY_TERMKCTRL0_ADDR},
	{PHY_PHY_TXEQVBST_VAL, PHY_TXEQVBST_ADDR},
};

static u32 pcie_phy_init_tbl_v2[][2] = {
	{0x10000,	 0x0AC8},
	{0x800400ff, 0x0B0C},
	{0x0,		 0x0B78},
	{0x1000,	 0x1540},
	{0x22201ff,	 0x1A1C},
	{0x4440000,	 0x1A18},
	{0x20180010, 0x1534},
	{0x410018,	 0x1A14},
	{0x40410439, 0x1580},
	{0x557,		 0x1588},
	{0x26100030, 0x1530},
	{0x25,		 0x1A10},
	{0x30a354,	 0x1A04},
	{0xee001b7f, 0x152C},
	{0x11295,	 0x1A20},
	{0x25A09104, 0x1564},
	{0x10053,	 0x1548},
	{0x2010,	 0x1C04},
	{0x4e150,	 0x15A0},
	{0xe66177,	 0x0A70},
	{0x136e1,	 0x0A84},
	{0x26200780, 0x0A40},
	{0xbb,		 0x0A9C},
	{0x6,		 0x1B10},
	{0xf,		 0x1B18},
	{0x0,		 0x1B54},
	{0x600,		 0x1B5C},
};

static inline void kphy_phy_write(struct pcie_phy *phy, uint32_t val, uint32_t reg)
{
	writel(val, phy->phy_base + reg);
}

static inline uint32_t kphy_phy_read(struct pcie_phy *phy, uint32_t reg)
{
	return readl(phy->phy_base + reg);
}

static inline void apb_phy_write(struct pcie_phy *phy, uint32_t val, uint32_t reg)
{
	writel(val, phy->apb_phy_base + reg);
}

static inline uint32_t apb_phy_read(struct pcie_phy *phy, uint32_t reg)
{
	return readl(phy->apb_phy_base + reg);
}

static inline void kphy_phy_core_write(struct pcie_phy *phy, uint32_t val, uint32_t reg)
{
	writel(val, phy->core_base + reg);
}

static inline uint32_t kphy_phy_core_read(struct pcie_phy *phy, uint32_t reg)
{
	return readl(phy->core_base + reg);
}

static void kphy_phy_w(struct pcie_phy *phy, uint32_t reg, uint32_t msb, uint32_t lsb, uint32_t val)
{
	uint32_t data = kphy_phy_read(phy, reg);

	data = DRV_32BIT_SET_FIELD(data, lsb, msb, val);
	kphy_phy_write(phy, data, reg);
}

static uint32_t kphy_phy_r(struct pcie_phy *phy, uint32_t reg, uint32_t msb, uint32_t lsb)
{
	uint32_t data = kphy_phy_read(phy, reg);

	return DRV_32BIT_READ_FIELD(data, lsb, msb);
}

static void collect_phy3_info(struct pcie_phy *phy)
{
	uint32_t reg_val;

	reg_val = kphy_phy_read(phy, PHY_DCC_INTR_FLAG);
	D("phy3: dcc overflow/underflow intr flag: 0x%x %d\n", reg_val,
			(reg_val & PHY_DCC_INTR_FLAG_MASK) >> PHY_DCC_INTR_FLAG_OFFSET);

	reg_val = kphy_phy_read(phy, PHY_TX_RX_TERMK_INTR_FLAG);
	D("phy3: tx termk overflow/underflow intr: 0x%x %d\n", reg_val,
			(reg_val & PHY_TX_TERMK_INTR_FLAG_MASK) >> PHY_TX_TERMK_INTR_FLAG_OFFSET);
	D("phy3: rx termk overflow/underflow intr: 0x%x %d\n", reg_val,
			(reg_val & PHY_RX_TERMK_INTR_FLAG_MASK) >> PHY_RX_TERMK_INTR_FLAG_OFFSET);

	reg_val = kphy_phy_read(phy, PHY_PLL_GEN12_VCO_BAND);
	D("phy3: vco band value in GEN1/2: 0x%x\n", reg_val);

	reg_val = kphy_phy_read(phy, PHY_PLL_GEN3_VCO_BAND);
	D("phy3: vco band value in GEN3: 0x%x\n", reg_val);

	reg_val = kphy_phy_read(phy, PHY_DCC_CALI_RES);
	D("phy3: dcc calibration result: 0x%x\n", reg_val);

	reg_val = kphy_phy_read(phy, PHY_TX_TERMK_RES);
	D("phy3: tx termk result: 0x%x\n", reg_val);

	reg_val = kphy_phy_read(phy, PHY_RX_TERMK_RES);
	D("phy3: rx termk result: 0x%x\n", reg_val);
}

#ifdef CONFIG_PCIE_PHY_DIGITAL_GATHER
static void phy3_digital_gather_init(struct pcie_phy *phy)
{
	E("+phy3_digital_gather_init+\n");
	kphy_phy_w(phy,0x900,2,2,0x0);
	kphy_phy_w(phy,0x904,31,0,0x0);
	kphy_phy_w(phy,0x9A4,31,16,0x0);
	kphy_phy_w(phy,0x90C,31,0,0x0);
	kphy_phy_w(phy,0x9A8,31,16,0x0);
	kphy_phy_w(phy,0x908,31,0,0x0);
	kphy_phy_w(phy,0x9A4,14,0,0x0);
	kphy_phy_w(phy,0x910,31,0,0xFFFFFFFF);
	kphy_phy_w(phy,0x9A8,15,0,0x7FFF);
	kphy_phy_w(phy,0x900,11,8,0x0);
	if (phy->version == PCIE_PHY_VERSION_1) {
		kphy_phy_w(phy, 0x900, 1, 1, 0x1);
		kphy_phy_w(phy, 0x918, 31, 0, 0xFC0);
		kphy_phy_w(phy, 0x91C, 31, 0, 0x40);
		kphy_phy_w(phy, 0x920, 31, 0, 0xF800000);
		kphy_phy_w(phy, 0x924, 9, 8, 0x1);
	} else {
		kphy_phy_w(phy, 0x900, 1, 1, 0x0);
		kphy_phy_w(phy, 0x918, 31, 0, 0x0);
		kphy_phy_w(phy, 0x91C, 31, 0, 0);
		kphy_phy_w(phy, 0x920, 31, 0, 0x1ffff);
	}
	kphy_phy_w(phy,0x900,7,4,0x0);
	E("-phy3_digital_gather_init-\n");
}

static void phy3_digital_gather_trigger(struct pcie_phy *phy)
{
	E("+kphy_phy_dfx_trigger+\n");
	kphy_phy_w(phy,0x900,0,0,0x1);
	kphy_phy_w(phy,0x604,2,2,0x0);
	kphy_phy_w(phy,0x604,2,2,0x1);
	E("-kphy_phy_dfx_trigger\n");
}

#define PCS_LOG_LINE_MAX 512
#define PCIE_PCS_TRACER_LOGSIZE 8192
#define PCIE_PCS_FIFO_SIZE 128
static void phy3_digital_gather_print(char *str)
{
	char c;
	int32_t pos;

	int32_t size = PCS_LOG_LINE_MAX;
	char *src = str;
	char dst[PCS_LOG_LINE_MAX];
	dst[0] = '\0';

	pos = 0;
	for (;;) {
		c = *src;
		if (c == '\0') {
			*(dst + pos) = '\0';
			E("%s", dst);
			break;
		}
		*(dst + pos) = c;
		src++;
		pos++;
		if ((pos >= (size - 1)) || (c == '\n')) {
			*(dst + pos) = '\0';
			pos = 0; // in a line
			E("%s", dst);
			continue;
	}
	}
}

static void phy3_digital_gather_dump(struct pcie_phy *phy)
{
	uint32_t i, st0, st1, st2, st3, addr;
	int32_t  offset, ret, size;
	void *buff = NULL;

	kphy_phy_w(phy, 0x900, 0, 0, 0x0);
	addr = kphy_phy_read(phy, 0x914);
	addr &= 0x80;
	if(addr & 0x80)
		addr = (addr + 1) % PCIE_PCS_FIFO_SIZE;
	else
		addr = 0;

	E("0x%-8x: %8x\n", 0x1000, kphy_phy_read(phy, 0x1000));
	E("0x%-8x: %8x\n", 0x1010, kphy_phy_read(phy, 0x1010));
	E("0x%-8x: %8x\n", 0x1400, kphy_phy_read(phy, 0x1400));
	E("phy dfx count:%d \n", addr);

	offset = 0;
	size = PCIE_PCS_TRACER_LOGSIZE;
	buff = kmalloc(PCIE_PCS_TRACER_LOGSIZE, GFP_KERNEL);
	if(!buff) {
		E("kmalloc fail!");
		return;
	}

	for (i = addr; i < PCIE_PCS_FIFO_SIZE; i++) {
		kphy_phy_w(phy, 0x0924, 6, 0, i);
		st0 = kphy_phy_r(phy, 0x928, 32, 0);
		st1 = kphy_phy_r(phy, 0x92c, 32, 0);
		st2 = kphy_phy_r(phy, 0x930, 32, 0);
		st3 = kphy_phy_r(phy, 0x914, 10, 9);
		ret = snprintf_s((char *)(buff + offset), size - offset, size - offset - 1,
			"trace_%03u[97:0],98'h%x%08x%08x%08x\n", i, st3, st2, st1, st0);
		if (ret < 0) {
			E("mem buff overrun, size=%d", size);
			kfree(buff);
			return;
		}
		offset += ret;
	}

	phy3_digital_gather_print((char*)buff);
	kfree(buff);
}
#endif
static void kphy_phy_init_v1(struct pcie_phy *phy)
{
	uint32_t i;
	size_t array_size;

	I("+%s+\n", __func__);

	array_size = ARRAY_SIZE(pcie_phy_init_tbl);
	for (i = 0; i < array_size; i++)
		kphy_phy_write(phy, pcie_phy_init_tbl[i][0], pcie_phy_init_tbl[i][1]);

	I("-%s-\n", __func__);
}

static void kphy_phy_init_v2(struct pcie_phy *phy)
{
	uint32_t i;
	size_t array_size;

	I("+%s+\n", __func__);

	array_size = ARRAY_SIZE(pcie_phy_init_tbl_v2);
	for (i = 0; i < array_size; i++)
		kphy_phy_write(phy, pcie_phy_init_tbl_v2[i][0], pcie_phy_init_tbl_v2[i][1]);

	I("-%s-\n", __func__);
}

static void kphy_phy_init(struct pcie_phy *phy)
{
	if (phy->version == PCIE_PHY_VERSION_1)
		kphy_phy_init_v1(phy);
	else
		kphy_phy_init_v2(phy);
}

static bool kphy_phy_ready(struct pcie_phy *phy)
{
	uint32_t reg_val, lock_res;
	int timeout = 0;

	while(1) {
		reg_val = kphy_phy_read(phy, PHY_PCS_POWER_STATUS_ADDR);
		reg_val &= ~PHY_PCS_STATUS_MASK;
		if ((reg_val >> PHY_POWERON_STATUS_BIT) & 1) {
			D("PHY3 power on in %d us\n", timeout);

			lock_res = kphy_phy_read(phy, PHY_DIGITAL_PLL_LOCK_RES);
			D("phy3: digital pll lock result: 0x%x\n", lock_res);
			return true;
		}

		udelay(5);
		timeout++;
		if (timeout > PHY_POWERON_TIMEOUT) {
			E("PHY3 power on timeout!\n");
			return false;
		}
	}

	return true;
}

static void kphy_phy_ffe_cfg(struct pcie_phy *phy)
{
	uint32_t reg_tx_rtermk;

	udelay(350);
	reg_tx_rtermk = kphy_phy_read(phy, PHY_RG_TX_RTERMK_V48_ADDR);
	reg_tx_rtermk = (reg_tx_rtermk >> PHY_RG_TX_RTERMK_V48_BIT) & 1;
	if (reg_tx_rtermk) {
		kphy_phy_write(phy, PHY_TXDEEMPHCTRL6DB_VAL, PHY_TXDEEMPHCTRL6DB_ADDR);
		kphy_phy_write(phy, PHY_TXDEEMPHCTRL3D5DB_VAL, PHY_TXDEEMPHCTRL3D5DB_ADDR);
		kphy_phy_write(phy, PHY_TXDEEMPHCTRL0DB_VAL, PHY_TXDEEMPHCTRL0DB_ADDR);
		if (phy->version == PCIE_PHY_VERSION_1)
			kphy_phy_write(phy, PHY_RG_PIPE_TXDEEMPH_CTRL_VAL, PHY_RG_PIPE_TXDEEMPH_CTRL_ADDR);
		/* config FS, LF */
		kphy_phy_write(phy, PHY_RG_LOCAL_LF_FS_VAL, PHY_RG_LOCAL_LF_FS_ADDR);
	} else {
		/* config FS, LF */
		kphy_phy_write(phy, PHY_RG_LOCAL_LF_FS_V48_VAL, PHY_RG_LOCAL_LF_FS_ADDR);
	}

	collect_phy3_info(phy);

	phy->rtermk_v48 = reg_tx_rtermk;
}

#ifdef CONFIG_PCIE_KPORT_TEST
static int phy3_eye_monitor_para(struct pcie_phy *phy, u32 quadrant, int vref, int pi)
{
	u32 reg_val;
	int timeout = PHY3_FOM_TIMEOUT;
	int loop_cnt = 0;
	int ch;

	kphy_phy_w(phy, PHY_RXUPDTCTRL_EN_ADDR, 11, 11, 0x0);
	kphy_phy_w(phy, PHY_RXFOMCTRL1_ADDR, 6, 6, 0x0);
	/* quadrant sel */
	kphy_phy_w(phy, PHY_RXFOMCTRL1_ADDR, 7, 0, quadrant);
	/* vref */
	kphy_phy_w(phy, PHY_RXFOMCTRL1_ADDR, 15, 12, vref);
	/* pi_step */
	kphy_phy_w(phy, PHY_RXFOMCTRL1_ADDR, 11, 8, pi);
	/* fom_sw_load */
	kphy_phy_w(phy, PHY_RXFOMCTRL0_ADDR, 1, 1, 0x1);
	udelay(1);
	/* fom en */
	kphy_phy_w(phy, PHY_RXFOMCTRL0_ADDR, 2, 2, 0x1);
	/* rx_rxg en */
	kphy_phy_w(phy, PHY_RXFOMCTRL0_ADDR, 5, 5, 0x1);
	/* PMA_CLK_CG_EN */
	kphy_phy_w(phy, PHY_CRG_PER_LANE_ADDR, 13, 13, 0x1);

	udelay(1);
	kphy_phy_w(phy, PHY_RXFOMCTRL0_ADDR, 0, 0, 0x1);
	udelay(2);

	/* wait from cpl intr. */
	reg_val = kphy_phy_r(phy, PHY_PCLK_LANE_INT_SRC_REG_ADDR, 18, 18);
	while (reg_val != 0x1) {
		reg_val = kphy_phy_r(phy, PHY_PCLK_LANE_INT_SRC_REG_ADDR, 18, 18);
		loop_cnt++;
		udelay(5);
		if (loop_cnt > timeout) {
			E("Timeout to wait fom cpl intr\n");
			return 'x';
		}
	}
	reg_val = kphy_phy_r(phy, PHY_RXOFSTSTATUS_ADDR, 15, 8);
	/* fom en = 0 */
	kphy_phy_w(phy, PHY_RXFOMCTRL0_ADDR, 2, 0, 0);
	kphy_phy_w(phy, PHY_RXUPDTCTRL_EN_ADDR, 5, 5, 0);

	if (reg_val == FOM_COUNT_MAX)
		ch = ' ';
	else if (reg_val == 0)
		ch = '-';
	else
		ch = '+';

	return ch;
}

u32 g_quadrant[4] = {QUADRANT1, QUADRANT4, QUADRANT2, QUADRANT3};
int g_dir[][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
char g_res[GRAPH_LEN][GRAPH_LEN + 1];
char g_eye_title[] = "FEDCBA98765432100123456789ABCDEF";
static int kphy_phy_eye_monitor(struct pcie_phy *phy)
{
	int i, j, pi_step, vref, ch;
	char out[IMAGE_WIDTH];

	if (memset_s(g_res, sizeof(g_res), 'x', sizeof(g_res)))
		return -1;

	for (i = 0; i < 4; i++) {
		for (vref = 0; vref < QUA_LEN; vref++) {
			for (pi_step = 0; pi_step < QUA_LEN; pi_step++) {
				ch = phy3_eye_monitor_para(phy, g_quadrant[i], vref, pi_step);
				g_res[QUA_LEN + (vref * g_dir[i][1])][QUA_LEN + (pi_step * g_dir[i][0])] = ch;
			}
		}
	}

	if (memcpy_s(g_res[0], sizeof(g_res), g_eye_title, sizeof(g_eye_title)))
		return -1;

	for (i = 0; i < GRAPH_LEN; i++) {
		g_res[i][GRAPH_LEN - 1] = 0;
		g_res[i][0] = g_eye_title[i];
		if (memset_s(out, sizeof(out), ' ', sizeof(out)))
			return -1;

		for (j = 0; j < GRAPH_LEN; j++)
			out[2 * j] = g_res[i][j];
		out[IMAGE_WIDTH - 1] = 0;
		I("%s", out);
	}

	return 0;
}

static void kphy_check_cali_result(struct pcie_phy *phy)
{
	uint32_t i;
	size_t cali_nums;
	const char *cali_res_name_tbl[] = {"DA_EQ_OFFSET", "DA_SA0_OFFSET", "DA_SA1_OFFSET", "DA_SA2_OFFSET",
		"DA_SA3_OFFSET", "DA_GS0_OFFSET", "DA_GS1_OFFSET", "RX_ATT", "SEL_R", "SEL_GAIN", "SEL_C", "DFE_EQ_TAP1",
		"DFE_EQ_TAP2", "DFE_EQ_TAP3"};
	const uint32_t cali_res_cal_tbl[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
		0xA, 0xB, 0x18, 0x19, 0x1A};

	cali_nums = ARRAY_SIZE(cali_res_name_tbl);
	for (i = 0; i < cali_nums; i++) {
		kphy_phy_w(phy, PHY_DBG_INFO_SEL_ADDR, 6, 2, cali_res_cal_tbl[i]);
		I("RC[%u]  %s bit[6:2]: 0x%x", phy->id, cali_res_name_tbl[i], kphy_phy_r(phy, PHY_DBG_STS_REG_ADDR, 20, 16));
	}
}


static int kphy_phy_debug_info_v1(struct pcie_phy *phy)
{
	I("RC[%u]  BGR_trimming bit[3:0]: 0x%x", phy->id, kphy_phy_r(phy, PHY_RG_BG_VREF_SEL_ADDR, 3, 0));

	I("RC[%u]  ana_ad_pll_lock bit0: 0x%x", phy->id, kphy_phy_r(phy, PHY_AD_PLL_LOCK_ADDR, 0, 0));

	I("RC[%u]  ana_dig_pll_lock bit2: 0x%x", phy->id, kphy_phy_r(phy, PHY_AD_PLL_LOCK_ADDR, 2, 2));

	I("RC[%u]  ad_pll_lock bit2: 0x%x", phy->id, kphy_phy_r(phy, PHY_AD_PLL_LOCK_STATUS_ADDR, 8, 8));

	I("RC[%u]  pll lock det dbg addr bit[31:0]: 0x%x", phy->id, kphy_phy_r(phy, PHY_PLL_LOCK_DET_DBG, 31, 0));

	I("RC[%u]  pll_lock_overtime bit3: 0x%x", phy->id, kphy_phy_r(phy, PHY_AD_PLL_LOCK_ADDR, 3, 3));

	I("RC[%u]  power status bit[3:0]: 0x%x",
			phy->id, kphy_phy_r(phy, PHY_PCS_POWER_STATUS_ADDR, 3, 0));
	I("RC[%u]  rate status bit[5:4]: 0x%x", phy->id, kphy_phy_r(phy, PHY_PCS_POWER_STATUS_ADDR, 5, 4));

	kphy_phy_w(phy, PHY_REG_PV_DEBUG_CFG_ADDR, 3, 0, 0x7);

	I("RC[%u]  pv_out: 0x%x", phy->id, kphy_phy_r(phy, PHY_ADC_CLK_DIV_ADDR, 15, 8));

	I("RC[%u]  vco_band_gen2: 0x%x", phy->id, kphy_phy_r(phy, PHY_PLL_BANDCTRLR1_ADDR, 9, 0));

	I("RC[%u]  vco_band_gen3: 0x%x", phy->id, kphy_phy_r(phy, PHY_PLL_BANDCTRLR2_ADDR, 9, 0));

	kphy_check_cali_result(phy);

	return 0;
}

static void kphy_check_cali_result_v2(struct pcie_phy *phy)
{
	uint32_t i;
	size_t cali_nums;
	const char *cali_res_name_tbl[] = {"DA_EQ_OFFSET", "DA_SA0_OFFSET", "DA_SA1_OFFSET", "DA_SA2_OFFSET",
		"DA_SA3_OFFSET", "DA_GS0_OFFSET", "DA_GS1_OFFSET", "RX_ATT", "SEL_R", "SEL_GAIN", "SEL_C", 
		"CTLE2_SEL_R", "CTLE2_SEL_C", "DFE_EQ_TAP1", "DFE_EQ_TAP2", "DFE_EQ_TAP3"};
	const uint32_t cali_res_cal_tbl[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
		0xA, 0xB, 0x1C, 0x1D, 0x18, 0x19, 0x1A};

	cali_nums = ARRAY_SIZE(cali_res_name_tbl);
	for (i = 0; i < cali_nums; i++) {
		kphy_phy_w(phy, PHY_DBG_INFO_SEL_ADDR, 6, 2, cali_res_cal_tbl[i]);
		I("RC[%u]  %s bit[6:2]: 0x%x", phy->id, cali_res_name_tbl[i], kphy_phy_r(phy, PHY_DBG_STS_REG_ADDR, 20, 16));
	}
}

static int kphy_phy_debug_info_v2(struct pcie_phy *phy)
{
	int i;
	u32 val;
	static struct {
		u32 reg;
		u32 data;
		char *desc;
		u32 msb;
		u32 lsb;
	} kphy_info[] = {
		{0x0AC8, 0x0 , "reg_bg_vref_sel               ", 3  , 0 },
		{0x0B70, 0x0 , "reg_pll_ad_pll_lock           ", 0  , 0 },
		{0x0B70, 0x0 , "reg_pll_dig_pll_lock          ", 2  , 2 },
		{0x09B0, 0x7 , "rg_pv_en rg_count_rstn rg_pv_div", 3  , 0 },
		{0x0E00, 0x0 , "reg_ad_pv_dout_debounce       ", 15 , 8 },
		{0x0B30, 0x0 , "reg_pll_kband_r1              ", 9  , 0 },
		{0x0B38, 0x0 , "reg_pll_kband_r2              ", 9  , 0 },
		{0x0B74, 0x0 , "reg_kband_cplt_synzd          ", 0  , 0 },
		{0x0F88, 0x0 , "reg_da_pll_kband_ctrl_sts     ", 17 , 8 },
		{0x0A00, 0x0 , "pll_lock_status               ", 8  , 8 },
		{0x154C, 0x0 , "tx_termp                      ", 18 , 16},
		{0x154C, 0x0 , "tx_termn                      ", 14 , 12},
		{0x154C, 0x0 , "rx_term                       ", 4  , 0 },
		{0x1400, 0x0 , "rate_status                   ", 5  , 4 },
		{0x1B84, 0x0 , "reg_pipe_txdeemph_ctrl_sts    ", 24 , 7 },
		{0x1400, 0x0 , "rate_status                   ", 5  , 4 },
		{0x1400, 0x0 , "power_status                  ", 3  , 0 },
		{0x158C, 0x1 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x2 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x3 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x4 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x5 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x6 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x7 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x8 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x9 , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0xa , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0xb , "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x18, "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x19, "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x158C, 0x1a, "reg_rx_dig_info_sel           ", 6  , 2 },
		{0x1594, 0x0 , "ad_rx_dig_info                ", 20 , 16},
		{0x1A18, 0x0 , "reg_rx_sel_ctle2_r_g1         ", 19 , 16},
		{0x1A18, 0x0 , "reg_rx_sel_ctle2_r_g2         ", 23 , 20},
		{0x1A18, 0x0 , "reg_rx_sel_ctle2_r_g3         ", 27 , 24},
		{0x1A14, 0x0 , "rg_rx_sel_ctle2_c             ", 4  , 0 },
	};

	for (i = 0; i < ARRAY_SIZE(kphy_info); i++) {
		if (kphy_info[i].data == 0) {
			val = kphy_phy_r(phy, kphy_info[i].reg, kphy_info[i].msb, kphy_info[i].lsb);
			E("[%s] reg: 0x%x, bit [%u:%u]:0x%x", kphy_info[i].desc,
				kphy_info[i].reg, kphy_info[i].msb, kphy_info[i].lsb, val);
		} else {
			kphy_phy_w(phy, kphy_info[i].reg, kphy_info[i].msb,
					kphy_info[i].lsb, kphy_info[i].data);
		}
	}

	kphy_check_cali_result_v2(phy);

	return 0;
}

static int kphy_phy_debug_info(struct pcie_phy *phy)
{
	if (phy->version == PCIE_PHY_VERSION_1)
		kphy_phy_debug_info_v1(phy);
	else
		kphy_phy_debug_info_v2(phy);

	return 0;
}
#endif

static void kphy_phy_stat_dump(struct pcie_phy *phy)
{
	I("+%s+\n", __func__);

	/* PCLK_COM Interrupt Source register */
	pr_info("PCLK_COM Interrupt Source register\n");
	pr_info("0x%-8x: %8x\n", 0x404, kphy_phy_read(phy, 0x404));
	/* APBCLK_LANE_INT_SRC_REG */
	pr_info("APBCLK_LANE_INT_SRC_REG\n");
	pr_info("0x%-8x: %8x\n", 0x220, kphy_phy_read(phy, 0x220));
	/* PCLK_LANE_INT_SRC_REG */
	pr_info("PCLK_LANE_INT_SRC_REG\n");
	pr_info("0x%-8x: %8x\n", 0x1000, kphy_phy_read(phy, 0x1000));
#ifdef CONFIG_PCIE_KPORT_TEST
		kphy_phy_debug_info(phy);
#endif

	I("+%s+\n", __func__);
}

static struct mcu_irq_info kphy_phy_irq_mcu[MCU_IRQ_MASK_NUM] = {
	{ 0x410, 0x9, }, /* MCU intr mask reg */
	{ 0x10C, 0x1, }, /* TALK TOP */
};

static void init_phy_irq_to_mcu(struct pcie_phy *phy, bool enable)
{
	uint32_t val;
	uint32_t reg;
	int32_t i;

	for (i = 0; i < MCU_IRQ_MASK_NUM; i++) {
		reg = kphy_phy_irq_mcu[i].mask_reg;
		val = kphy_phy_read(phy, reg);
		if (enable)
			val &= ~(kphy_phy_irq_mcu[i].mask_bits);
		else
			val |= kphy_phy_irq_mcu[i].mask_bits;
		kphy_phy_write(phy, val, reg);
	}
}

static void init_phy_irq_to_acore(struct pcie_phy *phy, bool enable)
{
	uint32_t val;

	val = kphy_phy_read(phy, PCS_TOP_MASK_REG);
	if (enable)
		val &= ~PCS_TOP_MASK_BIT;
	else
		val |= PCS_TOP_MASK_BIT;
	kphy_phy_write(phy, val, PCS_TOP_MASK_REG);
}

#ifdef CONFIG_PCIE_KPORT_JUN
/* trigger condition, for pcie-jun phy bug fix.
 * gp int3 cplt = 1, powerdown 4->2
 */
static void kphy_gp3_trigger_init(struct pcie_phy *phy)
{
	kphy_phy_w(phy, PHY_DFX_CRG_ADDR, 2, 2, 0x1);       /* REG Contorl DFX CLK Gating enable */
	kphy_phy_w(phy, PHY_PCLK_COM_INTR_MASK_REG, 3, 3, 0x0); /* enable gp3 intr */
	kphy_phy_w(phy, PHY_GP_INTR3_TRIG_MASK, 31, 0, 0x0);    /* clear gp3 trigger mask */
	kphy_phy_w(phy, PHY_GP_INTR3_TRIG_VAL, 31, 0, 0x0);
	kphy_phy_w(phy, PHY_GP_INTR3_TRIG_MASK, 10, 7, 0xF);
	kphy_phy_w(phy, PHY_GP_INTR3_TRIG_MASK, 27, 24, 0xF);
	kphy_phy_w(phy, PHY_GP_INTR3_TRIG_VAL, 10, 7, 0x2);
	kphy_phy_w(phy, PHY_GP_INTR3_TRIG_VAL, 27, 24, 0x4);
	kphy_phy_w(phy, PHY_GP_INTR23_TRIG_MASK_H, 31, 16, 0x0);
	kphy_phy_w(phy, PHY_GP_INTR23_TRIG_VAL_H, 31, 16, 0x0);
}
#endif

static void kphy_phy_irq_init(struct pcie_phy *phy)
{
	I("%s\n", __func__);

#ifdef XW_PHY_MCU
	I("clear init phy irq done\n");
	init_phy_irq_to_mcu(phy, ENABLE);
#else
	init_phy_irq_to_mcu(phy, DISABLE);
#endif
	I("init phy irq to mcu done\n");
	init_phy_irq_to_acore(phy, DISABLE);

#ifdef CONFIG_PCIE_KPORT_JUN
	kphy_gp3_trigger_init(phy);
#endif
}

static void kphy_phy_irq_disable(struct pcie_phy *phy)
{
	I("%s\n", __func__);
	init_phy_irq_to_mcu(phy, DISABLE);
	init_phy_irq_to_acore(phy, DISABLE);
}

struct pcie_phy_ops kphy_phy_ops = {
	.irq_init = kphy_phy_irq_init,
	.irq_disable = kphy_phy_irq_disable,
	.irq_handler = NULL,
	.phy_init = kphy_phy_init,
	.is_phy_ready = kphy_phy_ready,
	.phy_stat_dump = kphy_phy_stat_dump,
	.phy_r = kphy_phy_r,
	.phy_w = kphy_phy_w,
	.phy_ffe_cfg = kphy_phy_ffe_cfg,
#ifdef CONFIG_PCIE_KPORT_TEST
	.phy3_eye_monitor = kphy_phy_eye_monitor,
	.phy3_debug_info = kphy_phy_debug_info,
#endif
#ifdef CONFIG_PCIE_PHY_DIGITAL_GATHER
	.phy3_digital_gather_init = phy3_digital_gather_init,
	.phy3_digital_gather_trigger = phy3_digital_gather_trigger,
	.phy3_digital_gather_dump = phy3_digital_gather_dump,
#endif
};

/* parse rc-id from pcie_port device_node */
static int32_t kphy_phy_get_host_id(struct platform_device *pdev,
		struct pcie_phy *phy_info)
{
	int32_t ret;

	ret = of_property_read_u32(pdev->dev.of_node, "rc-id", &phy_info->id);
	if (ret) {
		E("Fail to read Host id\n");
		return ret;
	}

	D("Host id: %u\n", phy_info->id);
	return 0;
}

int32_t pcie_phy_register(struct platform_device *pdev)
{
	int32_t ret;
	struct pcie_phy *phy_info = NULL;
	struct device_node *phy_np = NULL;

	I("+%s+\n", __func__);
	phy_info = devm_kzalloc(&pdev->dev, sizeof(*phy_info), GFP_KERNEL);
	if (!phy_info)
		return -ENOMEM;

	ret = kphy_phy_get_host_id(pdev, phy_info);
	if (ret)
		return ret;

	phy_np = of_find_compatible_node(pdev->dev.of_node,
			NULL, "pcie,xw-phy");
	if (!phy_np) {
		E("Get PHY Node\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(phy_np, "version", &phy_info->version);
	if (ret != 0) {
		E("no version property\n");
		phy_info->version = PCIE_PHY_VERSION_1;
	}

	ret = pcie_phy_bind(phy_info->id, phy_info, &kphy_phy_ops);
	if (ret) {
		E("register phy ops\n");
		return ret;
	}

	mutex_init(&phy_info->irq_mutex_lock);

	I("-%s-\n", __func__);
	return 0;
}
