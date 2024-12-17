/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <dpu/soc_dpu_define.h>
#include "dpu_conn_mgr.h"
#include "peri/dkmd_peri.h"
#include "dkmd_mipi_panel_info.h"
#include "mipi_dsi_dev.h"

#define RIGHT_SHIFT_BYTE_NUM 3
#define BIT_NUM_PER_BYTE 8
#define DLEN_MAX 1024
#define PANEL_CHECK_ARRAY_SIZE 1

/* global definition for the cmd queue which will be send after next vactive start */
static spinlock_t g_mipi_trans_lock;

void mipi_transfer_lock_init(void)
{
	static bool g_mipi_trans_lock_inited = false;

	if (!g_mipi_trans_lock_inited) {
		g_mipi_trans_lock_inited = true;
		spin_lock_init(&g_mipi_trans_lock);
	}
}

void mipi_dsi_sread_request(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	if (!cm || !dsi_base) {
		dpu_pr_err("invalid param\n");
		return;
	}

	/* fill up header */
	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));
	hdr |= dsi_hdr_data2(0);

#ifdef MIPI_DSI_HOST_VID_LP_MODE
	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 25, 0);
#else
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 24, 0);
#endif
}

/*
 * mipi dsi short write with 0, 1 2 parameters
 * Write to GEN_HDR 24 bit register the value:
 * 1. 00h, MCS_command[15:8] ,VC[7:6],13h
 * 2. Data1[23:16], MCS_command[15:8] ,VC[7:6],23h
 */
static int32_t mipi_dsi_swrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;
	uint32_t len;

	if ((cm->dlen != 0) && (cm->payload == 0)) {
		dpu_pr_err("NO payload error!\n");
		return 0;
	}

	/* mipi dsi short write with 0, 1 2 parameters, total 3 param */
	if (cm->dlen > 2) {
		dpu_pr_err("cm->dlen is invalid");
		return -EINVAL;
	}
	len = cm->dlen;

	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	if (len == 1) {
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));
		hdr |= dsi_hdr_data2(0);
	} else if (len == 2) {
		hdr |= dsi_hdr_data1((uint32_t)(cm->payload[0]));
		hdr |= dsi_hdr_data2((uint32_t)(cm->payload[1]));
	} else {
		hdr |= dsi_hdr_data1(0);
		hdr |= dsi_hdr_data2(0);
	}

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 25, 0);

	dpu_pr_info("hdr = %#x!\n", hdr);
	return (int32_t)len;  /* 4 bytes */
}

/*
 * mipi dsi long write
 * Write to GEN_PLD_DATA 32 bit register the value:
 * Data3[31:24], Data2[23:16], Data1[15:8], MCS_command[7:0]
 * If need write again to GEN_PLD_DATA 32 bit register the value:
 * Data7[31:24], Data6[23:16], Data5[15:8], Data4[7:0]
 *
 * Write to GEN_HDR 24 bit register the value: WC[23:8] ,VC[7:6],29h
 */
static int32_t mipi_dsi_lwrite(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t pld = 0;

	if ((cm->dlen != 0) && (cm->payload == 0)) {
		dpu_pr_err("NO payload error!\n");
		return 0;
	}

	if (cm->dlen > DLEN_MAX) {
		dpu_pr_err("invalid dlen:%u\n", cm->dlen);
		return 0;
	}

	/* fill up payload, 4bytes set reg, remain 1 byte(8 bits) set reg */
	for (i = 0;  i < cm->dlen; i += 4) {
		if ((i + 4) <= cm->dlen) {
			pld = *((uint32_t *)(cm->payload + i));
		} else {
			for (j = i; j < cm->dlen; j++)
				pld |= (((uint32_t)cm->payload[j] & 0x0ff) << ((j - i) * 8));

			dpu_pr_info("pld = %#x!\n", pld);
		}

		set_reg(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(dsi_base), pld, 32, 0);
		pld = 0;
	}

	/* fill up header */
	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	hdr |= dsi_hdr_wc(cm->dlen);

	/* used for low power cmds trans under video mode */
	hdr |= cm->dtype & GEN_VID_LP_CMD;
	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 25, 0);

	dpu_pr_info("hdr = %#x!\n", hdr);

	return (int32_t)cm->dlen;
}

void mipi_dsi_max_return_packet_size(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	uint32_t hdr = 0;

	if (!cm || !dsi_base) {
		dpu_pr_err("invalid param\n");
		return;
	}

	/* fill up header */
	hdr |= dsi_hdr_dtype(cm->dtype);
	hdr |= dsi_hdr_vc(cm->vc);
	hdr |= dsi_hdr_wc(cm->dlen);

	set_reg(DPU_DSI_APB_WR_LP_HDR_ADDR(dsi_base), hdr, 24, 0);
}

static uint32_t mipi_dsi_read(uint32_t *out, const char __iomem *dsi_base)
{
	uint32_t pkg_status;
	uint32_t try_times = 700;  /* 35ms(50*700) */

	do {
		pkg_status = inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base));
		if ((pkg_status & 0x10) == 0)
			break;
		udelay(50);  /* 50us */
	} while (--try_times);

	*out = inp32(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(dsi_base));
	if (try_times == 0) {
		dpu_pr_err("CMD_PKT_STATUS[%#x], PHY_STATUS[%#x], INT_ST0[%#x], INT_ST1[%#x]\n",
			inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base)),
			inp32(DPU_DSI_CDPHY_STATUS_ADDR(dsi_base)),
			inp32(DPU_DSI_INT_ERROR_FORCE0_ADDR(dsi_base)),
			inp32(DPU_DSI_INT_ERROR_FORCE1_ADDR(dsi_base)));
		dpu_pr_err("PERIP_CHAR_CTRL[%#x], HS_WR_TO_TIME_CTRL[%#x], GEN_HP_HDR[%#x], GEN_HP_PLD_DATA[%#x]\n",
			inp32(DPU_DSI_PERIP_CHAR_CTRL_ADDR(dsi_base)),
			inp32(DPU_DSI_HS_WR_TO_TIME_CTRL_ADDR(dsi_base)),
			inp32(DPU_DSI_GEN_HP_HDR_ADDR(dsi_base)),
			inp32(DPU_DSI_GEN_HP_PLD_DATA_ADDR(dsi_base)));
		dpu_pr_err("HP_CMD_PKT_STATUS[%#x], HP_CMD_FIFO_CTRL[%#x], LP_CMD_FIFO_CTRL[%#x]\n",
			inp32(DPU_DSI_HP_CMD_PKT_STATUS_ADDR(dsi_base)),
			inp32(DPU_DSI_HP_CMD_FIFO_CTRL_ADDR(dsi_base)),
			inp32(DPU_DSI_LP_CMD_FIFO_CTRL_ADDR(dsi_base)));
	}

	return try_times;
}

static int mipi_get_offset_bit_num(int num, bool little_endian_support)
{
	if (little_endian_support)
		return (int)((RIGHT_SHIFT_BYTE_NUM ^ (unsigned int)num) * BIT_NUM_PER_BYTE);
	return num * BIT_NUM_PER_BYTE;
}

static int mipi_dsi_cmds_rx_with_check_fifo(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;
	int err_num = 0;
	uint32_t out[PANEL_CHECK_ARRAY_SIZE] = {0};

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (mipi_dsi_fifo_is_full(dsi_base) == 0) {
			if (mipi_dsi_lread_reg(&(out[i]), ARRAY_SIZE(out), cm, cm->dlen, dsi_base))
				err_num++;
		} else {
			err_num += (cnt - i);
			dpu_pr_err("dsi fifo full, read [%d] cmds, left [%d] cmds!!", i, cnt - i);
			break;
		}
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return err_num;
}

bool mipi_panel_check_reg(struct dpu_connector *connector)
{
	int ret;
	char __iomem *mipi_dsi_base = connector->connector_base;
	/* mipi reg default value */
	char dpu_reg_0a[] = {0x0a};

	struct dsi_cmd_desc lcd_check_reg[PANEL_CHECK_ARRAY_SIZE] = {
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(dpu_reg_0a), dpu_reg_0a},
	};

	ret = mipi_dsi_cmds_rx_with_check_fifo(lcd_check_reg, PANEL_CHECK_ARRAY_SIZE, mipi_dsi_base);
	if (ret) {
		dpu_pr_warn("Read error number: %d\n", ret);
		return false;
	}
	return true;
}

int mipi_dsi_get_read_value(struct dsi_cmd_desc *dsi_cmd,
	uint8_t *dest, uint32_t *src, uint32_t len, bool little_endian_support)
{
	int dlen;
	int cnt = 0;
	int start_index = 0;
	int div = sizeof(uint32_t) / sizeof(uint8_t);

	if (!dsi_cmd || !dest || !src) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	if (dsi_cmd->dlen > 1)
		start_index = (int)dsi_cmd->payload[1];

	for (dlen = 0; dlen < (int)dsi_cmd->dlen; dlen++) {
		if (dlen < (start_index - 1))
			continue;
		if (cnt >= (int)len) {
			dpu_pr_err("data len error\n");
			return -EINVAL;
		}
		switch (dlen % div) {
		case 0:
			dest[cnt] = (uint8_t)((src[dlen / div] >> mipi_get_offset_bit_num(dlen % div, little_endian_support)) & 0xFF);
			break;
		case 1:
			dest[cnt] = (uint8_t)((src[dlen / div] >> mipi_get_offset_bit_num(dlen % div, little_endian_support)) & 0xFF);
			break;
		case 2:
			dest[cnt] = (uint8_t)((src[dlen / div] >> mipi_get_offset_bit_num(dlen % div, little_endian_support)) & 0xFF);
			break;
		case 3:
			dest[cnt] = (uint8_t)((src[dlen / div] >> mipi_get_offset_bit_num(dlen % div, little_endian_support)) & 0xFF);
			break;
		default:
			break;
		}
		cnt++;
	}
	return 0;
}

bool mipi_dsi_cmd_is_read(struct dsi_cmd_desc *cm)
{
	if (!cm) {
		dpu_pr_err("invalid param\n");
		return false;
	}

	switch (dsi_hdr_dtype(cm->dtype)) {
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
	case DTYPE_DCS_READ:
		return true;
	default:
		return false;
	}
}

bool mipi_dsi_cmd_is_write(struct dsi_cmd_desc *cm)
{
	bool is_write = true;

	if (!cm) {
		dpu_pr_err("invalid param\n");
		return false;
	}

	switch (dsi_hdr_dtype(cm->dtype)) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:
		is_write = true;
		break;
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
	case DTYPE_DCS_READ:
		is_write = false;
		break;
	default:
		is_write = false;
		break;
	}
	return is_write;
}

#define FIFO_IDLE_CYCLE 100
#define FIFO_DELAY_TIME 100
int32_t mipi_dsi_fifo_idle_cycle(const char __iomem *dsi_base)
{
	uint32_t pkg_status;
	uint32_t phy_status;
	int is_timeout = FIFO_IDLE_CYCLE;  /* 10ms(100*100us) */

	if (!dsi_base) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	/* read status register */
	do {
		pkg_status = inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base));
		phy_status = inp32(DPU_DSI_CDPHY_STATUS_ADDR(dsi_base));
		if ((pkg_status & 0x2) != 0x2 && ((phy_status & 0x2) == 0))
			break;
		udelay(FIFO_DELAY_TIME);  /* 100us */
	} while (is_timeout-- > 0);

	if (is_timeout < 0) {
		dpu_pr_err("mipi check full fail:\n"
			"IPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"IPIDSI_PHY_STATUS = 0x%x\n"
			"IPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(dsi_base)),
			inp32(DPU_DSI_CDPHY_STATUS_ADDR(dsi_base)),
			inp32(DPU_DSI_INT_ERROR_FORCE1_ADDR(dsi_base)));

		return -1;
	}
	return FIFO_IDLE_CYCLE - is_timeout;
}

int mipi_dsi_fifo_is_full(const char __iomem *dsi_base)
{
	if (!dsi_base) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	return mipi_dsi_fifo_idle_cycle(dsi_base) >= 0 ? 0 : -1;
}

int32_t mipi_dsi_lread_reg(uint32_t *out, int out_len, struct dsi_cmd_desc *cm, uint32_t len, char *dsi_base)
{
	int32_t ret = 0;
	uint32_t i = 0;
	struct dsi_cmd_desc packet_size_cmd_set;

	if (!out || !cm || !dsi_base) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	if (mipi_dsi_cmd_is_read(cm) == 0) {
		dpu_pr_err("dtype=%x NOT supported!\n", cm->dtype);
		return -1;
	}

	packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
	packet_size_cmd_set.vc = 0;
	packet_size_cmd_set.dlen = len;
	mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dsi_base);
	mipi_dsi_sread_request(cm, dsi_base);
	for (i = 0; (i < (len + 3) / 4) && (i < (uint32_t)out_len); i++) {  /* read 4 bytes once */
		if (mipi_dsi_read(out, dsi_base) == 0) {
			ret = -1;
			dpu_pr_err("Read register %#x timeout\n", cm->payload[0]);
			break;
		}
		out++;
	}

	return ret;
}

void delay_for_next_cmd_by_sleep(uint32_t wait, uint32_t waittype)
{
	if (wait == 0)
		return;

	if (waittype == WAIT_TYPE_US) {
		udelay(wait);
	} else if (waittype == WAIT_TYPE_MS) {
		if (wait <= 10)  /* less then 10ms, use mdelay() */
			mdelay((unsigned long)wait);
		else
			msleep(wait);
	} else {
		msleep(wait * 1000);  /* ms */
	}
}

/*
 * prepare cmd buffer to be txed
 */
int32_t mipi_dsi_cmd_add(struct dsi_cmd_desc *cm, char __iomem *dsi_base)
{
	int32_t len = 0;
	unsigned long flags = 0;

	if (!cm || !dsi_base) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	spin_lock_irqsave(&g_mipi_trans_lock, flags);

	switch (dsi_hdr_dtype(cm->dtype)) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:

	case DTYPE_DCS_WRITE:
	case DTYPE_DCS_WRITE1:
	case DTYPE_DCS_WRITE2:
		len = mipi_dsi_swrite(cm, dsi_base);
		break;
	case DTYPE_GEN_LWRITE:
	case DTYPE_DCS_LWRITE:
	case DTYPE_DSC_LWRITE:

		len = mipi_dsi_lwrite(cm, dsi_base);
		break;
	default:
		dpu_pr_err("dtype=%x NOT supported!\n", cm->dtype);
		break;
	}

	spin_unlock_irqrestore(&g_mipi_trans_lock, flags);

	return len;
}

int32_t mipi_dsi_cmds_tx(struct dsi_cmd_desc *cmds, int32_t cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int32_t i;

	if (!cmds || !dsi_base) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	cm = cmds;
	for (i = 0; i < cnt; i++) {
		mipi_dsi_cmd_add(cm, dsi_base);
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return cnt;
}
