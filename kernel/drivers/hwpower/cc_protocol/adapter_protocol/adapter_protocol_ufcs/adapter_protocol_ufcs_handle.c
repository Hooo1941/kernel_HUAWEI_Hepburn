// SPDX-License-Identifier: GPL-2.0
/*
 * adapter_protocol_ufcs_handle.c
 *
 * ufcs protocol irq event handle
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

#include "adapter_protocol_ufcs_handle.h"
#include "adapter_protocol_ufcs_base.h"
#include "adapter_protocol_ufcs_interface.h"
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG ufcs_protocol_handle
HWLOG_REGIST();

static bool g_test_mode;

void hwufcs_handle_set_test_mode(bool flag)
{
	g_test_mode = flag;
}

static bool hwufcs_handle_in_test_mode(void)
{
	return g_test_mode;
}

static void hwufcs_handle_ping(struct hwufcs_package_data *pkt)
{
	hwlog_info("handle ping msg\n");
}

static void hwufcs_handle_soft_reset(struct hwufcs_package_data *pkt)
{
	hwlog_info("handle soft_reset msg\n");
}

static void hwufcs_handle_get_sink_info(struct hwufcs_package_data *pkt)
{
	int ret;
	int bat_curr = 0;
	int bat_volt = 0;
	struct hwufcs_sink_info_data data;

	hwlog_info("handle get_sink_info msg\n");
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CURRENT_NOW, &bat_curr,
		POWER_SUPPLY_DEFAULT_CURRENT_NOW, 1);
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &bat_volt,
		POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
	data.bat_curr = bat_curr;
	data.bat_volt = bat_volt;
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_TEMP, &data.bat_temp,
		POWER_SUPPLY_DEFAULT_TEMP / POWER_SUPPLY_BAT_TEMP_UNIT,
		POWER_SUPPLY_BAT_TEMP_UNIT);

	data.usb_temp = power_temp_get_average_value(POWER_TEMP_USB_PORT) / POWER_MC_PER_C;
	ret = hwufcs_send_sink_information_data_msg(&data);
	if (ret)
		hwufcs_send_control_msg(HWUFCS_CTL_MSG_SOFT_RESET, true);
}

static void hwufcs_handle_get_device_info(struct hwufcs_package_data *pkt)
{
	int ret;
	struct hwufcs_dev_info_data data = { 0 };

	hwlog_info("handle get_device_info msg\n");
	ret = hwufcs_send_device_information_data_msg(&data);
	if (ret)
		hwufcs_send_control_msg(HWUFCS_CTL_MSG_SOFT_RESET, true);
}

static void hwufcs_handle_get_error_info(struct hwufcs_package_data *pkt)
{
	int ret;
	struct hwufcs_error_info_data data = { 0 };

	/* Used for test authentication, no error by default */
	hwlog_info("handle get_error_info msg\n");
	ret = hwufcs_send_error_information_data_msg(&data);
	if (ret)
		hwufcs_send_control_msg(HWUFCS_CTL_MSG_SOFT_RESET, true);
}

static void hwufcs_handle_detect_cable_info(struct hwufcs_package_data *pkt)
{
	hwlog_info("handle detect_cable_info msg\n");
	(void)hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
}

static void hwufcs_handle_start_cable_detect(struct hwufcs_package_data *pkt)
{
	hwlog_info("handle start_cable_detect msg\n");
	(void)hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
}

static void hwufcs_handle_end_cable_detect(struct hwufcs_package_data *pkt)
{
	hwlog_info("handle end_cable_detect msg\n");
	(void)hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
}

static void hwufcs_handle_exit_ufcs_mode(struct hwufcs_package_data *pkt)
{
	hwlog_info("handle exit_ufcs_mode msg\n");
	(void)hwufcs_soft_reset_master();
}

static void hwufcs_handle_error_info(struct hwufcs_package_data *pkt)
{
	int len = 4; /* 4: error info data size */

	hwlog_info("handle error_info msg\n");
	if (pkt->len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n",
			pkt->len, len);
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_IDENTIFY, pkt);
		return;
	}

	hwlog_info("error_info msg=%u %u %u %u\n", pkt->data[0], pkt->data[1],
		pkt->data[2], pkt->data[3]);
}

static void hwufcs_parse_test_request_data(struct hwufcs_test_request_data *p,
	struct hwufcs_package_data *pkt)
{
	u8 len = sizeof(u16);
	u16 data = 0;
	u16 tmp_data;

	if (pkt->len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n", pkt->len, len);
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		return;
	}

	memcpy((u8 *)&data, pkt->data, pkt->len);
	tmp_data = cpu_to_be16(data);
	p->msg_cmd = (tmp_data >> HWUFCS_TEST_REQ_SHIFT_MSG_CMD) &
		HWUFCS_TEST_REQ_MASK_MSG_CMD;
	p->msg_type = (tmp_data >> HWUFCS_TEST_REQ_SHIFT_MSG_TYPE) &
		HWUFCS_TEST_REQ_MASK_MSG_TYPE;
	p->dev_address = (tmp_data >> HWUFCS_TEST_REQ_SHIFT_DEV_ADDRESS) &
		HWUFCS_TEST_REQ_MASK_DEV_ADDRESS;
	p->volt_test_mode = (tmp_data >> HWUFCS_TEST_REQ_SHIFT_VOLT_TEST_MODE) &
		HWUFCS_TEST_REQ_MASK_VOLT_TEST_MODE;
	/* not used */
	p->en_test_mode = (tmp_data >> HWUFCS_TEST_REQ_SHIFT_EN_TEST_MODE) &
		HWUFCS_TEST_REQ_MASK_EN_TEST_MODE;
}

static void hwufcs_handle_test_request_data_msg(u8 cmd, struct hwufcs_package_data *pkt)
{
	struct hwufcs_wtg_data wtg_data = { 0 };
	struct hwufcs_request_data req_data;

	switch (cmd) {
	case HWUFCS_DATA_MSG_REQUEST:
		/* test mode:set curr 1A,volt 8V */
		req_data.output_curr = 1000;
		req_data.output_volt = 8000;
		req_data.output_mode = HWUFCS_REQ_BASE_OUTPUT_MODE;
		(void)hwufcs_send_request_data_msg(&req_data);
		break;
	case HWUFCS_DATA_MSG_CONFIG_WATCHDOG:
		(void)hwufcs_send_config_watchdog_data_msg(&wtg_data);
		break;
	default:
		hwlog_err("in test mode, not support test request data cmd=%u\n", cmd);
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		break;
	}
}

static void hwufcs_handle_test_request_control_msg(u8 cmd, struct hwufcs_package_data *pkt)
{
	switch (cmd) {
	case HWUFCS_CTL_MSG_GET_OUTPUT_CAPABILITIES:
	case HWUFCS_CTL_MSG_GET_SOURCE_INFO:
	case HWUFCS_CTL_MSG_GET_CABLE_INFO:
		(void)hwufcs_send_control_msg(cmd, true);
		break;
	default:
		hwlog_err("in test mode, not support test request control cmd=%u\n", cmd);
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		break;
	}
}

static void hwufcs_handle_test_request(struct hwufcs_package_data *pkt)
{
	struct hwufcs_test_request_data test_data;

	hwlog_info("handle test_request msg\n");
	if (!hwufcs_handle_in_test_mode()) {
		hwlog_err("non test mode, not support test request cmd\n");
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		return;
	}

	hwufcs_parse_test_request_data(&test_data, pkt);
	switch (test_data.msg_type) {
	case HWUFCS_MSG_TYPE_CONTROL:
		hwufcs_handle_test_request_control_msg(test_data.msg_cmd, pkt);
		break;
	case HWUFCS_MSG_TYPE_DATA:
		hwufcs_handle_test_request_data_msg(test_data.msg_cmd, pkt);
		break;
	default:
		hwlog_err("in test mode, not support test request type=%u\n", test_data.msg_type);
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		break;
	}
}

static void hwufcs_handle_power_change(struct hwufcs_package_data *pkt)
{
	int ret;

	hwlog_info("handle power_change msg\n");
	ret = hwufcs_updata_power_change_data(pkt);
	if (ret)
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_IDENTIFY, pkt);
}

static void hwufcs_handle_output_capabilities(struct hwufcs_package_data *pkt)
{
	u8 len = HWUFCS_CAP_MAX_OUTPUT_MODE * sizeof(u64);

	hwlog_info("handle output_capabilities msg\n");
	if (!hwufcs_handle_in_test_mode()) {
		hwlog_err("non test mode, not support unsolicited output_capabilities msg\n");
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		return;
	}

	if ((pkt->len % sizeof(u64) != 0) || (pkt->len > len)) {
		hwlog_err("output_capabilities length=%u,%u invalid\n", pkt->len, len);
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_IDENTIFY, pkt);
	}
}

static void hwufcs_handle_ctrl_msg(struct hwufcs_package_data *pkt)
{
	if (!pkt) {
		hwlog_err("msg is null\n");
		return;
	}

	switch (pkt->cmd) {
	case HWUFCS_CTL_MSG_PING:
		hwufcs_handle_ping(pkt);
		break;
	case HWUFCS_CTL_MSG_SOFT_RESET:
		hwufcs_handle_soft_reset(pkt);
		break;
	case HWUFCS_CTL_MSG_GET_SINK_INFO:
		hwufcs_handle_get_sink_info(pkt);
		break;
	case HWUFCS_CTL_MSG_GET_DEVICE_INFO:
		hwufcs_handle_get_device_info(pkt);
		break;
	case HWUFCS_CTL_MSG_GET_ERROR_INFO:
		hwufcs_handle_get_error_info(pkt);
		break;
	case HWUFCS_CTL_MSG_DETECT_CABLE_INFO:
		hwufcs_handle_detect_cable_info(pkt);
		break;
	case HWUFCS_CTL_MSG_START_CABLE_DETECT:
		hwufcs_handle_start_cable_detect(pkt);
		break;
	case HWUFCS_CTL_MSG_END_CABLE_DETECT:
		hwufcs_handle_end_cable_detect(pkt);
		break;
	case HWUFCS_CTL_MSG_EXIT_HWUFCS_MODE:
		hwufcs_handle_exit_ufcs_mode(pkt);
		break;
	default:
		hwlog_err("ctrl cmd invalid\n");
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		break;
	}
}

static void hwufcs_handle_data_msg(struct hwufcs_package_data *pkt)
{
	if (!pkt) {
		hwlog_err("msg is null\n");
		return;
	}

	switch (pkt->cmd) {
	case HWUFCS_DATA_MSG_ERROR_INFO:
		hwufcs_handle_error_info(pkt);
		break;
	case HWUFCS_DATA_MSG_TEST_REQUEST:
		hwufcs_handle_test_request(pkt);
		break;
	case HWUFCS_DATA_MSG_POWER_CHANGE:
		hwufcs_handle_power_change(pkt);
		break;
	case HWUFCS_DATA_MSG_OUTPUT_CAPABILITIES:
		hwufcs_handle_output_capabilities(pkt);
		break;
	case HWUFCS_DATA_MSG_REFUSE:
		break;
	default:
		hwlog_err("data cmd invalid\n");
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
		break;
	}
}

static void hwufcs_handle_vendor_define_msg(struct hwufcs_package_data *pkt)
{
	if (!pkt) {
		hwlog_err("msg is null\n");
		return;
	}

	hwlog_info("handle vendor_define_msg\n");
	hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
}

void hwufcs_handle_msg(void)
{
	struct hwufcs_package_data pkt = { 0 };
	int ret;

	ret = hwufcs_request_communication(true);
	if (ret)
		return;

	ret = hwufcs_receive_msg(&pkt, false);
	if (ret) {
		hwlog_err("receive msg err\n");
		hwufcs_handle_communication_result(ret, &pkt);
		goto end;
	}

	switch (pkt.msg_type) {
	case HWUFCS_MSG_TYPE_CONTROL:
		hwufcs_handle_ctrl_msg(&pkt);
		break;
	case HWUFCS_MSG_TYPE_DATA:
		hwufcs_handle_data_msg(&pkt);
		break;
	case HWUFCS_MSG_TYPE_VENDOR_DEFINED:
		hwufcs_handle_vendor_define_msg(&pkt);
		break;
	default:
		hwlog_err("msg type invalid\n");
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, &pkt);
		break;
	}

end:
	hwufcs_request_communication(false);
}
