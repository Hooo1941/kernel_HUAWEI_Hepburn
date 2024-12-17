/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro channel source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "gyro_channel.h"

#include <linux/err.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include "contexthub_boot.h"

#include <securec.h>

#include "contexthub_route.h"
#include "gyro_sysfs.h"
#include "contexthub_recovery.h"
#define GYRO_CALI_TEMP_OFFSET  72
#define GYRO_CALI_PART1_LEN    72
#define PATH_MAXLEN            128
#define FILE_LIMIT		       0660
enum calibrate_data {
	BIAS_X = 0x0,
	BIAS_Y,
	BIAS_Z,
	SCALE_X,
	SCALE_Y,
	SCALE_Z,
	MATRIX_XX,
	MATRIX_XY,
	MATRIX_XZ,
	MATRIX_YX,
	MATRIX_YY,
	MATRIX_YZ,
	MATRIX_ZX,
	MATRIX_ZY,
	MATRIX_ZZ,
	ONLINE_X,
	ONLINE_Y,
	ONLINE_Z,
	OFFSET_SUM_X, // the follow define only for sh5001 temper cali
	OFFSET_SUM_Y,
	OFFSET_SUM_Z,
	TEMPER_OFFSET_X,  // 21
	TEMPER_OFFSET_Y,
	TEMPER_OFFSET_Z
};

int send_gyro1_calibrate_data_to_mcu(void)
{
	struct gyro_device_info *dev_info = NULL;
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();
#endif

	dev_info = gyro_get_device_info(TAG_GYRO1);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(GYRO1_OFFSET_NV_NUM,
		GYRO1_OFFSET_NV_SIZE, GYRO1_NV_NAME))
		return -1;

	dev_info->gyro_first_start_flag = 1;
	/* copy to gyro1 offset by pass */
	if (memcpy_s(gyro1_sensor_offset, sizeof(gyro1_sensor_offset),
		user_info.nv_data, sizeof(gyro1_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gyro1 offset: %d %d %d online offset:%d %d %d\n",
		gyro1_sensor_offset[0], gyro1_sensor_offset[1],
		gyro1_sensor_offset[2], gyro1_sensor_offset[15],
		gyro1_sensor_offset[16], gyro1_sensor_offset[17]);
#ifdef CONFIG_HUAWEI_DSM
	if (gyro1_sensor_offset[BIAS_X] == 0 && gyro1_sensor_offset[BIAS_Y] == 0 &&
		gyro1_sensor_offset[BIAS_Z] == 0) {
		dsm_client_record(client, "dmd_case = %d", TAG_GYRO1);
		dsm_client_notify(client, DSM_SHB_ERR_GSENSOR_DATA_ALL_ZERO);
		hwlog_info("set gyro1 offset data all zero\n");
	}
#endif
	if (memcpy_s(&gyro1_sensor_calibrate_data,
		sizeof(gyro1_sensor_calibrate_data),
		gyro1_sensor_offset,
		(sizeof(gyro1_sensor_calibrate_data) < GYRO1_OFFSET_NV_SIZE) ?
		sizeof(gyro1_sensor_calibrate_data) : GYRO1_OFFSET_NV_SIZE) !=
		EOK)
		return -1;
	if (send_calibrate_data_to_mcu(TAG_GYRO1, SUB_CMD_SET_OFFSET_REQ,
		gyro1_sensor_offset, GYRO1_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}

int send_gyro2_calibrate_data_to_mcu(void)
{
	struct gyro_device_info *dev_info = NULL;
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();
#endif

	dev_info = gyro_get_device_info(TAG_GYRO2);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(GYRO2_OFFSET_NV_NUM,
		GYRO2_OFFSET_NV_SIZE, GYRO2_NV_NAME))
		return -1;

	dev_info->gyro_first_start_flag = 1;
	/* copy to gyro2 offset by pass */
	if (memcpy_s(gyro2_sensor_offset, sizeof(gyro2_sensor_offset),
		user_info.nv_data, sizeof(gyro2_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gyro2 offset: %d %d %d online offset:%d %d %d\n",
		gyro2_sensor_offset[0], gyro2_sensor_offset[1],
		gyro2_sensor_offset[2], gyro2_sensor_offset[15],
		gyro2_sensor_offset[16], gyro2_sensor_offset[17]);
#ifdef CONFIG_HUAWEI_DSM
	if (gyro2_sensor_offset[BIAS_X] == 0 && gyro2_sensor_offset[BIAS_Y] == 0 &&
		gyro2_sensor_offset[BIAS_Z] == 0) {
		dsm_client_record(client, "dmd_case = %d", TAG_GYRO2);
		dsm_client_notify(client, DSM_SHB_ERR_GSENSOR_DATA_ALL_ZERO);
		hwlog_info("set gyro2 offset data all zero\n");
	}
#endif
	if (memcpy_s(&gyro2_sensor_calibrate_data,
		sizeof(gyro2_sensor_calibrate_data),
		gyro2_sensor_offset,
		(sizeof(gyro2_sensor_calibrate_data) < GYRO2_OFFSET_NV_SIZE) ?
		sizeof(gyro2_sensor_calibrate_data) : GYRO2_OFFSET_NV_SIZE) !=
		EOK)
		return -1;
	if (send_calibrate_data_to_mcu(TAG_GYRO2, SUB_CMD_SET_OFFSET_REQ,
		gyro2_sensor_offset, GYRO2_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}

int send_gyro_calibrate_data_to_mcu(void)
{
	struct gyro_device_info *dev_info = NULL;
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();
#endif

	dev_info = gyro_get_device_info(TAG_GYRO);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(GYRO_CALIDATA_NV_NUM,
		GYRO_CALIDATA_NV_SIZE, "GYRO"))
		return -1;

	dev_info->gyro_first_start_flag = 1;
	/* copy to gsensor_offset by pass */
	if (memcpy_s(gyro_sensor_offset, sizeof(gyro_sensor_offset),
		user_info.nv_data, sizeof(gyro_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gyro_sensor offset: %d %d %d sensitity:%d %d %d\n",
		gyro_sensor_offset[0], gyro_sensor_offset[1],
		gyro_sensor_offset[2], gyro_sensor_offset[3],
		gyro_sensor_offset[4], gyro_sensor_offset[5]);
	hwlog_info("nve_direct_access read gyro_sensor xis_angle: %d %d %d %d %d %d %d %d %d\n",
		gyro_sensor_offset[6], gyro_sensor_offset[7],
		gyro_sensor_offset[8], gyro_sensor_offset[9],
		gyro_sensor_offset[10], gyro_sensor_offset[11],
		gyro_sensor_offset[12], gyro_sensor_offset[13],
		gyro_sensor_offset[14]);
	hwlog_info("nve_direct_access read gyro_sensor online offset: %d %d %d len:%d nv:%d num:%d\n",
		gyro_sensor_offset[15], gyro_sensor_offset[16], gyro_sensor_offset[17],
		sizeof(gyro_sensor_offset),
		GYRO_CALIDATA_NV_SIZE, GYRO_CALIDATA_NV_NUM);
#ifdef CONFIG_HUAWEI_DSM
	if (gyro_sensor_offset[BIAS_X] == 0 && gyro_sensor_offset[BIAS_Y] == 0 &&
		gyro_sensor_offset[BIAS_Z] == 0) {
		dsm_client_record(client, "dmd_case = %d", TAG_GYRO);
		dsm_client_notify(client, DSM_SHB_ERR_GSENSOR_DATA_ALL_ZERO);
		hwlog_info("set gyro offset data all zero\n");
	}
#endif
	if (memcpy_s(&gyro_sensor_calibrate_data,
		sizeof(gyro_sensor_calibrate_data),
		gyro_sensor_offset,
		(sizeof(gyro_sensor_calibrate_data) < GYRO_CALIDATA_NV_SIZE) ?
		sizeof(gyro_sensor_calibrate_data) : GYRO_CALIDATA_NV_SIZE) !=
		EOK)
		return -1;
	if (send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_SET_OFFSET_REQ,
		gyro_sensor_offset, GYRO_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

int send_gyro_temperature_offset_to_mcu(void)
{
	if (read_calibrate_data_from_nv(GYRO_TEMP_CALI_NV_NUM,
		GYRO_TEMP_CALI_NV_SIZE, "GYTMP"))
		return -1;

	/* copy to gsensor_offset by pass */
	if (memcpy_s(gyro_temperature_offset, sizeof(gyro_temperature_offset),
		user_info.nv_data, sizeof(gyro_temperature_offset)) != EOK)
		return -1;

	if (send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_GYRO_TMP_OFFSET_REQ,
		gyro_temperature_offset, sizeof(gyro_temperature_offset), false))
		return -1;

	if (memcpy_s(&gyro_temperature_calibrate_data,
		sizeof(gyro_temperature_calibrate_data),
		gyro_temperature_offset,
		(sizeof(gyro_temperature_calibrate_data) <
		sizeof(gyro_temperature_offset)) ?
		sizeof(gyro_temperature_calibrate_data) :
		sizeof(gyro_temperature_offset)) != EOK)
		return -1;
	return 0;
}

int write_gyro2_sensor_offset_to_nv(const char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("write_gyro2_sensor_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (write_calibrate_data_to_nv(GYRO2_OFFSET_NV_NUM, length,
		GYRO2_NV_NAME, temp))
		return -1;

	if (memcpy_s(gyro2_sensor_calibrate_data,
		sizeof(gyro2_sensor_calibrate_data), temp,
		(sizeof(gyro2_sensor_calibrate_data) < length) ?
		sizeof(gyro2_sensor_calibrate_data) : length) != EOK)
		return -1;
	if (memcpy_s(gyro2_sensor_offset, sizeof(gyro2_sensor_offset),
		gyro2_sensor_calibrate_data,
		(sizeof(gyro2_sensor_offset) < GYRO2_OFFSET_NV_SIZE) ?
		sizeof(gyro2_sensor_offset) : GYRO2_OFFSET_NV_SIZE) != EOK)
		return -1;
	hwlog_info("gyro2 calibrate data %d %d %d %d %d %d lens=%d\n",
		gyro2_sensor_offset[0], gyro2_sensor_offset[1],
		gyro2_sensor_offset[2], gyro2_sensor_offset[15],
		gyro2_sensor_offset[16], gyro2_sensor_offset[17], length);
	return ret;
}

#define MCU_RESET_IDX  14
#define CONTINUES_I2C_ERROR  0xDEAD
#define I2C_ERROR_RESET_SENSORHUB_TIMES  3
int g_i2c_error_reset_sensorhub_times = 0;
int write_gyro1_sensor_offset_to_nv(const char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("write_gyro1_sensor_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (((int *)temp)[MCU_RESET_IDX] == CONTINUES_I2C_ERROR) {
#ifdef CONFIG_HUAWEI_DSM
		struct dsm_client *client = inputhub_get_shb_dclient();
#endif
		g_i2c_error_reset_sensorhub_times++;
		hwlog_err("i2c continuous read error times:%d\n", g_i2c_error_reset_sensorhub_times);
		if (g_i2c_error_reset_sensorhub_times <= I2C_ERROR_RESET_SENSORHUB_TIMES) {
			hwlog_err("try to reset sensorhub\n");
#ifdef CONFIG_HUAWEI_DSM
			dsm_client_record(client, "dmd_case = 0x%x", CONTINUES_I2C_ERROR);
			dsm_client_notify(client, DSM_SHB_ERR_SENSORSERVICE_DATA_UPDATE);
#endif
			iom3_need_recovery(SENSORHUB_USER_MODID, SH_FAULT_USER_DUMP);
		}
		return 0;
	}

	if (write_calibrate_data_to_nv(GYRO1_OFFSET_NV_NUM, length,
		GYRO1_NV_NAME, temp))
		return -1;

	if (memcpy_s(gyro1_sensor_calibrate_data,
		sizeof(gyro1_sensor_calibrate_data), temp,
		(sizeof(gyro1_sensor_calibrate_data) < length) ?
		sizeof(gyro1_sensor_calibrate_data) : length) != EOK)
		return -1;
	if (memcpy_s(gyro1_sensor_offset, sizeof(gyro1_sensor_offset),
		gyro1_sensor_calibrate_data,
		(sizeof(gyro1_sensor_offset) < GYRO1_OFFSET_NV_SIZE) ?
		sizeof(gyro1_sensor_offset) : GYRO1_OFFSET_NV_SIZE) != EOK)
		return -1;
	hwlog_info("gyro1 calibrate data %d %d %d %d %d %d lens=%d\n",
		gyro1_sensor_offset[0], gyro1_sensor_offset[1],
		gyro1_sensor_offset[2], gyro1_sensor_offset[15],
		gyro1_sensor_offset[16], gyro1_sensor_offset[17], length);
	return ret;
}

int write_gyro_sensor_offset_to_nv(const char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("write_gyro_sensor_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (write_calibrate_data_to_nv(GYRO_CALIDATA_NV_NUM, length, "GYRO", temp))
		return -1;

	if (memcpy_s(gyro_sensor_calibrate_data,
		sizeof(gyro_sensor_calibrate_data), temp,
		(sizeof(gyro_sensor_calibrate_data) < length) ?
		sizeof(gyro_sensor_calibrate_data) : length) != EOK) {
			hwlog_err("cope fail %d size:%d length:%d\n", __LINE__, sizeof(gyro_sensor_calibrate_data), length);
			return -1;
		}
	if (memcpy_s(gyro_sensor_offset, sizeof(gyro_sensor_offset),
		gyro_sensor_calibrate_data,
		(sizeof(gyro_sensor_offset) < GYRO_CALIDATA_NV_SIZE) ?
		sizeof(gyro_sensor_offset) : GYRO_CALIDATA_NV_SIZE) != EOK) {
			hwlog_err("cope fail %d %d\n", __LINE__, GYRO_CALIDATA_NV_SIZE);
			return -1;
		}
	hwlog_info("gyro calibrate data %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d,online offset %d %d %d lens=%d\n",
		gyro_sensor_offset[BIAS_X], gyro_sensor_offset[BIAS_Y],
		gyro_sensor_offset[BIAS_Z], gyro_sensor_offset[SCALE_X],
		gyro_sensor_offset[SCALE_Y], gyro_sensor_offset[SCALE_Z],
		gyro_sensor_offset[MATRIX_XX], gyro_sensor_offset[MATRIX_XY],
		gyro_sensor_offset[MATRIX_XZ], gyro_sensor_offset[MATRIX_YX],
		gyro_sensor_offset[MATRIX_YY], gyro_sensor_offset[MATRIX_YZ],
		gyro_sensor_offset[MATRIX_ZX], gyro_sensor_offset[MATRIX_ZY],
		gyro_sensor_offset[MATRIX_ZZ], gyro_sensor_offset[ONLINE_X],
		gyro_sensor_offset[ONLINE_Y], gyro_sensor_offset[ONLINE_Z], length);

		if (length > GYRO_CALI_TEMP_OFFSET)
			hwlog_info("gyro calibrate data %d %d %d %d %d %d\n", gyro_sensor_offset[OFFSET_SUM_X],
			gyro_sensor_offset[OFFSET_SUM_Y], gyro_sensor_offset[OFFSET_SUM_Z], gyro_sensor_offset[TEMPER_OFFSET_X],
			gyro_sensor_offset[TEMPER_OFFSET_Y], gyro_sensor_offset[TEMPER_OFFSET_Z]);
	return ret;
}

int write_gyro_temperature_offset_to_nv(const char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("write_gyro_temp_offset_to_nv fail, invalid para\n");
		return -1;
	}
	if (write_calibrate_data_to_nv(GYRO_TEMP_CALI_NV_NUM, length,
		"GYTMP", temp))
		return -1;

	if (memcpy_s(gyro_temperature_calibrate_data,
		sizeof(gyro_temperature_calibrate_data), temp,
		(sizeof(gyro_temperature_calibrate_data) < length) ?
		sizeof(gyro_temperature_calibrate_data) : length) != EOK)
		return -1;
	hwlog_info("write_gyro_temp_offset_to_nv suc len=%d\n", length);
	return ret;
}

#define GYRO_2_0_TEMP_DATA_FILE  "/data/log/sensorhub-log/gyro_2_0_temp_data.txt"
static char g_gyro_2_0_temp_file[PATH_MAXLEN] = GYRO_2_0_TEMP_DATA_FILE;
int save_gyro_2_0_temperature_data(const char *temp, int length)
{
	int ret = 0;
	int flags;
	struct file *fp = NULL;
	char *path = g_gyro_2_0_temp_file;

	flags = O_CREAT | O_RDWR | O_APPEND;
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		hwlog_err("%s():create file %s err\n", __func__, path);
		return -1;
	}

	ret = vfs_write(fp, temp, length, &(fp->f_pos));
	if (ret != length) {
		hwlog_err("%s:write file %s exception with ret %d\n",
			__func__, path, ret);
		return -1;
	}
	hwlog_info("write_gyro_2_0_temp_data_to file %s succ\n", path);
	filp_close(fp, NULL);
	return ret;
}

void reset_gyro_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(GYRO)) == 0)
		return;
	send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_SET_OFFSET_REQ,
		gyro_sensor_calibrate_data, GYRO_CALIDATA_NV_SIZE, true);
	send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_GYRO_TMP_OFFSET_REQ,
		gyro_temperature_calibrate_data, GYRO_TEMP_CALI_NV_SIZE,
		true);
}

void reset_gyro1_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(GYRO1)) == 0)
		return;
	send_calibrate_data_to_mcu(TAG_GYRO1, SUB_CMD_SET_OFFSET_REQ,
		gyro1_sensor_calibrate_data, GYRO1_OFFSET_NV_SIZE, true);
}

void reset_gyro2_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(GYRO2)) == 0)
		return;
	send_calibrate_data_to_mcu(TAG_GYRO2, SUB_CMD_SET_OFFSET_REQ,
		gyro2_sensor_calibrate_data, GYRO2_OFFSET_NV_SIZE, true);
}
