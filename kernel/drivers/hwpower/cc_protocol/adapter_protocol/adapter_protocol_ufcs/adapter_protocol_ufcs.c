// SPDX-License-Identifier: GPL-2.0
/*
 * adapter_protocol_ufcs.c
 *
 * ufcs protocol driver
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

#include "adapter_protocol_ufcs_base.h"
#include "adapter_protocol_ufcs_interface.h"
#include "adapter_protocol_ufcs_handle.h"
#include <chipset_common/hwpower/protocol/adapter_protocol_ufcs_auth.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG ufcs_protocol
HWLOG_REGIST();

enum UFCS_CMD_SET {
	UFCS_CMD_BEGIN = 0,
	UFCS_CMD_GET_OUTPUT_CAPABILITIES = UFCS_CMD_BEGIN,
	UFCS_CMD_GET_SOURCE_INFO,
	UFCS_CMD_GET_DEV_INFO,
	UFCS_CMD_CONFIG_WATCHDOG,
	UFCS_CMD_REQUEST_OUTPUT_DATA,
	UFCS_CMD_GET_ENCRYPTED_VALUE,
	UFCS_CMD_GET_CABLE_INFO,
	UFCS_CMD_START_CABLE_DETECT,
	UFCS_CMD_VDM_GET_SOURCE_ID,
	UFCS_CMD_VDM_GET_SCPB_PWR,
	UFCS_CMD_END,
};

typedef int (*ufcs_cmd_cb)(void *);
struct ufcs_cmd_info {
	unsigned int cmd;
	ufcs_cmd_cb cmd_cb;
};

static struct hwufcs_dev *g_hwufcs_dev;

static struct hwufcs_dev *hwufcs_get_dev(void)
{
	if (!g_hwufcs_dev) {
		hwlog_err("g_hwufcs_dev is null\n");
		return NULL;
	}

	return g_hwufcs_dev;
}

static int hwufcs_check_communication_state(unsigned int code)
{
	if (!code)
		return HWUFCS_OK;

	switch (code) {
	case HWUFCS_ERR_TIMEOUT:
	case HWUFCS_ERR_UNEXPECT_DATA:
	case HWUFCS_ERR_REFUSED_DATA:
	case HWUFCS_ERR_ILLEGAL_DATA:
	case HWUFCS_ERR_UNSUPPORT_DATA:
		return HWUFCS_NEED_RETRY;
	default:
		break;
	}

	return HWUFCS_FAIL;
}

static int hwufcs_set_default_state(void)
{
	int ret;

	ret = hwufcs_request_communication(true);
	if (ret)
		return -EPERM;

	ret = hwufcs_send_control_msg(HWUFCS_CTL_MSG_EXIT_HWUFCS_MODE, true);
	ret += hwufcs_request_communication(false);
	/* disable v510 ufcs when exit ufcs mode to solve bc12 failure */
	hwufcs_soft_reset_master();

	return ret;
}

static int hwufcs_set_default_param(void)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev)
		return -EPERM;

	(void)hwufcs_request_communication(false);
	memset(&l_dev->info, 0, sizeof(l_dev->info));
	return 0;
}

static int hwufcs_soft_reset_slave(void)
{
	int ret;

	hwlog_info("soft_reset_slave\n");
	ret = hwufcs_request_communication(true);
	if (ret)
		return -EPERM;

	ret = hwufcs_send_control_msg(HWUFCS_CTL_MSG_SOFT_RESET, true);
	ret += hwufcs_clear_rx_buff();
	ret += hwufcs_request_communication(false);
	return ret;
}

static int hwufcs_get_output_capabilities_cmd(void *p)
{
	int ret;
	struct hwufcs_dev *l_dev = p;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_control_msg(HWUFCS_CTL_MSG_GET_OUTPUT_CAPABILITIES, true);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	l_dev->info.output_mode = 0;
	ret = hwufcs_receive_output_capabilities_data_msg(
		&l_dev->info.cap[0], &l_dev->info.mode_quantity);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_get_source_info_cmd(void *p)
{
	int ret;
	struct hwufcs_source_info_data *data = p;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_control_msg(HWUFCS_CTL_MSG_GET_SOURCE_INFO, true);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_source_info_data_msg(data);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_get_dev_info_cmd(void *p)
{
	int ret;
	struct hwufcs_dev_info_data *data = p;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_control_msg(HWUFCS_CTL_MSG_GET_DEVICE_INFO, true);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_dev_info_data_msg(data);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_config_watchdog_cmd(void *p)
{
	int ret;
	struct hwufcs_wtg_data *data = p;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_config_watchdog_data_msg(data);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACCEPT, false);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_request_output_data_cmd(void *p)
{
	int i, ret;
	struct hwufcs_request_data *req = p;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_request_data_msg(req);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACCEPT, false);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	for (i = 0; i < HWUFCS_POWER_READY_RETRY; i++) {
		ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_POWER_READY, false);
		if (!ret)
			break;
	}
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_get_encrypted_value_cmd(void *p)
{
	int ret;
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	struct hwufcs_verify_data *data = p;

	if (!l_dev)
		return -EPERM;

	if (hwufcs_request_communication(true))
		return -EPERM;

	ret = hwufcs_send_verify_request_data_msg(&data->req_data);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACCEPT, false);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_GET_DEVICE_INFO, false);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_send_device_information_data_msg(&l_dev->info.dev_info);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_verify_response_data_msg(&data->rsp_data);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_get_cable_info_cmd(void *p)
{
	int ret;
	struct hwufcs_cable_info_data *data = p;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_control_msg_to_cable(HWUFCS_CTL_MSG_GET_CABLE_INFO, true);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_cable_info_data_msg(data);
	ret = hwufcs_check_communication_state(ret);

end:
	hwufcs_hard_reset_cable();
	hwufcs_send_control_msg(HWUFCS_CTL_MSG_END_CABLE_DETECT, true);
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_start_cable_detect_cmd(void *p)
{
	int ret;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_control_msg(HWUFCS_CTL_MSG_START_CABLE_DETECT, true);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACCEPT, false);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_vdm_get_source_id(void *p)
{
	int ret;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_vdm_control_msg(HWUFCS_VDM_CTL_MSG_GET_SOURCE_ID, true);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_vdm_source_id_data_msg((u16 *)p);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static int hwufcs_vdm_get_scpb_pwr(void *p)
{
	int ret;
	struct hwufcs_dev *l_dev = p;

	if (hwufcs_request_communication(true))
		return HWUFCS_FAIL;

	ret = hwufcs_send_vdm_control_msg(HWUFCS_VDM_CTL_MSG_GET_SCPB_POWER, true);
	ret = hwufcs_check_communication_state(ret);
	if (ret)
		goto end;

	ret = hwufcs_receive_vdm_scpb_pwr_data_msg(
		&l_dev->info.scpb_pwr_data[0], &l_dev->info.scpb_pwr_quantity);
	ret = hwufcs_check_communication_state(ret);

end:
	if (hwufcs_request_communication(false))
		return HWUFCS_FAIL;

	return ret;
}

static struct ufcs_cmd_info g_ufcs_cmd_data[] = {
	{ UFCS_CMD_GET_OUTPUT_CAPABILITIES, hwufcs_get_output_capabilities_cmd},
	{ UFCS_CMD_GET_SOURCE_INFO, hwufcs_get_source_info_cmd },
	{ UFCS_CMD_GET_DEV_INFO, hwufcs_get_dev_info_cmd },
	{ UFCS_CMD_CONFIG_WATCHDOG, hwufcs_config_watchdog_cmd },
	{ UFCS_CMD_REQUEST_OUTPUT_DATA, hwufcs_request_output_data_cmd },
	{ UFCS_CMD_GET_ENCRYPTED_VALUE, hwufcs_get_encrypted_value_cmd },
	{ UFCS_CMD_GET_CABLE_INFO, hwufcs_get_cable_info_cmd },
	{ UFCS_CMD_START_CABLE_DETECT, hwufcs_start_cable_detect_cmd },
	{ UFCS_CMD_VDM_GET_SOURCE_ID, hwufcs_vdm_get_source_id },
	{ UFCS_CMD_VDM_GET_SCPB_PWR, hwufcs_vdm_get_scpb_pwr },
};

static int hwufcs_cmd_retry(int cmd, void *p)
{
	int i, ret;
	int cnt = 0;
	int data_size = ARRAY_SIZE(g_ufcs_cmd_data);

	for (i = 0; i < data_size; i++) {
		if (g_ufcs_cmd_data[i].cmd == cmd)
			break;
	}

	if (i == data_size)
		return -EPERM;

	do {
		ret = g_ufcs_cmd_data[i].cmd_cb(p);
		if ((ret == HWUFCS_OK) || (ret == HWUFCS_FAIL))
			break;

		if (!g_hwufcs_dev->plugged_state || hwufcs_soft_reset_slave())
			break;
	} while (cnt++ < RETRY_THREE);

	return ret;
}

static int hwufcs_get_output_capabilities(struct hwufcs_dev *l_dev)
{
	int ret;

	if (!l_dev)
		return -EPERM;

	if (l_dev->info.outout_capabilities_rd_flag == HAS_READ_FLAG)
		return 0;

	ret = hwufcs_cmd_retry(UFCS_CMD_GET_OUTPUT_CAPABILITIES, (void *)l_dev);
	if (ret)
		return -EPERM;

	l_dev->info.outout_capabilities_rd_flag = HAS_READ_FLAG;
	return 0;
}

static int hwufcs_get_source_info(struct adapter_source_info *p)
{
	int ret;
	struct hwufcs_source_info_data data = { 0 };

	if (!p)
		return -EPERM;

	/* delay 20ms to avoid send msg densely */
	power_usleep(DT_USLEEP_20MS);
	ret = hwufcs_cmd_retry(UFCS_CMD_GET_SOURCE_INFO, (void *)&data);
	if (ret)
		return -EPERM;

	p->output_volt = data.output_volt;
	p->output_curr = data.output_curr;
	p->port_temp = data.port_temp;
	p->port_temp = data.port_temp;
	return 0;
}

static int hwufcs_get_dev_info(void)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	int ret;

	if (!l_dev)
		return -EPERM;

	if (l_dev->info.dev_info_rd_flag == HAS_READ_FLAG)
		return 0;

	ret = hwufcs_cmd_retry(UFCS_CMD_GET_DEV_INFO, (void *)&l_dev->info.dev_info);
	if (ret)
		return -EPERM;

	l_dev->info.dev_info_rd_flag = HAS_READ_FLAG;
	return 0;
}

static int hwufcs_config_watchdog(struct hwufcs_wtg_data *p)
{
	int ret;

	if (!p)
		return -EPERM;

	ret = hwufcs_cmd_retry(UFCS_CMD_CONFIG_WATCHDOG, (void *)p);
	return ret ? -EPERM : 0;
}

static int hwufcs_send_ping(void)
{
	int i, ret;

	/* protocol handshark: detect ufcs adapter */
	ret = hwufcs_detect_adapter();
	if (ret == HWUFCS_DETECT_FAIL) {
		hwlog_err("ufcs adapter detect fail\n");
		return ADAPTER_DETECT_FAIL;
	}

	for (i = 0; i < HWUFCS_BAUD_RATE_END; i++) {
		if (!g_hwufcs_dev->plugged_state)
			return ADAPTER_DETECT_FAIL;
		(void)hwufcs_config_baud_rate(i);
		ret = hwufcs_request_communication(true);
		if (ret)
			return -EPERM;

		ret = hwufcs_send_control_msg(HWUFCS_CTL_MSG_PING, true);
		(void)hwufcs_request_communication(false);
		if (ret == 0)
			break;
	}

	if (ret) {
		hwlog_err("ufcs adapter ping fail\n");
		hwufcs_soft_reset_master();
		return ADAPTER_DETECT_FAIL;
	}

	hwlog_info("ufcs adapter ping succ\n");
	return 0;
}

static void hwufcs_check_support_mode(struct hwufcs_device_info *info, int *mode)
{
	int vadap_min = info->cap[0].min_volt;
	int vadap_max = info->cap[info->mode_quantity - 1].max_volt;

	if ((vadap_min <= HWUFCS_5V_VOLT_MIN) && (vadap_max >=HWUFCS_5V_VOLT_MAX))
		*mode |= ADAPTER_SUPPORT_LVC;
	if ((vadap_min <= HWUFCS_10V_VOLT_MIN) && (vadap_max >=HWUFCS_10V_VOLT_MAX))
		*mode |= ADAPTER_SUPPORT_SC;
	if ((vadap_min <= HWUFCS_20V_VOLT_MIN) && (vadap_max >=HWUFCS_20V_VOLT_MAX))
		*mode |= ADAPTER_SUPPORT_SC4;
}

static int hwufcs_detect_adapter_support_mode(int *mode)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	struct hwufcs_wtg_data wtg = { 0 };
	int ret;

	if (!l_dev || !mode)
		return ADAPTER_DETECT_FAIL;

	/* set all parameter to default state */
	hwufcs_set_default_param();
	hwufcs_clear_power_change_data();
	l_dev->info.detect_finish_flag = 1; /* has detect adapter */
	l_dev->info.support_mode = ADAPTER_SUPPORT_UNDEFINED;

	/* send ping */
	ret = hwufcs_send_ping();
	if (ret)
		return ret;

	/*
	 * to solve suger cube charger bug
	 * When SCP protocol is switched to UFCS protocol, the charger does not respond and the VBUS falls, resulting in a failure of charge.
	 * need send a specific cmd after ping to fix the bug.
	 */
	hwufcs_send_specific_msg();

	/* check test mode:if reserved_1 is 0xffff and reserved_2 is 0xffff */
	if (hwufcs_get_dev_info())
		goto err_out;
	if ((l_dev->info.dev_info.reserved_1 == 0xffff) && (l_dev->info.dev_info.reserved_2 == 0xffff)) {
		l_dev->info.support_mode = ADAPTER_TEST_MODE;
		hwufcs_handle_set_test_mode(true);
		return ADAPTER_DETECT_SUCC;
	}

	/* disable wdt func */
	hwufcs_config_watchdog(&wtg);

	if (hwufcs_get_output_capabilities(l_dev))
		goto err_out;

	hwufcs_check_support_mode(&l_dev->info, mode);
	hwufcs_handle_set_test_mode(false);
	l_dev->info.support_mode = *mode;
	return ADAPTER_DETECT_SUCC;

err_out:
	hwufcs_set_default_state();
	return ADAPTER_DETECT_FAIL;
}

static int hwufcs_get_support_mode(int *mode)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !mode)
		return -EPERM;

	if (l_dev->info.detect_finish_flag)
		*mode = l_dev->info.support_mode;
	else
		hwufcs_detect_adapter_support_mode(mode);

	hwlog_info("support_mode: %d\n", *mode);
	return 0;
}

static int hwufcs_set_init_data(struct adapter_init_data *data)
{
	struct hwufcs_wtg_data wtg;

	wtg.time = data->watchdog_timer * HWUFCS_WTG_UNIT_TIME;
	if (hwufcs_config_watchdog(&wtg))
		return -EPERM;

	hwlog_info("set_init_data\n");
	return 0;
}

static int hwufcs_get_inside_temp(int *temp)
{
	struct adapter_source_info data;

	if (!temp)
		return -EPERM;

	if (hwufcs_get_source_info(&data))
		return -EPERM;

	*temp = data.dev_temp;

	hwlog_info("get_inside_temp: %d\n", *temp);
	return 0;
}

static int hwufcs_get_port_temp(int *temp)
{
	struct adapter_source_info data;

	if (!temp)
		return -EPERM;

	if (hwufcs_get_source_info(&data))
		return -EPERM;

	*temp = data.port_temp;

	hwlog_info("get_port_temp: %d\n", *temp);
	return 0;
}

static int hwufcs_get_chip_vendor_id(int *id)
{
	if (!id)
		return -EPERM;

	*id = ADAPTER_CHIP_UNKNOWN;
	hwlog_info("get_chip_vendor_id: %d\n", *id);
	return 0;
}

static int hwufcs_get_hw_version_id(int *id)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !id)
		return -EPERM;

	if (hwufcs_get_dev_info())
		return -EPERM;

	*id = l_dev->info.dev_info.hw_ver;

	hwlog_info("get_hw_version_id_f: 0x%x\n", *id);
	return 0;
}

static int hwufcs_get_sw_version_id(int *id)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !id)
		return -EPERM;

	if (hwufcs_get_dev_info())
		return -EPERM;

	*id = l_dev->info.dev_info.sw_ver;

	hwlog_info("get_sw_version_id_f: 0x%x\n", *id);
	return 0;
}

static int hwufcs_get_adp_type(int *type)
{
	if (!type)
		return -EPERM;

	*type = ADAPTER_TYPE_UNKNOWN;
	return 0;
}

static int hwufcs_request_output_data(struct hwufcs_request_data req)
{
	int ret;

	ret = hwufcs_cmd_retry(UFCS_CMD_REQUEST_OUTPUT_DATA, (void *)&req);
	return ret ? -EPERM : 0;
}

static int hwufcs_set_output_voltage(int volt)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	struct hwufcs_request_data req;
	int i;

	if (!l_dev)
		return -EPERM;

	for (i = 0; i < HWUFCS_CAP_MAX_OUTPUT_MODE; i++) {
		if ((volt > l_dev->info.cap[i].min_volt) && (volt <= l_dev->info.cap[i].max_volt)) {
			break;
		}
	}

	if (i == HWUFCS_CAP_MAX_OUTPUT_MODE) {
		hwlog_err("choose mode fail, request volt=%d\n", volt);
		return -EPERM;
	}

	/* save current output voltage */
	l_dev->info.output_volt = volt;
	l_dev->info.output_mode = l_dev->info.cap[i].output_mode;

	if ((l_dev->info.output_curr == 0) || (l_dev->info.output_curr > l_dev->info.cap[i].max_curr))
		req.output_curr = l_dev->info.cap[i].max_curr;
	else
		req.output_curr = l_dev->info.output_curr;
	req.output_mode = l_dev->info.output_mode;
	req.output_volt = volt;

	/* delay 20ms to avoid send msg densely */
	power_usleep(DT_USLEEP_20MS);
	return hwufcs_request_output_data(req);
}

static int hwufcs_set_output_current(int cur)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	struct hwufcs_request_data req;
	int max_curr = 0;
	int i;

	if (!l_dev)
		return -EPERM;

	for (i = 0; i < HWUFCS_CAP_MAX_OUTPUT_MODE; i++) {
		if (l_dev->info.cap[i].output_mode == l_dev->info.output_mode) {
			max_curr = l_dev->info.cap[i].max_curr;
			break;
		}
	}

	/* save current output current */
	l_dev->info.output_curr = cur > max_curr ? max_curr : cur;
	req.output_mode = l_dev->info.output_mode;
	req.output_curr = l_dev->info.output_curr;
	req.output_volt = l_dev->info.output_volt;

	/* delay 20ms to avoid send msg densely */
	power_usleep(DT_USLEEP_20MS);
	return hwufcs_request_output_data(req);
}

static int hwufcs_get_output_voltage(int *volt)
{
	struct adapter_source_info data;

	if (!volt)
		return -EPERM;

	if (hwufcs_get_source_info(&data))
		return -EPERM;

	*volt = data.output_volt;

	hwlog_info("get_output_voltage: %d\n", *volt);
	return 0;
}

static int hwufcs_get_output_current(int *cur)
{
	struct adapter_source_info data;

	if (!cur)
		return -EPERM;

	if (hwufcs_get_source_info(&data))
		return -EPERM;

	*cur = data.output_curr;

	hwlog_info("get_output_current: %d\n", *cur);
	return 0;
}

static int hwufcs_get_output_current_set(int *cur)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !cur)
		return -EPERM;

	*cur = l_dev->info.output_curr;

	hwlog_info("get_output_current_set: %d\n", *cur);
	return 0;
}

static int hwufcs_get_power_curve(struct adp_pwr_curve_para *val, int *num, int max_size)
{
	unsigned int i;
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !val || !num)
		return -EPERM;

	if (hwufcs_get_output_capabilities(l_dev))
		return -EPERM;

	*num = l_dev->info.mode_quantity;
	hwlog_info("get power_curve_num=%d\n", l_dev->info.mode_quantity);

	/* sync output cap to power curve: 0 max volt; 1 max curr */
	if (*num * 2 > max_size)
		return -EPERM;

	for (i = 0; i < *num; i++) {
		val[i].volt = l_dev->info.cap[i].max_volt;
		val[i].cur = l_dev->info.cap[i].max_curr;
	}

	return 0;
}

static int hwufcs_get_min_voltage(int *volt)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !volt)
		return -EPERM;

	if (hwufcs_get_output_capabilities(l_dev))
		return -EPERM;

	if (l_dev->info.mode_quantity < HWUFCS_REQ_BASE_OUTPUT_MODE)
		return -EPERM;

	*volt = l_dev->info.cap[0].min_volt;
	hwlog_info("get_min_voltage: %d\n", *volt);
	return 0;
}

static int hwufcs_get_max_voltage(int *volt)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	u8 cap_index;

	if (!l_dev || !volt)
		return -EPERM;

	if (hwufcs_get_output_capabilities(l_dev))
		return -EPERM;

	cap_index = l_dev->info.mode_quantity - HWUFCS_REQ_BASE_OUTPUT_MODE;
	*volt = l_dev->info.cap[cap_index].max_volt;

	hwlog_info("get_max_voltage: %d\n", *volt);
	return 0;
}

static int hwufcs_get_min_current(int *cur)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	u16 min_curr = l_dev->info.cap[0].min_curr;
	int i;

	if (!l_dev || !cur)
		return -EPERM;

	if (hwufcs_get_output_capabilities(l_dev))
		return -EPERM;

	for (i = 1; i < l_dev->info.mode_quantity; i++) {
		if (min_curr > l_dev->info.cap[i].min_curr)
			min_curr = l_dev->info.cap[i].min_curr;
	}

	*cur = min_curr;
	hwlog_info("get_min_current: %d\n", *cur);
	return 0;
}

static int hwufcs_get_max_current(int *cur)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	u16 max_curr = l_dev->info.cap[0].max_curr;
	int i;

	if (!l_dev || !cur)
		return -EPERM;

	if (hwufcs_get_output_capabilities(l_dev))
		return -EPERM;

	for (i = 1; i < l_dev->info.mode_quantity; i++) {
		if (max_curr < l_dev->info.cap[i].max_curr)
			max_curr = l_dev->info.cap[i].max_curr;
	}

	*cur = max_curr;
	hwlog_info("get_max_current: %d\n", *cur);
	return 0;
}

static int hwufcs_get_max_power(int *max_pwr)
{
	int i;
	int max_power = 0;
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !max_pwr)
		return -EPERM;

	if (hwufcs_get_output_capabilities(l_dev))
		return -EPERM;

	for (i = 0; i < l_dev->info.mode_quantity; i++) {
		if (l_dev->info.cap[i].max_curr * l_dev->info.cap[i].max_volt > max_power)
			max_power = l_dev->info.cap[i].max_curr * l_dev->info.cap[i].max_volt;
	}

	*max_pwr = max_power / POWER_MW_PER_W;
	hwlog_info("get_max_power: %d\n", *max_pwr);
	return 0;
}

static int hwufcs_get_power_drop_current(int *cur)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	int max_curr;

	if (!l_dev || !cur)
		return -EPERM;

	max_curr = hwufcs_get_power_change_curr(l_dev->info.output_mode);
	if (max_curr > 0)
		*cur = max_curr;

	hwlog_info("get_power_drop_cur: %d\n", *cur);
	return 0;
}

static int hwufcs_get_encrypted_value(struct hwufcs_verify_data *data)
{
	int ret;

	ret = hwufcs_cmd_retry(UFCS_CMD_GET_ENCRYPTED_VALUE, (void *)data);
	return ret ? -EPERM : 0;
}

/* When the authentication of some UFCS third-party adapters fails,
 * the watchdog setting and output capability obtaining records are cleared.
 * After the reset is triggered, the Get_Output_Capabilities command needs to
 * be executed again. */
static int hwufcs_retry_get_out_capabilities(void)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();
	int ret;
	struct hwufcs_wtg_data wtg = { 0 };

	/* disable wdt func */
	if (hwufcs_config_watchdog(&wtg))
		return -EPERM;

	ret = hwufcs_cmd_retry(UFCS_CMD_GET_OUTPUT_CAPABILITIES, (void *)l_dev);
	if (ret)
		return -EPERM;

	return 0;
}

static int hwufcs_auth_encrypt_start(int key)
{
	struct hwufcs_verify_data data;
	int ret, i;
	u8 *hash = hwufcs_auth_get_hash_data_header();

	if (!hash)
		return -EPERM;

	/* first: set key index */
	data.req_data.encrypt_index = key;

	/* second: host set random num to slave */
	for (i = 0; i < HWUFCS_RANDOM_SIZE; i++)
		get_random_bytes(&data.req_data.random[i], sizeof(u8));

	ret = hwufcs_get_encrypted_value(&data);
	if (ret) {
		ret = hwufcs_retry_get_out_capabilities();
		if (ret)
			hwlog_err("get_output_capabilities second fail\n");
		goto end;
	}

	/* third: copy hash value */
	hwufcs_auth_clean_hash_data();
	for (i = 0; i < HWUFCS_RANDOM_SIZE; i++)
		hash[i] = data.req_data.random[i];
	for (i = 0; i < HWUFCS_RANDOM_SIZE; i++)
		hash[i + HWUFCS_RANDOM_SIZE] = data.rsp_data.random[i];
	for (i = 0; i < HWUFCS_ENCRYPT_SIZE; i++)
		hash[i + HWUFCS_RANDOM_SIZE * 2] = data.rsp_data.encrypt[i];
	hash[HWUFCS_AUTH_HASH_LEN - 1] = (unsigned char)key;

	/* forth: wait hash calculate complete */
	ret = hwufcs_auth_wait_completion();
end:
	hwlog_info("auth_encrypt_start ret=%d\n", ret);
	hwufcs_auth_clean_hash_data();
	return ret;
}

static int hwufcs_get_device_info(struct adapter_device_info *info)
{
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !info)
		return -EPERM;

	if (hwufcs_get_hw_version_id(&info->hwver))
		return -EPERM;

	if (hwufcs_get_sw_version_id(&info->fwver))
		return -EPERM;

	if (hwufcs_get_min_voltage(&info->min_volt))
		return -EPERM;

	if (hwufcs_get_max_voltage(&info->max_volt))
		return -EPERM;

	if (hwufcs_get_min_current(&info->min_cur))
		return -EPERM;

	if (hwufcs_get_max_current(&info->max_cur))
		return -EPERM;

	info->volt_step = l_dev->info.cap[0].volt_step;
	info->curr_step = l_dev->info.cap[0].curr_step;
	info->output_mode = l_dev->info.cap[0].output_mode;

	hwlog_info("get_device_info\n");
	return 0;
}

static int hwufcs_get_protocol_register_state(void)
{
	return hwufcs_check_dev_id();
}

static int hwufcs_get_cable_info(int *curr)
{
	int ret;
	struct hwufcs_cable_info_data data = { 0 };
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!curr || !l_dev)
		return -EPERM;

	if (hwufcs_ignore_get_cable_info()) {
		hwlog_info("ignore_cable_info_detect\n");
		return -EPERM;
	}

	hwlog_info("detect_cable begin\n");
	if (l_dev->info.cable_info_rd_flag == HAS_READ_FLAG) {
		hwlog_info("already get cable info\n");
		*curr = l_dev->info.cable_info.max_curr;
		return 0;
	}

	hwufcs_hard_reset_master_mask(1);
	ret = hwufcs_cmd_retry(UFCS_CMD_START_CABLE_DETECT, NULL);
	if (ret)
		goto end;

	ret = hwufcs_get_cable_info_cmd((void *)&data);
	if (ret != HWUFCS_OK)
		goto end;

	l_dev->info.cable_info_rd_flag = HAS_READ_FLAG;
	*curr = data.max_curr;

end:
	hwufcs_hard_reset_master_mask(0);
	return ret ? -EPERM : 0;
}

static int hwufcs_get_vdm_source_id(int *source_id)
{
	int ret;

	if (!source_id)
		return -EPERM;

	hwlog_info("get_vdm_source_id begin\n");
	ret = hwufcs_cmd_retry(UFCS_CMD_VDM_GET_SOURCE_ID, (void *)source_id);
	if (ret)
		return -EPERM;

	return 0;
}

static int hwufcs_get_vdm_scpb_pwr(struct adp_scpb_pwr_data *scpb_pwr, int *size, int max_size)
{
	int ret, i;
	struct hwufcs_dev *l_dev = hwufcs_get_dev();

	if (!l_dev || !scpb_pwr)
		return -EPERM;

	hwlog_info("get_vdm_scpb_pwr begin\n");
	ret = hwufcs_cmd_retry(UFCS_CMD_VDM_GET_SCPB_PWR, (void *)l_dev);
	if (ret)
		return -EPERM;

	if (l_dev->info.scpb_pwr_quantity > max_size)
		return -EPERM;

	*size = l_dev->info.scpb_pwr_quantity;
	for (i = 0; i< *size; i++)
		scpb_pwr[i] = l_dev->info.scpb_pwr_data[i];

	return 0;
}

static bool hwufcs_is_scp_superior(void)
{
	int source_id = 0;

	hwufcs_get_vdm_source_id(&source_id);

	return source_id == HWUFCS_ADP_TYPE_FCR_66W; /* 1: FCR with ufcs protocol adaptor */
}

static struct adapter_protocol_ops adapter_protocol_hwufcs_ops = {
	.type_name = "hw_ufcs",
	.detect_adapter_support_mode = hwufcs_detect_adapter_support_mode,
	.get_support_mode = hwufcs_get_support_mode,
	.set_default_state = hwufcs_set_default_state,
	.set_default_param = hwufcs_set_default_param,
	.set_init_data = hwufcs_set_init_data,
	.soft_reset_master = hwufcs_soft_reset_master,
	.soft_reset_slave = hwufcs_soft_reset_slave,
	.get_chip_vendor_id = hwufcs_get_chip_vendor_id,
	.get_adp_type = hwufcs_get_adp_type,
	.get_inside_temp = hwufcs_get_inside_temp,
	.get_port_temp = hwufcs_get_port_temp,
	.get_source_info = hwufcs_get_source_info,
	.set_output_voltage = hwufcs_set_output_voltage,
	.get_output_voltage = hwufcs_get_output_voltage,
	.set_output_current = hwufcs_set_output_current,
	.get_output_current = hwufcs_get_output_current,
	.get_output_current_set = hwufcs_get_output_current_set,
	.get_min_voltage = hwufcs_get_min_voltage,
	.get_max_voltage = hwufcs_get_max_voltage,
	.get_min_current = hwufcs_get_min_current,
	.get_max_current = hwufcs_get_max_current,
	.get_max_power = hwufcs_get_max_power,
	.get_power_drop_current = hwufcs_get_power_drop_current,
	.get_power_curve = hwufcs_get_power_curve,
	.auth_encrypt_start = hwufcs_auth_encrypt_start,
	.get_device_info = hwufcs_get_device_info,
	.get_protocol_register_state = hwufcs_get_protocol_register_state,
	.get_cable_info = hwufcs_get_cable_info,
	.get_source_id = hwufcs_get_vdm_source_id,
	.get_scpb_pwr = hwufcs_get_vdm_scpb_pwr,
	.is_scp_superior = hwufcs_is_scp_superior,
};

static void hwufcs_handle_msg_work(struct work_struct *work)
{
	if (!work)
		return;

	hwufcs_handle_msg();
}

static int hwufcs_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct hwufcs_dev *di = hwufcs_get_dev();

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_USB_DISCONNECT:
		di->plugged_state = false;
		break;
	case POWER_NE_USB_CONNECT:
		di->plugged_state = true;
		break;
	case POWER_NE_UFCS_REC_UNSOLICITED_DATA:
		hwlog_info("receive unsolicited msg\n");
		schedule_work(&di->handle_msg_work);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int __init hwufcs_init(void)
{
	int ret;
	struct hwufcs_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_hwufcs_dev = l_dev;
	l_dev->plugged_state = false;
	INIT_WORK(&l_dev->handle_msg_work, hwufcs_handle_msg_work);
	l_dev->event_ufcs_nb.notifier_call = hwufcs_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_UFCS, &l_dev->event_ufcs_nb);
	if (ret)
		goto fail_register_ops;
	l_dev->event_nb.notifier_call = hwufcs_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &l_dev->event_nb);
	if (ret)
		goto fail_register_ops;


	ret = adapter_protocol_ops_register(&adapter_protocol_hwufcs_ops);
	if (ret)
		goto fail_register_ops;

	hwufcs_init_msg_number_lock();
	return 0;

fail_register_ops:
	kfree(l_dev);
	g_hwufcs_dev = NULL;
	return ret;
}

static void __exit hwufcs_exit(void)
{
	if (!g_hwufcs_dev)
		return;

	power_event_bnc_unregister(POWER_BNT_UFCS, &g_hwufcs_dev->event_ufcs_nb);
	power_event_bnc_unregister(POWER_BNT_CONNECT, &g_hwufcs_dev->event_nb);
	hwufcs_destroy_msg_number_lock();
	kfree(g_hwufcs_dev);
	g_hwufcs_dev = NULL;
}

subsys_initcall_sync(hwufcs_init);
module_exit(hwufcs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ufcs protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
