// SPDX-License-Identifier: GPL-2.0
/*
 * cps8601_qi.c
 *
 * cps8601 qi_protocol driver; ask: rx->tx; fsk: tx->rx
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "cps8601.h"

#define HWLOG_TAG wireless_cps8601_qi
HWLOG_REGIST();

static u8 cps8601_get_fsk_header(struct cps8601_dev_info *di, int data_len)
{
	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_fsk_hdr) {
		hwlog_err("get_fsk_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_fsk_hdr(data_len);
}

static int cps8601_qi_send_ask_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	return 0;
}

static int cps8601_qi_send_ask_msg_ack(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	return 0;
}

static int cps8601_qi_receive_fsk_msg(u8 *data, int data_len, void *dev_data)
{
	return 0;
}

static int cps8601_qi_send_fsk_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[CPS8601_SEND_MSG_DATA_LEN] = { 0 };
	struct cps8601_dev_info *di = dev_data;

	if ((data_len > CPS8601_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_fsk_msg: data number out of range\n");
		return -EINVAL;
	}

	if (cmd == HWQI_CMD_ACK)
		header = HWQI_CMD_ACK_HEAD;
	else
		header = cps8601_get_fsk_header(di, data_len + 1);
	if (header <= 0) {
		hwlog_err("send_fsk_msg: header wrong\n");
		return -EINVAL;
	}
	ret = cps8601_write_byte(di, CPS8601_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("send_fsk_msg: write header failed\n");
		return ret;
	}
	ret = cps8601_write_byte(di, CPS8601_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("send_fsk_msg: write cmd failed\n");
		return ret;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = cps8601_write_block(di, CPS8601_SEND_MSG_DATA_ADDR, write_data, data_len);
		if (ret) {
			hwlog_err("send_fsk_msg: write fsk reg failed\n");
			return ret;
		}
	}
	ret = cps8601_write_word_mask(di, CPS8601_TX_CMD_ADDR, CPS8601_TX_CMD_SEND_MSG,
		CPS8601_TX_CMD_SEND_MSG_SHIFT, CPS8601_TX_CMD_VAL);
	if (ret) {
		hwlog_err("send_fsk_msg: send fsk failed\n");
		return ret;
	}

	hwlog_info("[send_fsk_msg] succ\n");
	return 0;
}

static int cps8601_qi_receive_ask_pkt(u8 *pkt_data, int pkt_data_len, void *dev_data)
{
	int ret;
	int i;
	char buff[CPS8601_RCVD_PKT_BUFF_LEN] = { 0 };
	char pkt_str[CPS8601_RCVD_PKT_STR_LEN] = { 0 };
	struct cps8601_dev_info *di = dev_data;

	if (!pkt_data || (pkt_data_len <= 0) || (pkt_data_len > CPS8601_RCVD_MSG_PKT_LEN)) {
		hwlog_err("get_ask_pkt: para err\n");
		return -EINVAL;
	}
	ret = cps8601_read_block(di, CPS8601_RCVD_MSG_HEADER_ADDR, pkt_data, pkt_data_len);
	if (ret) {
		hwlog_err("get_ask_pkt: read failed\n");
		return ret;
	}
	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, CPS8601_RCVD_PKT_BUFF_LEN, "0x%02x ", pkt_data[i]);
		strncat(pkt_str, buff, CPS8601_RCVD_PKT_BUFF_LEN);
	}

	hwlog_info("[get_ask_pkt] RX back packet: %.10s\n", pkt_str);
	return 0;
}

static int cps8601_qi_set_rpp_format(u8 pmax, int mode, void *dev_data)
{
	return 0;
}

int cps8601_qi_get_fw_version(u8 *data, int len, void *dev_data)
{
	struct cps8601_chip_info chip_info = { 0 };
	struct cps8601_dev_info *di = dev_data;

	/* fw version length must be 4 */
	if (!data || (len != 4)) {
		hwlog_err("get_fw_version: para err");
		return -EINVAL;
	}

	if (cps8601_get_chip_info(di, &chip_info)) {
		hwlog_err("get_fw_version: get chip_info failed\n");
		return -EINVAL;
	}

	/* byte[0:1]=chip_id, byte[2:3]=mtp_ver */
	data[0] = (u8)((chip_info.chip_id >> 0) & POWER_MASK_BYTE);
	data[1] = (u8)((chip_info.chip_id >> POWER_BITS_PER_BYTE) & POWER_MASK_BYTE);
	data[2] = (u8)((chip_info.mtp_ver >> 0) & POWER_MASK_BYTE);
	data[3] = (u8)((chip_info.mtp_ver >> POWER_BITS_PER_BYTE) & POWER_MASK_BYTE);

	return 0;
}

static struct hwqi_ops cps8601_qi_ops = {
	.chip_name              = "cps8601",
	.send_msg               = cps8601_qi_send_ask_msg,
	.send_msg_with_ack      = cps8601_qi_send_ask_msg_ack,
	.receive_msg            = cps8601_qi_receive_fsk_msg,
	.send_fsk_msg           = cps8601_qi_send_fsk_msg,
	.get_ask_packet         = cps8601_qi_receive_ask_pkt,
	.get_chip_fw_version    = cps8601_qi_get_fw_version,
	.get_tx_id_pre          = NULL,
	.set_rpp_format_post    = cps8601_qi_set_rpp_format,
};

int cps8601_qi_ops_register(struct wltrx_ic_ops *ops, struct cps8601_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->qi_ops = kzalloc(sizeof(*(ops->qi_ops)), GFP_KERNEL);
	if (!ops->qi_ops)
		return -ENOMEM;

	memcpy(ops->qi_ops, &cps8601_qi_ops, sizeof(cps8601_qi_ops));
	ops->qi_ops->dev_data = (void *)di;

	return hwqi_ops_register(ops->qi_ops, di->ic_type);
}
