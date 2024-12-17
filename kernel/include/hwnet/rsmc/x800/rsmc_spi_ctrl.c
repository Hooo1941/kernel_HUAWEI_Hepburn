/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#include "rsmc_spi_ctrl.h"

#include <huawei_platform/log/hw_log.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

#include "../module_type.h"
#include "rsmc_x800_device.h"

#define SOC_CHECK_TIMES 5

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_SPI_CTRL
HWLOG_REGIST();

struct spi_ctx {
	volatile u32 fd_est_sec;
	volatile bool fd_est_running;
	struct task_struct *fd_task;
	struct semaphore sema;
};
static struct spi_ctx g_spi_ctx;

static void send_to_ctrl(u32 valid, s32 fd)
{
	struct local_fd_msg fd_msg;

	fd_msg.mh.module = MODULE_TYPE_CTRL;
	fd_msg.mh.type = CMD_UP_LOCAL_FD_IND;
	fd_msg.mh.len = sizeof(struct local_fd_msg);
	fd_msg.valid = valid;
	fd_msg.fd = fd;
	send_msg_to_ctrl((struct msg_head *)&fd_msg);
}

int wait_8pps(bool fd_est)
{
	s32 xms_cnt = 0;
	s32 old_xms_cnt = -1;
	u32 value = 0;
	struct reg07_time_param_stru *time_pr = NULL;
	const u32 xms_count = 7;
	const u32 max_xms_count = 255;
	s32 ret = PPS_NO;

	time_pr = (struct reg07_time_param_stru *)&value;
	while (fd_est ? g_spi_ctx.fd_est_running : 1) {
		smc_get_value(ADDR_07_TIMER_PARAM, &value);
		xms_cnt = (s32)time_pr->timer_xms;

		if (xms_cnt == old_xms_cnt)
			continue;

		if (old_xms_cnt < 0) {
			old_xms_cnt = xms_cnt;
			continue;
		}

		if (xms_cnt == 0) {
			if (old_xms_cnt == xms_count) {
				ret = PPS_END;
				break;
			} else if (old_xms_cnt != max_xms_count) {
				ret = PPS_BEGIN;
				break;
			}
		} else if (xms_cnt == 1) {
			if ((old_xms_cnt != max_xms_count) &&
				(old_xms_cnt != 0)) {
				ret = PPS_BEGIN;
				break;
			}
		} else if (xms_cnt > xms_count + 1) {
			hwlog_info("%s:rsmc cur:%d,old:%d",
				__func__, xms_cnt, old_xms_cnt);
			ret = PPS_NO;
			break;
		}
		old_xms_cnt = xms_cnt;
	}
	hwlog_info("%s: fdest running %d 8pps %d",
		__func__, (u32)g_spi_ctx.fd_est_running, ret);
	return ret;
}

int wait_1pps(bool fd_est)
{
	s32 xms_cnt = 0;
	s32 old_xms_cnt = -1;
	u32 value;
	struct reg07_time_param_stru *time_pr = NULL;
	const u32 max_xms_count = 255;
	s32 ret = PPS_NO;

	time_pr = (struct reg07_time_param_stru *)&value;
	while (fd_est ? g_spi_ctx.fd_est_running : true) {
		smc_get_value(ADDR_07_TIMER_PARAM, &value);
		xms_cnt = (s32)time_pr->timer_xms;

		if (xms_cnt == old_xms_cnt)
			continue;

		if (old_xms_cnt < 0) {
			old_xms_cnt = xms_cnt;
			continue;
		}

		if (xms_cnt == 0) {
			if (old_xms_cnt == XMS_COUNT) {
				ret = PPS_END;
				break;
			} else if (old_xms_cnt != max_xms_count) {
				ret = PPS_BEGIN;
				break;
			}
		} else if (xms_cnt == 1) {
			if ((old_xms_cnt != max_xms_count) &&
				(old_xms_cnt != 0)) {
				ret = PPS_BEGIN;
				break;
			}
		} else if (xms_cnt > XMS_COUNT + 1) {
			hwlog_info("%s:rsmc cur:%d,old:%d",
				__func__, xms_cnt, old_xms_cnt);
			ret = PPS_NO;
			break;
		}
		old_xms_cnt = xms_cnt;
	}
	hwlog_info("%s: fdest running %d 1pps %d",
		__func__, (u32)g_spi_ctx.fd_est_running, ret);
	return ret;
}

void fd_calc_8pps(void)
{
	s32 fd;
	s64 offset = 0;
	u32 i, value;
	bool pps_ok = true;
	struct reg08_mod_ctrl_stru *mode_cfg_reg = (struct reg08_mod_ctrl_stru *)&value;

	for (i = 0; i < g_spi_ctx.fd_est_sec * PPS_COUNT; i++) {
		if (wait_8pps(true) != PPS_END) {
			pps_ok = false;
			break;
		}
		smc_get_value(ADDR_08_MOD_CTRL, &value);
		offset += (s64)(mode_cfg_reg->timer_param_pps - OFFSET_8PPS + 1);
		hwlog_info("%s: rsmc offset:%lld,value:%d",
			__func__, offset, mode_cfg_reg->timer_param_pps);
	}
	if (pps_ok) {
		if (((s32)g_spi_ctx.fd_est_sec * REF_CLK + offset) != 0) {
			fd = (s32)((-1) * (offset * RF_HZ) /
				((s32)g_spi_ctx.fd_est_sec * REF_CLK + offset));
			send_to_ctrl(1, fd);
			hwlog_info("%s: fdest <RECORD> PPS CALC FD %d", __func__, fd);
		}
	}
}

void fd_calc_1pps(void)
{
	s32 fd;
	u32 i, value;
	s64 offset = 0;
	bool pps_ok = true;
	struct reg08_mod_ctrl_stru *mode_cfg_reg = (struct reg08_mod_ctrl_stru *)&value;

	for (i = 0; i < g_spi_ctx.fd_est_sec; i++) {
		if (wait_1pps(true) != PPS_END) {
			pps_ok = false;
			break;
		}
		smc_get_value(ADDR_08_MOD_CTRL, &value);
		if (mode_cfg_reg->timer_param_pps < OFFSET0_1PPS)
			offset +=
				(s64)(mode_cfg_reg->timer_param_pps + OFFSET1_1PPS + 1);
		else
			offset +=
				(s64)(mode_cfg_reg->timer_param_pps - OFFSET2_1PPS + 1);
		hwlog_info("%s:rsmc offset:%lld,value:%d",
			__func__, offset, mode_cfg_reg->timer_param_pps);
	}
	if (pps_ok) {
		if (((s32)g_spi_ctx.fd_est_sec * REF_CLK + offset) != 0) {
			fd = (s32)((-1) * (offset * RF_HZ) /
				((s32)g_spi_ctx.fd_est_sec * REF_CLK + offset));
			send_to_ctrl(1, fd);
			hwlog_info("%s: fdest <RECORD> PPS CALC FD %d", __func__, fd);
		}
	}
}

static int fd_est_func(void *data)
{
	hwlog_info("%s: fdest start %d", __func__, g_spi_ctx.fd_est_sec);

#ifdef PPS_MODE_8PPS
	while (g_spi_ctx.fd_est_running)
		fd_calc_8pps();
#else
	while (g_spi_ctx.fd_est_running)
		fd_calc_1pps();
#endif
	hwlog_info("%s: fdest stop %d", __func__, g_spi_ctx.fd_est_sec);
	up(&g_spi_ctx.sema);
	return 0;
}

void stop_clk_fd_est(void)
{
	hwlog_info("%s: enter", __func__);
	g_spi_ctx.fd_est_running = false;
	g_spi_ctx.fd_est_sec = 0;
	if (g_spi_ctx.fd_task != NULL)
		down(&g_spi_ctx.sema);
	g_spi_ctx.fd_task = NULL;
	hwlog_info("%s: exit", __func__);
}

void clk_fd_est(u32 seconds)
{
	hwlog_info("%s: sec,%d", __func__, seconds);
	if (seconds == 0) {
		stop_clk_fd_est();
	} else {
		g_spi_ctx.fd_est_sec = seconds;
		if (g_spi_ctx.fd_task == NULL) {
			g_spi_ctx.fd_est_running = true;
			g_spi_ctx.fd_task = kthread_run(fd_est_func, NULL, "clk_fd_est");
			if (g_spi_ctx.fd_task != NULL)
				sema_init(&g_spi_ctx.sema, 0);
		}
	}
}

void smc_set_value(u32 addr, u32 value)
{
	rsmc_sync_write(addr, value);
}

void smc_get_value(u32 addr, u32 *value)
{
	rsmc_sync_read(addr, value);
}

void chip_init(u32 *reg_value, u32 num)
{
	u32 i;
	u32 addr;
	u32 value;
	struct reg02_sys_cfg2_stru *cfg2 = NULL;

	if (reg_value == NULL)
		return;
	hwlog_info("%s: num:%d", __func__, num);
	for (i = 0; i < num; i++) {
		addr = ((reg_value[i] >> SPI_WORD_DATA_LEN) & 0xFF);
		value = (reg_value[i] & 0xFFFFFF);
		smc_set_value(addr, value);
		hwlog_info("%s: BB SET addr %X value %X",
			__func__, addr, value);
	}

	smc_get_value(ADDR_02_SYS_CFG_2, &value);
	cfg2 = (struct reg02_sys_cfg2_stru *)&value;
	cfg2->swap_iq = 1;
	smc_set_value(ADDR_02_SYS_CFG_2, value);
	hwlog_info("%s: BB SET addr %X value %X",
		__func__, ADDR_02_SYS_CFG_2, value);
	for (i = ADDR_00_SYS_CFG_0; i <= ADDR_0F_VERSION; i++) {
		smc_get_value(i, &value);
		hwlog_info("%s: BB GET addr %X value %X", __func__, i, value);
	}

	hwlog_info("%s: exit", __func__);
}

void rf_init(u32 *rf_value, u32 num)
{
	u32 i, addr, value;

	if (rf_value == NULL)
		return;
	for (i = 0; i < num; i++) {
		addr = (rf_value[i] >> SPI_WORD_DATA_LEN) & 0xFF;
		value = rf_value[i] & 0xFFFFFF;
		smc_set_value(addr, value);
		hwlog_info("%s: RF SET addr %X value %X", __func__, addr, value);
	}
	for (i = ADDR_RF_60; i <= ADDR_RF_7F; i++) {
		smc_get_value(i, &value);
		hwlog_info("%s: RF GET addr %X value %X", __func__, i, value);
	}
}

void update_tx_power(u32 power)
{
	hwlog_err("%s: power %X", __func__, power);
	smc_set_value((power >> SPI_WORD_DATA_LEN) & 0xFF, power & 0xFFFFFF);
}

void update_tx_freq(u32 freqency)
{
	hwlog_info("%s: freq %X", __func__, freqency);
	smc_set_value((freqency >> SPI_WORD_DATA_LEN) & 0xFF, freqency & 0xFFFFFF);
}

void set_chip_mode(int mode)
{
	hwlog_info("%s: mode %d", __func__, mode);
	if (mode == MODE_RX) {
		hwlog_info("%s: rx mode", __func__);
		enable_rx_ant(true);
		enable_tx_ant(false);
	} else if (mode == MODE_TX) {
		hwlog_info("%s: tx mode", __func__);
		enable_rx_ant(false);
		enable_tx_ant(true);
	}
}

bool chip_is_ready_retry(void)
{
	u32 version;
	u32 i;
	int init;
	int tcxo = -1;
	struct smc_core_data *cd = smc_get_core_data();

	for (i = 0; i < SOC_CHECK_TIMES; i++) {
		smc_get_value(ADDR_0F_VERSION, &version);
		init = gpio_get_value(cd->gpios.init_gpio);
		if (init == 0)
			gpio_set_value(cd->gpios.init_gpio, GPIO_HIGH);
		if (gpio_is_valid(cd->gpios.tcxo_pwr_gpio))
			tcxo = gpio_get_value(cd->gpios.tcxo_pwr_gpio);
		hwlog_info("%s:0x%X,init=%d,tcxo=%d", __func__, version, init, tcxo);
		if (version == VERSION)
			return true;
		msleep(SPI_MSLEEP_LONG_TIME);
	}
	return false;
}

bool chip_is_ready(void)
{
	u32 version, i;
	for (i = 0; i < SOC_CHECK_TIMES; i++) {
		smc_get_value(ADDR_0F_VERSION, &version);
		hwlog_info("%s: 0x%X", __func__, version);
		if (version == VERSION)
			return true;
		msleep(SPI_MSLEEP_LONG_TIME);
	}
	return false;
}

void wait_chip_ready(void)
{
	u32 version;

	hwlog_info("%s: enter", __func__);
	while (1) {
		smc_get_value(ADDR_0F_VERSION, &version);
		hwlog_info("%s: version 0x%X", __func__, version);
		if (version == VERSION) {
			hwlog_info("%s: exit", __func__);
			return;
		}
	}
}

void adjust_agc(void)
{
	u32 cnt;
	u32 agc_status;
	u32 agc_set;
	u32 val = 0;
	const u32 loop_cnt = 500; // use 10ms data calculate agc level

	hwlog_info("%s: enter", __func__);
	smc_set_value(ADDR_RF_60, 0x1FF); // open AGC
	for (cnt = 0; cnt < loop_cnt; cnt++) {
		smc_get_value(ADDR_RF_7E, &agc_status);
		// <14:9> RX1_BBAGC
		// <21> RX1_AGC_LOCK
		val += (agc_status >> 9) & 0x3F;
	}
	val = val / loop_cnt + 1;
	smc_set_value(ADDR_RF_60, 0x1FE); // close AGC
	agc_set = 0x580115 | ((val & 0x3F) << 12); // set agc level
	smc_set_value(ADDR_RF_62, agc_set); // set AGC
	hwlog_info("%s: agc set gate:%X,reg:%X", __func__, agc_set, val);
}

void spi_ctrl_init(void)
{
	g_spi_ctx.fd_est_running = false;
	g_spi_ctx.fd_est_sec = 0;
	g_spi_ctx.fd_task = NULL;
}

void spi_ctrl_deinit(void)
{
	hwlog_info("%s", __func__);
}

