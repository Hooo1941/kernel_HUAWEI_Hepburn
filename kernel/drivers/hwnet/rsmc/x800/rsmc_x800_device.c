/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This module is used to start the driver peripheral
 * and identify the peripheral chip type. The operations on smc_comm
 * are adapted step by step on the Acousto-Electronic BeiDou short packet
 * chip platform. Peripheral startup is controlled by the compilation
 * macro switch, Other peripheral startups depend on this module.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-22
 */

#include "rsmc_x800_device.h"

#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HW_RSMC_QCOM
#include <huawei_platform/power/huawei_charger_adaptor.h>
#endif
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/spi/spi.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/timer.h>

#include "../module_type.h"
#include "rsmc_spi_drv.h"
#include "rsmc_spi_ctrl.h"
#include "rsmc_tx_ctrl.h"
#include "rsmc_rx_ctrl.h"
#include "track.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_X800_DEVICE
HWLOG_REGIST();

#define EXPIRE_TIME 650
#define DEVICE_REG_TRY_MAX_COUNT 100
#define DEVICE_REG_SLEEP_INTERVAL 1000
#define SMC_PROBE_SLEEP_INTERVAL 2000
#define DC_SET_DISABLE_FLAGS 1
#define DC_CLEAR_DISABLE_FLAGS 0
#define DC_DISABLE_RSMC 4
#define LIMIT_VALUE 2000
#define LIMIT_VALUE_VALID (-1)

struct x800_ctx {
	notify_event *notifier;
	int trans_mode;
	bool is_chip_on;
	u32 mode_sel;
	struct mutex access_mutex;
};
static struct x800_ctx g_x800_ctx;

bool cmd_is_block(void)
{
	if (g_x800_ctx.trans_mode != 0)
		return true;
	return false;
}

static void chip_power_on(bool status)
{
	struct smc_core_data *cd = smc_get_core_data();
	int value;

	if (cd == NULL) {
		hwlog_err("%s: cd null", __func__);
		return;
	}
	hwlog_info("%s: status %d", __func__, (int)status);
	if (status) {
		if (gpio_is_valid(cd->gpios.tcxo_pwr_gpio)) {
			gpio_set_value(cd->gpios.tcxo_pwr_gpio, GPIO_HIGH);
			if (cd->gpios.tcxo_pwr_after_delay_ms > 0)
				msleep(cd->gpios.tcxo_pwr_after_delay_ms);
		}
		if (gpio_is_valid(cd->gpios.chip_pwr_gpio))
			gpio_set_value(cd->gpios.chip_pwr_gpio, GPIO_HIGH);
		gpio_set_value(cd->gpios.init_gpio, GPIO_HIGH);
		gpio_set_value(cd->gpios.pa_pwr_gpio, GPIO_HIGH);
	} else {
		gpio_set_value(cd->gpios.pa_pwr_gpio, GPIO_LOW);
		gpio_set_value(cd->gpios.init_gpio, GPIO_LOW);
		if (gpio_is_valid(cd->gpios.chip_pwr_gpio))
			gpio_set_value(cd->gpios.chip_pwr_gpio, GPIO_LOW);
		if (gpio_is_valid(cd->gpios.tcxo_pwr_gpio))
			gpio_set_value(cd->gpios.tcxo_pwr_gpio, GPIO_LOW);
	}

	value = gpio_get_value(cd->gpios.init_gpio);
	hwlog_info("%s: init_gpio value %d", __func__, value);
	if (gpio_is_valid(cd->gpios.chip_pwr_gpio)) {
		value = gpio_get_value(cd->gpios.chip_pwr_gpio);
		hwlog_info("%s: chip_pwr_gpio value %d", __func__, value);
	}
	if (gpio_is_valid(cd->gpios.tcxo_pwr_gpio)) {
		value = gpio_get_value(cd->gpios.tcxo_pwr_gpio);
		hwlog_info("%s: tcxo_pwr_gpio value %d delay %d",
			__func__, value, cd->gpios.tcxo_pwr_after_delay_ms);
	}
}

static int smc_enable(struct enable_msg *msg)
{
	if (msg == NULL)
		return -EINVAL;
	if (g_x800_ctx.is_chip_on) {
		struct rx_init_msg rx_msg;
		u32 i;

		rx_msg.chn_num = MAX_CHAN_NUM;
		for (i = 0; i < MAX_CHAN_NUM; i++)
			rx_msg.chn_init_param[i].chn_idx = i;
		stop_clk_fd_est();
		chan_close(&rx_msg);
		msleep(SPI_MSLEEP_LONG_TIME);
		if (!chip_is_ready())
			return -EINVAL;
	} else {
		g_x800_ctx.trans_mode = msg->mode;
		chip_power_on(true);
		if (!chip_is_ready_retry())
			return -EINVAL;

		rf_init(msg->rf_value, msg->rf_num);

		if (!chip_is_ready())
			return -EINVAL;
	}
	chip_init(msg->reg_value, msg->reg_num);
	g_x800_ctx.is_chip_on = true;
	rsmc_start_heartbeat();
#ifdef CONFIG_HW_RSMC_QCOM
	huawei_charger_set_dc_disable_flags(DC_SET_DISABLE_FLAGS,
		DC_DISABLE_RSMC);
	huawei_charger_set_rsmc_limit(LIMIT_VALUE);
	hwlog_info("%s: start charging limit flag = %d, type = %d, value = %d",
		__func__, DC_SET_DISABLE_FLAGS, DC_DISABLE_RSMC, LIMIT_VALUE);
#endif
	return 0;
}

static int smc_disable(struct enable_msg *msg)
{
	hwlog_info("%s: enter", __func__);
	if (g_x800_ctx.is_chip_on) {
		struct rx_init_msg rx_msg;
		u32 i;

		rx_msg.chn_num = MAX_CHAN_NUM;
		for (i = 0; i < MAX_CHAN_NUM; i++)
			rx_msg.chn_init_param[i].chn_idx = i;
		enable_tx_ant(false);
		enable_rx_ant(false);
		stop_clk_fd_est();
		rsmc_stop_heartbeat();
		chan_close(&rx_msg);
		msleep(SPI_MSLEEP_LONG_TIME);
		chip_power_on(false);
		rsmc_clear_dn_msg_list();
		g_x800_ctx.is_chip_on = false;
	}
#ifdef CONFIG_HW_RSMC_QCOM
	huawei_charger_set_dc_disable_flags(DC_CLEAR_DISABLE_FLAGS,
		DC_DISABLE_RSMC);
	huawei_charger_set_rsmc_limit(LIMIT_VALUE_VALID);
	hwlog_info("%s: stop charging limit flag = %d, type = %d, value = %d",
		__func__, DC_CLEAR_DISABLE_FLAGS,
		DC_DISABLE_RSMC, LIMIT_VALUE_VALID);
#endif
	return 0;
}

int smc_set_init(struct enable_msg *msg)
{
	int ret;

	if (msg == NULL) {
		hwlog_err("%s: msg null", __func__);
		return -EINVAL;
	}
	if ((msg->reg_num < 0) || (msg->reg_num > RSMC_REG_NUM)) {
		hwlog_err("%s: invalid reg num", __func__);
		return -EINVAL;
	}
	if ((msg->rf_num < 0) || (msg->rf_num > RSMC_RF_NUM)) {
		hwlog_err("%s: invalid rf num", __func__);
		return -EINVAL;
	}
	mutex_lock(&g_x800_ctx.access_mutex);
	if (msg->status == 1) {
		hwlog_info("%s: enable", __func__);
		ret = smc_enable(msg);
	} else {
		hwlog_info("%s: disable", __func__);
		ret = smc_disable(msg);
	}
	mutex_unlock(&g_x800_ctx.access_mutex);
	return ret;
}

bool enable_tx_ant(bool enable)
{
	bool ret = false;
	s32 set, get;
	struct smc_core_data *cd = smc_get_core_data();
	hwlog_info("%s: %d", __func__, (u32)enable);
	if (cd != NULL) {
		set = (enable ? GPIO_HIGH : GPIO_LOW);
		gpio_set_value(cd->gpios.tx_ant_gpio, set);
		if (enable == GPIO_HIGH)
			msleep(cd->gpios.ant_gpio_delay_ms);
		get = gpio_get_value(cd->gpios.tx_ant_gpio);
		if (set == get)
			ret = true;
	}
	hwlog_info("%s: gpio:%d,set:%d,get:%d,ret:%d",
		__func__, (cd != NULL) ? cd->gpios.tx_ant_gpio : 0, set, get, (u32)ret);
	return ret;
}

void enable_rx_ant(bool enable)
{
	s32 set, get;
	struct smc_core_data *cd = smc_get_core_data();
	hwlog_info("%s: %d", __func__, (u32)enable);
	if (cd != NULL) {
		set = (enable ? GPIO_HIGH : GPIO_LOW);
		gpio_set_value(cd->gpios.rx_ant_gpio, set);
		get = gpio_get_value(cd->gpios.rx_ant_gpio);
	}
	hwlog_info("%s: gpio %d set:%d,get:%d",
		__func__, (cd != NULL) ? cd->gpios.rx_ant_gpio : 0, set, get);
}

static void smc_init_msg_proc(struct enable_msg *msg)
{
	s32 ret;
	struct smc_cnf_msg cnf_msg;

	hwlog_info("%s:enter", __func__);

	if (g_x800_ctx.notifier == NULL) {
		hwlog_err("%s: ERROR NOTIFIER", __func__);
		return;
	}

	if (msg != NULL) {
		ret = smc_set_init(msg);
	} else {
		ret = -EINVAL;
		hwlog_err("%s: msg null", __func__);
	}
	if ((msg->head.type == CMD_INTER_INIT_REQ) && (msg->status == 0))
		cnf_msg.head.type = CMD_UP_SOC_ERR_IND;
	else
		cnf_msg.head.type = CMD_UP_INIT_CNF;
	cnf_msg.head.module = MODULE_TYPE_CTRL;
	cnf_msg.head.len = sizeof(struct smc_cnf_msg);
	cnf_msg.result = (ret == 0) ? 1 : 0;
	g_x800_ctx.notifier((struct msg_head *)&cnf_msg);
}

static void smc_freq_offset_est_proc(struct fd_msg *msg)
{
	if (msg == NULL)
		return;
	hwlog_info("%s:enter", __func__);
	if (!g_x800_ctx.is_chip_on || rx_loop_ready()) {
		hwlog_info("%s: chipon:%d,loop:%d", __func__, g_x800_ctx.is_chip_on, rx_loop_ready());
		return;
	} else if (msg->est_sec > 0 && !chip_is_ready()) {
		send_soc_err();
		return;
	}
	clk_fd_est(msg->est_sec);
}

static void smc_mode_set_msg_proc(struct mode_set_msg *msg)
{
	struct smc_cnf_msg cnf_msg;

	hwlog_info("%s:enter", __func__);

	if (g_x800_ctx.notifier == NULL)
		return;

	if (msg != NULL)
		rx_mode_set(msg->mode);
	else
		hwlog_err("%s: req is null.\n", __func__);

	cnf_msg.head.type = CMD_UP_MODE_SET_CNF;
	cnf_msg.head.module = MODULE_TYPE_CTRL;
	cnf_msg.head.len = sizeof(struct smc_cnf_msg);
	cnf_msg.result = 1;

	g_x800_ctx.notifier((struct msg_head *)&cnf_msg);
}

void send_only(struct tx_data_msg *data)
{
	int ret;
	struct smc_cnf_msg cnf_msg;

	if (!chip_is_ready()) {
		send_soc_err();
		ret = 0;
	} else if (data == NULL) {
		ret = 0;
	} else {
		if (enable_tx_ant(true)) {
			update_tx_power(data->power);
			update_tx_freq(data->freqency);
			ret = send_data(data);
			enable_tx_ant(false);
		} else {
			ret = 0;
		}
	}

	if (g_x800_ctx.notifier != NULL) {
		cnf_msg.head.type = CMD_UP_TX_CNF;
		cnf_msg.head.module = MODULE_TYPE_CTRL;
		cnf_msg.head.len = sizeof(struct smc_cnf_msg);
		cnf_msg.result = ret;
		g_x800_ctx.notifier((struct msg_head *)&cnf_msg);
	}
}

static void smc_msg_tx_proc(struct tx_data_msg *msg)
{
#ifdef X800_TX_ONLY
	if (!rx_loop_ready())
		send_only(msg);
	else
		init_tx_data(msg, FALSE);
#else
	init_tx_data(msg, !rx_loop_ready());
	if (!g_x800_ctx.is_chip_on)
		tx_rsp_ok();
	else if (!tx_init())
		tx_rsp_ok();
	else
		wait_loop_exit(true);
#endif
}

static void channel_init_msg_proc(struct rx_init_msg *msg)
{
	hwlog_info("%s:enter", __func__);
	if (g_x800_ctx.is_chip_on)
		rx_init(msg);
	else
		send_chn_init_rsp(0);
	hwlog_info("%s:exit", __func__);
}

static void channel_close_msg_proc(struct rx_init_msg *msg)
{
	if (msg == NULL)
		return;
	hwlog_info("%s:enter", __func__);
	if (msg->chn_num > MAX_CHAN_NUM) {
		hwlog_info("%s:chn idx error:%d", __func__, msg->chn_num);
		return;
	}
	if (g_x800_ctx.is_chip_on)
		chan_close(msg);
	else
		send_chn_close_rsp(0);
}

static void channel_acq2track_msg_proc(struct acq2track_msg *msg)
{
	if (msg == NULL)
		return;
	hwlog_info("%s:enter", __func__);
	if (msg->chn_idx >= MAX_CHAN_NUM)
		return;
	if (g_x800_ctx.is_chip_on) {
		acq2track(msg);
		init_track(msg);
	}
}

static void channel_track_adjust_msg_proc(struct track_adjust_msg *msg)
{
	if (msg == NULL)
		return;
	if (g_x800_ctx.is_chip_on)
		update_ms_idx(msg);
}

static void channel_single_cmd_proc(struct single_cmd_entry *msg)
{
	u32 value;

	if (msg == NULL)
		return;
	hwlog_info("%s: RSMC test single cmd", __func__);
	if (!g_x800_ctx.is_chip_on || rx_loop_ready()) {
		return;
	} else if (!chip_is_ready()) {
		send_soc_err();
		return;
	}
	g_x800_ctx.mode_sel = msg->target;
	if (msg->opara == 0)
		smc_set_value((msg->addr & 0xff), (msg->value & 0xffffff));
	else
		smc_get_value((msg->addr & 0xff), &value);
}

void channel_freq_offset_proc(struct freq_off_est_entry *msg)
{
	int duration;

	if (msg == NULL)
		return;
	hwlog_info("%s: enable=%d,frequency=%d,power=%d",
		__func__, msg->enable, msg->frequency, msg->power);

	if (!g_x800_ctx.is_chip_on) {
		return;
	} else if (!chip_is_ready()) {
		send_soc_err();
		return;
	}
	if (msg->enable == 0) {
		smc_set_value(ADDR_08_MOD_CTRL, 0x1);
		enable_tx_ant(false);
		hwlog_err("%s: shut down and return", __func__);
		return;
	}
	if (!enable_tx_ant(true))
		return;
	update_tx_freq(msg->frequency);
	update_tx_power(msg->power);
	smc_set_value(ADDR_08_MOD_CTRL, 0x5);

	hwlog_info("%s: start", __func__);

	if (msg->duration <= 0 || msg->duration > EXPIRE_TIME)
		duration = EXPIRE_TIME;
	else
		duration = msg->duration;
	msleep(duration);
	smc_set_value(ADDR_08_MOD_CTRL, 0x1);
	enable_tx_ant(false);
	hwlog_info("%s: stop duration %d %d", __func__, msg->duration, duration);
}

void send_msg_to_ctrl(struct msg_head *msg)
{
	if (msg == NULL)
		return;
	if (g_x800_ctx.notifier == NULL)
		return;
	g_x800_ctx.notifier(msg);
}

void send_soc_err(void)
{
	struct smc_cnf_msg cnf_msg;

	cnf_msg.head.type = CMD_UP_SOC_ERR_IND;
	cnf_msg.head.module = MODULE_TYPE_CTRL;
	cnf_msg.head.len = sizeof(struct smc_cnf_msg);
	send_msg_to_ctrl((struct msg_head *)&cnf_msg);
}

static void x800_device_msg_proc(struct msg_head *req)
{
	if (req == NULL)
		return;
	hwlog_info("%s: msg_type:%d", __func__, req->type);
	switch (req->type) {
	case CMD_DN_INIT_REQ:
	case CMD_INTER_INIT_REQ:
		smc_init_msg_proc((struct enable_msg *)req);
		break;
	case CMD_DN_MODE_SET_REQ:
		smc_mode_set_msg_proc((struct mode_set_msg *)req);
		break;
	case CMD_DN_FREQ_OFFSET_EST_REQ:
		smc_freq_offset_est_proc((struct fd_msg *)req);
		break;
	case CMD_DN_TX_REQ:
		smc_msg_tx_proc((struct tx_data_msg *)req);
		break;
	case CMD_DN_CHN_INIT_REQ:
		channel_init_msg_proc((struct rx_init_msg *)req);
		break;
	case CMD_DN_CHN_CLOSE_REQ:
		channel_close_msg_proc((struct rx_init_msg *)req);
		break;
	case CMD_DN_CHN_ACQ2TRK_REQ:
		channel_acq2track_msg_proc((struct acq2track_msg *)req);
		break;
	case CMD_DN_CHN_TRK_ADJUST_REQ:
		channel_track_adjust_msg_proc((struct track_adjust_msg *)req);
		break;
	case CMD_DN_SINGLE_CMD_REQ:
		channel_single_cmd_proc((struct single_cmd_entry *)req);
		break;
	case CMD_DN_FREQ_OFF_REQ:
		channel_freq_offset_proc((struct freq_off_est_entry *)req);
		break;
	case CMD_INTER_HB_TIMER_REQ:
		if (g_x800_ctx.is_chip_on)
			rsmc_start_heartbeat();
		break;
	default:
		break;
	}
}

static int rsmc_driver_module_init(void)
{
	int rc = 0;
	struct smc_device *dev = kzalloc(sizeof(struct smc_device), GFP_KERNEL);
#ifdef CONFIG_HW_RSMC_QCOM
	huawei_charger_set_dc_disable_flags(DC_CLEAR_DISABLE_FLAGS,
		DC_DISABLE_RSMC);
	huawei_charger_set_rsmc_limit(LIMIT_VALUE_VALID);
	hwlog_info("%s: stop charging limit flag = %d, type = %d, value = %d",
		__func__, DC_CLEAR_DISABLE_FLAGS,
		DC_DISABLE_RSMC, LIMIT_VALUE_VALID);
#endif
	if (dev == NULL)
		return -ENOMEM;
	dev->ic_name = X800_IC_NAME;
	dev->dev_node_name = RSMC_X800_DEV_NODE_NAME;

	rc = rsmc_register_dev(dev);
	if (rc) {
		hwlog_err("%s: register fail\n", __func__);
		goto err;
	}
	g_x800_ctx.mode_sel = 0;

	return rc;
err:
	kfree(dev);
	dev = NULL;
	return rc;
}

static void rsmc_driver_module_exit(void)
{
	struct smc_core_data *cd = smc_get_core_data();

	rsmc_unregister_dev(cd->smc_dev);
	hwlog_info("%s: called\n", __func__);
}

msg_process *x800_device_reg(notify_event *fun)
{
	int count = 0;
	struct smc_core_data *cd = NULL;

	hwlog_info("%s: enter", __func__);
	msleep(SMC_PROBE_SLEEP_INTERVAL); // wait do smc_probe later
	rsmc_spi_driver_init();
	cd = smc_get_core_data();
	while (cd == NULL && count < DEVICE_REG_TRY_MAX_COUNT) {
		msleep(DEVICE_REG_SLEEP_INTERVAL);
		count++;
		cd = smc_get_core_data();
	}
	hwlog_info("%s: count=%d, cd=%p", __func__, count, cd);

	rsmc_driver_module_init();
	spi_ctrl_init();
	g_x800_ctx.notifier = fun;
	g_x800_ctx.is_chip_on = false;
	hwlog_info("%s: exit", __func__);
	mutex_init(&g_x800_ctx.access_mutex);
	return x800_device_msg_proc;
}

void x800_device_unreg(int reason)
{
	mutex_destroy(&g_x800_ctx.access_mutex);
	rsmc_spi_driver_exit();
	rsmc_driver_module_exit();
	spi_ctrl_deinit();
}
