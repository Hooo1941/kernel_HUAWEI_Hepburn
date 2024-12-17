/*
 * cpld Driver
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
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


#include <securec.h>
#include "cpld_driver.h"

#define CPLD_DEVICE_NAME "huawei_cpld"
#define I2C_RW_RETRY_NUM 3
#define XFER_NUM 2
#define I2C_WAIT_TIME 25
#define CPLD_FW_UPDATE_RESULT 128
#define CPLD_START_TAG_VALUE 0x5A
#define CPLD_END_TAG_VALUE 0xB8
#define CPLD_REG_COUNT  18
#define CPLD_REG_INFO_SIZE  1024

#define HWLOG_TAG CPLD
HWLOG_REGIST();

#define cpld_log_info(x...) _hwlog_info(HWLOG_TAG, ##x)
#define cpld_log_err(x...) _hwlog_err(HWLOG_TAG, ##x)
#define cpld_log_debug(x...) do { \
		if (1) \
			_hwlog_info(HWLOG_TAG, ##x); \
	} while (0)

static struct cpld_core_data *g_cpld_core_data;

enum cpld_reg {
	REG_0 = 0, // cpld start tag
	REG_1,     // record the power-on time, max value:200, per 50us increased 1
	/* Host data */
	REG_2,     // 4'b0,handshake version number[9:8]
	REG_3,     // version number[7:0]
	REG_4,     // I2C_0[5:0],I2C_1[5:4]
	REG_5,     // I2C_1[3:0],I2C_2[5:2]
	REG_6,     // I2C_2[1:0],I2C_100hz[5:0]
	REG_7,     // I2Cx3 verify error[5:0],GPIO receive bit error[5:4]
	REG_8,     // GPIO receive bit error[3:0],GPIO receive timeout[5:2]
	REG_9,     // GPIO Receive timeout[1:0],RFFE transmission buffer bit error
	/* Slave data transfer to master */
	REG_10,    // 6'b0,version number[9:8]
	REG_11,    // version number[7:0]
	REG_12,    // I2C_0[5:0],I2C_1[5:4]
	REG_13,    // I2C_1[3:0],I2C_2[5:2]
	REG_14,    // I2C_2[1:0],I2C_100hz[5:0]
	REG_15,    // I2Cx3 verify error[5:0],GPIO receive bit error[5:4]
	REG_16,    // GPIO receive bit error[3:0],GPIO receive timeout[5:2]
	REG_17,    // GPIO Receive timeout[1:0],RFFE transmission buffer bit error
	REG_18,    // cpld end tag
};

enum type {
	MAIN = 1,
	SLAVE,
};

static int cpld_i2c_read(u8 *reg_addr, u16 reg_len, u8 *buf, u16 len)
{
	struct cpld_core_data *cd = g_cpld_core_data;
	int count = 0;
	int ret;
	int msg_len;
	struct i2c_msg *msg_addr = NULL;
	struct i2c_msg xfer[XFER_NUM];

	/* register addr */
	xfer[0].addr = cd->client->addr;
	xfer[0].flags = 0;
	xfer[0].len = reg_len;
	xfer[0].buf = reg_addr;

	/* Read data */
	xfer[1].addr = cd->client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = buf;

	if (reg_len > 0) {
		msg_len = XFER_NUM;
		msg_addr = &xfer[0];
	} else {
		msg_len = 1;
		msg_addr = &xfer[1];
	}
	do {
		ret = i2c_transfer(cd->client->adapter,
			msg_addr, msg_len);
		if (ret == msg_len) {
			return 0;
		}
		cpld_log_err("%s:i2c read status: %d\n", __func__, ret);
		msleep(I2C_WAIT_TIME);
	} while (++count < I2C_RW_RETRY_NUM);

	cpld_log_err("%s failed\n", __func__);
	return -EIO;
}

static int cpld_check_init_status(struct cpld_core_data *cpld_data)
{
	int ret = 0;
	u8 cpld_start_tag_addr = REG_0;
	u8 cpld_end_tag_addr = REG_18;

	ret = cpld_i2c_read(&cpld_start_tag_addr, 1, &(cpld_data->cpld_reg_data_arr[REG_0]), 1);
	cpld_log_err("%s:get reg_0 value = 0x%02x, ret = %d\n", __func__,
					cpld_data->cpld_reg_data_arr[REG_0], ret);
	if (ret < 0) {
		cpld_log_err("%s:get reg_0 failed.\n", __func__);
		return ret;
	}
	ret = cpld_i2c_read(&cpld_end_tag_addr, 1, &(cpld_data->cpld_reg_data_arr[REG_18]), 1);
	cpld_log_err("%s:get reg_18 value = 0x%02x, ret = %d\n", __func__,
					cpld_data->cpld_reg_data_arr[REG_18], ret);
	if (ret < 0) {
		cpld_log_err("%s:get reg_18 failed.\n", __func__);
		return ret;
	}

	if ((cpld_data->cpld_reg_data_arr[REG_0] == CPLD_START_TAG_VALUE) &&
		(cpld_data->cpld_reg_data_arr[REG_18] == CPLD_END_TAG_VALUE))
		return 0;

	return ret;
}

static int cpld_get_all_reg_data(struct cpld_core_data *cpld_data)
{
	int ret = 0;
	u8 reg_index = REG_0;
	for (; reg_index <= REG_18; reg_index++) {
		ret = cpld_i2c_read(&reg_index, 1, &(cpld_data->cpld_reg_data_arr[reg_index]), 1);
		if (ret < 0) {
			cpld_log_err("%s:get reg_%d failed.\n", __func__, reg_index);
			return ret;
		}
		cpld_log_err("%s:cpld reg_%d value = 0x%02x\n", __func__,
						reg_index, cpld_data->cpld_reg_data_arr[reg_index]);
	}
	return ret;
}

void cpld_show_all_reg_data(void)
{
	if (g_cpld_core_data == NULL) {
		cpld_log_err("%s:cpld data not ready.\n", __func__);
		return;
	}
	cpld_get_all_reg_data(g_cpld_core_data);
}

static int cpld_update_single_reg_data(struct cpld_core_data *cpld_data, const u8 reg)
{
	int ret = 0;
	u8 reg_index = reg;
	ret = cpld_i2c_read(&reg_index, 1, &(cpld_data->cpld_reg_data_arr[reg]), 1);
		if (ret < 0) {
		cpld_log_err("%s:get reg_%d failed.\n", __func__, reg_index);
		return ret;
	}
	return ret;
}

static u8 cpld_get_handshake_result(struct cpld_core_data *cpld_data)
{
	u8 cpld_handshake_result = 0;
	cpld_update_single_reg_data(cpld_data, REG_2);
	cpld_handshake_result |= ((cpld_data->cpld_reg_data_arr[REG_2] >> 2) & 0x03);
	cpld_log_err("%s:handshake result : 0x%02x\n", __func__, cpld_handshake_result);
	return cpld_handshake_result;
}

static void cpld_get_fw_version(struct cpld_core_data *cpld_data)
{
	u16 main_fw_version = 0;
	u16 slave_fw_version = 0;
	main_fw_version |= ((cpld_data->cpld_reg_data_arr[REG_2] & 0x3) << 8);
	main_fw_version |= cpld_data->cpld_reg_data_arr[REG_3];

	slave_fw_version |= ((cpld_data->cpld_reg_data_arr[REG_10] & 0x3) << 8);
	slave_fw_version |= cpld_data->cpld_reg_data_arr[REG_11];

	cpld_data->main_usercode = main_fw_version;
	cpld_data->slave_usercode = slave_fw_version;

	cpld_log_err("%s:main firmware version = 0x%04x\n", __func__, main_fw_version);
	cpld_log_err("%s:slave firmware version = 0x%04x\n", __func__, slave_fw_version);
}

static u8 cpld_get_left_half_data(struct cpld_core_data *cpld_data, const u8 reg, u8 bit_width)
{
	u8 cpld_left_half_data = 0;
	cpld_left_half_data |= (cpld_data->cpld_reg_data_arr[reg] >> (8 - bit_width));
	return cpld_left_half_data;
}

static u8 cpld_get_right_half_data(struct cpld_core_data *cpld_data, const u8 reg, u8 bit_width)
{
	u8 cpld_right_half_data = 0;
	u8 offset = 1;

	while (--bit_width)
		offset += (offset << 1);

	cpld_right_half_data |=
		((cpld_data->cpld_reg_data_arr[reg] & offset) << (6 - bit_width));

	return cpld_right_half_data;
}

static const u8 cpld_get_i2cx3_verify_error_data(const int type)
{
	u8 reg_data = 0;
	if (type == MAIN) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_7);
		reg_data = cpld_get_left_half_data(g_cpld_core_data, REG_7, 6);
	} else if (type == SLAVE) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_15);
		reg_data = cpld_get_left_half_data(g_cpld_core_data, REG_15, 6);
	}
	return reg_data;
}

static const u8 cpld_get_gpio_receive_error_data(const int type)
{
	u8 reg_data = 0;
	if (type == MAIN) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_7);
		cpld_update_single_reg_data(g_cpld_core_data, REG_8);
		reg_data |= cpld_get_right_half_data(g_cpld_core_data, REG_7, 2);
		reg_data |= cpld_get_left_half_data(g_cpld_core_data, REG_8, 4);
	} else if (type == SLAVE) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_15);
		cpld_update_single_reg_data(g_cpld_core_data, REG_16);
		reg_data |= cpld_get_right_half_data(g_cpld_core_data, REG_15, 2);
		reg_data |= cpld_get_left_half_data(g_cpld_core_data, REG_16, 4);
	}
	return reg_data;
}

static const u8 cpld_get_gpio_receive_timeout_data(const int type)
{
	u8 reg_data = 0;
	if (type == MAIN) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_8);
		cpld_update_single_reg_data(g_cpld_core_data, REG_9);
		reg_data |= cpld_get_right_half_data(g_cpld_core_data, REG_8, 4);
		reg_data |= cpld_get_left_half_data(g_cpld_core_data, REG_9, 2);
	} else if (type == SLAVE) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_16);
		cpld_update_single_reg_data(g_cpld_core_data, REG_17);
		reg_data |= cpld_get_right_half_data(g_cpld_core_data, REG_16, 4);
		reg_data |= cpld_get_left_half_data(g_cpld_core_data, REG_17, 2);
	}
	return reg_data;
}

static const u8 cpld_get_rffe_transmit_buffer_error_data(const int type)
{
	u8 reg_data = 0;
	if (type == MAIN) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_9);
		reg_data = cpld_get_right_half_data(g_cpld_core_data, REG_9, 6);
	} else if (type == SLAVE) {
		cpld_update_single_reg_data(g_cpld_core_data, REG_17);
		reg_data = cpld_get_right_half_data(g_cpld_core_data, REG_17, 6);
	}
	return reg_data;
}

static ssize_t cpld_i2c_connection_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 val = 0;
	val = cpld_check_init_status(g_cpld_core_data);
	if (val != 0)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "check init status failed!		\
						\n\rstart tag = 0x%02x, end tag = 0x%02x\n",					\
						g_cpld_core_data->cpld_reg_data_arr[REG_0], 					\
						g_cpld_core_data->cpld_reg_data_arr[REG_18]);

	val = cpld_get_handshake_result(g_cpld_core_data);
	if (val != 0x02)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "check handshake failed!	\
						\n\rcpld handshake result = 0x%02x\n", val);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "cpld i2c connection ok\n");
}

static ssize_t cpld_firmware_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "cpld firmware version:				\
										\n\rmain = 0x%04x, slave = 0x%04x\n",
										g_cpld_core_data->main_usercode,
										g_cpld_core_data->slave_usercode);
}

static ssize_t cpld_i2cx3_verify_error_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "i2cx3 verify error status:			\
										\n\rmain = 0x%02x, slave = 0x%02x\n",               \
										cpld_get_i2cx3_verify_error_data(MAIN),     		\
										cpld_get_i2cx3_verify_error_data(SLAVE));
}

static ssize_t cpld_rffe_transmit_buffer_error_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "FREE transmit buffer error code:		\
										\n\rmain = 0x%02x, slave = 0x%02x\n",				\
										cpld_get_rffe_transmit_buffer_error_data(MAIN),		\
										cpld_get_rffe_transmit_buffer_error_data(SLAVE));
}

static ssize_t cpld_gpio_recevice_error_code_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "GPIO recevice error code:		\
										\n\rmain = 0x%02x, slave = 0x%02x\n",		\
										cpld_get_gpio_receive_error_data(MAIN),		\
										cpld_get_gpio_receive_error_data(SLAVE));
}

static ssize_t cpld_gpio_recevice_timeout_code_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "GPIO recevice timeout code:		\
										\n\rmain = 0x%02x, slave = 0x%02x\n",			\
										cpld_get_gpio_receive_timeout_data(MAIN),		\
										cpld_get_gpio_receive_timeout_data(SLAVE));
}

static ssize_t cpld_print_all_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len = 0;
	int i = 0;
	char reg_data_buf[CPLD_REG_INFO_SIZE] = {'\0'};
	for (i = 0; i <= CPLD_REG_COUNT; i++)
		len += sprintf_s(reg_data_buf + len, CPLD_REG_INFO_SIZE - 1, "REG%d:0x%02x\n",
						i, g_cpld_core_data->cpld_reg_data_arr[i]);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "cpld all reg data:					\
													\r\n%s", reg_data_buf);
}

DEVICE_ATTR(connection_status, 0440, cpld_i2c_connection_status_show, NULL);
DEVICE_ATTR(i2cx3_verify_error_status, 0440, cpld_i2cx3_verify_error_status_show, NULL);
DEVICE_ATTR(RFFE_transmit_buffer_error_code, 0440, cpld_rffe_transmit_buffer_error_show, NULL);
DEVICE_ATTR(GPIO_recevice_error_code, 0440, cpld_gpio_recevice_error_code_show, NULL);
DEVICE_ATTR(GPIO_recevice_timeout_code, 0440, cpld_gpio_recevice_timeout_code_show, NULL);
DEVICE_ATTR(firmware_version, 0440, cpld_firmware_version_show, NULL);
DEVICE_ATTR(cpld_all_reg_data, 0440, cpld_print_all_reg_show, NULL);

static struct platform_device cpld_device = {
	.name = "cpld_device",
};

static int create_cpld_device(struct cpld_core_data *cpld_data)
{
	int ret;

	ret = platform_device_register(&cpld_device);
	if (ret) {
		cpld_log_err("%s: platform_device_register failed, ret:%d\n",
			__func__, ret);
		return -EINVAL;
	}
	if (device_create_file(&cpld_device.dev, &dev_attr_connection_status)) {
		cpld_log_err("%s:Unable to create connection status interface\n", __func__);
		return -EINVAL;
	}
	if (device_create_file(&cpld_device.dev, &dev_attr_firmware_version)) {
		cpld_log_err("%s:Unable to create firmware version interface\n", __func__);
		return -EINVAL;
	}
	if (device_create_file(&cpld_device.dev, &dev_attr_cpld_all_reg_data)) {
		cpld_log_err("%s:Unable to create all reg data interface\n", __func__);
		return -EINVAL;
	}
	if (device_create_file(&cpld_device.dev, &dev_attr_i2cx3_verify_error_status)) {
		cpld_log_err("%s:Unable to create i2cx3 verify error status interface\n", __func__);
		return -EINVAL;
	}
	if (device_create_file(&cpld_device.dev, &dev_attr_RFFE_transmit_buffer_error_code)) {
		cpld_log_err("%s:Unable to create RFFE transmit buffer error code interface\n", __func__);
		return -EINVAL;
	}
	if (device_create_file(&cpld_device.dev, &dev_attr_GPIO_recevice_error_code)) {
		cpld_log_err("%s:Unable to create GPIO recevice error code interface\n", __func__);
		return -EINVAL;
	}

	if (device_create_file(&cpld_device.dev, &dev_attr_GPIO_recevice_timeout_code)) {
		cpld_log_err("%s:Unable to create GPIO recevice timeout code interface\n", __func__);
		return -EINVAL;
	}

	cpld_log_err("%s:out\n", __func__);
	return 0;
}

static int create_platform_init(struct cpld_core_data *cpld_data)
{
	int ret;
	cpld_get_handshake_result(cpld_data);
	cpld_get_fw_version(cpld_data);
	ret = create_cpld_device(cpld_data);
	if (ret) {
		cpld_log_err("%s:platform data is required!\n", __func__);
		return ret;
	}

	cpld_log_info("%s:out, ret = %d\n", __func__, ret);
	return ret;
}

static int cpld_probe(struct i2c_client *i2c_client,
	const struct i2c_device_id *id)
{
	struct cpld_core_data *cpld_data = NULL;
	int ret;

	cpld_log_info("%s: enter\n", __func__);

	cpld_data = kzalloc(sizeof(struct cpld_core_data), GFP_KERNEL);
	if (!cpld_data) {
		cpld_log_err("%s:platform data is required!\n", __func__);
		return -EINVAL;
	}

	cpld_data->client = i2c_client;
	g_cpld_core_data = cpld_data;

	ret = cpld_check_init_status(cpld_data);
	if (ret) {
		cpld_log_err("%s:the cpld start tag or end tag not correct.\n", __func__);
		goto create_cpld_device_err;
	}

	ret = cpld_get_all_reg_data(cpld_data);
	if (ret) {
		cpld_log_err("%s:cpld get all reg data failed.\n", __func__);
		goto create_cpld_device_err;
	}

	ret = create_platform_init(cpld_data);
	cpld_log_err("%s: out, ret = %d\n", __func__, ret);
	return 0;
create_cpld_device_err:
	kfree(cpld_data);
	g_cpld_core_data = NULL;
	return 0;
}

static int cpld_remove(struct i2c_client *client)
{
	cpld_log_info("%s: out\n", __func__);
	return 0;
}

static const struct of_device_id g_cpld_match_table[] = {
	{ .compatible = "huawei,cpld", },
	{ },
};

MODULE_DEVICE_TABLE(of, g_cpld_match_table);

static const struct i2c_device_id g_cpld_device_id[] = {
	{ CPLD_DEVICE_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, g_cpld_device_id);

static struct i2c_driver g_huawei_cpld_driver = {
	.probe = cpld_probe,
	.remove = cpld_remove,
	.id_table = g_cpld_device_id,
	.driver = {
		.name = CPLD_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = g_cpld_match_table,
	},
};

static int __init cpld_i2c_init(void)
{
	int ret = 0;

	cpld_log_info("%s: enter\n", __func__);
	ret = i2c_add_driver(&g_huawei_cpld_driver);
	if (ret) {
		cpld_log_err("%s: error\n", __func__);
		return ret;
	}
	return 0;
}

late_initcall(cpld_i2c_init);

static void __exit cpld_i2c_exit(void)
{
	i2c_del_driver(&g_huawei_cpld_driver);
}

module_exit(cpld_i2c_exit);

MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei cpld Driver");
MODULE_LICENSE("GPL");

