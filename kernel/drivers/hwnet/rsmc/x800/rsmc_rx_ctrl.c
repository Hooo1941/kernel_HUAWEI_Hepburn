/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#include "rsmc_rx_ctrl.h"

#include <huawei_platform/log/hw_log.h>
#include <securec.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "../module_type.h"
#include "rsmc_spi_ctrl.h"
#include "rsmc_tx_ctrl.h"
#include "rsmc_x800_device.h"
#include "track.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_RX_CTRL
HWLOG_REGIST();

#define MAX_BUFF_ADDR_LEN 500
#define SPI_NORMAL 0
#define SPI_LOOP_RECV 1
#define SPI_LOOP_TX 2
#define PLL_DATA_LEN 5
#define SEARCH_CORR_STEP 200
#define AFFINITY_MAX_CPU_NUM 32

struct chan_info {
	volatile s32 mode;
	volatile s32 next_mode;
	volatile u32 ms_idx;
	volatile s32 save_mode;
	volatile u32 tap_idx;
	volatile u32 data_idx;
	struct complex corr_tap[ACQ_CORR_NUM + 1];
	volatile s32 timeout;
	volatile s32 cnt;
	volatile u32 carr_nco_word;
	volatile u32 code_nco_word;
	volatile u32 carr_nco_word_next;
	volatile u32 code_nco_word_next;
	volatile u32 code_nco_word_once;
	volatile s32 adjust_state;
	struct acquisition_msg acq_msg1;
	struct acquisition_msg acq_msg2;
	u32 valid_acq_msg_idx;
	struct track_msg trk_msg1;
	struct track_msg trk_msg2;
	u32 valid_trk_msg_idx;
	struct s2c_d_data_msg data_msg1;
	struct s2c_d_data_msg data_msg2;
	u32 valid_data_msg_idx;
	s32 flag;
};

struct cmd_buf {
	volatile s32 valid;
	volatile u32 send[MAX_BUFF_ADDR_LEN];
	volatile u32 recv[MAX_BUFF_ADDR_LEN];
	volatile s32 idx;
	volatile s32 end;
	volatile u32 last;
};

struct rx_ctx {
	s32 rx_mode;
	struct task_struct *task;
	volatile u16 chan_status;
	struct chan_info info[MAX_CHAN_NUM + 1];
	struct cmd_buf cmd;
	volatile u32 spi_status;
	volatile bool only_tx;
	struct semaphore enter_sema;
	struct semaphore exit_sema;
};
static struct rx_ctx g_rx_ctx = {0};

const u32 read_sys_status_cmd = (ADDR_05_SYS_STATUS | 0x80) << 24;
const u32 read_corr_fifo_cmd = (ADDR_0E_CORR_FIFO | 0x80) << 24;
const u32 read_version_cmd = (ADDR_0F_VERSION | 0x80) << 24;
const u32 read_rf_7e_cmd = (ADDR_RF_7E | 0x80) << 24;
const u32 read_rf_7f_cmd = (ADDR_RF_7F | 0x80) << 24;
const u32 read_rf_70_cmd = (ADDR_RF_70 | 0x80) << 24;
const u32 read_rf_71_cmd = (ADDR_RF_71 | 0x80) << 24;
const u32 flash_send_cmd = 0xFFFFFFFF;

void rx_mode_set(int mode)
{
	g_rx_ctx.rx_mode = mode;
}

static void set_acq_start(void)
{
	u16 flag = g_rx_ctx.chan_status;

	hwlog_info("%s: STATUS acq flag:%08x", __func__, flag);
	smc_set_value(0xff, 0xffffff);
}

static void sync_mode_update(u8 chn_idx)
{
	struct chan_info *chn_info = NULL;
	struct chan_info *chn_othr = NULL;
	u8 idx;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	chn_info = &g_rx_ctx.info[chn_idx];
	if (chn_info->mode == MODE_IDLE)
		return;
	hwlog_info("%s", __func__);
	if (chn_info->next_mode == MODE_TRK) {
		g_rx_ctx.chan_status |= (1 << chn_idx);
		cmd_write(ADDR_01_SYS_CFG_1, g_rx_ctx.chan_status);
		chn_info->cnt = 0;
#ifdef RSMC_DEBUG
		hwlog_info("%s: chn_idx:%d,next:%d,status:%08x 1",
			__func__, chn_idx,
			chn_info->next_mode, g_rx_ctx.chan_status);
#endif
		for (idx = 0; idx < MAX_CHAN_NUM; idx++) {
			if (idx != chn_idx) {
				chn_othr = &g_rx_ctx.info[idx];
				chn_othr->mode = MODE_IDLE;
				hwlog_info("%s: chn_idx:%d close",
					__func__, idx);
				cmd_write(addr_10_chnl_en(idx), 0);
			}
		}
	} else if (chn_info->mode == MODE_ACQ) {
		g_rx_ctx.chan_status &= (~(1 << chn_idx));
		cmd_write(ADDR_01_SYS_CFG_1, g_rx_ctx.chan_status);
		chn_info->cnt = 0;
#ifdef RSMC_DEBUG
		hwlog_info("%s: chn_idx:%d,next:%d,status:%08x 2",
			__func__, chn_idx, chn_info->next_mode,
			g_rx_ctx.chan_status);
#endif
	}
	chn_info->mode = chn_info->next_mode;
}

static void sync_adjust_phase(u8 chn_idx)
{
	struct chan_info *chn_info = NULL;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	hwlog_info("%s", __func__);
	chn_info = &g_rx_ctx.info[chn_idx];
	// carrier nco word discard low 2 bit
	cmd_write(addr_15_chnl_carr_nco_fw(chn_idx),
		(chn_info->carr_nco_word >> 2) & 0xFFFFFF);
	if (chn_info->adjust_state == PHASE_ADJUST_NEED) {
		// bit 17~10 chnl_code_nco_fw_h8 hight 8 bit
		cmd_write(addr_14_chnl_code_nco_phs(chn_idx),
			(chn_info->code_nco_word_once >> 24) << 10);
		cmd_write(addr_13_chnl_code_nco_fw(chn_idx),
			chn_info->code_nco_word_once & 0xFFFFFF);
		chn_info->adjust_state = PHASE_ADJUST_ALREADY;
	} else if (chn_info->adjust_state == PHASE_ADJUST_ALREADY) {
		// bit 17~10 chnl_code_nco_fw_h8 hight 8 bit
		cmd_write(addr_14_chnl_code_nco_phs(chn_idx),
			(chn_info->code_nco_word >> 24) << 10);
		cmd_write(addr_13_chnl_code_nco_fw(chn_idx),
			chn_info->code_nco_word & 0xFFFFFF);
		chn_info->adjust_state = 0;
	}
}

static void send_to_acq(struct acquisition_msg *msg)
{
	if (msg == NULL)
		return;
	msg->mh.module = MODULE_TYPE_FAST_CTRL;
	msg->mh.type = CMD_UP_ACQ_IND;
	msg->mh.len = sizeof(struct acquisition_msg) - sizeof(struct list_head);
	fast_notify_event((struct list_head *)msg);
}

static void adjust_delay(u8 chn_idx, s32 delta)
{
	struct chan_info *chn_info = NULL;
	u64 fact;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	chn_info = &g_rx_ctx.info[chn_idx];
	if (delta == 0)
		return;
	hwlog_info("%s: chn:%d,delta:%d", __func__, chn_idx, delta);
	fact = chn_info->code_nco_word;
	fact = fact * CODE_HZ;
	chn_info->code_nco_word_once =
		(u32)((s64)fact / (CODE_HZ + delta * TIMES_10));
	chn_info->adjust_state = PHASE_ADJUST_NEED;
	chn_info->cnt = 0;
}

static int chk_proc_tx(u8 chn_idx)
{
	int cnt, ret;
	struct chan_info *chn_info = NULL;

	if (chn_idx >= MAX_CHAN_NUM)
		return 0;
	chn_info = &g_rx_ctx.info[chn_idx];
	for (cnt = 0; cnt < MAX_CHAN_NUM; cnt++) {
		// find first acq channel
		if (g_rx_ctx.info[cnt].mode == MODE_ACQ ||
			g_rx_ctx.info[cnt].mode == MODE_TRK)
			break;
	}
	if (cnt >= MAX_CHAN_NUM)
		cnt = 0;
	if (cnt != chn_idx)
		return 0; // not first acq channel
	ret = start_tx();
	if (ret)
		chn_info->cnt = 0;
	return ret;
}

static void acq_1ms_evt(u8 chn_idx)
{
	int i;
	struct acquisition_msg *acq_msg = NULL;
	struct chan_info *chn_info = NULL;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	chn_info = &g_rx_ctx.info[chn_idx];
	if (chn_info->valid_acq_msg_idx == 1)
		acq_msg = &chn_info->acq_msg1;
	else
		acq_msg = &chn_info->acq_msg2;
	if (++chn_info->ms_idx >= FRM_PERIOD)
		chn_info->ms_idx = 0;
	if (chn_info->adjust_state != 0) {
		sync_adjust_phase(chn_idx);
		return;
	}
	if (chn_info->mode != chn_info->next_mode) {
		sync_mode_update(chn_idx);
		return;
	}
	if (chk_proc_tx(chn_idx) > 0)
		return;
	if (chn_info->mode == MODE_TRK) {
		hwlog_err("%s: err mode", __func__);
		return;
	}
	for (i = 0; i < ACQ_CORR_NUM; i++) {
		acq_msg->corr_buf[i][chn_info->cnt].real = chn_info->corr_tap[i].real;
		acq_msg->corr_buf[i][chn_info->cnt].imag = chn_info->corr_tap[i].imag;
	}
	chn_info->cnt++;
	if (chn_info->cnt >= FRM_PERIOD) {
		acq_msg->chn_idx = chn_idx;
		acq_msg->ms_idx = (chn_info->ms_idx) % FRM_PERIOD;
		if (chn_info->timeout > 0) {
			adjust_delay(chn_idx, SEARCH_CORR_STEP);
			chn_info->timeout--;
		}
		if (chn_info->valid_acq_msg_idx == 1)
			chn_info->valid_acq_msg_idx = IDX_2;
		else
			chn_info->valid_acq_msg_idx = IDX_1;
		send_to_acq(acq_msg);
		chn_info->cnt = 0;
	}
}

static s32 send_to_track(struct track_msg *msg)
{
	s32 ret;

	if (msg == NULL)
		return -1;
	ret = track_msg_calc(msg);
	msg->mh.module = MODULE_TYPE_FAST_CTRL;
	msg->mh.type = CMD_UP_TRK_IND;
	msg->mh.len = sizeof(struct track_msg) - sizeof(struct list_head);
	fast_notify_event((struct list_head *)msg);
	return ret;
}

static void send_to_codec(struct s2c_d_data_msg *msg)
{
	const u32 tmp = 100;

	if (msg == NULL)
		return;
	msg->mh.module = MODULE_TYPE_FAST_CTRL;
	msg->mh.type = CMD_UP_RX_IND;
	msg->mh.len = sizeof(struct s2c_d_data_msg) - sizeof(struct list_head) - tmp;
	fast_notify_event((struct list_head *)msg);
}

void update_nco_word(u8 chn_idx)
{
	struct chan_info *chn_info = NULL;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	chn_info = &g_rx_ctx.info[chn_idx];
	// bit 18~13 chnl_carr_nco_fw_h6 (carrier nco fw high 6 bit)
	cmd_write(addr_12_chnl_cnt_init(chn_idx),
		(chn_info->carr_nco_word_next >> 26) << 13);

	// carrier nco word discard low 2 bit
	cmd_write(addr_15_chnl_carr_nco_fw(chn_idx),
		(chn_info->carr_nco_word_next >> 2) & 0xFFFFFF);
	chn_info->carr_nco_word = chn_info->carr_nco_word_next;

	// bit 17~10 chnl_code_nco_fw_h8 hight 8 bit
	cmd_write(addr_14_chnl_code_nco_phs(chn_idx),
		(chn_info->code_nco_word_next >> 24) << 10);
	cmd_write(addr_13_chnl_code_nco_fw(chn_idx),
		(chn_info->code_nco_word_next) & 0xFFFFFF);
	chn_info->code_nco_word = chn_info->code_nco_word_next;

	hwlog_info("%s: chn_idx:%d, carr:%08x,code:%08x",
		__func__, chn_idx, chn_info->carr_nco_word,
		chn_info->code_nco_word);
}

static s8 u2c(u8 data, s32 flag)
{
	s8 out;
	u8 sign = 0xff;

	sign = (sign << 4);
	if ((sign & data) != 0)
		out = (s8)(sign | data);
	else
		out = (s8)(data);
	out = flag > 0 ? out : -out;
	return out;
}

static void update_trk_msg(u8 chn_idx)
{
	struct track_msg *trk_msg = NULL;
	struct s2c_d_data_msg *data_msg = NULL;
	struct chan_info *chn_info = NULL;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	chn_info = &g_rx_ctx.info[chn_idx];
	if (chn_info->valid_trk_msg_idx == 1)
		trk_msg = &chn_info->trk_msg1;
	else
		trk_msg = &chn_info->trk_msg2;

	if (chn_info->cnt >= MAX_TRACK_LEN)
		chn_info->cnt = 0;
	trk_msg->e_tap[chn_info->cnt].real = chn_info->corr_tap[IDX_0].real;
	trk_msg->e_tap[chn_info->cnt].imag = chn_info->corr_tap[IDX_0].imag;
	trk_msg->p_tap[chn_info->cnt].real = chn_info->corr_tap[IDX_1].real;
	trk_msg->p_tap[chn_info->cnt].imag = chn_info->corr_tap[IDX_1].imag;
	trk_msg->l_tap[chn_info->cnt].real = chn_info->corr_tap[IDX_2].real;
	trk_msg->l_tap[chn_info->cnt].imag = chn_info->corr_tap[IDX_2].imag;

	chn_info->cnt++;
	if (chn_info->cnt >= PLL_DATA_LEN) {
		trk_msg->chn_idx = chn_idx;
		trk_msg->ms_idx =
			(u8)(chn_info->ms_idx + FRM_PERIOD - PLL_DATA_LEN) % FRM_PERIOD;
		update_nco_word(chn_idx);
		if (chn_info->valid_trk_msg_idx == 1)
			chn_info->valid_trk_msg_idx = IDX_2;
		else
			chn_info->valid_trk_msg_idx = IDX_1;

		chn_info->flag = send_to_track(trk_msg);
		chn_info->cnt = 0;
		trk_msg->len = 0;
	}
	if (chn_info->ms_idx == 0) {
		if (chn_info->valid_data_msg_idx == 1) {
			data_msg = &chn_info->data_msg1;
			chn_info->valid_data_msg_idx = IDX_2;
		} else {
			data_msg = &chn_info->data_msg2;
			chn_info->valid_data_msg_idx = IDX_1;
		}
		data_msg->chn_idx = chn_idx;
		send_to_codec(data_msg);
	}
	chn_info->data_idx = chn_info->ms_idx * SYMBOL_PER_MS;
}

static void trk_1ms_evt(u8 chn_idx)
{
	struct chan_info *chn_info = NULL;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	chn_info = &g_rx_ctx.info[chn_idx];
	if (++chn_info->ms_idx >= FRM_PERIOD)
		chn_info->ms_idx = 0;
	if (chn_info->adjust_state != 0) {
		sync_adjust_phase(chn_idx);
		return;
	}
	if (chn_info->mode != chn_info->next_mode) {
		sync_mode_update(chn_idx);
		return;
	}
	if (chk_proc_tx(chn_idx) > 0)
		return;
	update_trk_msg(chn_idx);
}

static u32 cmd_read_cb(u32 value)
{
	u32 next = read_corr_fifo_cmd;
	if (g_rx_ctx.only_tx)
		next = read_version_cmd;

	if (unlikely(g_rx_ctx.cmd.valid <= 0))
		return 0;

	if (g_rx_ctx.cmd.send[g_rx_ctx.cmd.idx] == read_sys_status_cmd) {
		tx_update_buf(value);
		g_rx_ctx.cmd.idx++;
	}
	if (g_rx_ctx.cmd.idx >= g_rx_ctx.cmd.end) {
		g_rx_ctx.cmd.idx = 0;
		g_rx_ctx.cmd.end = 0;
		g_rx_ctx.cmd.valid = 0;
		return next;
	}
	if (g_rx_ctx.cmd.send[g_rx_ctx.cmd.idx] == flash_send_cmd) {
		tx_sm_set();
		g_rx_ctx.cmd.idx++;
	}
	if (g_rx_ctx.cmd.idx >= g_rx_ctx.cmd.end) {
		g_rx_ctx.cmd.idx = 0;
		g_rx_ctx.cmd.end = 0;
		g_rx_ctx.cmd.valid = 0;
		return next;
	}
	next = g_rx_ctx.cmd.send[g_rx_ctx.cmd.idx];
	g_rx_ctx.cmd.idx++;
	return next;
}

static void pilot_proc(u32 value)
{
	u32 idx;
	u32 chn_idx;
	struct chan_info *chn_info = NULL;
	struct reg0e_corr_p_stru *corr_p_reg_data = (struct reg0e_corr_p_stru *)&value;

	chn_idx = corr_p_reg_data->chn_idx;
	if (chn_idx >= MAX_CHAN_NUM) {
		hwlog_info("%s:chan idx %d error", __func__, chn_idx);
		return;
	}
	chn_info = &g_rx_ctx.info[chn_idx];
	idx = chn_info->tap_idx;
	if (idx >= ACQ_CORR_NUM - 1) {
		idx = ACQ_CORR_NUM - 1;
		chn_info->corr_tap[idx].real = (s16)corr_p_reg_data->real;
		chn_info->corr_tap[idx].imag = (s16)corr_p_reg_data->imag;
		chn_info->tap_idx = 0;
		acq_1ms_evt(chn_idx);
		chn_info->save_mode = CORR_P_ONLY;
	} else {
		chn_info->corr_tap[idx].real = (s16)corr_p_reg_data->real;
		chn_info->corr_tap[idx].imag = (s16)corr_p_reg_data->imag;
		chn_info->tap_idx = idx + 1;
	}
}

static void data_proc(u32 value)
{
	u32 chn_idx;
	struct chan_info *chn_info = NULL;
	struct reg0e_corr_d_stru *corr_d_reg_data = NULL;
	struct s2c_d_data_msg *data_msg = NULL;
	const u32 data_max = 4096;

	corr_d_reg_data = (struct reg0e_corr_d_stru *)&value;
	chn_idx = corr_d_reg_data->chn_idx;
	if (chn_idx >= MAX_CHAN_NUM) {
		hwlog_info("%s:chan idx %d error", __func__, chn_idx);
		return;
	}
	chn_info = &g_rx_ctx.info[chn_idx];
	if (chn_info->save_mode == CORR_D_AND_P) {
		if (chn_info->valid_data_msg_idx == 1)
			data_msg = &chn_info->data_msg1;
		else
			data_msg = &chn_info->data_msg2;

		if (chn_info->data_idx > data_max) {
			chn_info->data_idx = 0;
			hwlog_info("%s:data idx overlap", __func__);
			return;
		}
		data_msg->data[chn_info->data_idx++] =
			u2c(corr_d_reg_data->data0, chn_info->flag);
		data_msg->data[chn_info->data_idx++] =
			u2c(corr_d_reg_data->data1, chn_info->flag);
		data_msg->data[chn_info->data_idx++] =
			u2c(corr_d_reg_data->data2, chn_info->flag);
		data_msg->data[chn_info->data_idx++] =
			u2c(corr_d_reg_data->data3, chn_info->flag);
		if (chn_info->tap_idx > 0) {
			chn_info->tap_idx = 0;
			trk_1ms_evt(chn_idx);
		}
	} else {
		chn_info->save_mode = CORR_D_AND_P;
	}
}

static void corr_data_proc(u32 value)
{
	struct reg0e_corr_p_stru *corr_p_reg_data = (struct reg0e_corr_p_stru *)&value;

	if (corr_p_reg_data->flag == CORR_TYPE_PILOT)
		pilot_proc(value);
	else
		data_proc(value);
}

static void set_rf_reg(u32 value)
{
	if (g_rx_ctx.cmd.last == read_rf_7e_cmd)
		set_rf(ADDR_RF_7E, value);
	else if (g_rx_ctx.cmd.last == read_rf_7f_cmd)
		set_rf(ADDR_RF_7F, value);
	else if (g_rx_ctx.cmd.last == read_rf_70_cmd)
		set_rf(ADDR_RF_70, value);
	else if (g_rx_ctx.cmd.last == read_rf_71_cmd)
		set_rf(ADDR_RF_71, value);
}

static void tx_err_proc()
{
	set_tx_err();
	g_rx_ctx.spi_status = SPI_NORMAL;
	tx_rsp_ok();
	send_soc_err();
}

static u32 x800_spi_loop_cb(u32 value)
{
	u32 cmd;
	u32 next = read_corr_fifo_cmd;

	if (g_rx_ctx.only_tx) {
		next = read_version_cmd;
		if (g_rx_ctx.cmd.last == read_version_cmd && value != VERSION)
			tx_err_proc();
	}
	set_rf_reg(value);
	cmd = cmd_read_cb(value);
	if (cmd != 0) {
		next = cmd;
		goto cb_end;
	}
	if (unlikely(g_rx_ctx.spi_status == SPI_NORMAL)) {
		next = 0;
		goto cb_end;
	}
	if (value == 0) {
		g_rx_ctx.cmd.last = next;
#ifdef RSMC_DEBUG
		hwlog_info("%s: next:%X", __func__, next);
#endif
		return next;
	}
	if (g_rx_ctx.only_tx) {
		start_tx();
		if (tx_complete())
			g_rx_ctx.spi_status = SPI_NORMAL;
	} else {
		corr_data_proc(value);
	}
	if (g_rx_ctx.cmd.valid > 0) {
		if (g_rx_ctx.cmd.send[g_rx_ctx.cmd.idx] == flash_send_cmd) {
			tx_sm_set();
			g_rx_ctx.cmd.idx++;
		}
		next = g_rx_ctx.cmd.send[g_rx_ctx.cmd.idx];
		if (next != read_sys_status_cmd)
			g_rx_ctx.cmd.idx++;
		goto cb_end;
	}
cb_end:
	g_rx_ctx.cmd.last = next;
#ifdef RSMC_DEBUG
	hwlog_info("%s: value:%X,next:%X", __func__, value, next);
#endif
	return next;
}

void rsmc_set_cpu_affinity(struct smc_core_data *cd)
{
	cpumask_var_t mask;
	u32 i;
	u32 offset = cd->feature_config.cpu_affinity_offset;
	u32 dtsi_mask = cd->feature_config.cpu_affinity_mask;
	u32 cpu;

	if (dtsi_mask == 0) {
		hwlog_info("%s: not config cpu mask", __func__);
		return;
	}

	if (!alloc_cpumask_var(&mask, GFP_KERNEL | __GFP_ZERO))
		return;
	for (i = 0; i < AFFINITY_MAX_CPU_NUM; i++) {
		cpu = (dtsi_mask >> i) & 0x01;
		if (cpu) {
			cpumask_set_cpu(i + offset, mask);
			hwlog_info("%s: i=%u", __func__, i);
		}
	}
	if (!cpumask_empty(mask))
		if (sched_setaffinity(0, mask) != 0)
			hwlog_err("%s: set affinity to cpu error", __func__);
	free_cpumask_var(mask);
}

static int rsmc_loop_read(void *data)
{
	struct spi_device *sdev = NULL;
	struct smc_core_data *cd = NULL;
	struct spi_controller *master = NULL;
	struct sched_param sp = { .sched_priority = MAX_RT_PRIO -1 };

	hwlog_info("%s: enter", __func__);
	cd = smc_get_core_data();
	if (cd == NULL)
		return 0;
	rsmc_set_cpu_affinity(cd);
	sdev = cd->sdev;
	if (sdev == NULL) {
		hwlog_err("%s:sdev is null", __func__);
		return 0;
	}
	master = sdev->master;
	if (g_rx_ctx.only_tx)
		g_rx_ctx.spi_status = SPI_LOOP_TX;
	else
		g_rx_ctx.spi_status = SPI_LOOP_RECV;
#ifdef CONFIG_HW_RSMC_QCOM
	set_spi_loop_cb(x800_spi_loop_cb, master);
#endif
	WARN_ON_ONCE(sched_setscheduler_nocheck(master->kworker->task, SCHED_FIFO, &sp) != 0);
	hwlog_info("%s: function=%p", __func__, (spi_loop_cb *)x800_spi_loop_cb);
	up(&g_rx_ctx.enter_sema);
	set_acq_start();
	hwlog_info("%s: exit spi", __func__);
	g_rx_ctx.spi_status = SPI_NORMAL;
	msleep(SPI_MSLEEP_LONG_TIME);
	up(&g_rx_ctx.exit_sema);
	hwlog_info("%s: exit thread", __func__);
	return 0;
}

bool rx_loop_ready(void)
{
	if (g_rx_ctx.spi_status != SPI_NORMAL)
		return true;
	return false;
}

void wait_loop_exit(bool is_tx)
{
	hwlog_info("%s: enter", __func__);
	if (is_tx) {
		if (g_rx_ctx.task != NULL && g_rx_ctx.only_tx) {
			down(&g_rx_ctx.exit_sema);
			g_rx_ctx.task = NULL;
		}
	} else {
		if (g_rx_ctx.task != NULL && g_rx_ctx.spi_status == SPI_NORMAL) {
			down(&g_rx_ctx.exit_sema);
			g_rx_ctx.task = NULL;
		}
	}
	hwlog_info("%s: exit", __func__);
}

static void set_channel_status(struct channel_init_param *param, u32 num)
{
	u32 idx, val, chn_idx;
	int ret;

	if (param == NULL)
		return;
	smc_get_value(ADDR_01_SYS_CFG_1, &val);
	g_rx_ctx.chan_status = (u16)val;
	for (idx = 0; idx < num; idx++) {
		chn_idx = param[idx].chn_idx;
		if (chn_idx >= MAX_CHAN_NUM) {
			hwlog_err("%s:invalid chn_idx", __func__);
			continue;
		}
		ret = memset_s(&g_rx_ctx.info[chn_idx], sizeof(struct chan_info), 0, sizeof(struct chan_info));
		if (ret != EOK) {
			hwlog_err("%s: memset_s failed", __func__);
			return;
		}

		g_rx_ctx.info[chn_idx].mode = MODE_ACQ;
		g_rx_ctx.info[chn_idx].next_mode = MODE_ACQ;
		g_rx_ctx.info[chn_idx].save_mode = CORR_P_ONLY;
		g_rx_ctx.info[chn_idx].tap_idx = 0;
		g_rx_ctx.info[chn_idx].carr_nco_word = param[idx].carr_nco_word;
		g_rx_ctx.info[chn_idx].code_nco_word = param[idx].code_nco_word;
		g_rx_ctx.info[chn_idx].adjust_state = 0;
		g_rx_ctx.info[chn_idx].cnt = 0;
		g_rx_ctx.info[chn_idx].timeout = param[idx].timeout;
		g_rx_ctx.info[chn_idx].ms_idx = param[idx].ms_idx;
		g_rx_ctx.info[chn_idx].valid_acq_msg_idx = 1;
		g_rx_ctx.info[chn_idx].valid_trk_msg_idx = 1;
		g_rx_ctx.info[chn_idx].valid_data_msg_idx = 1;
	}
}

static bool channel_init(struct channel_init_param *chn_init_param, u32 num)
{
	u32 i, value;
	s32 ret;

	if ((chn_init_param == NULL) || (num > MAX_CHAN_NUM || num < 0))
		return false;

	for (i = 0; i < MAX_CHAN_NUM; i++)
		smc_set_value(addr_10_chnl_en(i), 0);

	smc_set_value(ADDR_04_SYS_CTRL, 0x18);

	for (i = 0; i < CORR_FIFO_MAX_CNT; i++) {
		smc_get_value(ADDR_0E_CORR_FIFO, &value);
		if (value == 0)
			break;
	}

	for (i = 0; i < num; i++) {
		struct channel_init_param *cip = &chn_init_param[i];

		if (cip->chn_idx >= MAX_CHAN_NUM) {
			hwlog_err("%s:err chn_idx", __func__);
			continue;
		}

		hwlog_info("%s:chn_idx=%d,g2=%08x,carr=%08x,code=%08x",
			__func__, cip->chn_idx, cip->g2_initphase,
			cip->carr_nco_word, cip->code_nco_word);
		smc_set_value(addr_10_chnl_en(cip->chn_idx), 0);
		smc_set_value(addr_11_chnl_code_param(cip->chn_idx), cip->g2_initphase);

		// carrier nco word discard low 2 bit
		value = (cip->carr_nco_word >> 2) & 0xFFFFFF;
		smc_set_value(addr_15_chnl_carr_nco_fw(cip->chn_idx), value);

		// bit 18~13 chnl_carr_nco_fw_h6 (carrier nco fw high 6 bit)
		value = ((cip->carr_nco_word >> 26) & 0x3F) << 13; // chnl_cnt_init 13 bit
		value |= cip->chnl_cnt_init >> 2;
		smc_set_value(addr_12_chnl_cnt_init(cip->chn_idx), value);

		value = cip->code_nco_word & 0xFFFFFF;
		smc_set_value(addr_13_chnl_code_nco_fw(cip->chn_idx), value);

		// bit 17~10 chnl_code_nco_fw_h8, code nco fw high 8 bit
		// bit 9~8 phs_corr_vl 2bit
		// bit 7~6 phs_corr_l 2bit
		// bit 5~4 phs_corr_p 2bit
		// bit 3~2 phs_corr_e 2bit
		// bit 1~0 phs_corr_ve 2bit
		value = cip->chnl_cnt_init & 0x03;
		value |= (value << 8) | (value << 6) | (value << 4) | (value << 2);
		value ^= 0x88;
		value |= (cip->code_nco_word >> 24) << 10;
		smc_set_value(addr_14_chnl_code_nco_phs(cip->chn_idx), value);
	}
#ifdef PPS_MODE_8PPS
	ret = wait_8pps(false);
#else
	ret = wait_1pps(false);
#endif
	if (ret == PPS_NO)
		return false;
	for (i = 0; i < num; i++)
		smc_set_value(addr_10_chnl_en(chn_init_param[i].chn_idx), 1);
	return true;
}

void send_chn_init_rsp(s32 result)
{
	struct smc_cnf_msg cnf_msg;

	cnf_msg.head.type = CMD_UP_CHN_INIT_CNF;
	cnf_msg.head.module = MODULE_TYPE_CTRL;
	cnf_msg.head.len = sizeof(struct smc_cnf_msg);
	cnf_msg.result = result;

	send_msg_to_ctrl((struct msg_head *)&cnf_msg);
}

void send_chn_close_rsp(s32 result)
{
	struct smc_cnf_msg cnf_msg;

	cnf_msg.head.type = CMD_UP_CHN_CLOSE_CNF;
	cnf_msg.head.module = MODULE_TYPE_CTRL;
	cnf_msg.head.len = sizeof(struct smc_cnf_msg);
	cnf_msg.result = result;

	send_msg_to_ctrl((struct msg_head *)&cnf_msg);
}

void rx_init(struct rx_init_msg *msg)
{
	bool ret = false;

	if (msg == NULL) {
		send_chn_init_rsp(0);
		return;
	} else if (msg->chn_num > MAX_CHAN_NUM) {
		send_chn_init_rsp(0);
		return;
	} else if (rx_loop_ready()) {
		send_chn_init_rsp(0);
		return;
	} else if (!chip_is_ready()) {
		send_chn_init_rsp(0);
		send_soc_err();
		return;
	}
	stop_clk_fd_est();
	set_channel_status(&msg->chn_init_param[0], msg->chn_num);
	set_magic_code(msg);
	enable_rx_ant(true);
	adjust_agc();
	clear_tx_data();
	ret = channel_init(&msg->chn_init_param[0], msg->chn_num);
	if (!ret) {
		enable_rx_ant(false);
		send_chn_init_rsp(0);
		return;
	}
	rf_tx_prepare();
	g_rx_ctx.only_tx = false;
	sema_init(&g_rx_ctx.enter_sema, 0);
	g_rx_ctx.task = kthread_run(rsmc_loop_read, NULL, "rsmc_loop_rx");
	if (g_rx_ctx.task != NULL) {
		sema_init(&g_rx_ctx.exit_sema, 0);
		down(&g_rx_ctx.enter_sema);
		send_chn_init_rsp(1);
	} else {
		enable_rx_ant(false);
		send_chn_init_rsp(0);
	}
}

bool tx_init(void)
{
	bool ret = false;
	if (g_rx_ctx.spi_status == SPI_NORMAL) {
		if (chip_is_ready()) {
			g_rx_ctx.only_tx = true;
			stop_clk_fd_est();
			rf_tx_prepare();
			sema_init(&g_rx_ctx.enter_sema, 0);
			g_rx_ctx.task = kthread_run(rsmc_loop_read, NULL, "rsmc_loop_tx");
			if (g_rx_ctx.task != NULL) {
				sema_init(&g_rx_ctx.exit_sema, 0);
				down(&g_rx_ctx.enter_sema);
				ret = true;
			}
		}
	} else {
		if (!g_rx_ctx.only_tx)
			ret = true;
	}
	hwlog_info("%s: spi %d only_tx %d task %p",
		__func__, (u32)g_rx_ctx.spi_status,
		(u32)g_rx_ctx.only_tx, g_rx_ctx.task);
	return ret;
}

void chan_close(struct rx_init_msg *msg)
{
	u32 i;
	struct chan_info *chn_info = NULL;

	if (msg == NULL)
		return;
	for (i = 0; i < msg->chn_num; i++) {
		struct channel_init_param *chn_init_param = &msg->chn_init_param[i];

		if (chn_init_param->chn_idx >= MAX_CHAN_NUM) {
			hwlog_err("%s: err chn_idx", __func__);
			continue;
		}
		chn_info = &g_rx_ctx.info[chn_init_param->chn_idx];
		hwlog_info("%s: chn_idx:%d,mode=%d",
			__func__, chn_init_param->chn_idx, chn_info->mode);
		chn_info->mode = MODE_IDLE;
		if (g_rx_ctx.spi_status != SPI_NORMAL)
			cmd_write(addr_10_chnl_en(chn_init_param->chn_idx), 0);
		else
			smc_set_value(addr_10_chnl_en(chn_init_param->chn_idx), 0);
	}

	for (i = 0; i < MAX_CHAN_NUM; i++) {
		chn_info = &g_rx_ctx.info[i];
		hwlog_info("%s: channel:%d, mode:%d", __func__, i, chn_info->mode);
		if (chn_info->mode != MODE_IDLE)
			break;
	}
	if (i == MAX_CHAN_NUM) {
		hwlog_info("%s: all channel close", __func__);
		g_rx_ctx.spi_status = SPI_NORMAL;
		wait_loop_exit(false);
		send_chn_close_rsp(1);
		enable_rx_ant(false);
	} else {
		send_chn_close_rsp(0);
	}
	barrier();
}

void acq2track(struct acq2track_msg *msg)
{
	struct chan_info *chn_info = NULL;
	u8 chn_idx;
	s32 delay;
	u32 carr_nco_word, code_nco_word;

	if (msg == NULL)
		return;
	chn_idx = msg->chn_idx;
	delay = msg->delay;
	carr_nco_word = msg->carr_nco_word;
	code_nco_word = msg->code_nco_word;

	if (chn_idx >= MAX_CHAN_NUM) {
		hwlog_err("%s: err chn_idx", __func__);
		return;
	}
	chn_info = &g_rx_ctx.info[chn_idx];
	g_rx_ctx.info[chn_idx].next_mode = MODE_TRK;
	chn_info->carr_nco_word = carr_nco_word;
	chn_info->code_nco_word = code_nco_word;
	chn_info->carr_nco_word_next = carr_nco_word;
	chn_info->code_nco_word_next = code_nco_word;
	g_rx_ctx.info[chn_idx].data_idx = 0;
	hwlog_info("%s: chn_idx:%d,delay:%d,carr:%08x,code:%08x,status:%d,old:%d,diff:%d,new:%d",
		__func__, chn_idx, delay, carr_nco_word, code_nco_word,
		msg->nxt_mode, g_rx_ctx.info[chn_idx].ms_idx, msg->ms_diff,
		(g_rx_ctx.info[chn_idx].ms_idx + msg->ms_diff + FRM_PERIOD) % FRM_PERIOD);
	if (msg->ms_diff != 0) {
		u32 ms_idx = (u32)((s32)g_rx_ctx.info[chn_idx].ms_idx +
			msg->ms_diff + FRM_PERIOD) % FRM_PERIOD;
		g_rx_ctx.info[chn_idx].ms_idx = ms_idx;
	}
	adjust_delay(chn_idx, delay);
}

void track_adjust(u8 chn_idx, u32 carr_nco_word, u32 code_nco_word)
{
	struct chan_info *chn_info = NULL;

	if (chn_idx >= MAX_CHAN_NUM)
		return;
	if (carr_nco_word == 0 || code_nco_word == 0)
		return;
	chn_info = &g_rx_ctx.info[chn_idx];
	hwlog_info("%s:chn_idx:%d,carr:%08x,code:%08x",
		__func__, chn_idx, carr_nco_word, code_nco_word);
	chn_info->carr_nco_word_next = carr_nco_word;
	chn_info->code_nco_word_next = code_nco_word;
}

void update_ms_idx(struct track_adjust_msg *msg)
{
	s32 ms_diff;

	if (msg == NULL)
		return;
	if (msg->chn_idx >= MAX_CHAN_NUM)
		return;
	ms_diff = msg->ms_diff;
	hwlog_info("%s: ms_diff:%d", __func__, ms_diff);
	if (ms_diff != 0) {
		u32 ms_idx = (u32)((s32)g_rx_ctx.info[msg->chn_idx].ms_idx +
			ms_diff + FRM_PERIOD) % FRM_PERIOD;
		g_rx_ctx.info[msg->chn_idx].ms_idx = ms_idx;
	}
}

void enter_nxt_status(void)
{
	u32 send = flash_send_cmd;

	if (g_rx_ctx.cmd.end >= MAX_BUFF_ADDR_LEN - 1) {
		hwlog_info("%s:cmd buff vover lop", __func__);
		return;
	}
	g_rx_ctx.cmd.send[g_rx_ctx.cmd.end] = send;
	g_rx_ctx.cmd.end++;
	g_rx_ctx.cmd.valid = 1;
}

void cmd_read(u32 addr)
{
	u8 tmp = addr | 0x80; // high 31bit set 0 for read
	u32 send = ((tmp & 0xFF) << 24); // high 24-30bit is reg address

	if (g_rx_ctx.cmd.end >= MAX_BUFF_ADDR_LEN - 1) {
		hwlog_info("%s:cmd buff vover lop", __func__);
		return;
	}
	g_rx_ctx.cmd.send[g_rx_ctx.cmd.end] = send;
	g_rx_ctx.cmd.end++;
	g_rx_ctx.cmd.valid = 1;
#ifdef RSMC_DEBUG
	hwlog_info("%s: %X", __func__, addr);
#endif
}

void cmd_write(u32 addr, u32 value)
{
	u32 send;

	barrier();
	send = ((addr & 0xFF) << 24) + value; // high 24-30bit is reg address
	if (g_rx_ctx.cmd.end >= MAX_BUFF_ADDR_LEN - 1) {
		hwlog_info("%s:cmd buff vover lop", __func__);
		return;
	}
	g_rx_ctx.cmd.send[g_rx_ctx.cmd.end] = send;
	g_rx_ctx.cmd.end++;
	g_rx_ctx.cmd.valid = 1;
	barrier();
#ifdef RSMC_DEBUG
	hwlog_info("%s: addr:%X,value:%X", __func__, addr, value);
#endif
}

