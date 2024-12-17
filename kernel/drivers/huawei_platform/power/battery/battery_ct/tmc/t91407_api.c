// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * t91407_api.c
 *
 * interface for t91407.c
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

#include "t91407_api.h"
#include <securec.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/battery/battery_type_identify.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <huawei_platform/power/power_mesg_srv.h>
#include "t91407_swi.h"
#include "core/algorithm.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG t91407_api
HWLOG_REGIST();

struct t91407_file_start_page {
	uint8_t file_no;
	uint8_t start_page;
	uint8_t page_num;
};

static const struct t91407_file_start_page g_page_no[] = {
	{T91407_FILE_ACT, T91407_FILE_ACT_PAGE, T91407_FILE_ACT_NUM},
	{T91407_FILE_BATTTYPE, T91407_FILE_BATTTYPE_PAGE, T91407_FILE_BATTTYPE_NUM},
	{T91407_FILE_LOCK, T91407_FILE_LOCK_PAGE, T91407_FILE_LOCK_NUM},
	{T91407_FILE_SN, T91407_FILE_SN_PAGE, T91407_FILE_SN_NUM},
	{T91407_FILE_GSN, T91407_FILE_GSN_PAGE, T91407_FILE_GSN_NUM},
};

static const unsigned char crc8_table1[16] = {
	0x00, 0x64, 0xC8, 0xAC, 0xE1, 0x85, 0x29, 0x4D,
	0xB3, 0xD7, 0X7B, 0x1F, 0x52, 0x36, 0x9A, 0xFE,
};

static const unsigned char crc8_table2[16] = {
	0x00, 0x17, 0x2E, 0x39, 0x5C, 0x4B, 0x72, 0x65,
	0xB8, 0xAF, 0X96, 0x81, 0xE4, 0xF3, 0xCA, 0xDD,
};

static const unsigned short crc16_table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a,
	0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294,
	0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462,
	0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509,
	0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695,
	0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5,
	0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948,
	0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4,
	0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b,
	0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f,
	0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
	0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046,
	0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290,
	0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e,
	0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691,
	0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
	0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d,
	0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16,
	0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8,
	0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e,
	0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93,
	0x3eb2, 0x0ed1, 0x1ef0
};

static void set_sched_affinity_to_current(void)
{
	long retval;
	int current_cpu;

	preempt_disable();
	current_cpu = smp_processor_id();
	preempt_enable();
	retval = sched_setaffinity(0, cpumask_of(current_cpu));
	if (retval)
		hwlog_info("[%s] Set cpu af to current cpu failed %ld\n", __func__, retval);
	else
		hwlog_info("[%s] Set cpu af to current cpu %d\n", __func__, current_cpu);
}

static void set_sched_affinity_to_all(void)
{
	long retval;
	cpumask_t dstp;

	cpumask_setall(&dstp);
	retval = sched_setaffinity(0, &dstp);
	if (retval)
		hwlog_info("[%s] Set cpu af to all valid cpus failed %ld\n", __func__, retval);
	else
		hwlog_info("[%s] Set cpu af to all valid cpus\n", __func__);
}

static void t91407_gen_rand(unsigned char *rnd, int byte_num)
{
	int i;

	for (i = 0; i < byte_num; i++)
		get_random_bytes(&rnd[i], sizeof(unsigned char));
}

static bool t91407_is_all_zero(const char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (buf[i])
			return false;
	}

	return true;
}

static unsigned char crc8_caculate(unsigned char by_accum, unsigned char *pby_buf, unsigned short w_len)
{
	unsigned char *ptr_end;
	unsigned char  by_data;

	ptr_end = pby_buf + w_len;
	for (; pby_buf < ptr_end; pby_buf++) {
		by_data  = by_accum ^ *pby_buf;
		by_accum = crc8_table1[by_data & T91407_MAX_OF_BYT1] ^ crc8_table2[by_data >> T91407_CRC_CALC_OFFSETS];
	}
	return by_accum;
}

static int t91407_receive(struct t91407_dev *di,  unsigned char *receive, int receive_length)
{
	int ret;
	unsigned char receive_buffer[T91407_LEN_OF_RECEIVE_DATA] = { 0 };

	/* Receive data or interrupt */
	if (!receive_length)
		return 0;

	if (!receive) {
		hwlog_err("[%s] pointer NULL\n", __func__);
		return -EINVAL;
	}
	t91407_udelay(T91407_SWI_RRA_PUSH_PULL);
	ret = t91407_read_data(di, receive_buffer, receive_length);
	if (!ret) {
		if (memcpy_s(receive, receive_length, receive_buffer, receive_length) != EOK) {
			hwlog_err("[%s] memcpy_s failed\n", __func__);
			return T91407_FAILED;
		}
	}
	return ret;
}

static int t91407_send_data_and_receive(struct t91407_dev *di, unsigned char *receive, unsigned char *send,
	unsigned char send_length, int receive_length)
{
	uint32_t send_data = 0;
	unsigned char code = T91407_SEND_CODE;
	unsigned char data = 0;
	unsigned char invert_flag = 0;
	unsigned int counter = 0;
	char write[T91407_DATA_READ_LEN] = { 0 };

#ifdef ONEWIRE_STABILITY_DEBUG
	t91407_print_data(send, send_length, write, T91407_DATA_READ_LEN);
	hwlog_info("[%s] t91407_write_data: %s\n", __func__, write);
#endif
	for (int i = 0; i < send_length; i++) {
		/* Config invert flag */
		data = send[i];

		for (int j = 0, counter = 0; j < T91407_LEN_OF_DATA; j++) {
			counter += (data & 0x01);
			data >>= 1;
		}
		data = send[i];

		if (counter > T91407_LEN_OF_INVERT) {
			code ^= T91407_SEND_INVERT_CODE;
			data = ~data;
			invert_flag = 1;
		}

		send_data = (code << T91407_SEND_CODE_OFFSETS) + (data << T91407_SEND_DATA_OFFSETS) + invert_flag;
		t91407_write_data(di, send_data);
		code = T91407_SEND_CODE;
		invert_flag = 0;
	}

	return t91407_receive(di, receive, receive_length);
}

static int t91407_send_bus_cmd_and_receive(struct t91407_dev *di, unsigned char *receive, unsigned char code,
	unsigned char data, int receive_length)
{
	unsigned char cmd_header[T91407_LEN_OF_SEND_CMD_HEADER] = { 0 };
	int ret;

	/* Only INT to be returned as immediately response, receive_length = 1 in this case */
	if (receive_length > 1)
		return T91407_ERR_WRONG_PARAMETER;

	/* Calculate CRC with 1110 + code + data */
	cmd_header[0] = T91407_BUSCMD_FRAME_TAG | code;
	cmd_header[1] = data;
	cmd_header[2] = crc8_caculate(0, cmd_header, T91407_REC_LEN_BYT);

	/* Invert bit to be configured before sending */
	ret = t91407_send_data_and_receive(di, receive, cmd_header, T91407_LEN_OF_SEND_CMD_HEADER, receive_length);
	if (ret)
		hwlog_err("[%s] Send Bus CMD failed: %u\n", __func__, ret);

	return ret;
}

static void t91407_select_device(struct t91407_dev *di)
{
	int ret;
	unsigned char buf[T91407_LEN_OF_SELECT_DEV_REC] = { 0 };

	ret = t91407_send_bus_cmd_and_receive(di, buf, T91407_SWI_BC, T91407_SWI_SFCD, T91407_LEN_OF_SELECT_DEV_REC);
	/* wait interupt */
	if (ret)
		hwlog_err("[%s] Selet device failed\n", __func__);
	t91407_udelay(T91407_DELAY_OF_WRITE_RAM);
}

static void t91407_power_on(struct t91407_dev *di)
{
	if (!di) {
		hwlog_err("[%s] di is null\n", __func__);
		return;
	}
	t91407_gpio_dir_output(di->onewire_gpio, T91407_LOW_VOLTAGE);
	t91407_udelay(T91407_SWI_DELAY_POWERUP_330US);
	t91407_gpio_dir_output(di->onewire_gpio, T91407_HIGH_VOLTAGE);
	t91407_udelay(T91407_SWI_DELAY_POWERUP_6MS);
	t91407_gpio_dir_output(di->onewire_gpio, T91407_LOW_VOLTAGE);
	t91407_udelay(T91407_SWI_DELAY_POWERUP_1MS);
	t91407_gpio_dir_output(di->onewire_gpio, T91407_HIGH_VOLTAGE);
	t91407_udelay(T91407_SWI_DELAY_POWERUP_6MS);

	t91407_select_device(di);

	di->device_inited = 1;
}

static void t91407_power_off(struct t91407_dev *di)
{
	if (!di) {
		hwlog_err("[%s] di is null\n", __func__);
		return;
	}
	t91407_gpio_dir_input(di->onewire_gpio);
}

static void t91407_dev_on(struct t91407_dev *di)
{
	set_sched_affinity_to_current();
	local_irq_save(di->irq_flags);
	t91407_power_on(di);
	local_irq_restore(di->irq_flags);
}

static void t91407_dev_off(struct t91407_dev *di)
{
	t91407_power_off(di);
	set_sched_affinity_to_all();
}

static unsigned short crc16_caculate(unsigned char *data_src, unsigned int data_len)
{
	unsigned char *dat_ptr = data_src;
	unsigned short crc_reg = 0;

	while (data_len--)
		crc_reg = (crc_reg << T91407_ONE_BYT_OFFSETS) ^
			crc16_table[((crc_reg >> T91407_ONE_BYT_OFFSETS) ^ *dat_ptr++) & T91407_MAX_OF_BYT2];
	return crc_reg;
}

static int t91407_clear_process_state(struct t91407_dev *di)
{
	int ret = 0;

	/* 预留功能 */
	return ret;
}

static void t91407_split_address(unsigned short address, unsigned char *high_address, unsigned char *low_address)
{
	*high_address = (unsigned char)((address >> T91407_ONE_BYT_OFFSETS) & T91407_MAX_OF_ADDR);
	*low_address = (unsigned char)(address & T91407_MAX_OF_ADDR);
}

static int t91407_get_response_receive(struct t91407_dev *di,  unsigned char *receive, unsigned char package_len)
{
	int ret = 0;

	if (package_len)
		return 0;
	if (receive[1] == 0) {
		/*
		 * Get response data length (include execution result and CRC), maximum receive length = 256
		 * receive length = 3rd byte
		 */
		ret = t91407_send_data_and_receive(di, receive, NULL, 0, receive[T91407_REC_LEN_BYT]);
		if (ret) {
			hwlog_err("[%s] Receive Response data failed!: %u\n", __func__, ret);
			return ret;
		}
	} else {
		/* CRC verification */
		if (crc8_caculate(0, receive + 1, T91407_REC_LEN_BYT)) {
			ret = T91407_ERR_CRC_VERIGY;
			hwlog_err("[%s] Receive Response failed!: %u\n", __func__, ret);
			return ret;
		}
	}
	return ret;
}

static int t91407_get_response(struct t91407_dev *di, unsigned char *receive,
	unsigned char package_id, unsigned char package_len)
{
	int receive_length;
	int ret;
	unsigned char cmd_header[T91407_LEN_OF_SEND_CMD_HEADER] = {T91407_GET_RESPONSE_TAG, package_id, package_len};

	if (!receive)
		return T91407_ERR_WRONG_PARAMETER;
	/* Config receive length */
	receive_length = package_len;
	if (!package_id) {
		if (package_len && (package_len != T91407_LEN_OF_PACKAGE))
			return T91407_ERR_WRONG_PARAMETER;
		receive_length = T91407_LEN_OF_PACKAGE;
	} else if (package_id == 1) {
		if ((package_len > T91407_LEN_OF_RECEIVE_DATA) || (!package_len))
			return T91407_ERR_WRONG_PARAMETER;
	} else {   /* package ID > 1 */
		if ((package_len > T91407_PACKAGE_DEEP) || (!package_len))
			return T91407_ERR_WRONG_PARAMETER;
	}

	/* Invert bit to be configured before sending, incoming data in receive buffer */
	ret = t91407_send_data_and_receive(di, receive, cmd_header, T91407_LEN_OF_SEND_CMD_HEADER, receive_length);
	if (ret) {
		hwlog_err("[%s] Send Get Response failed!: %u\n", __func__, ret);
		return ret;
	}

	/* finish data receiving and return data, check CRC in uplayer */
	if (package_id == 0) {
		/* pacakgeID = 0, check frame tag and incoming data length */
		if (receive[0] != T91407_DATA_FRAME_TAG) {
			ret = T91407_ERR_READ_CODE_ERROR;
			hwlog_err("[%s] Receive Response failed!: %u\n", __func__, ret);
			return ret;
		}
		ret = t91407_get_response_receive(di, receive, package_len);
		if (ret) {
			hwlog_err("[%s] t91407_get_response_receive failed!: %u\n", __func__, ret);
			return ret;
		}
	}
	return ret;
}

static int t91407_cmd_receive(struct t91407_dev *di, unsigned char *receive, int receive_length)
{
	int ret;
	unsigned int response_length;
	unsigned char receive_buffer[T91407_LEN_OF_RECEIVE_DATA + T91407_REC_LEN_BYT] = { 0 };

	if (!receive_length)
		return 0;

	/* receiving all response data in one time */
	response_length = receive_length + 1;
	ret = t91407_get_response(di, receive_buffer, 1, response_length);
	t91407_udelay(T91407_DELAY_OF_WRITE_RAM);
	if (ret) {
		hwlog_err("[%s] Get CMD response data failed!: %u\n", __func__, ret);
		return ret;
	}

	/* CRC verification */
	if (crc8_caculate(0, receive_buffer, response_length)) {
		ret = T91407_ERR_CRC_VERIGY;
		hwlog_err("[%s] Get CMD response crc failed!: %u\n", __func__, ret);
		return ret;
	}
	ret = memcpy_s(receive, receive_length, receive_buffer, receive_length);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}
	return ret;
}

static int t91407_send_cmd_and_receive(struct t91407_dev *di, unsigned char *receive,
	unsigned char *cmd_header, int data_length, int receive_length)
{
	unsigned char send_buffer[T91407_LEN_OF_SEND_DATA] = { 0 };
	unsigned int send_length;
	int ret;

	if (receive_length > T91407_LEN_OF_RECEIVE_DATA)
		return T91407_ERR_WRONG_PARAMETER;

	send_buffer[0] = T91407_DATA_FRAME_TAG;
	send_buffer[1] = 0;
	send_buffer[2] = data_length + T91407_LEN_OF_CMD_HEADER + 1;

	/* copy cmd header + cmd data */
	ret = memcpy_s(send_buffer + T91407_LEN_OF_SEND_CMD_HEADER, T91407_LEN_OF_CMD_HEADER + data_length,
		cmd_header, T91407_LEN_OF_CMD_HEADER + data_length);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}

	/* calculate CRC and config length of data to be sent */
	send_length = data_length + T91407_LEN_OF_SENDING_FORMAT; /* 发送格式分包指令0xAA和1位CRC校验 */
	send_buffer[send_length - 1] = crc8_caculate(0, cmd_header, T91407_LEN_OF_CMD_HEADER + data_length);

	/* Invert bit to be configured before sending */
	ret = t91407_send_data_and_receive(di, NULL, send_buffer, send_length, 0);
	t91407_udelay(di->t91407_swi.ow_execution_delay);

	if (ret) {
		hwlog_err("[%s] Send CMD failed!: %u\n", __func__, ret);
		return ret;
	}

	return t91407_cmd_receive(di, receive, receive_length);
}

static int t91407_get_state(struct t91407_dev *di, unsigned char state_address)
{
	unsigned char state_buffer[T91407_LEN_OF_REC_BUF] = { 0 };
	unsigned char state = 0;
	int ret;

	ret = t91407_get_response(di, state_buffer, 0, 0);
	t91407_udelay(T91407_DELAY_OF_WRITE_RAM);

	state = state_buffer[1];

	if (!ret) {
		if ((state == T91407_RES_STATE_SUCC) || (state == T91407_RES_STATE_SUCC1))
			state = T91407_SUCCESS;
		ret = state;
	}
	if (ret)
		hwlog_err("[%s] failed: %d\n", __func__, ret);
	return ret;
}

static void get_cmd_data(unsigned char *cmd_data, unsigned short address, uint8_t length, unsigned char ins)
{
	unsigned char high_addr = 0;
	unsigned char low_addr = 0;
	unsigned char high_addr_end = 0;
	unsigned char low_addr_end = 0;

	t91407_split_address(address, &high_addr, &low_addr);
	t91407_split_address((address + length), &high_addr_end, &low_addr_end);

	cmd_data[1] = ins;
	cmd_data[2] = high_addr;
	cmd_data[3] = low_addr;
	cmd_data[4] = length;
}

static void t91407_data_failed_handle(struct t91407_dev *di, int ret)
{
	if ((ret == T91407_ERR_TIME_OUT) || (ret == T91407_ERR_READ_FAILED)) {
		local_irq_save(di->irq_flags);
		di->device_inited = 0;
		t91407_power_on(di);
		local_irq_restore(di->irq_flags);
	}
	set_sched_affinity_to_all();
	msleep(T91407_SWI_DELAY_PACKAGE_RETRY_MS);
	set_sched_affinity_to_current();
}

static int t91407_read_data_by_address(struct t91407_dev *di, unsigned char *buffer,
	unsigned short address, uint8_t length)
{
	unsigned char cmd_header[T91407_LEN_OF_CMD_HEADER] = { 0 };
	int ret;

	/* Clear process state */
	ret = t91407_clear_process_state(di);
	if (ret)
		return ret;

	get_cmd_data(cmd_header, address, length, T91407_INS_READ);
	ret = t91407_send_cmd_and_receive(di, buffer, cmd_header, 0, length);
	if (ret)
		hwlog_err("[%s] Read data failed: %u\n", __func__, ret);
	return ret;
}

static int t91407_read_data_retry(struct t91407_dev *di, unsigned char *buffer,
	unsigned short address, uint8_t length)
{
	int ret;

	for (uint8_t retry = 0; retry < T91407_MAX_PACKAGE_RETRY_COUNT; retry++) {
		local_irq_save(di->irq_flags);
		ret = t91407_read_data_by_address(di, buffer, address, length);
		local_irq_restore(di->irq_flags);
		if (!ret)
			break;
		hwlog_err("[%s] fail\n", __func__);
		t91407_data_failed_handle(di, ret);
	}

	return ret;
}

static int t91407_write_data_by_address(struct t91407_dev *di, unsigned char *buffer,
	unsigned short address, uint8_t length)
{
	unsigned char cmd_data[T91407_LEN_OF_SEND_DATA] = { 0 };
	int ret;
 
	/* Clear process state */
	ret = t91407_clear_process_state(di);
	if (ret)
		return ret;

	get_cmd_data(cmd_data, address, length, T91407_INS_WRITE);
	ret = memcpy_s(cmd_data + T91407_CMD_OFFSETS, length, buffer, length);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}

	di->t91407_swi.ow_execution_delay = T91407_DELAY_OF_WRITE_FLASH;
	ret = t91407_send_cmd_and_receive(di, NULL, cmd_data, length, 0);
	di->t91407_swi.ow_execution_delay = T91407_DELAY_OF_EXECUTION;
	if (ret) {
		hwlog_err("[%s] Write data failed!: %u\n", __func__, ret);
		return ret;
	}
	return t91407_get_state(di, T91407_SWI_PROCESSSW);
}

static int t91407_write_data_retry(struct t91407_dev *di, unsigned char *buffer,
	unsigned short address, uint8_t length)
{
	int ret;

	for (uint8_t retry = 0; retry < T91407_MAX_PACKAGE_RETRY_COUNT; retry++) {
		local_irq_save(di->irq_flags);
		ret = t91407_write_data_by_address(di, buffer, address, length);
		local_irq_restore(di->irq_flags);
		if (!ret)
			break;
		hwlog_err("[%s] fail\n", __func__);
		t91407_data_failed_handle(di, ret);
	}

	return ret;
}

static int t91407_read_raw_romid(struct t91407_dev *di, uint8_t *buf)
{
	uint8_t uid_ret[T91407_LEN_OF_UID + T91407_LEN_OF_UID_CRC];
	int ret;
	uint32_t i;

	ret = t91407_read_data_retry(di, uid_ret, T91407_SWI_UID, T91407_LEN_OF_UID + T91407_LEN_OF_UID_CRC);
	if (ret) {
		hwlog_err("[%s] fail : %d\n", __func__, ret);
		return ret;
	}

#ifdef ONEWIRE_STABILITY_DEBUG
	for (i = 0; i < T91407_LEN_OF_UID; i++)
		hwlog_err("[%s] uid_ret[%d] = %x\n", __func__, i, uid_ret[i]);
#endif

	if (t91407_is_all_zero(uid_ret, T91407_LEN_OF_UID)) {
		hwlog_err("[%s] uid_ret all zero\n", __func__);
		return -EINVAL;
	}

	if (crc16_caculate(uid_ret, (T91407_LEN_OF_UID + T91407_LEN_OF_UID_CRC))) {
		hwlog_err("[%s] uid_ret crc16_caculate fail\n", __func__);
		return -EINVAL;
	}

	ret = memcpy_s(buf, T91407_LEN_OF_UID, uid_ret, T91407_LEN_OF_UID);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

int t91407_ic_ck(struct t91407_dev *di)
{
	int ret;

	if (!di)
		return -ENODEV;
	t91407_dev_on(di);
	ret = t91407_read_raw_romid(di, di->mem.uid);
	t91407_dev_off(di);

	return ret;
}

static bool get_uid_cache(struct t91407_dev *di, char *buf)
{
	int reslt;
	char write[T91407_DATA_READ_LEN] = { 0 };

	if (di->mem.uid_ready) {
		reslt = memmove_s(buf, T91407_UID_LEN, di->mem.uid, T91407_UID_LEN);
		if (reslt != EOK) {
			hwlog_err("[%s] memmove_s failed\n", __func__);
			return false;
		}
		t91407_print_data(di->mem.uid, T91407_UID_LEN, write, T91407_DATA_READ_LEN);
		hwlog_info("[%s] cache data is %s\n", __func__, write);
	}
	return true;
}

static void set_uid_cache(struct t91407_dev *di, char *buf)
{
	int reslt;

	if (di->mem.uid_ready)
		return;
	reslt = memmove_s(di->mem.uid, T91407_UID_LEN, buf, T91407_UID_LEN);
	if (reslt != EOK) {
		hwlog_err("[%s] memmove_s failed\n", __func__);
		return;
	}
	hwlog_info("[%s] data is %x %x\n", __func__, di->mem.uid[0], di->mem.uid[1]);
	di->mem.uid_ready = true;
}

int t91407_read_romid(struct t91407_dev *di, char *buf)
{
	int ret;
	int reslt;
	char uid_buffer[T91407_UID_LEN] = { 0 };

	if (!di || !buf)
		return -ENODEV;

	if (di->mem.uid_ready) {
		ret = get_uid_cache(di, buf);
		if (ret)
			return 0;
	}
	t91407_dev_on(di);
	ret = t91407_read_raw_romid(di, uid_buffer);
	t91407_dev_off(di);
	if (!ret) {
		reslt = memcpy_s(buf, T91407_UID_LEN, uid_buffer, T91407_UID_LEN);
		if (reslt != EOK) {
			hwlog_err("[%s] memcpy_s failed\n", __func__);
			return T91407_FAILED;
		}
		set_uid_cache(di, buf);
	}
	hwlog_info("[%s] data is %x %x\n", __func__, di->mem.uid[0], di->mem.uid[1]);
	return ret;
}

static int t91407_read_page_data(struct t91407_dev *di, unsigned char *buffer, int page, int offset, int length)
{
	int ret;

	if ((!length) || ((offset + length) > T91407_BYTE_OF_ONE_PAGE) || (page >= T91407_USER_PAGE_NUM)) {
		hwlog_err("[%s] params error\n", __func__);
		return T91407_ERR_WRONG_PARAMETER;
	}

	ret = t91407_read_data_retry(di, buffer, (T91407_SWI_USERPAGE_START +
								(page * T91407_BYTE_OF_ONE_PAGE) + offset), length);
	if (ret)
		hwlog_err("[%s] fail : %d\n", __func__, ret);

	return ret;
}

static int t91407_write_page_data(struct t91407_dev *di, unsigned char *buffer, int page, int offset, int length)
{
	int ret;

	if ((!length) || ((offset + length) > T91407_BYTE_OF_ONE_PAGE) || (page >= T91407_USER_PAGE_NUM)) {
		hwlog_err("[%s] params error\n", __func__);
		return T91407_ERR_WRONG_PARAMETER;
	}

	ret = t91407_write_data_retry(di, buffer, (T91407_SWI_USERPAGE_START +
								(page * T91407_BYTE_OF_ONE_PAGE) + offset), length);
	if (ret)
		hwlog_err("[%s] fail : %d\n", __func__, ret);

	return ret;
}

static int find_fid(uint8_t *page, uint8_t *num, int fid)
{
	for (uint8_t i = 0; i < ARRAY_SIZE(g_page_no); i++) {
		if (fid == g_page_no[i].file_no) {
			*page = g_page_no[i].start_page;
			*num = g_page_no[i].page_num;
			return 0;
		}
	}
	hwlog_err("[%s] find_fid fail\n", __func__);
	return -EINVAL;
}

static int t91407_read_file(struct t91407_dev *di, int fid, unsigned char *buf, int size)
{
	int ret;
	uint8_t rbuf[T91407_READ_RECV_MAX_LEN];
	uint8_t start_page = 0;
	uint8_t page_num = 0;
	uint8_t page_count;

	if (!buf) {
		hwlog_err("[%s] : pointer NULL\n", __func__);
		return -EINVAL;
	}

	ret = find_fid(&start_page, &page_num, fid);
	if (ret)
		return T91407_ERR_WRONG_PARAMETER;

	t91407_dev_on(di);
	page_count = (size / T91407_BYTE_OF_ONE_PAGE);
	if (size % T91407_BYTE_OF_ONE_PAGE)
		page_count += 1;
	for (uint8_t i = 0; i < T91407_MAX_COMMAND_RETRY_COUNT; i++) {
		for (uint8_t page_no = start_page; page_no < (start_page + page_count); page_no++) {
			if (page_no < (start_page + (size / T91407_BYTE_OF_ONE_PAGE)))
				ret = t91407_read_page_data(di, &rbuf[(page_no - start_page) * T91407_BYTE_OF_ONE_PAGE],
											page_no, 0, T91407_BYTE_OF_ONE_PAGE);
			else
				ret = t91407_read_page_data(di, &rbuf[(page_no - start_page) * T91407_BYTE_OF_ONE_PAGE],
											page_no, 0, (size % T91407_BYTE_OF_ONE_PAGE));
			if (ret)
				break;
		}
		if (!ret)
			break;
		set_sched_affinity_to_all();
		msleep(T91407_SWI_DELAY_COMMAND_RETRY_MS);
		set_sched_affinity_to_current();
	}
	t91407_dev_off(di);

	if (ret) {
		hwlog_err("[%s] t91407_read_file fail : %d\n", __func__, ret);
		return ret;
	}

	ret = memcpy_s(buf, size, rbuf, size);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}
	return 0;
}

static int t91407_write_file(struct t91407_dev *di, int fid, unsigned char *buf, int size)
{
	int ret;
	uint8_t wbuf[T91407_READ_RECV_MAX_LEN];
	uint8_t start_page = 0;
	uint8_t page_num = 0;
	uint8_t page_count;

	if (!buf) {
		hwlog_err("[%s] : pointer NULL\n", __func__);
		return -EINVAL;
	}

	ret = memcpy_s(wbuf, T91407_READ_RECV_MAX_LEN, buf, size);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}

	ret = find_fid(&start_page, &page_num, fid);
	if (ret)
		return T91407_ERR_WRONG_PARAMETER;

	t91407_dev_on(di);
	page_count = (size / T91407_BYTE_OF_ONE_PAGE);
	if (size % T91407_BYTE_OF_ONE_PAGE)
		page_count += 1;
	for (uint8_t i = 0; i < T91407_MAX_COMMAND_RETRY_COUNT; i++) {
		for (uint8_t page_no = start_page; page_no < (start_page + page_count); page_no++) {
			if (page_no < (start_page + (size / T91407_BYTE_OF_ONE_PAGE)))
				ret = t91407_write_page_data(di, &wbuf[(page_no - start_page) * T91407_BYTE_OF_ONE_PAGE],
											page_no, 0, T91407_BYTE_OF_ONE_PAGE);
			else
				ret = t91407_write_page_data(di, &wbuf[(page_no - start_page) * T91407_BYTE_OF_ONE_PAGE],
											page_no, 0, (size % T91407_BYTE_OF_ONE_PAGE));
			if (ret)
				break;
		}
		if (!ret)
			break;
		set_sched_affinity_to_all();
		msleep(T91407_SWI_DELAY_COMMAND_RETRY_MS);
		set_sched_affinity_to_current();
	}
	t91407_dev_off(di);

	if (ret) {
		hwlog_err("[%s] t91407_write_file fail : %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int t91407_get_lock_state(struct t91407_dev *di, unsigned char *lock_state)
{
	int ret;

	ret = t91407_read_data_retry(di, lock_state, T91407_SWI_PAGELOCK0, T91407_LEN_OF_LOCKSTATE);
	if (ret) {
		hwlog_err("[%s] fail : %d\n", __func__, ret);
		return ret;
	}
	hwlog_info("[%s] is %x %x %x %x\n", __func__, lock_state[0], lock_state[1],
		lock_state[2], lock_state[3]);
	return T91407_SUCCESS;
}

static int t91407_get_file_lock_state(struct t91407_dev *di, int fid, unsigned char *buf, int buf_len)
{
	int ret;
	int unlock_reslt;
	unsigned char lock_state[T91407_LEN_OF_LOCKSTATE] = { 0 };
	unsigned char page_state;
	uint8_t start_page = 0;
	uint8_t page_num = 0;

	t91407_dev_on(di);
	ret = t91407_get_lock_state(di, lock_state);
	t91407_dev_off(di);
	if (ret) {
		hwlog_err("[%s] t91407_get_lock_state fail : %u\n", __func__, ret);
		return ret;
	}

	ret = find_fid(&start_page, &page_num, fid);
	if (ret)
		return T91407_ERR_WRONG_PARAMETER;

	for (uint8_t k = start_page; k < (start_page + page_num); k++) {
		int j = T91407_MAX_INDEX_OF_BYTE - (k - (k / T91407_BYTE_OF_ONE_PAGE) * T91407_BYTE_OF_ONE_PAGE);
		if (k == start_page)
			page_state = (lock_state[k / T91407_BYTE_OF_ONE_PAGE] & (1 << j)) >> j;
		else
			page_state &= (lock_state[k / T91407_BYTE_OF_ONE_PAGE] & (1 << j)) >> j;
	}
	if (page_state) {
		ret = snprintf_s(buf, buf_len, buf_len - 1, "locked\n");
		if (ret < 0)
			hwlog_err("[%s] snprintf_s is failed\n", __func__);
	} else {
		unlock_reslt = snprintf_s(buf, buf_len, buf_len - 1, "unlocked\n");
		if (unlock_reslt < 0)
			hwlog_err("[%s] snprintf_s is failed\n", __func__);
	}
	return T91407_SUCCESS;
}

static int t91407_lock_file(struct t91407_dev *di, int fid)
{
	unsigned char lock_state[T91407_LEN_OF_LOCKSTATE] = { 0 };
	int ret;
	uint8_t start_page = 0;
	uint8_t page_num = 0;

	t91407_dev_on(di);
	ret = t91407_get_lock_state(di, lock_state);
	t91407_dev_off(di);
	if (ret) {
		hwlog_err("[%s] t91407_get_lock_state fail : %u\n", __func__, ret);
		return ret;
	}

	ret = find_fid(&start_page, &page_num, fid);
	if (ret)
		return T91407_ERR_WRONG_PARAMETER;

	for (uint8_t k = start_page; k < (start_page + page_num); k++) {
		int j = T91407_MAX_INDEX_OF_BYTE - (k - (k / T91407_BYTE_OF_ONE_PAGE) * T91407_BYTE_OF_ONE_PAGE);
		lock_state[k / T91407_BYTE_OF_ONE_PAGE] |= 1 << j;
	}
	di->t91407_swi.ow_execution_delay = T91407_DELAY_OF_LOCK;
	t91407_dev_on(di);
	ret = t91407_write_data_retry(di, lock_state, T91407_SWI_PAGELOCK0, T91407_LEN_OF_LOCKSTATE);
	t91407_dev_off(di);
	di->t91407_swi.ow_execution_delay = T91407_DELAY_OF_EXECUTION;
	if (ret) {
		hwlog_err("[%s] t91407_write_data_retry fail : %u\n", __func__, ret);
		return ret;
	}
	return T91407_SUCCESS;
}

int t91407_operate_file(struct t91407_dev *di, uint8_t file, int operation, unsigned char *buf, int buf_len)
{
	int ret;

	switch (operation) {
	case T91407_READ:
		ret = t91407_read_file(di, file, buf, buf_len);
		break;
	case T91407_WRITE:
		ret = t91407_write_file(di, file, buf, buf_len);
		break;
	case T91407_LOCK:
		ret = t91407_lock_file(di, file);
		break;
	case T91407_LOCK_STATUS:
		ret = t91407_get_file_lock_state(di, file, buf, buf_len);
		break;
	default:
		hwlog_err("[%s] no operetion\n", __func__);
		return -EINVAL;
	}

	return ret;
}

static int t91407_read_certification(struct t91407_dev *di, unsigned char* certification, int length)
{
	int ret;
	int reslt;
	uint8_t page_count;
	uint8_t rbuf[T91407_READ_RECV_MAX_LEN];

	if (length != T91407_LEN_OF_CERTIFICATION) {
		hwlog_err("[%s] Params error\n", __func__);
		return T91407_ERR_WRONG_PARAMETER;
	}

	page_count = length / T91407_BYTE_OF_ONE_PAGE;
	for (uint8_t i = 0; i < T91407_MAX_COMMAND_RETRY_COUNT; i++) {
		for (uint8_t page_no = 0; page_no < page_count; page_no++) {
			ret = t91407_read_data_retry(di, &rbuf[page_no * T91407_BYTE_OF_ONE_PAGE],
				(T91407_SWI_CERT + page_no * T91407_BYTE_OF_ONE_PAGE), T91407_BYTE_OF_ONE_PAGE);
			if (ret)
				break;
		}
		if (!ret)
			break;
		set_sched_affinity_to_all();
		msleep(T91407_SWI_DELAY_COMMAND_RETRY_MS);
		set_sched_affinity_to_current();
	}

	if (ret) {
		hwlog_err("[%s] t91407_read_certification fail : %d\n", __func__, ret);
		return ret;
	}

	reslt = memcpy_s(certification, length, rbuf, length);
	if (reslt != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}
	return 0;
}

static int t91407_calc_hash(unsigned char *dest, unsigned char *src, int length)
{
	unsigned char puc_last_blk[T91407_LEN_OF_HASH_32] = { 0 };
	unsigned char hash[T91407_LEN_OF_HASH_32] = { 0 };
	int ret;
	int i = 0;

	for (i = 0; i < (length / T91407_LEN_OF_HASHUPDATE); i++) {
		if (i == 0)
			tmc_sha256update(hash, &src[i * T91407_LEN_OF_HASHUPDATE], puc_last_blk, 1);
		else
			tmc_sha256update(hash, &src[i * T91407_LEN_OF_HASHUPDATE], hash, 0);
	}

	if (length % T91407_LEN_OF_HASHUPDATE) {
		length = (length - ((length / T91407_LEN_OF_HASHUPDATE) * T91407_LEN_OF_HASHUPDATE));
		ret = tmc_sha256final(hash, &src[i * T91407_LEN_OF_HASHUPDATE], hash, i, length);
		if (ret != T91407_CALC_HASH_SUCC) {
			hwlog_err("[%s] Calc hash256 failed!\n", __func__);
			return T91407_ERR_CALC_HASH_FAILED;
		}
	}

	ret = memcpy_s(dest, T91407_LEN_OF_HASH_32, hash, T91407_LEN_OF_HASH_32);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}

	return T91407_SUCCESS;
}

static int t91407_verify_certification(int key_length, unsigned char *certification, int cert_length,
	const unsigned char *cert_public_key_256, int pubkey_length)
{
	unsigned char hash[T91407_LEN_OF_HASH_32] = { 0 };
	int ret;

	if ((key_length != T91407_PUBKEY_LEN) || (cert_length != T91407_LEN_OF_CERTIFICATION) ||
		(pubkey_length != T91407_PUBKEY_LEN)) {
		hwlog_err("[%s] Params error\n", __func__);
		return T91407_ERR_WRONG_PARAMETER;
	}

	ret = t91407_calc_hash(hash, &certification[T91407_SIGN_LEN], (T91407_LEN_OF_UID + T91407_PUBKEY_LEN));
	if (ret) {
		hwlog_err("[%s] t91407_calc_hash fail\n", __func__);
		return ret;
	}

	/* Verify certification sign */
	ret = tmc_ecc_verify(cert_public_key_256, hash, certification);
	if (ret != VERIFY_VALID) {
		hwlog_err("[%s] ECC verify failed\n", __func__);
		return T91407_FAILED;
	}
	return T91407_SUCCEED;
}

static int t91407_send_random(struct t91407_dev *di, unsigned char* random, int length)
{
	int ret;
	uint8_t page_count;
	uint8_t wbuf[T91407_READ_RECV_MAX_LEN];
	uint8_t last_page_length;

	if (length != T91407_RANDOM_LEN) {
		hwlog_err("[%s] Params error\n", __func__);
		return T91407_ERR_WRONG_PARAMETER;
	}

	ret = memcpy_s(wbuf, T91407_READ_RECV_MAX_LEN, random, length);
	if (ret != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}

	page_count = length / T91407_BYTE_OF_ONE_PAGE;
	for (uint8_t page_no = 0; page_no < page_count; page_no++) {
		local_irq_save(di->irq_flags);
		ret = t91407_write_data_by_address(di, &wbuf[page_no * T91407_BYTE_OF_ONE_PAGE],
			(T91407_SWI_CHALLENGE + (page_no * T91407_BYTE_OF_ONE_PAGE)), T91407_BYTE_OF_ONE_PAGE);
		local_irq_restore(di->irq_flags);
		if (ret) {
			hwlog_err("[%s] t91407_write_data_by_address failed\n", __func__);
			return ret;
		}
	}

	return 0;
}

static int t91407_start_ecc(struct t91407_dev *di)
{
	int ret;

	local_irq_save(di->irq_flags);
	ret = t91407_send_bus_cmd_and_receive(di, NULL, T91407_SWI_BC, T91407_SWI_ECCE, 0);
	local_irq_restore(di->irq_flags);
	if (ret) {
		hwlog_err("[%s] fail : %d\n", __func__, ret);
		return ret;
	}
	t91407_gpio_dir_output(di->onewire_gpio, T91407_HIGH_VOLTAGE);

	set_sched_affinity_to_all();
	msleep(T91407_DELAY_OF_M_SIGN); /* wait ECC calculate */
	set_sched_affinity_to_current();

	local_irq_save(di->irq_flags);
	ret = t91407_get_state(di, T91407_SWI_CMDSW);
	local_irq_restore(di->irq_flags);
	return ret;
}

static int t91407_get_sign(struct t91407_dev *di, unsigned char *sign, int length)
{
	int ret;
	uint8_t page_count;
	uint8_t rbuf[T91407_READ_RECV_MAX_LEN];

	if (length != T91407_SIGN_LEN) {
		hwlog_err("[%s] Params error\n", __func__);
		return T91407_ERR_WRONG_PARAMETER;
	}

	page_count = length / T91407_BYTE_OF_ONE_PAGE;
	for (uint8_t page_no = 0; page_no < page_count; page_no++) {
		for (uint8_t retry = 0; retry < T91407_MAX_PACKAGE_RETRY_COUNT; retry++) {
			local_irq_save(di->irq_flags);
			ret = t91407_read_data_by_address(di, &rbuf[page_no * T91407_BYTE_OF_ONE_PAGE],
				T91407_SWI_SIGNATURE + (page_no * T91407_BYTE_OF_ONE_PAGE), T91407_BYTE_OF_ONE_PAGE);
			local_irq_restore(di->irq_flags);
			if (!ret)
				break;
			if ((ret == T91407_ERR_READ_FAILED) && (retry < (T91407_MAX_PACKAGE_RETRY_COUNT - 1))) {
				hwlog_err("[%s] t91407_read_data_by_address failed countinue: %d\n", __func__, ret);
				set_sched_affinity_to_all();
				msleep(T91407_SWI_DELAY_PACKAGE_RETRY_MS);
				set_sched_affinity_to_current();
				continue;
			}
			hwlog_err("[%s] t91407_read_data_by_address failed: %d\n", __func__, ret);
			return ret;
		}
	}

	if (memcpy_s(sign, length, rbuf, length) != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}
	return 0;
}

static int t91407_verify_sign(unsigned char *key, int key_length, unsigned char *random, int random_length,
	unsigned char* sign, int sign_length)
{
	int ret;

	if ((sign_length != T91407_SIGN_LEN) || (key_length != T91407_PUBKEY_LEN) ||
		(random_length != T91407_RANDOM_LEN)) {
		hwlog_err("[%s] Params error!\n", __func__);
		return T91407_ERR_WRONG_PARAMETER;
	}

	ret = tmc_ecc_verify(key, random, sign);
	if (ret != VERIFY_VALID) {
		hwlog_err("[%s] Verify failed!\n", __func__);
		return T91407_ERR_VERIFY_FAILED;
	}

	return T91407_SUCCEED;
}

static int t91407_do_authentication_prepare(struct t91407_dev *di, unsigned char *rand_data,
	unsigned char *sign_rand)
{
	int ret;

	t91407_dev_on(di);
	for (uint8_t i = 0; i < T91407_MAX_PACKAGE_RETRY_COUNT; i++) {
		if (i) {
			local_irq_save(di->irq_flags);
			t91407_power_on(di);
			local_irq_restore(di->irq_flags);
		}
		ret = t91407_send_random(di, rand_data, T91407_RANDOM_LEN);
		if (ret)
			continue;

		t91407_udelay(T91407_SWI_DELAY_POWERUP_1MS);
		ret = t91407_start_ecc(di);
		if (ret) {
			set_sched_affinity_to_all();
			msleep(T91407_SWI_DELAY_PACKAGE_RETRY_MS);
			set_sched_affinity_to_current();
			continue;
		}

		t91407_udelay(T91407_SWI_DELAY_POWERUP_1MS);
		ret = t91407_get_sign(di, sign_rand, T91407_SIGN_LEN);
		if (!ret)
			break;
	}
	t91407_dev_off(di);

	return ret;
}

static int t91407_do_authentication_retry(struct t91407_dev *di, struct power_genl_attr *key_res)
{
	int ret;
	unsigned char certification[T91407_LEN_OF_CERTIFICATION] = { 0 };
	unsigned char se_pub_key[T91407_PUBKEY_LEN] = { 0 };
	unsigned char rand_data[T91407_RANDOM_LEN] = { 0 };
	unsigned char sign_rand[T91407_SIGN_LEN] = { 0 };

	t91407_dev_on(di);
	ret = t91407_read_certification(di, certification, T91407_LEN_OF_CERTIFICATION);
	t91407_dev_off(di);
	if (ret)
		return -EAGAIN;
	ret = t91407_verify_certification(T91407_PUBKEY_LEN, certification, T91407_LEN_OF_CERTIFICATION,
									key_res->data, key_res->len);
	if (ret)
		return -EINVAL;
	if (memcpy_s(se_pub_key, T91407_PUBKEY_LEN, &certification[(T91407_SIGN_LEN + T91407_LEN_OF_UID)],
		T91407_PUBKEY_LEN) != EOK) {
		hwlog_err("[%s] memcpy_s failed\n", __func__);
		return T91407_FAILED;
	}
	t91407_gen_rand(rand_data, T91407_RANDOM_LEN);

	ret = t91407_do_authentication_prepare(di, rand_data, sign_rand);
	if (ret)
		return -EINVAL;

	ret = t91407_verify_sign(se_pub_key, T91407_PUBKEY_LEN, rand_data, T91407_RANDOM_LEN, sign_rand, T91407_SIGN_LEN);
	if (ret)
		return -EINVAL;
	return 0;
}

int t91407_do_authentication(struct t91407_dev *di, struct power_genl_attr *key_res)
{
	int ret;

	for (uint8_t retry = 0; retry < T91407_MAX_AUTHENTICATION_RETRY_COUNT; retry++) {
		hwlog_info("[%s] t91407_do_authentication retry\n", __func__);
		ret = t91407_do_authentication_retry(di, key_res);
		if (!ret) {
			hwlog_info("[%s] t91407_do_authentication success\n", __func__);
			break;
		}
		msleep(T91407_SWI_DELAY_COMMAND_RETRY_MS);
	}
	return ret;
}