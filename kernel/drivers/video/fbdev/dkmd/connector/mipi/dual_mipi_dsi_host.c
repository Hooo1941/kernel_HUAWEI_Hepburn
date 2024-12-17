/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
#include <securec.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <dpu/soc_dpu_define.h>

#include "peri/dkmd_peri.h"
#include "mipi_dsi_dev.h"

static struct dsi_delayed_cmd_queue g_delayed_cmd_queue;
static bool g_delayed_cmd_queue_inited = false;

static int mipi_dsi_cmds_tx_with_check_fifo(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base)
{
	struct dsi_cmd_desc *cm = NULL;
	int i;

	if (!cmds)
		return -1;

	if (!dsi_base)
		return -1;

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (mipi_dsi_fifo_is_full(dsi_base) == 0) {
			mipi_dsi_cmd_add(cm, dsi_base);
		} else {
			dpu_pr_err("dsi fifo full, write [%d] cmds, left [%d] cmds!!", i, cnt-i);
			break;
		}
		delay_for_next_cmd_by_sleep(cm->wait, cm->waittype);
		cm++;
	}

	return cnt;
}

#define FIFO_IDLE_CYCLE 100
#define FIFO_DELAY_TIME 100
static int mipi_dual_dsi_fifo_is_full(const char __iomem *dsi_base_0, const char __iomem *dsi_base_1)
{
	uint32_t pkg_status_0;
	uint32_t pkg_status_1;
	uint32_t phy_status_0;
	uint32_t phy_status_1;
	int is_timeout = FIFO_IDLE_CYCLE;  /* 10ms */

	if (!dsi_base_0 || !dsi_base_1) {
		dpu_pr_err("dsi_base is NULL!\n");
		return -1;
	}

	/* read status register */
	do {
		pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status_0 = inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET);
		pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status_1 = inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status_0 & 0x2) != 0x2 && ((phy_status_0 & 0x2) == 0) &&
			(pkg_status_1 & 0x2) != 0x2 && ((phy_status_1 & 0x2) == 0))
			break;
		udelay(FIFO_DELAY_TIME); /* 100us */
	} while (is_timeout-- > 0);

	/* adding fifo idle cycle check for solving problem */
	if (is_timeout != FIFO_IDLE_CYCLE)
		dpu_pr_info("mipi dsi fifo idle cycle is %d!\n", FIFO_IDLE_CYCLE - is_timeout);
	if (is_timeout < 0) {
		dpu_pr_err("mipi check full fail: dsi0-\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n"
			"MIPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
		dpu_pr_err("mipi check full fail: dsi1-\n"
			"MIPIDSI_CMD_PKT_STATUS = 0x%x\n"
			"MIPIDSI_PHY_STATUS = 0x%x\n"
			"MIPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));
		return -1;
	}

	return 0;
}

/* just for Cyclomatic Complexity, no need to check input param */
static inline int mipi_dual_dsi_tx_normal_same_delay(struct dsi_cmd_desc *Cmd0,
	struct dsi_cmd_desc *Cmd1, int Cmdset_cnt, char __iomem *dsi_base_0, char __iomem *dsi_base_1, bool need_check_fifo)
{
	int i;
	int send_cnt = 0;

	for (i = 0; i < Cmdset_cnt; i++) {
		if (!need_check_fifo) {
			mipi_dsi_cmd_add(Cmd0, dsi_base_0);
			mipi_dsi_cmd_add(Cmd1, dsi_base_1);
		} else if (mipi_dual_dsi_fifo_is_full(dsi_base_0, dsi_base_1) == 0) {
			mipi_dsi_cmd_add(Cmd0, dsi_base_0);
			mipi_dsi_cmd_add(Cmd1, dsi_base_1);
		} else {
			dpu_pr_err("dsi fifo full, send [%d] cmds, left [%d] cmds!!",
				send_cnt, Cmdset_cnt * 2 - send_cnt);
			break;
		}

		delay_for_next_cmd_by_sleep(Cmd0->wait, Cmd0->waittype);
		send_cnt += 2;

		Cmd0++;
		Cmd1++;
	}

	return send_cnt;
}

static void mipi_dsi_get_cmd_queue_resource(struct dsi_cmd_desc **cmd_queue,
	spinlock_t **spinlock, uint32_t *queue_len, bool is_low_priority)
{
	if (is_low_priority) {
		*cmd_queue = g_delayed_cmd_queue.cmd_queue_low_priority;
		*spinlock = &g_delayed_cmd_queue.cmd_queue_low_priority_lock;
		*queue_len = MAX_CMD_QUEUE_LOW_PRIORITY_SIZE;
	} else {
		*cmd_queue = g_delayed_cmd_queue.cmd_queue_high_priority;
		*spinlock = &g_delayed_cmd_queue.cmd_queue_high_priority_lock;
		*queue_len = MAX_CMD_QUEUE_HIGH_PRIORITY_SIZE;
	}
}

static void mipi_dsi_get_cmd_queue_prio_status(uint32_t **w_ptr, uint32_t **r_ptr,
	bool **is_queue_full, bool **is_queue_working, bool is_low_priority)
{
	if (is_low_priority) {
		*w_ptr = &g_delayed_cmd_queue.cmd_queue_low_priority_wr;
		*r_ptr = &g_delayed_cmd_queue.cmd_queue_low_priority_rd;
		*is_queue_full = &g_delayed_cmd_queue.is_cmd_queue_low_priority_full;
		*is_queue_working = &g_delayed_cmd_queue.is_cmd_queue_low_priority_working;
	} else {
		*w_ptr = &g_delayed_cmd_queue.cmd_queue_high_priority_wr;
		*r_ptr = &g_delayed_cmd_queue.cmd_queue_high_priority_rd;
		*is_queue_full = &g_delayed_cmd_queue.is_cmd_queue_high_priority_full;
		*is_queue_working = &g_delayed_cmd_queue.is_cmd_queue_high_priority_working;
	}
}

static int mipi_dsi_cmd_queue_init(struct dsi_cmd_desc *cmd_queue_elem, struct dsi_cmd_desc *cmd)
{
	u32 j;

	cmd_queue_elem->dtype = cmd->dtype;
	cmd_queue_elem->vc = cmd->vc;
	cmd_queue_elem->wait = cmd->wait;
	cmd_queue_elem->waittype = cmd->waittype;
	cmd_queue_elem->dlen = cmd->dlen;
	if (cmd->dlen > 0) {
		cmd_queue_elem->payload = (char *)kmalloc(cmd->dlen * sizeof(char), GFP_ATOMIC);
		if (!cmd_queue_elem->payload) {
			dpu_pr_err("cmd[%u/%u] payload malloc [%u] fail!\n", cmd->dtype, cmd->vc, cmd->dlen);
			return -ENOMEM;  /* skip this cmd */
		}
		memset(cmd_queue_elem->payload, 0, cmd->dlen * sizeof(char));
		for (j = 0; j < cmd->dlen; j++)
			cmd_queue_elem->payload[j] = cmd->payload[j];
	}

	return 0;
}

static int mipi_dsi_delayed_cmd_queue_write(struct dsi_cmd_desc *cmd_set,
	int cmd_set_cnt, bool is_low_priority)
{
	spinlock_t *spinlock = NULL;
	uint32_t *w_ptr = NULL;
	uint32_t *r_ptr = NULL;
	struct dsi_cmd_desc *cmd_queue = NULL;
	bool *is_queue_full = NULL;
	bool *is_queue_working = NULL;
	uint32_t queue_len;
	int i;
	struct dsi_cmd_desc *cmd = cmd_set;

	if (!cmd_set)
		return -1;

	if (!g_delayed_cmd_queue_inited) {
		dpu_pr_err("delayed cmd queue is not inited yet!\n");
		return 0;
	}

	mipi_dsi_get_cmd_queue_resource(&cmd_queue, &spinlock, &queue_len, is_low_priority);

	spin_lock(spinlock);

	mipi_dsi_get_cmd_queue_prio_status(&w_ptr, &r_ptr, &is_queue_full, &is_queue_working, is_low_priority);

	if (*is_queue_full) {
		dpu_pr_err("Fail, delayed cmd queue [%d] is full!\n", is_low_priority);
		spin_unlock(spinlock);
		return 0;
	}

	for (i = 0; i < cmd_set_cnt; i++) {
		if (mipi_dsi_cmd_queue_init(&cmd_queue[(*w_ptr)], cmd) < 0)
			continue;

		(*w_ptr) = (*w_ptr) + 1;
		if ((*w_ptr) >= queue_len)
			(*w_ptr) = 0;

		(*is_queue_working) = true;

		if ((*w_ptr) == (*r_ptr)) {
			(*is_queue_full) = true;
			dpu_pr_err("Fail, delayed cmd queue [%d] become full, %d cmds are not added in queue!\n",
			is_low_priority, (cmd_set_cnt - i));
		}

		cmd++;
	}

	spin_unlock(spinlock);

	dpu_pr_debug("%d cmds are added to delayed cmd queue [%d]\n", i, is_low_priority);

	return i;
}

/* just for Cyclomatic Complexity, no need to check input param */
static inline int mipi_dual_dsi_tx_auto_mode(struct dsi_cmd_desc *Cmd0, struct dsi_cmd_desc *Cmd1,
	int Cmdset_cnt, char __iomem *dsi_base_0, char __iomem *dsi_base_1, bool need_check_fifo)
{
	int send_cnt;
	struct timespec64 ts;
	s64 timestamp;
	s64 delta_time;
	s64 oneframe_time;

	dpu_pr_debug("+\n");

	if (Cmd0 != Cmd1) {
		dpu_pr_debug("different cmd for two dsi, using normal mode\n");
		goto NormalMode;
	}

	if (!g_delayed_cmd_queue_inited) {
		dpu_pr_err("delayed cmd queue is not inited yet!\n");
		goto NormalMode;
	}

	ktime_get_ts64(&ts);
	timestamp = ts.tv_sec;
	timestamp *= NSEC_PER_SEC;
	timestamp += ts.tv_nsec;
	delta_time = timestamp - g_delayed_cmd_queue.timestamp_frame_start;
	oneframe_time = g_delayed_cmd_queue.oneframe_time;

	/* the CMD_AUTO_MODE_THRESHOLD is not accurate value, can be ajusted later */
	if ((delta_time < oneframe_time * CMD_AUTO_MODE_THRESHOLD) || (delta_time > oneframe_time)) {
		dpu_pr_debug("enough time[%lld] to send in this frame, using normal mode\n", delta_time);
		goto NormalMode;
	}

	dpu_pr_debug("NOT enough time[%lld] to send in this frame, using low priority delayed mode\n", delta_time);
	send_cnt = mipi_dsi_delayed_cmd_queue_write(Cmd0, Cmdset_cnt, true) * 2;

	dpu_pr_debug("-\n");
	return send_cnt;

NormalMode:
	send_cnt = mipi_dual_dsi_tx_normal_same_delay(Cmd0, Cmd1, Cmdset_cnt, dsi_base_0, dsi_base_1, need_check_fifo);
	dpu_pr_debug("--\n");

	return send_cnt;
}

static void delay_by_msleep(uint32_t usec)
{
	uint32_t msec;

	if (usec > 1000) {  /* 1ms */
		msec = usec / 1000;  /* calculate integer */
		usec = usec - msec * 1000;  /* calculate decimal */

		if (msec <= 10)  /* less then 10ms, use mdelay() */
			mdelay((unsigned long)msec);
		else
			msleep(msec);
		if (usec)
			udelay(usec);
	} else if (usec) {
		udelay(usec);
	}
}

static bool mipi_dsi_add_tx_remain_cmd(struct mipi_dual_dsi_param *dual_dsi, uint32_t nextwait,
	int cmd_cnt)
{
	int cnt = cmd_cnt;

	if (cnt < dual_dsi->cmdset_cnt) {
		/* delay the left time */
		delay_by_msleep(nextwait);
		for (; cnt < dual_dsi->cmdset_cnt; cnt++) {
			mipi_dsi_cmd_add(dual_dsi->cmd, dual_dsi->dsi_base);

			delay_for_next_cmd_by_sleep(dual_dsi->cmd->wait, dual_dsi->cmd->waittype);
			dual_dsi->cmd++;
		}
		return true;
	}

	return false;
}

static uint32_t calc_next_wait_time(uint32_t wait, uint32_t waittype)
{
	if (wait) {
		if (waittype == WAIT_TYPE_US)
			return wait;
		else if (waittype == WAIT_TYPE_MS)
			return wait * 1000;  /* ms to us, 1ms = 1000us */
		else
			return wait * 1000 * 1000;  /* other to us */
	}

	return 0;
}

static void mipi_dsi_add_tx_cmd(struct mipi_dual_dsi_param *dual_dsi, uint32_t *nextwait, int *cmd_cnt)
{
	if (*nextwait == 0) {
		mipi_dsi_cmd_add(dual_dsi->cmd, dual_dsi->dsi_base);

		*nextwait = calc_next_wait_time(dual_dsi->cmd->wait, dual_dsi->cmd->waittype);
		dual_dsi->cmd++;
		(*cmd_cnt)++;
	}
}

/* just for Cyclomatic Complexity, no need to check input param */
static int mipi_dual_dsi_cmds_tx_normal(struct mipi_dual_dsi_param *dual_dsi0,
	struct mipi_dual_dsi_param *dual_dsi1)
{
	int i = 0;
	int j = 0;
	uint32_t nextwait = 0;
	uint32_t nextwait_0 = 0;
	uint32_t nextwait_1 = 0;

	while ((i < dual_dsi0->cmdset_cnt) && (j < dual_dsi1->cmdset_cnt)) {
		mipi_dsi_add_tx_cmd(dual_dsi0, &nextwait_0, &i);
		mipi_dsi_add_tx_cmd(dual_dsi1, &nextwait_1, &j);

		nextwait = (nextwait_0 > nextwait_1) ? nextwait_1 : nextwait_0;
		delay_by_msleep(nextwait);
		nextwait_0 -= nextwait;
		nextwait_1 -= nextwait;
	}

	/* send the left Cmds */
	if (mipi_dsi_add_tx_remain_cmd(dual_dsi0, nextwait_0, i) == false)
		mipi_dsi_add_tx_remain_cmd(dual_dsi1, nextwait_0, j);

	return dual_dsi0->cmdset_cnt + dual_dsi1->cmdset_cnt;
}

static void mipi_dsi_cmd_send_lock(void)
{
	if (g_delayed_cmd_queue_inited)
		spin_lock(&g_delayed_cmd_queue.cmd_send_lock);
}

static void mipi_dsi_cmd_send_unlock(void)
{
	if (g_delayed_cmd_queue_inited)
		spin_unlock(&g_delayed_cmd_queue.cmd_send_lock);
}

static int mipi_dual_dsi_read(uint32_t *value_out_0, uint32_t *value_out_1,
	const char __iomem *dsi_base_0, const char __iomem *dsi_base_1)
{
	uint32_t pkg_status_0 = 0;
	uint32_t pkg_status_1 = 0;
	uint32_t try_times = 700;  /* 35ms(50*700) */
	bool read_done_0 = false;
	bool read_done_1 = false;
	int ret = 2;

	if (!dsi_base_0 || !dsi_base_1) {
		dpu_pr_err("dsi_base is NULL!\n");
		return 0;
	}

	if (!value_out_0 || !value_out_1) {
		dpu_pr_err("out buffer is NULL!\n");
		return 0;
	}

	do {
		if (!read_done_0) {
			pkg_status_0 = inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if ((pkg_status_0 & 0x10) == 0) {
				read_done_0 = true;
				*value_out_0 = inp32(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(dsi_base_0));
			}
		}
		if (!read_done_1) {
			pkg_status_1 = inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if ((pkg_status_1 & 0x10) == 0) {
				read_done_1 = true;
				*value_out_1 = inp32(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(dsi_base_1));
			}
		}

		if (read_done_0 && read_done_1)
			break;
		udelay(50);  /* 50us */
	} while (--try_times);

	if (!read_done_0) {
		ret -= 1;
		dpu_pr_err("%s, DSI0 CMD_PKT_STATUS[0x%x], PHY_STATUS[0x%x], INT_ST0[0x%x], INT_ST1[0x%x]\n",
			__func__,
			inp32(dsi_base_0 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_0 + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base_0 + MIPIDSI_INT_ST1_OFFSET));
	}
	if (!read_done_1) {
		ret -= 1;
		dpu_pr_err("%s, DSI1 CMD_PKT_STATUS[0x%x], PHY_STATUS[0x%x], INT_ST0[0x%x], INT_ST1[0x%x]\n",
			__func__, inp32(dsi_base_1 + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base_1 + MIPIDSI_INT_ST0_OFFSET), inp32(dsi_base_1 + MIPIDSI_INT_ST1_OFFSET));
	}

	return ret;
}

/* only support same cmd for two dsi */
static int32_t mipi_dual_dsi_lread_reg(struct mipi_dual_dsi_param *dual_dsi0,
	struct dsi_cmd_desc *p_cmd, uint32_t dlen, struct mipi_dual_dsi_param *dual_dsi1)
{
	int32_t ret = 0;
	uint32_t i = 0;
	struct dsi_cmd_desc packet_size_cmd_set;

	if (!p_cmd || (dlen == 0)) {
		dpu_pr_err("Cmd is NULL!\n");
		return -1;
	}

	if (!dual_dsi0->dsi_base || !dual_dsi1->dsi_base) {
		dpu_pr_err("dsi_base is NULL!\n");
		return -1;
	}

	if (!dual_dsi0->value_out || !dual_dsi1->value_out) {
		dpu_pr_err("out buffer is NULL!\n");
		return -1;
	}

	if (mipi_dsi_cmd_is_read(p_cmd)) {
		if (mipi_dual_dsi_fifo_is_full(dual_dsi0->dsi_base, dual_dsi1->dsi_base) == 0) {
			packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
			packet_size_cmd_set.vc = 0;
			packet_size_cmd_set.dlen = dlen;

			mipi_dsi_cmd_send_lock();

			mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dual_dsi0->dsi_base);
			mipi_dsi_max_return_packet_size(&packet_size_cmd_set, dual_dsi1->dsi_base);
			mipi_dsi_sread_request(p_cmd, dual_dsi0->dsi_base);
			mipi_dsi_sread_request(p_cmd, dual_dsi1->dsi_base);
			for (i = 0; i < (dlen + 3) / 4; i++) { /* 4byte Align */
				if (mipi_dual_dsi_read(dual_dsi0->value_out, dual_dsi1->value_out,
					dual_dsi0->dsi_base, dual_dsi1->dsi_base) < 2) {
					ret = -1;
					dpu_pr_err("Read register 0x%X timeout\n", p_cmd->payload[0]);
					break;
				}
				dual_dsi0->value_out++;
				dual_dsi1->value_out++;
			}

			mipi_dsi_cmd_send_unlock();
		} else {
			ret = -1;
			dpu_pr_err("dsi fifo full, read fail!!\n");
		}
	} else {
		ret = -1;
		dpu_pr_err("dtype=%x NOT supported!\n", p_cmd->dtype);
	}

	return ret;
}

int32_t  mipi_dual_dsi_cmds_tx(struct dsi_cmd_desc *cmd0, int cnt0, char __iomem *dsi_base_0,
	struct dsi_cmd_desc *cmd1, int cnt1, char __iomem *dsi_base_1, uint8_t tx_mode, bool need_check_fifo)
{
	struct mipi_dual_dsi_param dual_dsi0 = { NULL, dsi_base_0, cmd0, cmd0, 0, cnt0 };
	struct mipi_dual_dsi_param dual_dsi1 = { NULL, dsi_base_1, cmd1, cmd1, 0, cnt1 };
	struct dsi_cmd_desc *Cmd0 = dual_dsi0.p_cmdset;
	struct dsi_cmd_desc *Cmd1 = dual_dsi1.p_cmdset;
	int send_cnt = 0;
	bool sameCmdDelay = false;

	if (!cmd0 || !dsi_base_0 || !cmd1 || !dsi_base_1) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	if (!dual_dsi0.p_cmdset || (dual_dsi0.cmdset_cnt == 0)) {
		dpu_pr_err("dsi0 cmdset %d fail!\n", dual_dsi0.cmdset_cnt);
		return -1;
	}
	if (!dual_dsi0.dsi_base) {
		dpu_pr_err("dsi0 dsi base is null fail!\n");
		return -1;
	}

	/* if only one dsi_base exist, fallback to mipi_dsi_cmds_tx() interface */
	if (!dual_dsi1.dsi_base)
		if (need_check_fifo)
			return mipi_dsi_cmds_tx_with_check_fifo(dual_dsi0.p_cmdset,
				dual_dsi0.cmdset_cnt, dual_dsi0.dsi_base);
		else
			return mipi_dsi_cmds_tx(dual_dsi0.p_cmdset,
				dual_dsi0.cmdset_cnt, dual_dsi0.dsi_base);
	/* only one valid Cmdset, use same Cmd_set for two dsi */
	if (!dual_dsi1.p_cmdset || (dual_dsi1.cmdset_cnt == 0)) {
		Cmd1 = dual_dsi0.p_cmdset;
		sameCmdDelay = true;
	} else if (dual_dsi0.cmdset_cnt == dual_dsi1.cmdset_cnt) {
		/* support different cmd value bysame step */
		sameCmdDelay = true;
	}

	if (sameCmdDelay) {
		if (tx_mode == EN_DSI_TX_NORMAL_MODE)
			/* only support same cmdset length and wait, just cmd value different */
			send_cnt = mipi_dual_dsi_tx_normal_same_delay(Cmd0, Cmd1, dual_dsi0.cmdset_cnt,
				dual_dsi0.dsi_base, dual_dsi1.dsi_base, need_check_fifo);
		else if (tx_mode == EN_DSI_TX_LOW_PRIORITY_DELAY_MODE)
			/* only support same cmd set yet */
			send_cnt = mipi_dsi_delayed_cmd_queue_write(Cmd0, dual_dsi0.cmdset_cnt, true) * 2;
		else if (tx_mode == EN_DSI_TX_HIGH_PRIORITY_DELAY_MODE)
			/* only support same cmd set yet */
			send_cnt = mipi_dsi_delayed_cmd_queue_write(Cmd0, dual_dsi0.cmdset_cnt, false) * 2;
		else if (tx_mode == EN_DSI_TX_AUTO_MODE)
			/* only support same cmdset length and wait, just cmd value different */
			send_cnt = mipi_dual_dsi_tx_auto_mode(Cmd0, Cmd1, dual_dsi0.cmdset_cnt,
				dual_dsi0.dsi_base, dual_dsi1.dsi_base, need_check_fifo);
	} else { /* two different Cmdsets case */
		/* just use normal mode, expect no need use */
		send_cnt = mipi_dual_dsi_cmds_tx_normal(&dual_dsi0, &dual_dsi1);
	}

	return send_cnt;
}

int32_t mipi_dual_dsi_cmds_rx(char __iomem *dsi_base_0, uint8_t *dsi0_out, char __iomem *dsi_base_1,
	uint8_t *dsi1_out, int out_len, struct dsi_cmd_desc *cmd,  bool little_endian_support)
{
	int ret;
	int cnt = 0;
	uint32_t j;
	uint8_t dsi0_val[BUF_MAX] = {0};
	uint8_t dsi1_val[BUF_MAX] = {0};
	struct mipi_dual_dsi_param dual_dsi0 = {0};
	struct mipi_dual_dsi_param dual_dsi1 = {0};
	uint32_t dsi0_tmp_val[READ_MAX] = {0};
	uint32_t dsi1_tmp_val[READ_MAX] = {0};

	if (!dsi_base_0 || !dsi0_out || !dsi_base_1 || !dsi1_out || !cmd) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	dual_dsi0.dsi_base = dsi_base_0;
	dual_dsi0.value_out = dsi0_tmp_val;
	dual_dsi1.dsi_base = dsi_base_1;
	dual_dsi1.value_out = dsi1_tmp_val;

	if (mipi_dsi_cmd_is_write(cmd)) {
		(void)mipi_dual_dsi_cmds_tx(cmd, 1, dsi_base_0, cmd, 1, dsi_base_1, EN_DSI_TX_NORMAL_MODE, true);
	} else {
		ret = mipi_dual_dsi_lread_reg(&dual_dsi0, cmd, cmd->dlen, &dual_dsi1);
		if (ret != 0) {
			dpu_pr_err("mipi read error\n");
			return -EINVAL;
		}
		ret = mipi_dsi_get_read_value(cmd, dsi0_val, dsi0_tmp_val, (uint32_t)out_len, little_endian_support);
		if (ret != 0) {
			dpu_pr_err("get read value error\n");
			return ret;
		}
		ret = mipi_dsi_get_read_value(cmd, dsi1_val, dsi1_tmp_val, (uint32_t)out_len, little_endian_support);
		if (ret != 0) {
			dpu_pr_err("get read value error\n");
			return ret;
		}

		for (j = 0; (j < cmd->dlen) && (cnt <= out_len) && (j < BUF_MAX); j++) {
			dsi0_out[cnt] = dsi0_val[j];
			dsi1_out[cnt] = dsi1_val[j];
			cnt++;
		}
	}

	return 0;
}
