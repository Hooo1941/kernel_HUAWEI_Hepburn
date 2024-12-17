// SPDX-License-Identifier: GPL-2.0
/*
 * adapter_protocol_ufcs_base.c
 *
 * ufcs protocol base function
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
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG ufcs_protocol
HWLOG_REGIST();

static struct mutex g_msg_number_lock;
static u8 g_msg_number;
static struct hwufcs_power_change_data g_power_change_data[HWUFCS_POWER_CHANGE_MAX_OUTPUT_MODE];

static const char * const g_hwufcs_ctl_msg_table[HWUFCS_CTL_MSG_END] = {
	[HWUFCS_CTL_MSG_PING] = "ping",
	[HWUFCS_CTL_MSG_ACK] = "ack",
	[HWUFCS_CTL_MSG_NCK] = "nck",
	[HWUFCS_CTL_MSG_ACCEPT] = "accept",
	[HWUFCS_CTL_MSG_SOFT_RESET] = "soft_reset",
	[HWUFCS_CTL_MSG_POWER_READY] = "power_ready",
	[HWUFCS_CTL_MSG_GET_OUTPUT_CAPABILITIES] = "get_output_capabilities",
	[HWUFCS_CTL_MSG_GET_SOURCE_INFO] = "get_source_info",
	[HWUFCS_CTL_MSG_GET_SINK_INFO] = "get_sink_info",
	[HWUFCS_CTL_MSG_GET_CABLE_INFO] = "get_cable_info",
	[HWUFCS_CTL_MSG_GET_DEVICE_INFO] = "get_device_info",
	[HWUFCS_CTL_MSG_GET_ERROR_INFO] = "get_error_info",
	[HWUFCS_CTL_MSG_DETECT_CABLE_INFO] = "detect_cable_info",
	[HWUFCS_CTL_MSG_START_CABLE_DETECT] = "start_cable_detect",
	[HWUFCS_CTL_MSG_END_CABLE_DETECT] = "end_cable_detect",
	[HWUFCS_CTL_MSG_EXIT_HWUFCS_MODE] = "exit_ufcs_mode",
};

static const char * const g_hwufcs_data_msg_table[HWUFCS_DATA_MSG_END] = {
	[HWUFCS_DATA_MSG_OUTPUT_CAPABILITIES] = "output_capabilities",
	[HWUFCS_DATA_MSG_REQUEST] = "request",
	[HWUFCS_DATA_MSG_SOURCE_INFO] = "source_info",
	[HWUFCS_DATA_MSG_SINK_INFO] = "sink_info",
	[HWUFCS_DATA_MSG_CABLE_INFO] = "cable_info",
	[HWUFCS_DATA_MSG_DEVICE_INFO] = "device_info",
	[HWUFCS_DATA_MSG_ERROR_INFO] = "error_info",
	[HWUFCS_DATA_MSG_CONFIG_WATCHDOG] = "config_watchdog",
	[HWUFCS_DATA_MSG_REFUSE] = "refuse",
	[HWUFCS_DATA_MSG_VERIFY_REQUEST] = "verify_request",
	[HWUFCS_DATA_MSG_VERIFY_RESPONSE] = "verify_response",
	[HWUFCS_DATA_MSG_TEST_REQUEST] = "test_request",
};

static const char *hwufcs_get_ctl_msg_name(unsigned int msg)
{
	if ((msg >= HWUFCS_CTL_MSG_BEGIN) && (msg < HWUFCS_CTL_MSG_END))
		return g_hwufcs_ctl_msg_table[msg];

	return "illegal ctl_msg";
}

static const char *hwufcs_get_data_msg_name(unsigned int msg)
{
	if ((msg >= HWUFCS_DATA_MSG_BEGIN) && (msg < HWUFCS_DATA_MSG_END))
		return g_hwufcs_data_msg_table[msg];

	return "illegal data_msg";
}

void hwufcs_init_msg_number_lock(void)
{
	mutex_init(&g_msg_number_lock);
}

void hwufcs_destroy_msg_number_lock(void)
{
	mutex_destroy(&g_msg_number_lock);
}

void hwufcs_clear_power_change_data(void)
{
	int i;

	hwlog_info("clear_power_change_data\n");
	for (i = 0; i < HWUFCS_POWER_CHANGE_MAX_OUTPUT_MODE; i++) {
		g_power_change_data[i].max_curr = 0;
		g_power_change_data[i].output_mode = 0;
	}
}

int hwufcs_get_power_change_curr(unsigned int mode)
{
	int i;

	for (i = 0; i < HWUFCS_POWER_CHANGE_MAX_OUTPUT_MODE; i++) {
		if (g_power_change_data[i].output_mode == mode)
			return g_power_change_data[i].max_curr;
	}
	return 0;
}

int hwufcs_updata_power_change_data(struct hwufcs_package_data *pkt)
{
	int i, num;
	u8 data[HWUFCS_POWER_CHANGE_DATA_SIZE * HWUFCS_POWER_CHANGE_MAX_OUTPUT_MODE] = { 0 };
	u32 tmp_data;

	if (pkt->len % HWUFCS_POWER_CHANGE_DATA_SIZE != 0) {
		hwlog_err("power_change length=%uinvalid\n", pkt->len);
		return HWUFCS_ERR_ILLEGAL_DATA;
	}

	memcpy((u8 *)&data, pkt->data, pkt->len);
	num = pkt->len / HWUFCS_POWER_CHANGE_DATA_SIZE;
	for (i = 0; i < num; i++) {
		/*
		 During communication, the high byte is sent first, and then the low byte.
		 need convert data according to protocol specifications.
		 */
		tmp_data = (((data[3 * i + 0] << 8) | data[3 * i + 1]) << 8) | data[3 * i + 2];
		g_power_change_data[i].max_curr = (tmp_data >> HWUFCS_POWER_CHANGE_SHIFT_MAX_CURR) &
			HWUFCS_POWER_CHANGE_MASK_MAX_CURR;
		g_power_change_data[i].max_curr *= HWUFCS_POWER_CHANGE_UNIT_CURR;
		g_power_change_data[i].output_mode = (tmp_data >> HWUFCS_POWER_CHANGE_SHIFT_OUTPUT_MODE) &
			HWUFCS_POWER_CHANGE_MASK_OUTPUT_MODE;
		hwlog_info("power_change[%u]=%umA %u\n", i, g_power_change_data[i].max_curr,
			g_power_change_data[i].output_mode);
	}

	return 0;
}

static u8 hwufcs_get_msg_number(void)
{
	u8 msg_number;

	mutex_lock(&g_msg_number_lock);
	msg_number = g_msg_number;
	mutex_unlock(&g_msg_number_lock);
	return msg_number;
}

static void hwufcs_set_msg_number(u8 msg_number)
{
	hwlog_info("old_msg_number=%u new_msg_number=%u\n",
		g_msg_number, msg_number);
	mutex_lock(&g_msg_number_lock);
	g_msg_number = (msg_number % HWUFCS_MSG_MAX_COUNTER);
	mutex_unlock(&g_msg_number_lock);
}

int hwufcs_request_communication(bool flag)
{
	int ret = 0;
	int i;

	for (i = 0; i < HWUFCS_REQUEST_COMMUNICATION_RETRY; i++) {
		ret = hwufcs_set_communicating_flag(flag);
		if (!ret)
			break;

		power_usleep(DT_USLEEP_10MS);
	}

	hwlog_info("request communication finish, ret=%d\n", ret);
	return ret;
}

static void hwufcs_packet_head(struct hwufcs_package_data *pkt, u8 *buf)
{
	u16 data = 0;

	/* packet tx header data */
	data |= ((pkt->msg_type & HWUFCS_HDR_MASK_MSG_TYPE) <<
		HWUFCS_HDR_SHIFT_MSG_TYPE);
	data |= ((pkt->prot_version & HWUFCS_HDR_MASK_PROT_VERSION) <<
		HWUFCS_HDR_SHIFT_PROT_VERSION);
	data |= ((pkt->msg_number & HWUFCS_HDR_MASK_MSG_NUMBER) <<
		HWUFCS_HDR_SHIFT_MSG_NUMBER);
	data |= ((pkt->dev_address & HWUFCS_HDR_MASK_DEV_ADDRESS) <<
		HWUFCS_HDR_SHIFT_DEV_ADDRESS);

	/* fill tx header data */
	buf[HWUFCS_HDR_HEADER_H_OFFSET] = (data >> POWER_BITS_PER_BYTE) & POWER_MASK_BYTE;
	buf[HWUFCS_HDR_HEADER_L_OFFSET] = (data >> 0) & POWER_MASK_BYTE;
}

static void hwufcs_packet_ctrl_and_data_msg(struct hwufcs_package_data *pkt, u8 *buf)
{
	hwlog_info("type=%u ver=%u num=%u addr=%u cmd=0x%x len=%u\n",
		pkt->msg_type, pkt->prot_version, pkt->msg_number,
		pkt->dev_address, pkt->cmd, pkt->len);

	/* fill tx body data */
	buf[HWUFCS_CTRL_DATA_MSG_CMD_OFFSET] = pkt->cmd;
	buf[HWUFCS_CTRL_DATA_MSG_LEN_OFFSET] = pkt->len;
	if (pkt->len)
		memcpy(&buf[HWUFCS_CTRL_DATA_MSG_DATA_OFFSET], pkt->data, pkt->len);
}

static void hwufcs_packet_vendor_define_msg(struct hwufcs_package_data *pkt, u8 *buf)
{
	hwlog_info("type=%u ver=%u num=%u addr=%u vendor_id=0x%x len=%u\n",
		pkt->msg_type, pkt->prot_version, pkt->msg_number,
		pkt->dev_address, pkt->vender_id, pkt->len);

	/* fill tx body data */
	buf[HWUFCS_VD_MSG_VER_ID_H_OFFSET] = pkt->vender_id >> POWER_BITS_PER_BYTE;
	buf[HWUFCS_VD_MSG_VER_ID_L_OFFSET] = pkt->vender_id & POWER_MASK_BYTE;
	buf[HWUFCS_VD_MSG_MSG_LEN_OFFSET] = pkt->len;
	if (pkt->len)
		memcpy(&buf[HWUFCS_VD_MSG_DATA_OFFSET], pkt->data, pkt->len);
}

static int hwufcs_packet_tx_data(struct hwufcs_package_data *pkt, int pkt_len,
	u8 *buf, int buf_len)
{
	if (pkt_len > buf_len) {
		hwlog_err("length err, pkt_len=%d, buf_len=%d\n", pkt_len, buf_len);
		return -EPERM;
	}

	hwufcs_packet_head(pkt, buf);
	switch (pkt->msg_type) {
	case HWUFCS_MSG_TYPE_CONTROL:
	case HWUFCS_MSG_TYPE_DATA:
		hwufcs_packet_ctrl_and_data_msg(pkt, buf);
		break;
	case HWUFCS_MSG_TYPE_VENDOR_DEFINED:
		hwufcs_packet_vendor_define_msg(pkt, buf);
		break;
	default:
		hwlog_info("invalid msg_type=%d\n", pkt->msg_type);
		break;
	}

	return 0;
}

static void hwufcs_unpacket_rx_header(struct hwufcs_package_data *pkt, u8 *buf)
{
	u16 data;

	/* unpacket rx header data */
	data = (buf[HWUFCS_HDR_HEADER_H_OFFSET] << POWER_BITS_PER_BYTE) |
		buf[HWUFCS_HDR_HEADER_L_OFFSET];
	pkt->msg_type = (data >> HWUFCS_HDR_SHIFT_MSG_TYPE) &
		HWUFCS_HDR_MASK_MSG_TYPE;
	pkt->prot_version = (data >> HWUFCS_HDR_SHIFT_PROT_VERSION) &
		HWUFCS_HDR_MASK_PROT_VERSION;
	pkt->msg_number = (data >> HWUFCS_HDR_SHIFT_MSG_NUMBER) &
		HWUFCS_HDR_MASK_MSG_NUMBER;
	pkt->dev_address = (data >> HWUFCS_HDR_SHIFT_DEV_ADDRESS) &
		HWUFCS_HDR_MASK_DEV_ADDRESS;
}

static void hwufcs_unpacket_ctrl_and_data_msg(struct hwufcs_package_data *pkt, u8 *buf)
{
	/* get rx body data */
	pkt->cmd = buf[HWUFCS_CTRL_DATA_MSG_CMD_OFFSET];
	pkt->len = buf[HWUFCS_CTRL_DATA_MSG_LEN_OFFSET];
	if (pkt->len && (pkt->len <= HWUFCS_CTRL_DATA_MSG_MAX_BUF_SIZE))
		memcpy(pkt->data, &buf[HWUFCS_CTRL_DATA_MSG_DATA_OFFSET], pkt->len);

	hwlog_info("type=%u ver=%u num=%u addr=%u cmd=0x%x len=%u\n",
		pkt->msg_type, pkt->prot_version, pkt->msg_number,
		pkt->dev_address, pkt->cmd, pkt->len);
}

static void hwufcs_unpacket_vendor_define_msg(struct hwufcs_package_data *pkt, u8 *buf)
{
	u16 data;

	/* get rx body data */
	pkt->vender_id = (buf[HWUFCS_VD_MSG_VER_ID_H_OFFSET] << POWER_BITS_PER_BYTE) |
		buf[HWUFCS_VD_MSG_VER_ID_L_OFFSET];
	pkt->len = buf[HWUFCS_VD_MSG_MSG_LEN_OFFSET] - HWUFCS_VDR_HEADER_LEN;
	data = (buf[HWUFCS_VD_MSG_CMD_H_OFFSET] << POWER_BITS_PER_BYTE) |
		buf[HWUFCS_VD_MSG_CMD_L_OFFSET];
	pkt->cmd = (data >> HWUFCS_VDR_SHIFT_MSG_CMD) &
		HWUFCS_VDR_MASK_MSG_CMD;
	pkt->vdm_type = (data >> HWUFCS_VDR_SHIFT_MSG_CMD_TYPE) &
		HWUFCS_VDR_MASK_MSG_CMD_TYPE;
	if (pkt->len && (pkt->len <= HWUFCS_VD_MSG_MAX_BUF_SIZE))
		memcpy(pkt->data, &buf[HWUFCS_VD_MSG_DATA_OFFSET + HWUFCS_VDR_HEADER_LEN], pkt->len);

	hwlog_info("type=%u ver=%u num=%u addr=%u vendor_id=0x%x vdm_type=0x%x cmd=0x%x len=%u\n",
		pkt->msg_type, pkt->prot_version, pkt->msg_number,
		pkt->dev_address, pkt->vender_id, pkt->vdm_type, pkt->cmd, pkt->len);
}

static int hwufcs_unpacket_rx_data(struct hwufcs_package_data *pkt, u8 *buf, int buf_len)
{
	if (buf_len > sizeof(struct hwufcs_package_data)) {
		hwlog_err("length err, buf_len=%d\n", buf_len);
		return -EPERM;
	}

	hwufcs_unpacket_rx_header(pkt, buf);
	switch (pkt->msg_type) {
	case HWUFCS_MSG_TYPE_CONTROL:
	case HWUFCS_MSG_TYPE_DATA:
		hwufcs_unpacket_ctrl_and_data_msg(pkt, buf);
		break;
	case HWUFCS_MSG_TYPE_VENDOR_DEFINED:
		hwufcs_unpacket_vendor_define_msg(pkt, buf);
		break;
	default:
		hwlog_info("invalid msg_type=%d\n", pkt->msg_type);
		break;
	}

	return 0;
}

static int hwufcs_check_refuse_reason(u8 reason)
{
	int ret = HWUFCS_ERR_REFUSED_DATA;

	switch (reason) {
	case HWUFCS_REFUSE_REASON_NOT_IDENTIFY:
	case HWUFCS_REFUSE_REASON_NOT_SUPPORT:
		hwlog_info("refuse reason is not identify or unsupport\n");
		ret = HWUFCS_ERR_REFUSED_SUPPORT_DATA;
		break;
	default:
		break;
	}
	return ret;
}

static int hwufcs_receive_refuse_data_msg(struct hwufcs_package_data *pkt)
{
	u32 data = 0;
	u32 tmp_data;
	u8 len = sizeof(u32);
	struct hwufcs_refuse_data p = {0};

	if (len != pkt->len) {
		hwlog_err("refuse length=%u,%u invalid\n", len, pkt->len);
		return HWUFCS_ERR_ILLEGAL_DATA;
	}

	memcpy((u8 *)&data, pkt->data, pkt->len);
	tmp_data = cpu_to_be32(data);
	/* reason */
	p.reason = (tmp_data >> HWUFCS_REFUSE_SHIFT_REASON) &
		HWUFCS_REFUSE_MASK_REASON;
	/* command number */
	p.cmd_number = (tmp_data >> HWUFCS_REFUSE_SHIFT_CMD_NUMBER) &
		HWUFCS_REFUSE_MASK_CMD_NUMBER;
	/* message type */
	p.msg_type = (tmp_data >> HWUFCS_REFUSE_SHIFT_MSG_TYPE) &
		HWUFCS_REFUSE_MASK_MSG_TYPE;
	/* message number */
	p.msg_number = (tmp_data >> HWUFCS_REFUSE_SHIFT_MSG_NUMBER) &
		HWUFCS_REFUSE_MASK_MSG_NUMBER;

	hwlog_info("refuse=%lx %x %x %x %x\n", tmp_data,
		p.reason, p.cmd_number, p.msg_type, p.msg_number);
	return hwufcs_check_refuse_reason(p.reason);
}

static int hwufcs_handle_recevice_exception_msg(struct hwufcs_package_data *pkt)
{
	int ret = 0;

	hwlog_info("type=%d,cmd=0x%x\n", pkt->msg_type, pkt->cmd);
	if (pkt->msg_type != HWUFCS_MSG_TYPE_DATA)
		return HWUFCS_ERR_UNEXPECT_DATA;

	/* check exception msg:power_change msg */
	if (pkt->cmd == HWUFCS_DATA_MSG_POWER_CHANGE)
		ret = hwufcs_updata_power_change_data(pkt);

	if (pkt->cmd == HWUFCS_DATA_MSG_REFUSE)
		ret = hwufcs_receive_refuse_data_msg(pkt);

	return ret ? ret : HWUFCS_ERR_UNEXPECT_DATA;
}

static int hwufcs_send_control_cmd(u8 prot_version, u8 msg_number, u8 cmd, u8 addr)
{
	struct hwufcs_package_data pkt = { 0 };
	u8 buf[HWUFCS_MSG_MAX_BUFFER_SIZE] = { 0 };
	u8 flag = HWUFCS_WAIT_SEND_PACKET_COMPLETE | HWUFCS_ACK_RECEIVE_TIMEOUT;
	int len = HWUFCS_HDR_HEADER_LEN + HWUFCS_MSG_CMD_LEN;
	int ret;

	pkt.msg_type = HWUFCS_MSG_TYPE_CONTROL;
	pkt.prot_version = prot_version;
	pkt.msg_number = msg_number;
	pkt.dev_address = addr;
	pkt.cmd = cmd;
	pkt.len = 0;

	hwlog_info("send ctrl msg=%s\n", hwufcs_get_ctl_msg_name(pkt.cmd));
	ret = hwufcs_packet_tx_data(&pkt, len, buf, HWUFCS_MSG_MAX_BUFFER_SIZE);
	if (ret) {
		hwlog_err("packet control msg msg_number=%u cmd=%u fail\n",
			msg_number, cmd);
		return -EPERM;
	}

	ret = hwufcs_write_msg(buf, len, flag);
	if (ret) {
		hwlog_err("send control msg msg_number=%u cmd=%u fail\n",
			msg_number, cmd);
		return -EPERM;
	}

	return 0;
}

static int hwufcs_send_data_cmd(u8 msg_number, u8 cmd, u8 *data, u8 len)
{
	struct hwufcs_package_data pkt = { 0 };
	u8 buf[HWUFCS_MSG_MAX_BUFFER_SIZE] = { 0 };
	u8 flag = HWUFCS_WAIT_SEND_PACKET_COMPLETE | HWUFCS_ACK_RECEIVE_TIMEOUT;
	int ret;
	int len_all = HWUFCS_HDR_HEADER_LEN + HWUFCS_MSG_CMD_LEN +
		HWUFCS_MSG_LENGTH_LEN + len;

	if (len > HWUFCS_CTRL_DATA_MSG_MAX_BUF_SIZE) {
		hwlog_err("invalid length=%u\n", len);
		return -EINVAL;
	}

	pkt.msg_type = HWUFCS_MSG_TYPE_DATA;
	pkt.prot_version = HWUFCS_MSG_PROT_VERSION;
	pkt.msg_number = msg_number;
	pkt.dev_address = HWUFCS_DEV_ADDRESS_SOURCE;
	pkt.cmd = cmd;
	pkt.len = len;
	memcpy(pkt.data, data, len);

	hwlog_info("send data msg=%s\n", hwufcs_get_data_msg_name(pkt.cmd));
	ret = hwufcs_packet_tx_data(&pkt, len_all, buf, HWUFCS_MSG_MAX_BUFFER_SIZE);
	if (ret) {
		hwlog_err("packet data msg msg_number=%u cmd=%u fail\n",
			msg_number, cmd);
		return -EPERM;
	}

	ret = hwufcs_write_msg(buf, len_all, flag);
	if (ret) {
		hwlog_err("send data msg msg_number=%u cmd=%u fail\n",
			msg_number, cmd);
		return -EPERM;
	}

	return 0;
}

static int hwufcs_send_vdm_control_cmd(u8 prot_version, u8 msg_number, u8 *data, u8 len, u8 addr)
{
	struct hwufcs_package_data pkt = { 0 };
	u8 buf[HWUFCS_MSG_MAX_BUFFER_SIZE] = { 0 };
	u8 flag = HWUFCS_WAIT_SEND_PACKET_COMPLETE | HWUFCS_ACK_RECEIVE_TIMEOUT;
	int ret;
	int len_all = HWUFCS_HDR_HEADER_LEN + HWUFCS_MSG_VER_ID_LEN +
		HWUFCS_MSG_LENGTH_LEN + len;

	if (len > HWUFCS_VD_MSG_MAX_BUF_SIZE) {
		hwlog_err("invalid length=%u\n", len);
		return -EINVAL;
	}

	pkt.msg_type = HWUFCS_MSG_TYPE_VENDOR_DEFINED;
	pkt.prot_version = prot_version;
	pkt.msg_number = msg_number;
	pkt.dev_address = HWUFCS_DEV_ADDRESS_SOURCE;
	pkt.vender_id = HWUFCS_MSG_VERSION_ID;
	pkt.len = len;
	memcpy(pkt.data, data, len);

	ret = hwufcs_packet_tx_data(&pkt, len_all, buf, HWUFCS_MSG_MAX_BUFFER_SIZE);
	if (ret) {
		hwlog_err("packet vendor defined msg msg_number=%u fail\n",
			msg_number);
		return -EPERM;
	}

	ret = hwufcs_write_msg(buf, len_all, flag);
	if (ret) {
		hwlog_err("send vendor defined msg msg_number=%u fail\n",
			msg_number);
		return -EPERM;
	}

	return 0;
}

static int hwufcs_check_vdm_msg_legality(u16 vendor_id, u8 vdm_type, u8 cmd)
{
	if (vendor_id != HWUFCS_MSG_VERSION_ID)
		return HWUFCS_ERR_UNSUPPORT_DATA;

	switch (vdm_type) {
	case HWUFCS_VDM_TYPE_CONTROL:
		if ((cmd >= HWUFCS_VDM_CTL_MSG_END) || (cmd < HWUFCS_VDM_CTL_MSG_BEGIN))
			return HWUFCS_ERR_UNSUPPORT_DATA;
		break;
	case HWUFCS_VDM_TYPE_DATA:
		if ((cmd >= HWUFCS_VDM_DATA_MSG_END) || (cmd < HWUFCS_VDM_DATA_MSG_BEGIN))
			return HWUFCS_ERR_UNSUPPORT_DATA;
		break;
	default:
		return HWUFCS_ERR_UNSUPPORT_DATA;
	}
	return 0;
}

static int hwufcs_check_msg_legality(u8 type, u16 vendor_id, u8 vdm_type, u8 cmd)
{
	if ((type >= HWUFCS_MSG_TYPE_END) || (type < HWUFCS_MSG_TYPE_BEGIN))
		return HWUFCS_ERR_UNSUPPORT_DATA;

	switch (type) {
	case HWUFCS_MSG_TYPE_CONTROL:
		if ((cmd >= HWUFCS_CTL_MSG_END) || (cmd < HWUFCS_CTL_MSG_BEGIN))
			return HWUFCS_ERR_UNSUPPORT_DATA;
		break;
	case HWUFCS_MSG_TYPE_DATA:
		if ((cmd >= HWUFCS_DATA_MSG_END) || (cmd < HWUFCS_DATA_MSG_BEGIN))
			return HWUFCS_ERR_UNSUPPORT_DATA;
		break;
	case HWUFCS_MSG_TYPE_VENDOR_DEFINED:
		return hwufcs_check_vdm_msg_legality(vendor_id, vdm_type, cmd);
	default:
		return HWUFCS_ERR_UNSUPPORT_DATA;
	}

	return 0;
}

int hwufcs_receive_msg(struct hwufcs_package_data *pkt, bool need_wait_msg)
{
	u8 buf[HWUFCS_MSG_MAX_BUFFER_SIZE] = { 0 };
	u8 flag = HWUFCS_WAIT_CRC_ERROR | HWUFCS_WAIT_DATA_READY;
	u8 buf_len = 0;
	int ret;

	/* wait msg from rx */
	if (need_wait_msg && hwufcs_wait_msg_ready(flag)) {
		hwlog_err("wait for msg time out\n");
		return HWUFCS_ERR_TIMEOUT;
	}

	ret = hwufcs_get_rx_length(&buf_len);
	if (ret || !buf_len || (buf_len > HWUFCS_MSG_MAX_BUFFER_SIZE)) {
		hwlog_err("get data length err, len=%d,ret=%d\n", buf_len, ret);
		return HWUFCS_ERR_ILLEGAL_DATA;
	}

	hwlog_info("rx msg len=%d\n", buf_len);
	/* receive msg header from rx */
	ret = hwufcs_read_msg(buf, buf_len);
	if (ret)
		return HWUFCS_ERR_ILLEGAL_DATA;

	ret = hwufcs_end_read_msg();
	if (ret)
		return HWUFCS_ERR_UNKNOWN;

	ret = hwufcs_unpacket_rx_data(pkt, buf, buf_len);
	if (ret)
		return HWUFCS_ERR_UNKNOWN;

	ret = hwufcs_check_msg_legality(pkt->msg_type, pkt->vender_id, pkt->vdm_type, pkt->cmd);
	if (ret) {
		hwlog_err("receive_msg msg_type=%u vender_id=%u vdm_type=%u msg_cmd=%u illegal\n",
			pkt->msg_type, pkt->vender_id, pkt->vdm_type, pkt->cmd);
		return ret;
	}

	return 0;
}

int hwufcs_receive_control_msg(u8 cmd, bool check_msg_num)
{
	int ret;
	struct hwufcs_package_data pkt = { 0 };
	u8 msg_number = hwufcs_get_msg_number();

	if ((cmd == HWUFCS_CTL_MSG_ACK) && !hwufcs_need_check_ack())
		return 0;

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		return ret;
	}

	/* check control msg header */
	if (check_msg_num && (pkt.msg_number != msg_number)) {
		hwlog_err("msg_number=%u,%u invalid\n",
			pkt.msg_number, msg_number);
		return HWUFCS_ERR_UNKNOWN;
	}

	if ((pkt.msg_type != HWUFCS_MSG_TYPE_CONTROL) || (pkt.cmd != cmd)) {
		hwlog_err("receive_control_msg msg_type=%u cmd=%u unexpect\n", pkt.msg_type, pkt.cmd);
		return hwufcs_handle_recevice_exception_msg(&pkt);
	}

	return 0;
}

static int hwufcs_check_receive_data_msg(struct hwufcs_package_data *pkt, u8 expect_cmd)
{
	if ((pkt->msg_type != HWUFCS_MSG_TYPE_DATA) || (pkt->cmd != expect_cmd)) {
		hwlog_err("receive_data_msg msg_type=%u cmd=%u unexpect\n", pkt->msg_type, pkt->cmd);
		return hwufcs_handle_recevice_exception_msg(pkt);
	}
	return 0;
}

static int hwufcs_check_receive_vdm_data_msg(struct hwufcs_package_data *pkt, u8 expect_cmd)
{
	if ((pkt->vdm_type != HWUFCS_VDM_TYPE_DATA) || (pkt->cmd != expect_cmd)) {
		hwlog_err("receive_data_msg msg_type=%u cmd=%u unexpect\n", pkt->vdm_type, pkt->cmd);
		return hwufcs_handle_recevice_exception_msg(pkt);
	}
	return 0;
}

int hwufcs_send_control_msg(u8 cmd, bool ack)
{
	u8 msg_number = hwufcs_get_msg_number();
	int ret;

	/* send control cmd to tx */
	ret = hwufcs_send_control_cmd(HWUFCS_MSG_PROT_VERSION, msg_number, cmd, HWUFCS_DEV_ADDRESS_SOURCE);
	if (ret)
		return HWUFCS_ERR_UNKNOWN;

	if (!ack)
		goto end;

	/* wait ack from rx */
	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACK, true);
	if (ret) {
		hwlog_err("wait for ack err,ret=%d\n", ret);
		return ret;
	}

end:
	hwufcs_set_msg_number(++msg_number);
	return 0;
}

int hwufcs_send_specific_msg(void)
{
	int ret;

	ret = hwufcs_send_control_cmd(0, 0, HWUFCS_CTL_MSG_PING, HWUFCS_DEV_ADDRESS_SOURCE);
	if (ret)
		return HWUFCS_ERR_UNKNOWN;

	/* wait ack from rx */
	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACK, true);
	if (ret) {
		hwlog_err("wait for ack err,ret=%d\n", ret);
		return ret;
	}

	return 0;
}

int hwufcs_send_control_msg_to_cable(u8 cmd, bool ack)
{
	u8 msg_number = hwufcs_get_msg_number();
	int ret;

	/* send control cmd to tx */
	ret = hwufcs_send_control_cmd(HWUFCS_MSG_PROT_VERSION, msg_number, cmd, HWUFCS_DEV_ADDRESS_CABLE_ELECTRONIC_LABEL);
	if (ret)
		return HWUFCS_ERR_UNKNOWN;

	if (!ack)
		goto end;

	/* wait ack from rx */
	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACK, true);
	if (ret) {
		hwlog_err("wait for ack err,ret=%d\n", ret);
		return ret;
	}

end:
	hwufcs_set_msg_number(++msg_number);
	return 0;
}

static int hwufcs_send_data_msg(u8 cmd, u8 *data, u8 len, bool ack)
{
	u8 msg_number = hwufcs_get_msg_number();
	int ret;

	/* send data cmd to tx */
	ret = hwufcs_send_data_cmd(msg_number, cmd, data, len);
	if (ret)
		return HWUFCS_ERR_UNKNOWN;

	if (!ack)
		goto end;

	/* wait ack from rx */
	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACK, true);
	if (ret) {
		hwlog_err("wait for ack err,ret=%d\n", ret);
		return ret;
	}

end:
	hwufcs_set_msg_number(++msg_number);
	return 0;
}

int hwufcs_send_vdm_control_msg(u8 cmd, bool ack)
{
	u8 msg_number = hwufcs_get_msg_number();
	u16 data = 0;
	u16 tmp_data;
	int ret;

	data |= (u16)(cmd & HWUFCS_VDR_MASK_MSG_CMD) << HWUFCS_VDR_SHIFT_MSG_CMD;
	data |= (u16)(HWUFCS_VDM_TYPE_CONTROL & HWUFCS_VDR_MASK_MSG_CMD_TYPE) << HWUFCS_VDR_SHIFT_MSG_CMD_TYPE;
	tmp_data = be16_to_cpu(data);

	/* send control cmd to tx */
	ret = hwufcs_send_vdm_control_cmd(HWUFCS_MSG_PROT_VERSION, msg_number, (u8 *)&tmp_data,
		HWUFCS_VDR_HEADER_LEN, HWUFCS_DEV_ADDRESS_SOURCE);
	if (ret)
		return HWUFCS_ERR_UNKNOWN;

	if (!ack)
		goto end;

	/* wait ack from rx */
	ret = hwufcs_receive_control_msg(HWUFCS_CTL_MSG_ACK, true);
	if (ret) {
		hwlog_err("wait for ack err,ret=%d\n", ret);
		return ret;
	}

end:
	hwufcs_set_msg_number(++msg_number);
	return 0;
}

/* data message from sink to source: 0x02 request */
int hwufcs_send_request_data_msg(struct hwufcs_request_data *p)
{
	u64 data = 0;
	u64 tmp_data;
	u8 len = sizeof(u64);

	data |= ((u64)((p->output_curr / HWUFCS_REQ_UNIT_OUTPUT_CURR) &
		HWUFCS_REQ_MASK_OUTPUT_CURR) << HWUFCS_REQ_SHIFT_OUTPUT_CURR);
	data |= ((u64)((p->output_volt / HWUFCS_REQ_UNIT_OUTPUT_VOLT) &
		HWUFCS_REQ_MASK_OUTPUT_VOLT) << HWUFCS_REQ_SHIFT_OUTPUT_VOLT);
	data |= ((u64)(p->output_mode & HWUFCS_REQ_MASK_OUTPUT_MODE) <<
		HWUFCS_REQ_SHIFT_OUTPUT_MODE);
	tmp_data = be64_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_REQUEST,
		(u8 *)&tmp_data, len, true);
}

/* data message from sink to source: 0x06 device_information */
int hwufcs_send_device_information_data_msg(struct hwufcs_dev_info_data *p)
{
	u64 data = 0;
	u64 tmp_data;
	u8 len = sizeof(u64);

	data |= ((u64)(p->sw_ver & HWUFCS_DEV_INFO_MASK_SW_VER) <<
		HWUFCS_DEV_INFO_SHIFT_SW_VER);
	data |= ((u64)(p->hw_ver & HWUFCS_DEV_INFO_MASK_HW_VER) <<
		HWUFCS_DEV_INFO_SHIFT_HW_VER);
	data |= ((u64)(p->reserved_1 & HWUFCS_DEV_INFO_MASK_RESERVED_1) <<
		HWUFCS_DEV_INFO_SHIFT_RESERVED_1);
	data |= ((u64)(p->reserved_2 & HWUFCS_DEV_INFO_MASK_RESERVED_2) <<
		HWUFCS_DEV_INFO_SHIFT_RESERVED_2);
	tmp_data = be64_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_DEVICE_INFO,
		(u8 *)&tmp_data, len, true);
}

/* data message from sink to source: 0x04 sink_information */
int hwufcs_send_sink_information_data_msg(struct hwufcs_sink_info_data *p)
{
	u64 data = 0;
	u64 tmp_data;
	u8 len = sizeof(u64);

	data |= ((u64)((p->bat_curr / HWUFCS_SINK_INFO_UNIT_CURR) &
		HWUFCS_SINK_INFO_MASK_BAT_CURR) << HWUFCS_SINK_INFO_SHIFT_BAT_CURR);
	data |= ((u64)((p->bat_volt / HWUFCS_SINK_INFO_UNIT_VOLT) &
		HWUFCS_SINK_INFO_MASK_BAT_VOLT) << HWUFCS_SINK_INFO_SHIFT_BAT_VOLT);
	data |= ((u64)((p->usb_temp + HWUFCS_SINK_INFO_TEMP_BASE) &
		HWUFCS_SINK_INFO_MASK_USB_TEMP) << HWUFCS_SINK_INFO_SHIFT_USB_TEMP);
	data |= ((u64)((p->bat_temp + HWUFCS_SINK_INFO_TEMP_BASE) &
		HWUFCS_SINK_INFO_MASK_BAT_TEMP) << HWUFCS_SINK_INFO_SHIFT_BAT_TEMP);
	tmp_data = be64_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_SINK_INFO,
		(u8 *)&tmp_data, len, true);
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
/* data message from sink to source: 0x05 cable_information */
static int hwufcs_send_cable_information_data_msg(struct hwufcs_cable_info_data *p)
{
	u64 data = 0;
	u64 tmp_data;
	u8 len = sizeof(u64);

	data |= ((u64)((p->max_curr / HWUFCS_CABLE_INFO_UNIT_CURR) &
		HWUFCS_CABLE_INFO_MASK_MAX_CURR) << HWUFCS_CABLE_INFO_SHIFT_MAX_CURR);
	data |= ((u64)((p->max_volt / HWUFCS_CABLE_INFO_UNIT_VOLT) &
		HWUFCS_CABLE_INFO_MASK_MAX_VOLT) << HWUFCS_CABLE_INFO_SHIFT_MAX_VOLT);
	data |= ((p->resistance & HWUFCS_CABLE_INFO_MASK_RESISTANCE) <<
		HWUFCS_CABLE_INFO_SHIFT_RESISTANCE);
	data |= ((u64)(p->reserved_1 & HWUFCS_CABLE_INFO_MASK_RESERVED_1) <<
		HWUFCS_CABLE_INFO_SHIFT_RESERVED_1);
	data |= ((u64)(p->reserved_2 & HWUFCS_CABLE_INFO_MASK_RESERVED_2) <<
		HWUFCS_CABLE_INFO_SHIFT_RESERVED_2);
	tmp_data = be64_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_CABLE_INFO,
		(u8 *)&tmp_data, len, true);
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

/* data message from sink to source: 0x07 error_information */
int hwufcs_send_error_information_data_msg(struct hwufcs_error_info_data *p)
{
	u32 data = 0;
	u32 tmp_data;
	u8 len = sizeof(u32);

	data |= ((u32)(p->output_ovp & HWUFCS_ERROR_INFO_MASK_OUTPUT_OVP) <<
		HWUFCS_ERROR_INFO_SHIFT_OUTPUT_OVP);
	data |= ((u32)(p->output_uvp & HWUFCS_ERROR_INFO_MASK_OUTPUT_UVP) <<
		HWUFCS_ERROR_INFO_SHIFT_OUTPUT_UVP);
	data |= ((u32)(p->output_ocp & HWUFCS_ERROR_INFO_MASK_OUTPUT_OCP) <<
		HWUFCS_ERROR_INFO_SHIFT_OUTPUT_OCP);
	data |= ((u32)(p->output_scp & HWUFCS_ERROR_INFO_MASK_OUTPUT_SCP) <<
		HWUFCS_ERROR_INFO_SHIFT_OUTPUT_SCP);
	data |= ((u32)(p->usb_otp & HWUFCS_ERROR_INFO_MASK_USB_OTP) <<
		HWUFCS_ERROR_INFO_SHIFT_USB_OTP);
	data |= ((u32)(p->device_otp & HWUFCS_ERROR_INFO_MASK_DEVICE_OTP) <<
		HWUFCS_ERROR_INFO_SHIFT_DEVICE_OTP);
	data |= ((u32)(p->cc_ovp & HWUFCS_ERROR_INFO_MASK_CC_OVP) <<
		HWUFCS_ERROR_INFO_SHIFT_CC_OVP);
	data |= ((u32)(p->dminus_ovp & HWUFCS_ERROR_INFO_MASK_DMINUS_OVP) <<
		HWUFCS_ERROR_INFO_SHIFT_DMINUS_OVP);
	data |= ((u32)(p->dplus_ovp & HWUFCS_ERROR_INFO_MASK_DPLUS_OVP) <<
		HWUFCS_ERROR_INFO_SHIFT_DPLUS_OVP);
	data |= ((u32)(p->input_ovp & HWUFCS_ERROR_INFO_MASK_INPUT_OVP) <<
		HWUFCS_ERROR_INFO_SHIFT_INPUT_OVP);
	data |= ((u32)(p->input_uvp & HWUFCS_ERROR_INFO_MASK_INPUT_UVP) <<
		HWUFCS_ERROR_INFO_SHIFT_INPUT_UVP);
	data |= ((u32)(p->over_leakage & HWUFCS_ERROR_INFO_MASK_OVER_LEAKAGE) <<
		HWUFCS_ERROR_INFO_SHIFT_OVER_LEAKAGE);
	data |= ((u32)(p->input_drop & HWUFCS_ERROR_INFO_MASK_INPUT_DROP) <<
		HWUFCS_ERROR_INFO_SHIFT_INPUT_DROP);
	data |= ((u32)(p->crc_error & HWUFCS_ERROR_INFO_MASK_CRC_ERROR) <<
		HWUFCS_ERROR_INFO_SHIFT_CRC_ERROR);
	data |= ((u32)(p->wtg_overflow & HWUFCS_ERROR_INFO_MASK_WTG_OVERFLOW) <<
		HWUFCS_ERROR_INFO_SHIFT_WTG_OVERFLOW);
	tmp_data = be32_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_ERROR_INFO,
		(u8 *)&tmp_data, len, true);
}

/* data message from sink to source: 0x08 config_watchdog */
int hwufcs_send_config_watchdog_data_msg(struct hwufcs_wtg_data *p)
{
	u16 data = 0;
	u16 tmp_data;
	u8 len = sizeof(u16);

	data |= ((u16)(p->time & HWUFCS_WTG_MASK_TIME) <<
		HWUFCS_WTG_SHIFT_TIME);
	tmp_data = be16_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_CONFIG_WATCHDOG,
		(u8 *)&tmp_data, len, true);
}

/* data message from sink to source: 0x0a verify_request */
int hwufcs_send_verify_request_data_msg(struct hwufcs_verify_request_data *p)
{
	u8 len = sizeof(struct hwufcs_verify_request_data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_VERIFY_REQUEST,
		(u8 *)p, len, true);
}

/* data message from sink to source: 0x09 refuse */
int hwufcs_send_refuse_data_msg(u8 reason, struct hwufcs_package_data *pkt)
{
	u32 data = 0;
	u32 tmp_data;
	u8 len = sizeof(u32);

	data |= ((u32)(reason & HWUFCS_REFUSE_MASK_REASON) <<
		HWUFCS_REFUSE_SHIFT_REASON);
	data |= ((u32)(pkt->cmd & HWUFCS_REFUSE_MASK_CMD_NUMBER) <<
		HWUFCS_REFUSE_SHIFT_CMD_NUMBER);
	data |= ((u32)(pkt->msg_type & HWUFCS_REFUSE_MASK_MSG_TYPE) <<
		HWUFCS_REFUSE_SHIFT_MSG_TYPE);
	data |= ((u32)(pkt->msg_number & HWUFCS_REFUSE_MASK_MSG_NUMBER) <<
		HWUFCS_REFUSE_SHIFT_MSG_NUMBER);
	tmp_data = be32_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_REFUSE,
		(u8 *)&tmp_data, len, true);
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
/* data message from sink to source: 0x0b verify_response */
static int hwufcs_send_verify_response_data_msg(struct hwufcs_verify_response_data *p)
{
	u8 len = sizeof(struct hwufcs_verify_response_data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_VERIFY_RESPONSE,
		(u8 *)p, len, true);
}

/* data message from sink to source: 0xff test request */
static int hwufcs_send_test_request_data_msg(struct hwufcs_test_request_data *p)
{
	u16 data = 0;
	u16 tmp_data;
	u8 len = sizeof(u16);

	data |= ((u16)(p->msg_number & HWUFCS_TEST_REQ_MASK_MSG_NUMBER) <<
		HWUFCS_TEST_REQ_SHIFT_MSG_NUMBER);
	data |= ((u16)(p->msg_type & HWUFCS_TEST_REQ_MASK_MSG_TYPE) <<
		HWUFCS_TEST_REQ_SHIFT_MSG_TYPE);
	data |= ((u16)(p->dev_address & HWUFCS_TEST_REQ_MASK_DEV_ADDRESS) <<
		HWUFCS_TEST_REQ_SHIFT_DEV_ADDRESS);
	data |= ((u16)(p->volt_test_mode & HWUFCS_TEST_REQ_MASK_VOLT_TEST_MODE) <<
		HWUFCS_TEST_REQ_SHIFT_VOLT_TEST_MODE);
	tmp_data = be16_to_cpu(data);

	return hwufcs_send_data_msg(HWUFCS_DATA_MSG_TEST_REQUEST,
		(u8 *)&tmp_data, len, true);
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

void hwufcs_handle_communication_result(int ret, struct hwufcs_package_data *pkt)
{
	if (ret == HWUFCS_ERR_ILLEGAL_DATA)
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_IDENTIFY, pkt);
	if (ret == HWUFCS_ERR_UNSUPPORT_DATA)
		hwufcs_send_refuse_data_msg(HWUFCS_REFUSE_REASON_NOT_SUPPORT, pkt);
}

static void hwufcs_parse_output_capabilities_data(u64 data, struct hwufcs_capabilities_data *p)
{
	/* min output current */
	p->min_curr = (data >> HWUFCS_CAP_SHIFT_MIN_CURR) &
		HWUFCS_CAP_MASK_MIN_CURR;
	p->min_curr *= HWUFCS_CAP_UNIT_CURR;
	/* max output current */
	p->max_curr = (data >> HWUFCS_CAP_SHIFT_MAX_CURR) &
		HWUFCS_CAP_MASK_MAX_CURR;
	p->max_curr *= HWUFCS_CAP_UNIT_CURR;
	/* min output voltage */
	p->min_volt = (data >> HWUFCS_CAP_SHIFT_MIN_VOLT) &
		HWUFCS_CAP_MASK_MIN_VOLT;
	p->min_volt *= HWUFCS_CAP_UNIT_VOLT;
	/* max output voltage */
	p->max_volt = (data >> HWUFCS_CAP_SHIFT_MAX_VOLT) &
		HWUFCS_CAP_MASK_MAX_VOLT;
	p->max_volt *= HWUFCS_CAP_UNIT_VOLT;
	/* voltage adjust step */
	p->volt_step = (data >> HWUFCS_CAP_SHIFT_VOLT_STEP) &
		HWUFCS_CAP_MASK_VOLT_STEP;
	p->volt_step++;
	p->volt_step *= HWUFCS_CAP_UNIT_VOLT;
	/* current adjust step */
	p->curr_step = (data >> HWUFCS_CAP_SHIFT_CURR_STEP) &
		HWUFCS_CAP_MASK_CURR_STEP;
	p->curr_step++;
	p->curr_step *= HWUFCS_CAP_UNIT_CURR;
	/* output mode */
	p->output_mode = (data >> HWUFCS_CAP_SHIFT_OUTPUT_MODE) &
		HWUFCS_CAP_MASK_OUTPUT_MODE;
}

/* data message from source to sink: 0x01 output_capabilities */
int hwufcs_receive_output_capabilities_data_msg(struct hwufcs_capabilities_data *p,
	u8 *ret_len)
{
	u64 data[HWUFCS_CAP_MAX_OUTPUT_MODE] = { 0 };
	u64 tmp_data;
	u8 len = HWUFCS_CAP_MAX_OUTPUT_MODE * sizeof(u64);
	int i, ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_data_msg(&pkt, HWUFCS_DATA_MSG_OUTPUT_CAPABILITIES);
	if (ret)
		goto end;

	if ((pkt.len % sizeof(u64) != 0) || (pkt.len > len)) {
		hwlog_err("output_capabilities length=%u,%u invalid\n", pkt.len, len);
		return HWUFCS_ERR_ILLEGAL_DATA;
	}

	memcpy(&data, pkt.data, pkt.len);
	*ret_len = pkt.len / sizeof(u64);
	for (i = 0; i < *ret_len; i++) {
		tmp_data = cpu_to_be64(data[i]);
		hwufcs_parse_output_capabilities_data(tmp_data, &p[i]);
	}

	for (i = 0; i < *ret_len; i++)
		hwlog_info("cap[%u]=%umA %umA %umV %umV %umV %umA %u\n", i, p[i].min_curr,
		p[i].max_curr, p[i].min_volt, p[i].max_volt, p[i].volt_step, p[i].curr_step, p[i].output_mode);
end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

/* data message from source to sink: 0x03 source_information */
int hwufcs_receive_source_info_data_msg(struct hwufcs_source_info_data *p)
{
	u64 data = 0;
	u64 tmp_data;
	u8 len = sizeof(u64);
	int ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_data_msg(&pkt, HWUFCS_DATA_MSG_SOURCE_INFO);
	if (ret)
		goto end;

	if (pkt.len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n", pkt.len, len);
		ret =  HWUFCS_ERR_ILLEGAL_DATA;
		goto end;
	}

	memcpy(&data, pkt.data, pkt.len);
	tmp_data = cpu_to_be64(data);

	/* current output current */
	p->output_curr = (tmp_data >> HWUFCS_SOURCE_INFO_SHIFT_OUTPUT_CURR) &
		HWUFCS_SOURCE_INFO_MASK_OUTPUT_CURR;
	p->output_curr *= HWUFCS_SOURCE_INFO_UNIT_CURR;
	/* current output voltage */
	p->output_volt = (tmp_data >> HWUFCS_SOURCE_INFO_SHIFT_OUTPUT_VOLT) &
		HWUFCS_SOURCE_INFO_MASK_OUTPUT_VOLT;
	p->output_volt *= HWUFCS_SOURCE_INFO_UNIT_VOLT;
	/* current usb port temp */
	p->port_temp = (tmp_data >> HWUFCS_SOURCE_INFO_SHIFT_PORT_TEMP) &
		HWUFCS_SOURCE_INFO_MASK_PORT_TEMP;
	p->port_temp -= HWUFCS_SOURCE_INFO_TEMP_BASE;
	/* current device temp */
	p->dev_temp = (tmp_data >> HWUFCS_SOURCE_INFO_SHIFT_DEV_TEMP) &
		HWUFCS_SOURCE_INFO_MASK_DEV_TEMP;
	p->dev_temp -= HWUFCS_SOURCE_INFO_TEMP_BASE;

	hwlog_info("source_info=%lx %umA %umV %dC %dC\n", tmp_data,
		p->output_curr, p->output_volt, p->port_temp, p->dev_temp);
end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

/* data message from source to sink: 0x05 cable_information */
int hwufcs_receive_cable_info_data_msg(struct hwufcs_cable_info_data *p)
{
	u64 data = 0;
	u64 tmp_data;
	u8 len = sizeof(u64);
	int ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_data_msg(&pkt, HWUFCS_DATA_MSG_CABLE_INFO);
	if (ret)
		goto end;

	if (pkt.len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n", pkt.len, len);
		ret =  HWUFCS_ERR_ILLEGAL_DATA;
		goto end;
	}

	memcpy(&data, pkt.data, pkt.len);
	tmp_data = cpu_to_be64(data);

	/* max loading current */
	p->max_curr = (tmp_data >> HWUFCS_CABLE_INFO_SHIFT_MAX_CURR) &
		HWUFCS_CABLE_INFO_MASK_MAX_CURR;
	p->max_curr *= HWUFCS_CABLE_INFO_UNIT_CURR;
	/* max loading voltage */
	p->max_volt = (tmp_data >> HWUFCS_CABLE_INFO_SHIFT_MAX_VOLT) &
		HWUFCS_CABLE_INFO_MASK_MAX_VOLT;
	p->max_volt *= HWUFCS_CABLE_INFO_UNIT_VOLT;
	/* cable resistance */
	p->resistance = (tmp_data >> HWUFCS_CABLE_INFO_SHIFT_RESISTANCE) &
		HWUFCS_CABLE_INFO_MASK_RESISTANCE;
	p->resistance *= HWUFCS_CABLE_INFO_UNIT_RESISTANCE;
	/* cable electronic lable vendor id */
	p->reserved_1 = (tmp_data >> HWUFCS_CABLE_INFO_SHIFT_RESERVED_1) &
		HWUFCS_CABLE_INFO_MASK_RESERVED_1;
	/* cable vendor id */
	p->reserved_2 = (tmp_data >> HWUFCS_CABLE_INFO_SHIFT_RESERVED_2) &
		HWUFCS_CABLE_INFO_MASK_RESERVED_2;

	hwlog_info("cable_info=%lx %xmA %xmV %xmO %x %x\n", tmp_data,
		p->max_curr, p->max_volt, p->resistance, p->reserved_1, p->reserved_2);
end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

/* data message from source to sink: 0x06 device_information */
int hwufcs_receive_dev_info_data_msg(struct hwufcs_dev_info_data *p)
{
	u64 data = 0;
	u64 tmp_data;
	u8 len = sizeof(u64);
	int ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_data_msg(&pkt, HWUFCS_DATA_MSG_DEVICE_INFO);
	if (ret)
		goto end;

	if (pkt.len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n", pkt.len, len);
		ret =  HWUFCS_ERR_ILLEGAL_DATA;
		goto end;
	}

	memcpy(&data, pkt.data, pkt.len);
	tmp_data = cpu_to_be64(data);
	/* software version */
	p->sw_ver = (tmp_data >> HWUFCS_DEV_INFO_SHIFT_SW_VER) &
		HWUFCS_DEV_INFO_MASK_SW_VER;
	/* hardware version */
	p->hw_ver = (tmp_data >> HWUFCS_DEV_INFO_SHIFT_HW_VER) &
		HWUFCS_DEV_INFO_MASK_HW_VER;
	/* reserved 1 */
	p->reserved_1 = (tmp_data >> HWUFCS_DEV_INFO_SHIFT_RESERVED_1) &
		HWUFCS_DEV_INFO_MASK_RESERVED_1;
	/* reserved 2 */
	p->reserved_2 = (tmp_data >> HWUFCS_DEV_INFO_SHIFT_RESERVED_2) &
		HWUFCS_DEV_INFO_MASK_RESERVED_2;

	hwlog_info("dev_info=%lx %x %x %x %x\n", tmp_data,
		p->sw_ver, p->hw_ver, p->reserved_1, p->reserved_2);
end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

static void hwufcs_parse_error_info_data(u32 data, struct hwufcs_error_info_data *p)
{
	/* output ovp */
	p->output_ovp = (data >> HWUFCS_ERROR_INFO_SHIFT_OUTPUT_OVP) &
		HWUFCS_ERROR_INFO_MASK_OUTPUT_OVP;
	/* output uvp */
	p->output_uvp = (data >> HWUFCS_ERROR_INFO_SHIFT_OUTPUT_UVP) &
		HWUFCS_ERROR_INFO_MASK_OUTPUT_UVP;
	/* output ocp */
	p->output_ocp = (data >> HWUFCS_ERROR_INFO_SHIFT_OUTPUT_OCP) &
		HWUFCS_ERROR_INFO_MASK_OUTPUT_OCP;
	/* output scp */
	p->output_scp = (data >> HWUFCS_ERROR_INFO_SHIFT_OUTPUT_SCP) &
		HWUFCS_ERROR_INFO_MASK_OUTPUT_SCP;
	/* usb otp */
	p->usb_otp = (data >> HWUFCS_ERROR_INFO_SHIFT_USB_OTP) &
		HWUFCS_ERROR_INFO_MASK_USB_OTP;
	/* device otp */
	p->device_otp = (data >> HWUFCS_ERROR_INFO_SHIFT_DEVICE_OTP) &
		HWUFCS_ERROR_INFO_MASK_DEVICE_OTP;
	/* cc ovp */
	p->cc_ovp = (data >> HWUFCS_ERROR_INFO_SHIFT_CC_OVP) &
		HWUFCS_ERROR_INFO_MASK_CC_OVP;
	/* d- ovp */
	p->dminus_ovp = (data >> HWUFCS_ERROR_INFO_SHIFT_DMINUS_OVP) &
		HWUFCS_ERROR_INFO_MASK_DMINUS_OVP;
	/* d+ ovp */
	p->dplus_ovp = (data >> HWUFCS_ERROR_INFO_SHIFT_DPLUS_OVP) &
		HWUFCS_ERROR_INFO_MASK_DPLUS_OVP;
	/* input ovp */
	p->input_ovp = (data >> HWUFCS_ERROR_INFO_SHIFT_INPUT_OVP) &
		HWUFCS_ERROR_INFO_MASK_INPUT_OVP;
	/* input uvp */
	p->input_uvp = (data >> HWUFCS_ERROR_INFO_SHIFT_INPUT_UVP) &
		HWUFCS_ERROR_INFO_MASK_INPUT_UVP;
	/* over leakage current */
	p->over_leakage = (data >> HWUFCS_ERROR_INFO_SHIFT_OVER_LEAKAGE) &
		HWUFCS_ERROR_INFO_MASK_OVER_LEAKAGE;
	/* input drop */
	p->input_drop = (data >> HWUFCS_ERROR_INFO_SHIFT_INPUT_DROP) &
		HWUFCS_ERROR_INFO_MASK_INPUT_DROP;
	/* crc error */
	p->crc_error = (data >> HWUFCS_ERROR_INFO_SHIFT_CRC_ERROR) &
		HWUFCS_ERROR_INFO_MASK_CRC_ERROR;
	/* watchdog overflow */
	p->wtg_overflow = (data >> HWUFCS_ERROR_INFO_SHIFT_WTG_OVERFLOW) &
		HWUFCS_ERROR_INFO_MASK_WTG_OVERFLOW;
}

/* data message from source to sink: 0x07 error_information */
int hwufcs_receive_error_info_data_msg(struct hwufcs_error_info_data *p)
{
	u32 data = 0;
	u32 tmp_data;
	u8 len = sizeof(u32);
	int ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_data_msg(&pkt, HWUFCS_DATA_MSG_ERROR_INFO);
	if (ret)
		goto end;

	if (pkt.len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n", pkt.len, len);
		ret =  HWUFCS_ERR_ILLEGAL_DATA;
		goto end;
	}

	memcpy(&data, pkt.data, pkt.len);
	tmp_data = cpu_to_be32(data);
	hwufcs_parse_error_info_data(tmp_data, p);

	hwlog_info("error_info=%lx\n", tmp_data);
end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

/* data message from source to sink: 0x0b verify_response */
int hwufcs_receive_verify_response_data_msg(struct hwufcs_verify_response_data *p)
{
	u8 len = HWUFCS_VERIFY_RSP_ENCRYPT_SIZE + HWUFCS_VERIFY_RSP_RANDOM_SIZE;
	int ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_data_msg(&pkt, HWUFCS_DATA_MSG_VERIFY_RESPONSE);
	if (ret)
		goto end;

	if (pkt.len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n", pkt.len, len);
		ret =  HWUFCS_ERR_ILLEGAL_DATA;
		goto end;
	}

	memcpy(p, pkt.data, pkt.len);
end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

/* vdm data message from source to sink: 0x01 source_id */
int hwufcs_receive_vdm_source_id_data_msg(u16 *p)
{
	u16 data = 0;
	u8 len = sizeof(u16);
	int ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_vdm_data_msg(&pkt, HWUFCS_VDM_DATA_MSG_SOURCE_ID);
	if (ret)
		goto end;

	if (pkt.len != len) {
		hwlog_err("receive_data_msg length=%u,%u invalid\n", pkt.len, len);
		ret = HWUFCS_ERR_ILLEGAL_DATA;
		goto end;
	}

	memcpy(&data, pkt.data, pkt.len);
	*p = cpu_to_be16(data);
	hwlog_info("source_id=0x%x\n", *p);
end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

static void hwufcs_parse_vdm_scpb_pwr_curve_data(u64 data, struct adp_scpb_pwr_curve_data *p)
{
	p->min_curr = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_MIN_CURR) &
		HWUFCS_VDM_SCPB_PWR_MASK_MIN_CURR;
	p->min_curr *= HWUFCS_VDM_SCPB_PWR_UNIT_CURR;
	p->max_curr = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_MAX_CURR) &
		HWUFCS_VDM_SCPB_PWR_MASK_MAX_CURR;
	p->max_curr *= HWUFCS_VDM_SCPB_PWR_UNIT_CURR;
	p->min_volt = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_MIN_VOLT) &
		HWUFCS_VDM_SCPB_PWR_MASK_MIN_VOLT;
	p->min_volt *= HWUFCS_VDM_SCPB_PWR_UNIT_VOLT;
	p->max_volt = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_MAX_VOLT) &
		HWUFCS_VDM_SCPB_PWR_MASK_MAX_VOLT;
	p->max_volt *= HWUFCS_VDM_SCPB_PWR_UNIT_VOLT;
}

static void hwufcs_parse_vdm_scpb_pwr_gear_data(u64 data, struct adp_scpb_pwr_gear_data *p)
{
	p->pwr_5v = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_5V) &
		HWUFCS_VDM_SCPB_PWR_MASK_PWR_5V;
	p->pwr_5v /= HWUFCS_VDM_SCPB_PWR_STEP;
	p->pwr_10v = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_10V) &
		HWUFCS_VDM_SCPB_PWR_MASK_PWR_10V;
	p->pwr_10v /= HWUFCS_VDM_SCPB_PWR_STEP;
	p->pwr_15v = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_15V) &
		HWUFCS_VDM_SCPB_PWR_MASK_PWR_15V;
	p->pwr_15v /= HWUFCS_VDM_SCPB_PWR_STEP;
	p->pwr_20v = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_20V) &
		HWUFCS_VDM_SCPB_PWR_MASK_PWR_20V;
	p->pwr_20v /= HWUFCS_VDM_SCPB_PWR_STEP;
}

static void hwufcs_parse_vdm_scpb_pwr_data(u64 data, struct adp_scpb_pwr_data *p)
{
	p->output_mode = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_OUTPUT_MODE) &
		HWUFCS_VDM_SCPB_PWR_MASK_OUTPUT_MODE;
	p->pwr_type = (data >> HWUFCS_VDM_SCPB_PWR_SHIFT_PWR_TYPE) &
		HWUFCS_VDM_SCPB_PWR_MASK_PWR_TYPE;
	hwlog_info("scpb_pwr output_mode=%d pwr_type=%d\n", p->output_mode, p->pwr_type);
	switch (p->pwr_type) {
	case HWUFCS_VDM_SCPB_PWR_GEAR_TYPE:
		hwufcs_parse_vdm_scpb_pwr_gear_data(data, &p->gear_data);
		hwlog_info("pwr_5v=%dmW pwr_10v=%dmW pwr_15v=%dmW pwr_20v=%dmW\n",
			p->gear_data.pwr_5v, p->gear_data.pwr_10v, p->gear_data.pwr_15v, p->gear_data.pwr_20v);
		break;
	case HWUFCS_VDM_SCPB_PWR_CURVE_TYPE:
		hwufcs_parse_vdm_scpb_pwr_curve_data(data, &p->curve_data);
		hwlog_info("%umA %umA %umV %umV\n", p->curve_data.min_curr,
			p->curve_data.max_curr, p->curve_data.min_volt, p->curve_data.max_volt);
		break;
	default:
		hwlog_err("invalid pwr_type=%d\n", p->pwr_type);
		break;
	}
}

/* vdm data message from source to sink: 0x02 scpb_power */
int hwufcs_receive_vdm_scpb_pwr_data_msg(struct adp_scpb_pwr_data *p,
	u8 *ret_len)
{
	u64 data[HWUFCS_SCPB_MAX_PWR_MODE] = { 0 };
	u64 tmp_data;
	u8 len = HWUFCS_SCPB_MAX_PWR_MODE * sizeof(u64);
	int i, ret;
	struct hwufcs_package_data pkt = { 0 };

	ret = hwufcs_receive_msg(&pkt, true);
	if (ret) {
		hwlog_err("receive msg err\n");
		goto end;
	}

	/* check data msg */
	ret = hwufcs_check_receive_vdm_data_msg(&pkt, HWUFCS_VDM_DATA_MSG_SCPB_POWER);
	if (ret)
		goto end;

	if ((pkt.len % sizeof(u64) != 0) || (pkt.len > len)) {
		hwlog_err("output_capabilities length=%u,%u invalid\n", pkt.len, len);
		return HWUFCS_ERR_ILLEGAL_DATA;
	}

	memcpy(&data, pkt.data, pkt.len);
	*ret_len = pkt.len / sizeof(u64);
	for (i = 0; i < *ret_len; i++) {
		tmp_data = cpu_to_be64(data[i]);
		hwlog_err("lytttt tmp_data=0x%lx\n", tmp_data);
		hwufcs_parse_vdm_scpb_pwr_data(tmp_data, &p[i]);
	}

end:
	hwufcs_handle_communication_result(ret, &pkt);
	return ret;
}

