/*
 * Huawei hi1162 wltx
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

#include "hi116x_driver.h"
#include <chipset_common/hwpower/wireless_charge/wireless_accessory.h>

struct wireless_acc_key_info g_wltx_acc_info_tab[WL_TX_ACC_INFO_MAX] = {
	{ "DEVICENO", "0" },
	{ "DEVICESTATE", "UNKNOWN" },
	{ "DEVICEMAC", "FF:FF:FF:FF:FF:FF" },
	{ "DEVICEMODELID", "000000" },
	{ "DEVICESUBMODELID", "00" },
	{ "DEVICEVERSION", "00" },
	{ "DEVICEBUSINESS", "00" },
	{ "DEVICESLEADDRFLAG", "0" },
};

static void hi1162_get_acc_dev_state(u8 state)
{
	switch (state) {
	case WL_ACC_DEV_STATE_ONLINE:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_CONNECTED_STR);
		break;
	case WL_ACC_DEV_STATE_OFFLINE:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_DISCONNECTED_STR);
		break;
	case WL_ACC_DEV_STATE_PINGING:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_PING_STR);
		break;
	case WL_ACC_DEV_STATE_PING_SUCC:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_PING_SUCC_STR);
		break;
	case WL_ACC_DEV_STATE_PING_TIMEOUT:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_PING_TIMEOUT_STR);
		break;
	case WL_ACC_DEV_STATE_PING_ERROR:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_PING_ERR_STR);
		break;
	case WL_ACC_DEV_STATE_CHARGE_DONE:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_CHARGE_DONE_STR);
		break;
	case WL_ACC_DEV_STATE_CHARGE_ERROR:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_CHARGE_ERROR_STR);
		break;
	default:
		snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", ACC_UNKNOWN_STR);
		break;
	}

	sparklink_infomsg("hi1162 get acc state: %s\n",
		g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value);
}

static int hi1162_get_acc_info(struct hi1162_device_info *di, u8 *dev_no)
{
	int ret;
	u8 acc_info[HI1162_ACC_INFO_LEN] = { 0 };

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_block(di->client, HI1162_ACC_INFO_REG, HI1162_ACC_INFO_LEN, acc_info);
	if (ret < 0) {
		sparklink_infomsg("hi1162 read acc info failed\n");
		return -EINVAL;
	}

	*dev_no = acc_info[HI1162_ACC_DEV_NO];
	hi1162_get_acc_dev_state(acc_info[HI1162_ACC_DEV_STATE]);

	snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_NO].value, ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1,
		"%d", acc_info[HI1162_ACC_DEV_NO]);
	sparklink_infomsg("hi1162 get acc NO: %s\n", g_wltx_acc_info_tab[WL_TX_ACC_INFO_NO].value);
	/* the lenth of acc mac is 6 */
	snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_MAC].value, ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1,
		"%02x:%02x:%02x:%02x:%02x:%02x", acc_info[HI1162_ACC_DEV_MAC],
		acc_info[HI1162_ACC_DEV_MAC + 1], acc_info[HI1162_ACC_DEV_MAC + 2], acc_info[HI1162_ACC_DEV_MAC + 3],
		acc_info[HI1162_ACC_DEV_MAC + 4], acc_info[HI1162_ACC_DEV_MAC + 5]);
	/* the lenth of acc model id is 3 */
	snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_MODEL_ID].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%02x%02x%02x", acc_info[HI1162_ACC_DEV_MODEL_ID],
		acc_info[HI1162_ACC_DEV_MODEL_ID + 1], acc_info[HI1162_ACC_DEV_MODEL_ID + 2]);
	sparklink_infomsg("hi1162 get acc model id: %s\n", g_wltx_acc_info_tab[WL_TX_ACC_INFO_MODEL_ID].value);
	snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_SUBMODEL_ID].value, ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1,
		"%02x", acc_info[HI1162_ACC_DEV_SUBMODEL_ID]);
	sparklink_infomsg("hi1162 get acc submodel id: %s\n", g_wltx_acc_info_tab[WL_TX_ACC_INFO_SUBMODEL_ID].value);
	snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_VERSION].value, ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1,
		"%02x", acc_info[HI1162_ACC_DEV_VERSION]);
	sparklink_infomsg("hi1162 get acc version: %s\n", g_wltx_acc_info_tab[WL_TX_ACC_INFO_VERSION].value);
	snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_BUSINESS].value, ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1,
		"%02x", acc_info[HI1162_ACC_DEV_BUSINESS]);
	sparklink_infomsg("hi1162 get acc business: %s\n", g_wltx_acc_info_tab[WL_TX_ACC_INFO_BUSINESS].value);
	snprintf_s(g_wltx_acc_info_tab[WL_TX_ACC_INFO_SLE].value, ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1,
		"%x", acc_info[HI1162_ACC_DEV_INFO_SLE]);
	sparklink_infomsg("hi1162 get acc sle: %s\n", g_wltx_acc_info_tab[WL_TX_ACC_INFO_SLE].value);

	return 0;
}

int hi1162_report_acc_info(struct hi1162_device_info *di)
{
	int ret;
	u8 dev_no;

	if (!di)
		return -EINVAL;

	ret = hi1162_get_acc_info(di, &dev_no);
	if (ret < 0) {
		sparklink_errmsg("hill62 get acc info failed\n");
		return ret;
	}

	sparklink_infomsg("wireless acc report uevent: %s\n",
		g_wltx_acc_info_tab[WL_TX_ACC_INFO_STATE].value);
	wireless_acc_report_uevent(g_wltx_acc_info_tab, WL_TX_ACC_INFO_MAX, dev_no);
	sparklink_infomsg("wltx aux notify uevent end\n");

	return 0;
}

int hi1162_wltx_open_tx(struct hi1162_device_info *di, long val)
{
	int ret;

	if (!di)
		return -EINVAL;

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_write_byte(di->client, HI1162_CMD_REG_ADDR, val);
	if (ret < 0) {
		sparklink_errmsg("hill62 write tx open val %d failed\n", val);
		return -EINVAL;
	}

	sparklink_infomsg("hill62 write tx open val %d success\n", val);
	return 0;
}

void hi1162_wltx_get_rx_product_type(struct hi1162_device_info *di)
{
	u8 rx_product_type[HI1162_RX_PRODUCT_TYPE_READ_LEN] = { 0 };
	int ret;
	int str_len = 0;
	int i;
	int size;

	if (!di)
		return;

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_block(di->client, HI1162_RX_PRODUCT_TYPE_REG,
		HI1162_RX_PRODUCT_TYPE_READ_LEN, rx_product_type);
	if (ret < 0) {
		sparklink_errmsg("hill62 read rx_product_type failed\n");
		return;
	}

	size = sizeof(di->rx_product_type);
	memset_s(di->rx_product_type, size, 0, size);
	for (i = 0; i < sizeof(rx_product_type); i++) {
		snprintf_s(di->rx_product_type + str_len, size - str_len, size - str_len - 1, "%02x", rx_product_type[i]);
		str_len += 2; /* 2: hex -> 2 bytes str */
	}

	sparklink_infomsg("hill62 get rx_product_type: %s\n", di->rx_product_type);
}

void hi1162_wltx_get_tx_info_byte(struct hi1162_device_info *di, u16 *info, u8 reg)
{
	int ret;

	if (!di)
		return;

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_byte(di->client, reg);
	if (ret < 0) {
		sparklink_errmsg("hi1162 get tx info faild\n");
		return;
	}

	*info = ret;
	sparklink_infomsg("hill62 get tx info success\n");
}

void hi1162_wltx_get_tx_info_block(struct hi1162_device_info *di, u16 *info, u8 reg)
{
	int ret;
	u8 tx_info[HI1162_TX_CHARGE_INFO_LEN];

	if (!di || !info)
		return;

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_block(di->client, reg, HI1162_TX_CHARGE_INFO_LEN, tx_info);
	if (ret < 0) {
		sparklink_errmsg("hi1162 get tx info faild\n");
		return;
	}

	*info = (tx_info[0] << 8) | tx_info[1];
	sparklink_errmsg("hi1162 get tx info success\n");
}

void hi1162_get_hall_state(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return;

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_byte(di->client, HI1162_HALL_STATUS_REG);
	if (ret < 0) {
		sparklink_errmsg("hi1162 read hall state reg faild, clear\n");
		di->hall_state = HI1162_DEFAULT_HALL_STATE;
		return;
	}

	di->hall_state = ret;
	sparklink_infomsg("hill62 set hall_state: %u\n", di->hall_state);
}
