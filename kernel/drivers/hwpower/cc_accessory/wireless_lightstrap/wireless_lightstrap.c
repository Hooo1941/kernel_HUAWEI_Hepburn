// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_lightstrap.c
 *
 * wireless lightstrap driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/accessory/wireless_lightstrap.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwsensor/hall/hall_interface.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include <securec.h>

#define HWLOG_TAG wirelesscase
HWLOG_REGIST();

static struct wirelesscase_di *g_wirelesscase_di;
static bool g_is_first_cover = false;
static bool g_coolingcase_bst5v_flag = false;

struct wirelesscase_di *wirelesscase_get_di(void)
{
	if (!g_wirelesscase_di) {
		hwlog_err("g_wirelesscase_di is null\n");
		return NULL;
	}

	return g_wirelesscase_di;
}

bool lightstrap_online_state(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di)
		return false;

	if (di->product_type == LIGHTSTRAP_PRODUCT_TYPE)
		return true;

	if ((wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) &&
		hall_interface_get_hall_status())
		return true;

	return false;
}

bool wirelesscase_hall_only(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di || (di->product_type == COOLINGCASE_PRODUCT_TYPE) ||
		(di->product_type == LIGHTSTRAP_PRODUCT_TYPE))
		return false;

	if (!hall_interface_get_hall_status())
		return false;

	return true;
}

bool coolingcase_online_state(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di)
		return false;

	if (di->product_type == COOLINGCASE_PRODUCT_TYPE)
		return true;

	if ((wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) &&
		hall_interface_get_hall_status())
		return true;

	return false;
}

enum wirelesscase_online_state wirelesscase_get_online_state(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di)
		return WLCASE_OFFLINE;

	if (lightstrap_online_state())
		return WLCASE_ONLINE_LIGHTSTRAP;

	if (coolingcase_online_state())
		return WLCASE_ONLINE_COOLINGCASE;

	return WLCASE_OFFLINE;
}

enum wirelesscase_online_state wirelesscase_get_online_state_new(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di)
		return WLCASE_OFFLINE;

	if (!di->online_state_new)
		return wirelesscase_get_online_state();

	if (di->product_type == LIGHTSTRAP_PRODUCT_TYPE)
		return WLCASE_ONLINE_LIGHTSTRAP;

	if (di->product_type == COOLINGCASE_PRODUCT_TYPE)
		return WLCASE_ONLINE_COOLINGCASE;

	return WLCASE_OFFLINE;
}

enum wltx_pwr_src lightstrap_specify_pwr_src(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di)
		return PWR_SRC_NA;

	return di->wltx_pwr_src;
}

static bool wirelesscase_can_do_reverse_charging(void)
{
	int soc = 0;

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &soc, POWER_SUPPLY_DEFAULT_CAPACITY, 1);
	if ((soc <= WL_TX_SOC_MIN) && (wirelesscase_get_online_state() != WLCASE_OFFLINE) &&
			(wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF)) {
		wireless_tx_set_tx_status(WL_TX_STATUS_TX_CLOSE);
		hwlog_info("wirelesscase can not do reverse charging\n");
		return false;
	}

	return true;
}

void lightstrap_reinit_tx_chip(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di)
		return;

	(void)wltx_ic_set_ping_freq(WLTRX_IC_MAIN, di->ping_freq_light);
	hwlog_info("lightstrap ping freq : %d\n", di->ping_freq_light);
	(void)wltx_ic_set_min_fop(WLTRX_IC_MAIN, di->work_freq_light);
	hwlog_info("lightstrap work freq : %d\n", di->work_freq_light);
	(void)wltx_ic_set_max_fop(WLTRX_IC_MAIN, di->work_freq_light);
}

static bool tx_voltage_is_more_than_5v(void)
{
	u16 tx_vol_vin = 0;

	(void)wltx_ic_get_vin(WLTRX_IC_MAIN, &tx_vol_vin);
	hwlog_info("more_than_5V -- get tx_vol_vin: %d\n", tx_vol_vin);

	if (tx_vol_vin >= BOOST5V_THRESHOLD)
		return true;

	return false;
}

static void coolingcase_monitor_voltage_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		monitor_voltage_work.work);
	u16 tx_vin = 0;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	(void)wltx_ic_get_vin(WLTRX_IC_MAIN, &tx_vin);
	hwlog_info("monitor_voltage_work -- get tx_vin: %d\n", tx_vin);

	if ((g_coolingcase_bst5v_flag && (tx_vin < LV_THRESHOLD)) ||
		(!g_coolingcase_bst5v_flag && (tx_vin > HV_THRESHOLD))) {
		hwlog_info("vin changed, close tx");
		wltx_open_tx(WLTX_CLIENT_COOLINGCASE, false);
		if (delayed_work_pending(&di->delay_close_work))
			wltx_open_tx(WLTX_CLIENT_COOLINGCASE, true);
	}

	if (wireless_tx_get_tx_open_flag())
		schedule_delayed_work(&di->monitor_voltage_work,
			msecs_to_jiffies(COOLINGCASE_MONITOR_VOL_DELAY));
}

void coolingcase_reinit_tx_chip(void)
{
	struct wirelesscase_di *di = g_wirelesscase_di;

	if (!di)
		return;

	hwlog_info("coolingcase ping freq : %d\n", di->ping_freq_cool);
	(void)wltx_ic_set_ping_freq(WLTRX_IC_MAIN, di->ping_freq_cool);

	if (delayed_work_pending(&di->duty_low_power_work))
		cancel_delayed_work(&di->duty_low_power_work);

	if (tx_voltage_is_more_than_5v()) {
		g_coolingcase_bst5v_flag = true;
		hwlog_info("hv work freq --min:%d, max:%d, hv boot duty--min:%d, max:%d\n",
			(int)di->coolingcase_freq.hv_min, (int)di->coolingcase_freq.hv_max,
			(int)di->coolingcase_duty.hv_boot_min, (int)di->coolingcase_duty.hv_boot_max);
		(void)wltx_ic_set_min_fop(WLTRX_IC_MAIN, (int)di->coolingcase_freq.hv_min);
		(void)wltx_ic_set_max_fop(WLTRX_IC_MAIN, (int)di->coolingcase_freq.hv_max);
		(void)wltx_ic_set_min_duty(WLTRX_IC_MAIN, (int)di->coolingcase_duty.hv_boot_min);
		(void)wltx_ic_set_max_duty(WLTRX_IC_MAIN, (int)di->coolingcase_duty.hv_boot_max);

		schedule_delayed_work(&di->duty_low_power_work,
			msecs_to_jiffies(COOLINGCASE_RESET_DUTY_DELAY));
	} else {
		g_coolingcase_bst5v_flag = false;
		hwlog_info("lv work freq --min:%d, max:%d,lv boot duty--min:%d, max:%d\n",
			(int)di->coolingcase_freq.lv_min, (int)di->coolingcase_freq.lv_max,
			(int)di->coolingcase_duty.lv_min, (int)di->coolingcase_duty.lv_max);
		(void)wltx_ic_set_min_fop(WLTRX_IC_MAIN, di->coolingcase_freq.lv_min);
		(void)wltx_ic_set_max_fop(WLTRX_IC_MAIN, di->coolingcase_freq.lv_max);
		(void)wltx_ic_set_min_duty(WLTRX_IC_MAIN, (int)di->coolingcase_duty.lv_min);
		(void)wltx_ic_set_max_duty(WLTRX_IC_MAIN, (int)di->coolingcase_duty.lv_max);
	}

	(void)wltx_ic_set_open_loop(WLTRX_IC_MAIN, true);
	schedule_delayed_work(&di->monitor_voltage_work,
		msecs_to_jiffies(COOLINGCASE_MONITOR_VOL_DELAY));
}

static void lightstrap_send_light_up_msg(void)
{
	unsigned char msg = HWQI_PARA_LIGHTSTRAP_LIGHT_UP_MSG;
	int ret;

	hwlog_info("send_light_up_msg: %02x\n", msg);
	ret = wireless_send_lightstrap_ctrl_msg(WLTRX_IC_MAIN,
		WIRELESS_PROTOCOL_QI, &msg, LIGHTSTRAP_LIGHT_UP_MSG_LEN);
	if (ret)
		hwlog_err("send_light_up_msg fail\n");
	else
		hwlog_info("send_light_up_msg success\n");
}

static void lightstrap_light_up(void)
{
	hwlog_info("lightstrap light up\n");
	wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, true);
	msleep(LIGHTSTRAP_LIGHT_UP_DELAY);
	lightstrap_send_light_up_msg();
}

static void lightstrap_close_wltx(struct wirelesscase_di *di)
{
	if (di->is_opened_by_hall) {
		cancel_delayed_work(&di->check_work);
		di->is_opened_by_hall = false;
	}

	if (di->tx_status_ping) {
		cancel_delayed_work(&di->tx_ping_work);
		di->tx_status_ping = false;
	}

	wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, false);
}

static void coolingcase_delay_close_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		delay_close_work.work);

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	hwlog_info("coolingcase turn off after 2.5min\n");
	lightstrap_close_wltx(di);
	msleep(LIGHTSTRAP_UEVENT_DELAY);
}

static void lightstrap_control_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		control_work.work);

	if (!di)
		return;

	if (!lightstrap_online_state() && !coolingcase_online_state())
		return;

	switch (di->sysfs_val) {
	case WIRELESSCASE_POWER_OFF:
	    if (di->product_type == LIGHTSTRAP_PRODUCT_TYPE) {
			hwlog_info("lightstrap power off\n");
			wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, false);
		} else if (di->product_type == COOLINGCASE_PRODUCT_TYPE) {
			hwlog_info("coolingcase power off\n");
			if (delayed_work_pending(&di->delay_close_work) || (wltx_get_client() == WLTX_CLIENT_UI)) {
				hwlog_info("2.5min working, coolingcase cannot close\n");
				break;
			}
			wltx_open_tx(WLTX_CLIENT_COOLINGCASE, false);
		}
		break;
	case WIRELESSCASE_POWER_ON:
		if (di->product_type == LIGHTSTRAP_PRODUCT_TYPE) {
			hwlog_info("lightstrap power on\n");
			wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, true);
		} else if (di->product_type == COOLINGCASE_PRODUCT_TYPE) {
			hwlog_info("coolingcase power on\n");
			wltx_open_tx(WLTX_CLIENT_COOLINGCASE, true);
		}
		break;
	case LIGHTSTRAP_LIGHT_UP:
		lightstrap_light_up();
		break;
	default:
		break;
	}
}

static int lightstrap_check_version(u8 product_id)
{
	switch (product_id) {
	case WIRELESSCASE_VER_ZERO_ID:
		return LIGHTSTRAP_VERSION_ONE;
	case WIRELESSCASE_VER_TWO_ID:
		return LIGHTSTRAP_VERSION_TWO;
	default:
		return WIRELESSCASE_OFFLINE;
	}
}

static int coolingcase_check_version(u8 product_id)
{
	switch (product_id) {
	case WIRELESSCASE_VER_ZERO_ID:
	case WIRELESSCASE_VER_ONE_ID:
		return COOLINGCASE_VERSION_ONE;
	default:
		return WIRELESSCASE_OFFLINE;
	}
}

static int wirelesscase_check_version(struct wirelesscase_di *di)
{
	if (di->product_type == LIGHTSTRAP_PRODUCT_TYPE)
		return lightstrap_check_version(di->product_id);
	else if (di->product_type == COOLINGCASE_PRODUCT_TYPE)
		return coolingcase_check_version(di->product_id);
	else
		return WIRELESSCASE_OFFLINE;
}

static void wirelesscase_report_dmd(enum wirelesscase_dmd_type type)
{
	char buff[POWER_DSM_BUF_SIZE_0128] = {0};

	switch (type) {
	case WIRELESSCASE_ATTACH_DMD:
		snprintf_s(buff, sizeof(buff), sizeof(buff) - 1, "wirelesscase_attach\n");
		power_dsm_report_dmd(POWER_DSM_LIGHTSTRAP, POWER_DSM_LIGHTSTRAP_STATUS, buff);
		break;
	case WIRELESSCASE_DETACH_DMD:
		snprintf_s(buff, sizeof(buff), sizeof(buff) - 1, "wirelesscase_detach\n");
		power_dsm_report_dmd(POWER_DSM_LIGHTSTRAP, POWER_DSM_LIGHTSTRAP_STATUS, buff);
		break;
	default:
		break;
	}
}

static void wirelesscase_send_on_uevent(struct wirelesscase_di *di)
{
	char *envp[LIGHTSTRAP_MAX_RX_SIZE] = {
		"WIRELESSCASE=ON",
		NULL,
		NULL,
		"SUBMODELID=0",
		NULL,
		NULL
	};

	envp[LIGHTSTRAP_ENVP_OFFSET1] = kzalloc(LIGHTSTRAP_INFO_LEN, GFP_KERNEL);
	if (!envp[LIGHTSTRAP_ENVP_OFFSET1])
		return;

	envp[LIGHTSTRAP_ENVP_OFFSET2] = kzalloc(LIGHTSTRAP_INFO_LEN, GFP_KERNEL);
	if (!envp[LIGHTSTRAP_ENVP_OFFSET2])
		return;

	envp[LIGHTSTRAP_ENVP_OFFSET4] = kzalloc(LIGHTSTRAP_INFO_LEN, GFP_KERNEL);
	if (!envp[LIGHTSTRAP_ENVP_OFFSET4])
		goto alloc_fail;

	snprintf(envp[LIGHTSTRAP_ENVP_OFFSET1], LIGHTSTRAP_INFO_LEN, "PRODUCTTYPE=%02x",
		di->product_type);
	snprintf(envp[LIGHTSTRAP_ENVP_OFFSET2], LIGHTSTRAP_INFO_LEN, "MODELID=%s",
		di->model_id);
	snprintf(envp[LIGHTSTRAP_ENVP_OFFSET4], LIGHTSTRAP_INFO_LEN, "PRODUCTID=%02x",
		di->product_id);
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	hwlog_info("wirelesscase send case=on uevent\n");
	kfree(envp[LIGHTSTRAP_ENVP_OFFSET4]);

alloc_fail:
	kfree(envp[LIGHTSTRAP_ENVP_OFFSET2]);
}

static void wirelesscase_send_off_uevent(struct wirelesscase_di *di)
{
	char *envp[LIGHTSTRAP_MAX_RX_SIZE] = {
		"WIRELESSCASE=OFF",
		NULL,
		NULL,
		"SUBMODELID=0",
		"PRODUCTID=00",
		NULL
	};

	envp[LIGHTSTRAP_ENVP_OFFSET1] = kzalloc(LIGHTSTRAP_INFO_LEN, GFP_KERNEL);
	if (!envp[LIGHTSTRAP_ENVP_OFFSET1])
		return;

	envp[LIGHTSTRAP_ENVP_OFFSET2] = kzalloc(LIGHTSTRAP_INFO_LEN, GFP_KERNEL);
	if (!envp[LIGHTSTRAP_ENVP_OFFSET2])
		return;

	snprintf(envp[LIGHTSTRAP_ENVP_OFFSET1], LIGHTSTRAP_INFO_LEN, "PRODUCTTYPE=%02x",
		di->product_type);
	snprintf(envp[LIGHTSTRAP_ENVP_OFFSET2], LIGHTSTRAP_INFO_LEN, "MODELID=%s",
		di->model_id);
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	hwlog_info("wirelesscase send case=off uevent\n");
	kfree(envp[LIGHTSTRAP_ENVP_OFFSET2]);
}

static void lightstrap_set_status(struct wirelesscase_di *di, unsigned int status)
{
	mutex_lock(&di->lock);
	di->status = status;
	mutex_unlock(&di->lock);
	hwlog_info("set wirelesscase status : %d\n", di->status);
}

static void lightstrap_check_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		check_work.work);

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->is_opened_by_hall) {
		wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, false);
		di->is_opened_by_hall = false;
		lightstrap_set_status(di, LIGHTSTRAP_STATUS_INIT);
	}
}

static void lightstrap_tx_ping_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		tx_ping_work.work);

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	di->tx_status_ping = false;
	lightstrap_set_status(di, LIGHTSTRAP_STATUS_INIT);
	if (wireless_tx_get_tx_status() == WL_TX_STATUS_PING)
		(void)wltx_ic_set_ping_freq(WLTRX_IC_MAIN, LIGHTSTRAP_RESET_TX_PING_FREQ);
}

static int  wirelesscase_parse_product_info(struct wirelesscase_di *di, void *data)
{
	u8 product_type;

	if (!di || !data)
		return -EINVAL;

	product_type = ((u8 *)data)[0];
	if ((product_type != LIGHTSTRAP_PRODUCT_TYPE) &&
		(product_type != COOLINGCASE_PRODUCT_TYPE))
		return -EINVAL;

	di->product_type = product_type;
	di->product_id = ((u8 *)data)[1];
	hwlog_info("product_type=%02x, product_id=%02x\n", di->product_type,
		di->product_id);
	if ((di->product_type == COOLINGCASE_PRODUCT_TYPE) &&
		(di->product_id == WIRELESSCASE_VER_ZERO_ID))
		wlrx_plim_set_src(WLTRX_DRV_MAIN, WLRX_PLIM_SRC_COOLINGCASE);

	return 0;
}

static void wirelesscase_clear_product_info(struct wirelesscase_di *di)
{
	di->product_type = OFFLINE_PRODUCT_TYPE;
	di->product_id = LIGHTSTRAP_OFF;
}

static void lightstrap_detect_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		detect_work.work);

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	di->is_opened_by_hall = true;
	wltx_open_tx(di->default_wltx_client, true);
	schedule_delayed_work(&di->check_work, msecs_to_jiffies(LIGHTSTRAP_TIMEOUT));
	lightstrap_set_status(di, LIGHTSTRAP_STATUS_WPI);
}

/* handle wlrx disconnect event on status: waiting wlrx end */
static void lightstrap_handle_wlrx_disconnect_on_wwe(struct wirelesscase_di *di)
{
	if (!hall_interface_get_hall_status()) {
		lightstrap_set_status(di, LIGHTSTRAP_STATUS_INIT);
		return;
	}

	schedule_delayed_work(&di->detect_work, msecs_to_jiffies(LIGHTSTRAP_DETECT_WORK_DELAY));
}

/* handle wlrx connect event on status: waiting product info */
static void lightstrap_handle_wlrx_connect_on_wpi(struct wirelesscase_di *di)
{
	if (di->is_opened_by_hall) {
		cancel_delayed_work(&di->check_work);
		di->is_opened_by_hall = false;
		return;
	}

	if (di->tx_status_ping) {
		cancel_delayed_work(&di->tx_ping_work);
		di->tx_status_ping = false;
	}
}

/* handle hall off event on status: waiting product info */
static void lightstrap_handle_hall_off_on_wpi(struct wirelesscase_di *di)
{
	if (di->is_opened_by_hall) {
		cancel_delayed_work(&di->check_work);
		di->is_opened_by_hall = false;
		wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, false);
		return;
	}

	if (di->tx_status_ping) {
		cancel_delayed_work(&di->tx_ping_work);
		di->tx_status_ping = false;
		(void)wltx_ic_set_ping_freq(WLTRX_IC_MAIN, LIGHTSTRAP_RESET_TX_PING_FREQ);
	}
}

/* handle hall approach event on status: initialized */
static void lightstrap_handle_hall_approach_on_init(struct wirelesscase_di *di)
{
	int tx_status = wireless_tx_get_tx_status();

	switch (tx_status) {
	case WL_TX_STATUS_PING_SUCC:
	case WL_TX_STATUS_IN_CHARGING:
		break;
	case WL_TX_STATUS_PING:
		di->tx_status_ping = true;
		if (di->product_type == LIGHTSTRAP_PRODUCT_TYPE)
			(void)wltx_ic_set_ping_freq(WLTRX_IC_MAIN, di->ping_freq_light);
		else
			(void)wltx_ic_set_ping_freq(WLTRX_IC_MAIN, di->ping_freq_cool);
		schedule_delayed_work(&di->tx_ping_work, msecs_to_jiffies(LIGHTSTRAP_TIMEOUT));
		lightstrap_set_status(di, LIGHTSTRAP_STATUS_WPI);
		break;
	default:
		schedule_delayed_work(&di->detect_work, msecs_to_jiffies(0));
		break;
	}
}

static void set_duty_low_power_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		duty_low_power_work.work);

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	hwlog_info("set working duty--min:%d, max:%d\n",
			(int)di->coolingcase_duty.hv_lp_min, (int)di->coolingcase_duty.hv_lp_max);
	(void)wltx_ic_set_min_duty(WLTRX_IC_MAIN, (int)di->coolingcase_duty.hv_lp_min);
	(void)wltx_ic_set_max_duty(WLTRX_IC_MAIN, (int)di->coolingcase_duty.hv_lp_max);
}

static void lightstrap_process_product_info_event(struct wirelesscase_di *di)
{
	switch (di->status) {
	case LIGHTSTRAP_STATUS_WPI:
		if ((di->product_type == COOLINGCASE_PRODUCT_TYPE) && g_is_first_cover) {
			g_is_first_cover = false;
			cancel_delayed_work(&di->check_work);
			if (di->default_wltx_client != WLTX_CLIENT_COOLINGCASE) {
				wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, false);
				wltx_open_tx(WLTX_CLIENT_COOLINGCASE, true);
			}
			schedule_delayed_work(&di->delay_close_work,
				msecs_to_jiffies(COOLINGCASE_CLOSE_DELAY));
		} else {
			lightstrap_close_wltx(di);
			msleep(LIGHTSTRAP_UEVENT_DELAY);
		}
		break;
	case LIGHTSTRAP_STATUS_INIT:
	case LIGHTSTRAP_STATUS_WWE:
	case LIGHTSTRAP_STATUS_DEV:
	default:
		break;
	}

	if (!di->uevent_flag) {
		wirelesscase_send_on_uevent(di);
		wirelesscase_report_dmd(WIRELESSCASE_ATTACH_DMD);
		di->uevent_flag = true;
	}

	lightstrap_set_status(di, LIGHTSTRAP_STATUS_DEV);
}

static void lightstrap_process_wlrx_disconnect_event(struct wirelesscase_di *di)
{
	switch (di->status) {
	case LIGHTSTRAP_STATUS_WWE:
		lightstrap_handle_wlrx_disconnect_on_wwe(di);
		break;
	case LIGHTSTRAP_STATUS_INIT:
	case LIGHTSTRAP_STATUS_WPI:
	case LIGHTSTRAP_STATUS_DEV:
	default:
		break;
	}
}

static void lightstrap_process_wlrx_connect_event(struct wirelesscase_di *di)
{
	switch (di->status) {
	case LIGHTSTRAP_STATUS_INIT:
		lightstrap_set_status(di, LIGHTSTRAP_STATUS_WWE);
		break;
	case LIGHTSTRAP_STATUS_WWE:
		break;
	case LIGHTSTRAP_STATUS_WPI:
		lightstrap_handle_wlrx_connect_on_wpi(di);
		lightstrap_set_status(di, LIGHTSTRAP_STATUS_WWE);
		break;
	case LIGHTSTRAP_STATUS_DEV:
		break;
	default:
		break;
	}
}

static void lightstrap_process_hall_off_event(struct wirelesscase_di *di)
{
	di->uevent_flag = false;

	if (!di->startup_identify_timeout && di->identify_delay_time > 0) {
		hwlog_err("startup identify not finished, ignore hall away event\n");
		return;
	}

	switch (di->status) {
	case LIGHTSTRAP_STATUS_INIT:
		break;
	case LIGHTSTRAP_STATUS_WWE:
		break;
	case LIGHTSTRAP_STATUS_WPI:
		lightstrap_handle_hall_off_on_wpi(di);
		break;
	case LIGHTSTRAP_STATUS_DEV:
		if (di->product_type == COOLINGCASE_PRODUCT_TYPE) {
			if (delayed_work_pending(&di->delay_close_work))
				cancel_delayed_work(&di->delay_close_work);
			if (wireless_tx_get_tx_open_flag())
				wltx_open_tx(WLTX_CLIENT_COOLINGCASE, false);
		}
		wirelesscase_send_off_uevent(di);
		wirelesscase_report_dmd(WIRELESSCASE_DETACH_DMD);
		break;
	default:
		break;
	}

	lightstrap_set_status(di, LIGHTSTRAP_STATUS_INIT);
}

static void lightstrap_process_hall_approach_event(struct wirelesscase_di *di)
{
	if (!di->startup_identify_timeout && di->identify_delay_time > 0) {
		hwlog_err("startup identify not finished, ignore hall approach event\n");
		return;
	}

	switch (di->status) {
	case LIGHTSTRAP_STATUS_INIT:
		lightstrap_handle_hall_approach_on_init(di);
		break;
	case LIGHTSTRAP_STATUS_WWE:
	case LIGHTSTRAP_STATUS_WPI:
	case LIGHTSTRAP_STATUS_DEV:
	default:
		break;
	}
}

static void lightstrap_init_identify_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		identify_work.work);
	int chk_mtp_complete_times = WIRELESSCASE_CHK_MTP_TIMES;
	int i;

	if (!di)
		return;

	di->startup_identify_timeout = true;
	hwlog_info("startup_identify_timeout is true\n");

	/* resolve conflicts between wltx and firmware loading */
	do {
		if (!hall_interface_get_hall_status())
			return;
		if (wireless_fw_is_mtp_chk_complete())
			break;
		power_msleep(DT_MSLEEP_100MS, 0, NULL);
	} while (--chk_mtp_complete_times > 0);
	hwlog_info("chk_mtp_complete_times : %d\n", chk_mtp_complete_times);

	for (i = 0; i < di->retry_times; i++) {
		hwlog_info("init identify retry times : %d\n", i);
		if (lightstrap_online_state() || coolingcase_online_state())
			break;

		if (hall_interface_get_hall_status())
			lightstrap_process_hall_approach_event(di);

		msleep(LIGHTSTRAP_INIT_IDENTIFY_DELAY);
	}
}

static void lightstrap_event_work(struct work_struct *work)
{
	struct wirelesscase_di *di = container_of(work, struct wirelesscase_di,
		event_work);

	if (!di)
		return;

	switch (di->event_type) {
	case POWER_NE_LIGHTSTRAP_ON:
		lightstrap_process_hall_approach_event(di);
		break;
	case POWER_NE_LIGHTSTRAP_OFF:
		lightstrap_process_hall_off_event(di);
		wirelesscase_clear_product_info(di);
		wlrx_plim_clear_src(WLTRX_DRV_MAIN, WLRX_PLIM_SRC_COOLINGCASE);
		break;
	case POWER_NE_WIRELESS_CONNECT:
		lightstrap_process_wlrx_connect_event(di);
		break;
	case POWER_NE_WIRELESS_DISCONNECT:
		lightstrap_process_wlrx_disconnect_event(di);
		break;
	case POWER_NE_LIGHTSTRAP_GET_PRODUCT_INFO:
		lightstrap_process_product_info_event(di);
		break;
	case POWER_NE_LIGHTSTRAP_EPT:
		wltx_open_tx(WLTX_CLIENT_LIGHTSTRAP, false);
		break;
	default:
		break;
	}
}

static int lightstrap_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct wirelesscase_di *di = wirelesscase_get_di();

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_LIGHTSTRAP_ON:
		g_is_first_cover = true;
		break;
	case POWER_NE_LIGHTSTRAP_OFF:
		break;
	case POWER_NE_WIRELESS_CONNECT:
		break;
	case POWER_NE_WIRELESS_DISCONNECT:
		break;
	case POWER_NE_LIGHTSTRAP_GET_PRODUCT_INFO:
		if (wirelesscase_parse_product_info(di, data))
			return NOTIFY_OK;

		break;
	case POWER_NE_LIGHTSTRAP_EPT:
		break;
	default:
		return NOTIFY_OK;
	}

	mutex_lock(&di->lock);
	di->event_type = event;
	mutex_unlock(&di->lock);
	schedule_work(&di->event_work);

	return NOTIFY_OK;
}

static struct wltx_logic_ops g_lightstrap_logic_ops = {
	.type = WLTX_CLIENT_LIGHTSTRAP,
	.can_rev_charge_check = wirelesscase_can_do_reverse_charging,
	.need_specify_pwr_src = hall_interface_get_hall_status,
	.specify_pwr_src = lightstrap_specify_pwr_src,
	.reinit_tx_chip = lightstrap_reinit_tx_chip,
};

static struct wltx_logic_ops g_coolingcase_logic_ops = {
	.type = WLTX_CLIENT_COOLINGCASE,
	.can_rev_charge_check = wirelesscase_can_do_reverse_charging,
	.need_specify_pwr_src = hall_interface_get_hall_status,
	.specify_pwr_src = lightstrap_specify_pwr_src,
	.reinit_tx_chip = coolingcase_reinit_tx_chip,
};

#ifdef CONFIG_SYSFS
static ssize_t lightstrap_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t lightstrap_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info lightstrap_sysfs_field_tbl[] = {
	power_sysfs_attr_wo(lightstrap, 0220, LIGHTSTRAP_SYSFS_CONTROL, control),
	power_sysfs_attr_ro(lightstrap, 0440, LIGHTSTRAP_SYSFS_CHECK_VERSION, check_ver),
};

#define LIGHTSTRAP_SYSFS_ATTRS_SIZE ARRAY_SIZE(lightstrap_sysfs_field_tbl)

static struct attribute *lightstrap_sysfs_attrs[LIGHTSTRAP_SYSFS_ATTRS_SIZE + 1];
static const struct attribute_group lightstrap_sysfs_attr_group = {
	.attrs = lightstrap_sysfs_attrs,
};

static ssize_t lightstrap_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wirelesscase_di *di = g_wirelesscase_di;

	info = power_sysfs_lookup_attr(attr->attr.name,
		lightstrap_sysfs_field_tbl, LIGHTSTRAP_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case LIGHTSTRAP_SYSFS_CHECK_VERSION:
		return snprintf(buf, PAGE_SIZE, "%u\n", wirelesscase_check_version(di));
	default:
		return 0;
	}
}

static ssize_t lightstrap_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wirelesscase_di *di = g_wirelesscase_di;
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		lightstrap_sysfs_field_tbl, LIGHTSTRAP_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case LIGHTSTRAP_SYSFS_CONTROL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;

		di->sysfs_val = val;
		schedule_delayed_work(&di->control_work, msecs_to_jiffies(0));
		break;
	default:
		break;
	}

	return count;
}

static struct device *lightstrap_sysfs_create_group(void)
{
	power_sysfs_init_attrs(lightstrap_sysfs_attrs,
		lightstrap_sysfs_field_tbl, LIGHTSTRAP_SYSFS_ATTRS_SIZE);

	return power_sysfs_create_group("hw_power", "lightstrap",
		&lightstrap_sysfs_attr_group);
}

static void lightstrap_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &lightstrap_sysfs_attr_group);
}
#else
static inline struct device *lightstrap_sysfs_create_group(void)
{
	return NULL;
}

static inline void lightstrap_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void lightstrap_parse_dts(struct device_node *np,
	struct wirelesscase_di *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ping_freq",
		&di->ping_freq_light, LIGHTSTRAP_PING_FREQ_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_work_freq",
		&di->work_freq_light, LIGHTSTRAP_WORK_FREQ_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ping_freq_cool",
		&di->ping_freq_cool, COOLINGCASE_PING_FREQ_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "default_wltx_client",
		&di->default_wltx_client, WLTX_CLIENT_LIGHTSTRAP);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "identify_delay_time",
		&di->identify_delay_time, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "retry_times",
		&di->retry_times, LIGHTSTRAP_RETRY_TIMES_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "wltx_pwr_src",
		&di->wltx_pwr_src, PWR_SRC_BEGIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "online_state_new",
		&di->online_state_new, 0);
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np, "coolingcase_freq",
		(u32 *)&di->coolingcase_freq, COOLINGCASE_FREQ_LEN)) {
		di->coolingcase_freq.hv_min = COOLINGCASE_WORK_FREQ_DEFAULT;
		di->coolingcase_freq.hv_max = COOLINGCASE_WORK_FREQ_DEFAULT;
		di->coolingcase_freq.lv_min = COOLINGCASE_WORK_FREQ_DEFAULT;
		di->coolingcase_freq.lv_max = COOLINGCASE_WORK_FREQ_DEFAULT;
	}
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np, "coolingcase_duty",
		(u32 *)&di->coolingcase_duty, COOLINGCASE_DUTY_LEN)) {
		di->coolingcase_duty.hv_boot_min = COOLINGCASE_DUTY_DEFAULT;
		di->coolingcase_duty.hv_boot_max = COOLINGCASE_DUTY_DEFAULT;
		di->coolingcase_duty.hv_lp_min = COOLINGCASE_DUTY_DEFAULT;
		di->coolingcase_duty.hv_lp_max = COOLINGCASE_DUTY_DEFAULT;
		di->coolingcase_duty.lv_min = COOLINGCASE_DUTY_DEFAULT;
		di->coolingcase_duty.lv_max = COOLINGCASE_DUTY_DEFAULT;
	}
	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np, "model_id",
		&di->model_id))
		di->model_id = "none";
}

static void wirelesscase_init_delayed_work(struct wirelesscase_di *di)
{
	INIT_WORK(&di->event_work, lightstrap_event_work);
	INIT_DELAYED_WORK(&di->check_work, lightstrap_check_work);
	INIT_DELAYED_WORK(&di->tx_ping_work, lightstrap_tx_ping_work);
	INIT_DELAYED_WORK(&di->control_work, lightstrap_control_work);
	INIT_DELAYED_WORK(&di->identify_work, lightstrap_init_identify_work);
	INIT_DELAYED_WORK(&di->detect_work, lightstrap_detect_work);
	INIT_DELAYED_WORK(&di->delay_close_work, coolingcase_delay_close_work);
	INIT_DELAYED_WORK(&di->duty_low_power_work, set_duty_low_power_work);
	INIT_DELAYED_WORK(&di->monitor_voltage_work, coolingcase_monitor_voltage_work);
}

static int lightstrap_probe(struct platform_device *pdev)
{
	int ret;
	struct wirelesscase_di *di = NULL;
	struct device_node *np = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wirelesscase_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;

	lightstrap_parse_dts(np, di);
	wirelesscase_init_delayed_work(di);
	mutex_init(&di->lock);
	platform_set_drvdata(pdev, di);
	di->startup_identify_timeout = false;
	di->lightstrap_nb.notifier_call = lightstrap_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_LIGHTSTRAP, &di->lightstrap_nb);
	if (ret)
		goto lightstrap_notifier_regist_fail;

	di->connect_nb.notifier_call = lightstrap_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &di->connect_nb);
	if (ret)
		goto connect_notifier_regist_fail;

	ret = wireless_tx_logic_ops_register(&g_lightstrap_logic_ops);
	if (ret)
		goto logic_ops_regist_fail;

	ret = wireless_tx_logic_ops_register(&g_coolingcase_logic_ops);
	if (ret)
		goto logic_ops_regist_fail;

	di->dev = lightstrap_sysfs_create_group();
	lightstrap_set_status(di, LIGHTSTRAP_STATUS_INIT);
	schedule_delayed_work(&di->identify_work, msecs_to_jiffies(di->identify_delay_time));

	return 0;

logic_ops_regist_fail:
	(void)power_event_bnc_unregister(POWER_BNT_CONNECT, &di->connect_nb);
connect_notifier_regist_fail:
	(void)power_event_bnc_unregister(POWER_BNT_LIGHTSTRAP, &di->lightstrap_nb);
lightstrap_notifier_regist_fail:
	platform_set_drvdata(pdev, NULL);
	mutex_destroy(&di->lock);
	kfree(di);
	g_wirelesscase_di = NULL;

	return ret;
}

static int lightstrap_remove(struct platform_device *pdev)
{
	struct wirelesscase_di *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	lightstrap_sysfs_remove_group(di->dev);
	power_event_bnc_unregister(POWER_BNT_LIGHTSTRAP, &di->lightstrap_nb);
	power_event_bnc_unregister(POWER_BNT_CONNECT, &di->connect_nb);
	platform_set_drvdata(pdev, NULL);
	mutex_destroy(&di->lock);
	kfree(di);
	g_wirelesscase_di = NULL;

	return 0;
}

static const struct of_device_id lightstrap_match_table[] = {
	{
		.compatible = "huawei,lightstrap",
		.data = NULL,
	},
	{},
};

static struct platform_driver lightstrap_driver = {
	.probe = lightstrap_probe,
	.remove = lightstrap_remove,
	.driver = {
		.name = "huawei,lightstrap",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lightstrap_match_table),
	},
};

static int __init lightstrap_init(void)
{
	return platform_driver_register(&lightstrap_driver);
}

static void __exit lightstrap_exit(void)
{
	platform_driver_unregister(&lightstrap_driver);
}

device_initcall_sync(lightstrap_init);
module_exit(lightstrap_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless lightstrap module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
