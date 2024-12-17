/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#include "rsmc_tx_ctrl.h"

#include <huawei_platform/log/hw_log.h>
#include <linux/delay.h>
#include <securec.h>

#include "../module_type.h"
#include "rsmc_spi_ctrl.h"
#include "rsmc_x800_device.h"
#include "track.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_TX_CTRL
HWLOG_REGIST();

#define SEND_LEVEL_NO_TX 1
#define SEND_LEVEL_INIT_TX_RF 2
#define SEND_LEVEL_INIT_TX_DIGITAL 3
#define SEND_LEVEL_READY_TX 10
#define SEND_LEVEL_INIT_RX_RF 11
#define SEND_LEVEL_FINISH_TX 12

#define DATA_LEN 5000
#define BUFF_SIZE 7
#define INT2BYTE 4

#define EXT_BUF_SIZE 6
#define MAX_SEND_BUFF_LEN 1250

#define TX_RES_FAIL 0
#define TX_RES_SUCC 1
#define TX_RES_PART 2

#define WAIT_FOR_RF_SET_TIMEOUT 10

struct txctrl_ctx {
	volatile bool init;
	volatile struct tx_data_msg msg;
	volatile u32 buf[MAX_SEND_BUFF_LEN];
	volatile u32 idx;
	volatile u32 len;
	volatile bool start_ready;
	volatile u32 bit_len;
	volatile u32 level;
	volatile bool tx_proc;
	volatile bool complete;
	volatile bool tx_succ;
	volatile bool tx_gpio_err;
	volatile bool tx_soc_err;
	bool only_tx;
	bool rf_close;
	bool power_unload;
	u32 reg6c;
	u32 reg6d;
	u32 reg71;
	u32 reg72;
	u32 reg73;
	u32 reg7e;
	u32 reg7f;
	u32 reg70;
};

static struct txctrl_ctx g_tx_ctx;

const static u32 mod_fifo_depth = 7;
static u32 dat_for_send[DATA_LEN] = {0};
static u8 tx_buff[BUFF_SIZE][INT2BYTE] = {0};
static struct spi_transfer tx_trans[BUFF_SIZE] = {0};

void print_buffer(u8 *buff, int len, char *tag)
{
	const u32 offset_limit = 800;
	int index;
	int offset = 0;
	int ret;
	int buff_len = len * 2 + 10; // buffer 10
	char *out = NULL;

	if (buff == NULL || tag == NULL)
		return;
	out = kmalloc(buff_len, GFP_ATOMIC);
	if (out == NULL)
		return;

	for (index = 0; index < len; index++) {
		ret = sprintf_s(out + offset, buff_len, "%02X", buff[index] & 0xFF);
		if (ret < 0) {
			hwlog_info("buff gen is error");
			break;
		}
		offset += ret;
		if (offset >= offset_limit) {
			out[offset] = 0;
			hwlog_info("%s Byte: %s", tag, out);
			offset = 0;
		}
	}
	out[offset] = 0;
	hwlog_info("%s Byte:%s", tag, out);
	kfree(out);
}

static u32 revert_24bit(u32 din)
{
	const u32 bit_len = 24;
	u32 dout = 0;
	s32 i;

	for (i = 0; i < bit_len; i++)
		dout |= ((din >> i) & 0x01) << (bit_len - i - 1);
	return dout;
}

static u32 conv32b24b(u32 data[], u32 idx)
{
	u32 i1, i2, retd;
	const u32 bitlen8 = 8;
	const u32 bitlen16 = 16;
	const u32 bitlen24 = 24;

	i1 = idx / 4 * 3;
	i2 = idx % 4;
	if (i2 == 0)
		retd = (data[i1] >> bitlen8) & 0xffffff;
	else if (i2 == 1)
		retd = ((data[i1] & 0xff) << bitlen16) |
			((data[i1 + 1] >> bitlen16) & 0xffff);
	else if (i2 == 2)
		retd = ((data[i1 + 1] & 0xffff) << bitlen8) |
			((data[i1 + 2] >> bitlen24) & 0xff);
	else
		retd = (data[i1 + 2] & 0xffffff);
	return retd;
}

static u32 get_data(u32 data[], u32 idx)
{
	return revert_24bit(conv32b24b(data, idx));
}

static void tx_config(struct tx_data_msg *msg)
{
	u32 dat_rd, frame_len;
	u32 *trans_frame = NULL;
	int ret;
	struct reg05_sys_status_stru *status_value = NULL;

	if (msg == NULL)
		return;
	trans_frame = (u32 *)(msg + 1);
	frame_len = msg->frame_len;
	ret = memset_s(dat_for_send, sizeof(dat_for_send), 0x00, sizeof(dat_for_send));
	if (ret != EOK) {
		hwlog_err("%s: memset_s failed", __func__);
		return;
	}
	ret = memcpy_s(dat_for_send, sizeof(dat_for_send), trans_frame, frame_len);
	if (ret != EOK) {
		hwlog_err("%s: memcpy_s failed", __func__);
		return;
	}

	print_buffer((u8 *)trans_frame, frame_len, "RECEIVE DATA");

	smc_set_value(ADDR_03_SYS_CFG_3, msg->m_phs);
	smc_set_value(ADDR_09_MOD_CODE_NCO_FW_H,
		((msg->sync_len & 0xFFFF)<<8) | ((msg->code_nco>>24) & 0xFF));
	smc_set_value(ADDR_0A_MOD_CODE_NCO_FW_L, msg->code_nco & 0xFFFFFF);
	smc_set_value(ADDR_0B_MOD_CODE_PHS, msg->m2_phs);
	smc_set_value(ADDR_0C_MOD_INFO_BIT_LEN, msg->info_len);

	smc_set_value(ADDR_04_SYS_CTRL, 0b100); // reset modem FIFO
	// clear modulation err reg
	smc_get_value(ADDR_04_SYS_CTRL, &dat_rd);
	smc_set_value(ADDR_04_SYS_CTRL, dat_rd | (0x01 << 4));

	smc_get_value(ADDR_05_SYS_STATUS, &dat_rd);
	status_value = (struct reg05_sys_status_stru *)&dat_rd;
	if (status_value->mod_err) {
		hwlog_info("%s: mod_cnt %d", __func__, status_value->mod_cnt);
		hwlog_info("%s: mod_err %d", __func__, status_value->mod_err);
		hwlog_info("%s: mod_end %d", __func__, status_value->mod_end);
	}
}

static int do_tx(struct tx_data_msg *msg)
{
	u32 dat_rd, dat_wr, total_word, i;
	u32 dat_idx = 0;
	struct reg05_sys_status_stru *sts = NULL;

	if (msg == NULL)
		return 0;
	smc_get_value(ADDR_05_SYS_STATUS, &dat_rd);
	sts = (struct reg05_sys_status_stru *)&dat_rd;

	for (i = 1; i < mod_fifo_depth - sts->mod_cnt; i++)
		smc_set_value(ADDR_0D_MOD_FIFO_DATA, get_data(dat_for_send, dat_idx++));

	// start modem
	smc_set_value(ADDR_04_SYS_CTRL, 0b111);
	smc_get_value(ADDR_05_SYS_STATUS, &dat_rd);
	total_word = ((msg->sync_len + msg->info_len) / SPI_WORD_DATA_LEN) + EXT_BUF_SIZE;
	while (1) {
		if (dat_idx >= total_word)
			break;
		if (sts->mod_err) {
			hwlog_err("%s: FIFO ERROR,cnt=%d,empty=%d,full=%d,end=%d",
				__func__, sts->mod_cnt, sts->mod_empty,
				sts->mod_full, sts->mod_end);
			return 0;
		}
		if (mod_fifo_depth - sts->mod_cnt > 1) {
			for (i = 0; i < (mod_fifo_depth - sts->mod_cnt - 1); i++) {
				dat_wr = get_data(dat_for_send, dat_idx++);
				tx_buff[i][1] = ADDR_0D_MOD_FIFO_DATA & 0x7F;
				tx_buff[i][0] = (dat_wr >> 16) & 0xFF;
				tx_buff[i][3] = (dat_wr >> 8) & 0xFF;
				tx_buff[i][2] = dat_wr & 0xFF;
				tx_trans[i].tx_buf = tx_buff[i];
				tx_trans[i].rx_buf = tx_buff[i];
				tx_trans[i].len = sizeof(u32);
			}
			tx_buff[i][1] = ADDR_05_SYS_STATUS | 0x80;
			tx_trans[i].tx_buf = tx_buff[i];
			tx_trans[i].rx_buf = tx_buff[i];
			tx_trans[i].len = sizeof(u32);
			rsmc_sync_read_write(tx_trans,
				mod_fifo_depth - sts->mod_cnt);
			dat_rd =
				(tx_buff[i][IDX_0] << 16) |
				(tx_buff[i][IDX_3] << 8) |
				tx_buff[i][IDX_2];
		} else {
			smc_get_value(ADDR_05_SYS_STATUS, &dat_rd);
		}
	}
	return 1;
}

static void wait_complete(void)
{
	struct reg05_sys_status_stru *sts = NULL;
	u32 dat_rd;

	hwlog_info("%s: enter", __func__);
	while (1) {
		smc_get_value(ADDR_05_SYS_STATUS, &dat_rd);
		sts = (struct reg05_sys_status_stru *)&dat_rd;
		if (sts->mod_end) {
			hwlog_info("%s: mod_end %d", __func__, sts->mod_end);
			break;
		}
	}
	hwlog_info("%s: send FINISH", __func__);
}

int send_data(struct tx_data_msg *msg)
{
	int rc;

	if (msg == NULL)
		return 0;
	tx_config(msg);
	rc = do_tx(msg);
	if (!rc)
		return rc;
	wait_complete();
	return 1;
}

static int send_init(void)
{
	int i;
	u32 idx;

	hwlog_info("%s: frq=%X,m=%X,m1=%X,code=%X,syn_len=%d,inf_len=%d",
		__func__, g_tx_ctx.msg.freq_point,
		g_tx_ctx.msg.m_phs, g_tx_ctx.msg.m2_phs,
		g_tx_ctx.msg.code_nco, g_tx_ctx.msg.sync_len,
		g_tx_ctx.msg.info_len);
	cmd_write(ADDR_03_SYS_CFG_3, g_tx_ctx.msg.m_phs);
	cmd_write(ADDR_04_SYS_CTRL, 0b00100);
	cmd_write(ADDR_04_SYS_CTRL, 0b10110);

	// bit 23~8 mod_sync_bit_len
	// bit 7~0 mod_code_nco_fw
	cmd_write(ADDR_09_MOD_CODE_NCO_FW_H,
		((g_tx_ctx.msg.sync_len & 0xFFFF) << 8) |
		((g_tx_ctx.msg.code_nco >> 24) & 0xFF));
	cmd_write(ADDR_0A_MOD_CODE_NCO_FW_L, g_tx_ctx.msg.code_nco & 0xFFFFFF);
	cmd_write(ADDR_0B_MOD_CODE_PHS, g_tx_ctx.msg.m2_phs);
	if (g_tx_ctx.msg.rf_tx_aft_on &&
		g_tx_ctx.msg.power_unload > 0) {
		cmd_write(ADDR_0C_MOD_INFO_BIT_LEN,
			g_tx_ctx.msg.info_len + SPI_WORD_DATA_LEN + SPI_WORD_DATA_LEN);
	} else if (g_tx_ctx.msg.rf_tx_aft_on ||
		g_tx_ctx.msg.power_unload > 0) {
		cmd_write(ADDR_0C_MOD_INFO_BIT_LEN,
			g_tx_ctx.msg.info_len + SPI_WORD_DATA_LEN);
	} else {
		cmd_write(ADDR_0C_MOD_INFO_BIT_LEN, g_tx_ctx.msg.info_len);
	}
	for (i = 1; i < mod_fifo_depth; i++) {
		idx = g_tx_ctx.idx;
		cmd_write(ADDR_0D_MOD_FIFO_DATA, get_data((s32 *)g_tx_ctx.buf, idx));
		g_tx_ctx.idx++;
	}
	cmd_write(ADDR_04_SYS_CTRL, 0b111);
	return 0;
}

static void set_tx_freq(u32 freqency)
{
	hwlog_info("%s: freq %X", __func__, freqency);
	cmd_write((freqency >> SPI_WORD_DATA_LEN) & 0xFF, freqency & 0xFFFFFF);
}

static void set_tx_power(u32 power)
{
	hwlog_info("%s: power %X", __func__, power);
	cmd_write((power >> SPI_WORD_DATA_LEN) & 0xFF, power & 0xFFFFFF);
}

void init_tx(void)
{
	hwlog_info("%s:enter", __func__);
	g_tx_ctx.level = SEND_LEVEL_NO_TX;
	enter_nxt_status();
	// Need Set RF TX
	if (!g_tx_ctx.only_tx)
		refresh_nco();
	set_tx_power(g_tx_ctx.msg.power);
	set_tx_freq(g_tx_ctx.msg.freqency);
	cmd_write((g_tx_ctx.msg.obw >> SPI_WORD_DATA_LEN) & 0xFF,
		g_tx_ctx.msg.obw & 0xFFFFFF);
	cmd_write((g_tx_ctx.msg.obw_work >> SPI_WORD_DATA_LEN) & 0xFF,
		g_tx_ctx.msg.obw_work & 0xFFFFFF);
	if (g_tx_ctx.msg.rf_tx_aft_on)
		cmd_write(ADDR_RF_68, REG68_RF_OPEN);
	if (g_tx_ctx.msg.rf_tx_pre_on) {
		struct reg72_stru *reg72_value = NULL;
		u32 reg72 = g_tx_ctx.reg72;
		u32 reg73 = g_tx_ctx.reg73;

		reg72_value = (struct reg72_stru *)&reg72;
		reg72_value->afc_start = 1;
		cmd_write(ADDR_RF_6C, reg72);
		cmd_write(ADDR_RF_6D, reg73);
	}
	enter_nxt_status();
	if (g_tx_ctx.msg.rf_tx_pre_on) {
		cmd_read(ADDR_RF_7E);
		cmd_read(ADDR_RF_7F);
		cmd_read(ADDR_RF_70);
		cmd_read(ADDR_RF_71);
	}
	save_sign_buff();
	if (!g_tx_ctx.msg.rf_tx_pre_on)
		send_init();
	enter_nxt_status();
}

int start_tx(void)
{
	if (!g_tx_ctx.init) {
		init_tx();
		g_tx_ctx.init = true;
	}
	if (g_tx_ctx.start_ready) {
		cmd_read(ADDR_05_SYS_STATUS);
		return 1; // has tx data
	}
	return 0;
}

void set_tx_err(void)
{
	hwlog_info("%s", __func__);
	g_tx_ctx.tx_soc_err = true;
}

void rf_tx_prepare(void)
{
	u32 reg7e;
	struct reg7e_stru *reg7e_value = (struct reg7e_stru *)&reg7e;
	smc_get_value(ADDR_RF_6C, &g_tx_ctx.reg6c);
	smc_get_value(ADDR_RF_6D, &g_tx_ctx.reg6d);
	smc_get_value(ADDR_RF_7E, &reg7e);
	smc_get_value(ADDR_RF_72, &g_tx_ctx.reg72);
	smc_get_value(ADDR_RF_73, &g_tx_ctx.reg73);
	smc_get_value(ADDR_RF_7F, &g_tx_ctx.reg7f);
	smc_get_value(ADDR_RF_70, &g_tx_ctx.reg70);
	smc_get_value(ADDR_RF_71, &g_tx_ctx.reg71);
	hwlog_info("%s: REG6C:%X,REG6D:%X,REG72:%X,REG73:%X,REG7E:%X,REG7F:%X,REG70:%X,REG71:%X",
		__func__, g_tx_ctx.reg6c, g_tx_ctx.reg6d, g_tx_ctx.reg72,
		g_tx_ctx.reg73, reg7e, g_tx_ctx.reg7f, g_tx_ctx.reg70, g_tx_ctx.reg71);
	hwlog_info("%s: REG7E:%X,LDX:%d,AFC_OK:%d",
		__func__, reg7e, reg7e_value->ldx, reg7e_value->afc_ok);
}

void set_rf(u32 addr, u32 value)
{
	if (g_tx_ctx.msg.rf_tx_pre_on) {
		struct reg7e_stru *reg7e_value = NULL;
		struct reg7f_stru *reg7f_value = NULL;
		struct reg70_stru *reg70_value = NULL;
		struct reg71_stru *reg71_value = NULL;
		if (addr == ADDR_RF_7E) {
			reg7e_value = (struct reg7e_stru *)&value;
			hwlog_info("%s: REG7E:%X,LDX:%d,AFC_OK:%d",
				__func__, value, reg7e_value->ldx, reg7e_value->afc_ok);
		} else if (addr == ADDR_RF_7F) {
			g_tx_ctx.reg7f = value;
			hwlog_info("%s: REG7F:%X", __func__, value);
		} else if (addr == ADDR_RF_70) {
			g_tx_ctx.reg70 = value;
			hwlog_info("%s: REG70:%X", __func__, value);
		} else if (addr == ADDR_RF_71) {
			u32 reg7f = g_tx_ctx.reg7f;
			u32 reg70 = g_tx_ctx.reg70;
			u32 reg71 = value;
			g_tx_ctx.reg71 = value;
			hwlog_info("%s: REG71:%X", __func__, value);

			reg7f_value = (struct reg7f_stru *)&reg7f;
			reg70_value = (struct reg70_stru *)&reg70;
			reg70_value->cs_vco = reg7f_value->vco_cs;
			cmd_write(ADDR_RF_70, reg70);

			reg71_value = (struct reg71_stru *)&reg71;
			reg71_value->afc_en = 0;
			cmd_write(ADDR_RF_71, reg71);
			send_init();
		}
	}
}

void init_tx_data(struct tx_data_msg *msg, bool only_tx)
{
	u32 frame_len;
	int ret;

	if (msg == NULL)
		return;
	if (g_tx_ctx.tx_proc)
		return;
	if (msg->frame_len > MAX_SEND_BUFF_LEN) {
		hwlog_err("%s: invalid frame len", __func__);
		return;
	}
	frame_len = msg->frame_len;
	ret = memcpy_s((void *)g_tx_ctx.buf, sizeof(g_tx_ctx.buf), (void *)(msg + 1), frame_len);
	if (ret != EOK) {
		hwlog_err("%s: memcpy_s failed", __func__);
		return;
	}
	ret = memcpy_s((void *)&g_tx_ctx.msg, sizeof(struct tx_data_msg), (void *)msg, sizeof(struct tx_data_msg));
	if (ret != EOK) {
		hwlog_err("%s: memcpy_s failed", __func__);
		return;
	}

	barrier();
	g_tx_ctx.init = false;
	g_tx_ctx.idx = 0;
	g_tx_ctx.start_ready = false;
	g_tx_ctx.tx_proc = true;
	g_tx_ctx.complete = false;
	g_tx_ctx.tx_succ = false;
	g_tx_ctx.tx_gpio_err = false;
	g_tx_ctx.tx_soc_err = false;
	g_tx_ctx.only_tx = only_tx;
	g_tx_ctx.rf_close = false;
	g_tx_ctx.power_unload = false;

	hwlog_info("%s: f:%X,m:%X,m1:%X,code:%X,sl:%d,fl:%d,pre:%d,aft:%d",
		__func__, (u32)(msg->freq_point),
		(u32)(msg->m_phs), (u32)(msg->m2_phs),
		(u32)(msg->code_nco), (u32)(msg->sync_len),
		frame_len, msg->rf_tx_pre_on, msg->rf_tx_aft_on);
}

void clear_tx_data(void)
{
	g_tx_ctx.init = true;
	g_tx_ctx.tx_proc = false;
}

bool tx_complete(void)
{
	return !g_tx_ctx.tx_proc;
}

u32 rf_close_and_power_unload(u32 value)
{
	u32 loop_cnt;
	struct reg05_sys_status_stru *sts = (struct reg05_sys_status_stru *)&value;
	u32 wd_cnt = (g_tx_ctx.bit_len + SPI_WORD_DATA_LEN + SPI_WORD_DATA_LEN) / SPI_WORD_DATA_LEN;
	u32 wd_real_cnt = g_tx_ctx.bit_len / SPI_WORD_DATA_LEN;
	bool wd = ((g_tx_ctx.bit_len + SPI_WORD_DATA_LEN + SPI_WORD_DATA_LEN) % SPI_WORD_DATA_LEN == 0);
	bool wd_real = (g_tx_ctx.bit_len % SPI_WORD_DATA_LEN == 0);
	u32 loop_real = wd_real ? wd_real_cnt : (wd_real_cnt + 1);
	loop_cnt = wd ? wd_cnt : (wd_cnt + 1);
	if (!g_tx_ctx.power_unload) {
		if (g_tx_ctx.idx - sts->mod_cnt >= loop_real) {
			set_tx_power(g_tx_ctx.msg.power_unload);
			g_tx_ctx.power_unload = true;
			hwlog_info("%s: power unload,%d,%d,%d",
				__func__, g_tx_ctx.idx, sts->mod_cnt, loop_real);
		}
	}
	if (!g_tx_ctx.rf_close) {
		if (g_tx_ctx.idx - sts->mod_cnt >= loop_real + 1) {
			cmd_write(ADDR_RF_68, REG68_RF_CLOSE);
			g_tx_ctx.rf_close = true;
			hwlog_info("%s: rf close,%d,%d,%d",
				__func__, g_tx_ctx.idx, sts->mod_cnt, loop_real + 1);
		}
	}
	return loop_cnt;
}

u32 rf_close(u32 value)
{
	u32 loop_cnt;
	struct reg05_sys_status_stru *sts = (struct reg05_sys_status_stru *)&value;
	u32 wd_cnt = (g_tx_ctx.bit_len + SPI_WORD_DATA_LEN) / SPI_WORD_DATA_LEN;
	u32 wd_real_cnt = g_tx_ctx.bit_len / SPI_WORD_DATA_LEN;
	bool wd = ((g_tx_ctx.bit_len + SPI_WORD_DATA_LEN) % SPI_WORD_DATA_LEN == 0);
	bool wd_real = (g_tx_ctx.bit_len % SPI_WORD_DATA_LEN == 0);
	u32 loop_real = wd_real ? wd_real_cnt : (wd_real_cnt + 1);
	loop_cnt = wd ? wd_cnt : (wd_cnt + 1);
	if (!g_tx_ctx.rf_close) {
		if (g_tx_ctx.idx - sts->mod_cnt >= loop_real) {
			cmd_write(ADDR_RF_68, REG68_RF_CLOSE);
			g_tx_ctx.rf_close = true;
			hwlog_info("%s: rf close,%d,%d,%d",
				__func__, g_tx_ctx.idx, sts->mod_cnt, loop_real);
		}
	}
	return loop_cnt;
}

u32 power_unload(u32 value)
{
	u32 loop_cnt;
	struct reg05_sys_status_stru *sts = (struct reg05_sys_status_stru *)&value;
	u32 wd_cnt = (g_tx_ctx.bit_len + SPI_WORD_DATA_LEN) / SPI_WORD_DATA_LEN;
	u32 wd_real_cnt = g_tx_ctx.bit_len / SPI_WORD_DATA_LEN;
	bool wd = ((g_tx_ctx.bit_len + SPI_WORD_DATA_LEN) % SPI_WORD_DATA_LEN == 0);
	bool wd_real = (g_tx_ctx.bit_len % SPI_WORD_DATA_LEN == 0);
	u32 loop_real = wd_real ? wd_real_cnt : (wd_real_cnt + 1);
	loop_cnt = wd ? wd_cnt : (wd_cnt + 1);
	if (!g_tx_ctx.power_unload) {
		if (g_tx_ctx.idx - sts->mod_cnt >= loop_real) {
			set_tx_power(g_tx_ctx.msg.power_unload);
			g_tx_ctx.power_unload = true;
			hwlog_info("%s: power unload,%d,%d,%d",
				__func__, g_tx_ctx.idx, sts->mod_cnt, loop_real);
		}
	}
	return loop_cnt;
}

u32 rf_tx_aft_init(u32 value)
{
	u32 loop_cnt;
	if (g_tx_ctx.msg.rf_tx_aft_on && g_tx_ctx.msg.power_unload > 0) {
		loop_cnt = rf_close_and_power_unload(value);
	} else if (g_tx_ctx.msg.rf_tx_aft_on) {
		loop_cnt = rf_close(value);
	} else if (g_tx_ctx.msg.power_unload > 0) {
		loop_cnt = power_unload(value);
	} else {
		u32 wd_cnt = g_tx_ctx.bit_len / SPI_WORD_DATA_LEN;
		bool wd = (g_tx_ctx.bit_len % SPI_WORD_DATA_LEN == 0);
		loop_cnt = wd ? wd_cnt : (wd_cnt + 1);
	}
	return loop_cnt;
}

void restore_rf(void)
{
	if (g_tx_ctx.msg.rf_tx_pre_on) {
		cmd_write(ADDR_RF_71, g_tx_ctx.reg71);
		cmd_write(ADDR_RF_6C, g_tx_ctx.reg6c);
		cmd_write(ADDR_RF_6D, g_tx_ctx.reg6d);
	}
}

void tx_update_buf(u32 value)
{
	u32 tx_dat, i;
	u32 loop_cnt;
	struct reg05_sys_status_stru *sts = (struct reg05_sys_status_stru *)&value;
	if (value == 0 || g_tx_ctx.tx_gpio_err || g_tx_ctx.tx_soc_err) {
		hwlog_err("%s: status:%x,gpio_err:%d,soc_err:%d",
			__func__, value, (u32)g_tx_ctx.tx_gpio_err, (u32)g_tx_ctx.tx_soc_err);
		restore_rf();
		enter_nxt_status();
		g_tx_ctx.start_ready = false;
		g_tx_ctx.tx_succ = false;
		return;
	}
	loop_cnt = rf_tx_aft_init(value);
	if (!g_tx_ctx.complete && g_tx_ctx.idx >= loop_cnt) {
		g_tx_ctx.complete = true;
		hwlog_info("%s: add fifo complete,%d", __func__, g_tx_ctx.idx);
	}

	if (!g_tx_ctx.complete && sts->mod_err) {
		hwlog_info("%s: cnt:%d,err:%d,end:%d,full:%d,empty:%d", __func__,
			sts->mod_cnt, sts->mod_err, sts->mod_end, sts->mod_full, sts->mod_empty);
		hwlog_info("%s:send fail,%d", __func__, g_tx_ctx.idx);
		restore_rf();
		enter_nxt_status();
		g_tx_ctx.start_ready = false;
		g_tx_ctx.tx_succ = false;
		return;
	} else if (g_tx_ctx.complete && sts->mod_end) {
		hwlog_info("%s: cnt:%d,err:%d,end:%d,full:%d,empty:%d", __func__,
			sts->mod_cnt, sts->mod_err, sts->mod_end, sts->mod_full, sts->mod_empty);
		hwlog_info("%s:send succ,%d", __func__, g_tx_ctx.idx);
		restore_rf();
		enter_nxt_status();
		g_tx_ctx.start_ready = false;
		g_tx_ctx.tx_succ = true;
		return;
	}

	if (g_tx_ctx.complete)
		return;
	if (mod_fifo_depth - sts->mod_cnt <= 1)
		return;
	for (i = 1; i < (mod_fifo_depth - sts->mod_cnt); i++) {
		tx_dat = get_data((s32 *)g_tx_ctx.buf, g_tx_ctx.idx);
		cmd_write(ADDR_0D_MOD_FIFO_DATA, tx_dat);
		g_tx_ctx.idx++;
		if (g_tx_ctx.idx == loop_cnt / TIMES_2 &&
			g_tx_ctx.msg.power != g_tx_ctx.msg.power_change)
			set_tx_power(g_tx_ctx.msg.power_change);
	}
}

void tx_sm_set(void)
{
	switch (g_tx_ctx.level) {
	case SEND_LEVEL_NO_TX:
		hwlog_info("%s:init TX RF", __func__);
		// Need to RF SPI
		if (!g_tx_ctx.only_tx)
			enable_rx_ant(false);
		if (!enable_tx_ant(true))
			g_tx_ctx.tx_gpio_err = true;
		g_tx_ctx.level = SEND_LEVEL_INIT_TX_RF;
		break;
	case SEND_LEVEL_INIT_TX_RF:
		hwlog_info("%s:init TX digital", __func__);
		msleep(WAIT_FOR_RF_SET_TIMEOUT);
		// Need to BB SPI
		g_tx_ctx.level = SEND_LEVEL_INIT_TX_DIGITAL;
		break;
	case SEND_LEVEL_INIT_TX_DIGITAL:
		hwlog_info("%s:enter TX send", __func__);
		g_tx_ctx.start_ready = true;
		g_tx_ctx.level = SEND_LEVEL_INIT_RX_RF;
		break;
	case SEND_LEVEL_READY_TX:
		hwlog_info("%s:TX finish,convert to RX", __func__);
		if (!g_tx_ctx.only_tx)
			enable_rx_ant(true);
		enable_tx_ant(false);
		// Need Set RF
		enter_nxt_status();
		g_tx_ctx.level = SEND_LEVEL_INIT_RX_RF;
		break;
	case SEND_LEVEL_INIT_RX_RF:
		hwlog_info("%s:set RX", __func__);
		// Need to BB SPI
		if (!g_tx_ctx.only_tx)
			enable_rx_ant(true);
		enable_tx_ant(false);
		tx_rsp_ok();
		g_tx_ctx.level = SEND_LEVEL_FINISH_TX;
		g_tx_ctx.tx_proc = false;
		break;
	default:
		hwlog_info("%s:status error", __func__);
		break;
	}
}

void save_sign_buff(void)
{
	u32 frame_len = g_tx_ctx.msg.frame_len;

	if (frame_len > MAX_SEND_BUFF_LEN) {
		hwlog_info("frame buf len is error");
		return;
	}
	g_tx_ctx.len = frame_len;
	g_tx_ctx.idx = 0;
	g_tx_ctx.bit_len = g_tx_ctx.msg.sync_len +
		g_tx_ctx.msg.info_len;
	if (g_tx_ctx.bit_len > MAX_SEND_BUFF_LEN * 8) {
		hwlog_info("info buf len is error");
		return;
	}
}

void tx_rsp_ok(void)
{
	struct smc_cnf_msg cnf_msg;

	cnf_msg.head.type = CMD_UP_TX_CNF;
	cnf_msg.head.module = MODULE_TYPE_CTRL;
	cnf_msg.head.len = sizeof(struct smc_cnf_msg);
	if (g_tx_ctx.tx_succ)
		cnf_msg.result = TX_RES_SUCC;
	else if (g_tx_ctx.idx > 0)
		cnf_msg.result = TX_RES_PART;
	else
		cnf_msg.result = TX_RES_FAIL;

	send_msg_to_ctrl((struct msg_head *)&cnf_msg);
}

