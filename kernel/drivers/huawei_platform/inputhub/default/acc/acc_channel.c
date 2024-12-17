/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc channel source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "acc_channel.h"

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

#include "acc_sysfs.h"
#include "contexthub_route.h"

/* write gsensor1_offset calibrate value to nv */
int write_gsensor1_offset_to_nv(const char *temp, int length)
{
	if (!temp) {
		hwlog_err("acc1 offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (write_calibrate_data_to_nv(ACC1_OFFSET_NV_NUM,
		length, ACC1_NV_NAME, temp))
		return -1;
	if (memcpy_s((void *)gsensor1_calibrate_data,
		sizeof(gsensor1_calibrate_data),
		(void *)temp, ACC1_CALIBRATE_DATA_LENGTH) != EOK)
		return -1;
	if (memcpy_s(gsensor1_offset, sizeof(gsensor1_offset),
		gsensor1_calibrate_data,
		(sizeof(gsensor1_offset) < ACC1_OFFSET_NV_SIZE) ?
		sizeof(gsensor1_offset) : ACC1_OFFSET_NV_SIZE) != EOK)
		return -1;
	/* acc1 temp value */
	hwlog_info("nve_direct_access write acc1 temp %d %d %d\n",
		gsensor1_offset[0], gsensor1_offset[1], gsensor1_offset[2]);
	return 0;
}

/* write gsensor2_offset calibrate value to nv */
int write_gsensor2_offset_to_nv(const char *temp, int length)
{
	if (!temp) {
		hwlog_err("acc2 offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (write_calibrate_data_to_nv(ACC2_OFFSET_NV_NUM,
		length, ACC2_NV_NAME, temp))
		return -1;
	if (memcpy_s((void *)gsensor2_calibrate_data,
		sizeof(gsensor2_calibrate_data),
		(void *)temp, ACC2_CALIBRATE_DATA_LENGTH) != EOK)
		return -1;
	if (memcpy_s(gsensor2_offset, sizeof(gsensor2_offset),
		gsensor2_calibrate_data,
		(sizeof(gsensor2_offset) < ACC2_OFFSET_NV_SIZE) ?
		sizeof(gsensor2_offset) : ACC2_OFFSET_NV_SIZE) != EOK)
		return -1;
	/* acc2 temp value */
	hwlog_info("nve_direct_access write acc2 temp %d %d %d\n",
		gsensor2_offset[0], gsensor2_offset[1], gsensor2_offset[2]);
	return 0;
}

/* write gsensor_offset calibrate value to nv */
int write_gsensor_offset_to_nv(const char *temp, int length)
{
	if (!temp) {
		hwlog_err("write_gsensor_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (write_calibrate_data_to_nv(ACC_OFFSET_NV_NUM,
		length, "gsensor", temp))
		return -1;
	if (memcpy_s((void *)gsensor_calibrate_data,
		sizeof(gsensor_calibrate_data),
		(void *)temp, MAX_SENSOR_CALIBRATE_DATA_LENGTH) != EOK)
		return -1;
	if (memcpy_s(gsensor_offset, sizeof(gsensor_offset),
		gsensor_calibrate_data,
		(sizeof(gsensor_offset) < ACC_OFFSET_NV_SIZE) ?
		sizeof(gsensor_offset) : ACC_OFFSET_NV_SIZE) != EOK)
		return -1;
	/* output the temp values */
	hwlog_info("nve_direct_access write temp %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		gsensor_offset[0], gsensor_offset[1], gsensor_offset[2],
		gsensor_offset[3], gsensor_offset[4], gsensor_offset[5],
		gsensor_offset[6], gsensor_offset[7], gsensor_offset[8],
		gsensor_offset[9], gsensor_offset[10], gsensor_offset[11],
		gsensor_offset[12], gsensor_offset[13], gsensor_offset[14]);
	return 0;
}

int send_gsensor2_calibrate_data_to_mcu(void)
{
	struct acc_device_info *dev_info = NULL;
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();
#endif

	dev_info = acc_get_device_info(TAG_ACC2);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(ACC2_OFFSET_NV_NUM,
		ACC2_OFFSET_NV_SIZE, ACC2_NV_NAME))
		return -1;

	dev_info->acc_first_start_flag = 1;
	/* copy to assistant_acc_offset by pass */
	if (memcpy_s(gsensor2_offset, sizeof(gsensor2_offset),
		user_info.nv_data, sizeof(gsensor2_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read acc2 offset %d %d %d\n",
		gsensor2_offset[0], gsensor2_offset[1], gsensor2_offset[2]);
#ifdef CONFIG_HUAWEI_DSM
	if (gsensor2_offset[0] == 0 && gsensor2_offset[1] == 0 && gsensor2_offset[2] == 0) {
		dsm_client_record(client, "dmd_case = %d", TAG_ACC2);
		dsm_client_notify(client, DSM_SHB_ERR_GSENSOR_DATA_ALL_ZERO);
		hwlog_info("set acc2 offset data all zero\n");
	}
#endif
	if (memcpy_s(gsensor2_calibrate_data, sizeof(gsensor2_calibrate_data),
		gsensor2_offset,
		(sizeof(gsensor2_calibrate_data) < ACC2_OFFSET_NV_SIZE) ?
		sizeof(gsensor2_calibrate_data) : ACC2_OFFSET_NV_SIZE) != EOK)
		return -1;

	if (send_calibrate_data_to_mcu(TAG_ACC2, SUB_CMD_SET_OFFSET_REQ,
		gsensor2_offset, ACC2_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}

int send_gsensor1_calibrate_data_to_mcu(void)
{
	struct acc_device_info *dev_info = NULL;
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();
#endif

	dev_info = acc_get_device_info(TAG_ACC1);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(ACC1_OFFSET_NV_NUM,
		ACC1_OFFSET_NV_SIZE, ACC1_NV_NAME))
		return -1;

	dev_info->acc_first_start_flag = 1;
	/* copy to assistant_acc_offset by pass */
	if (memcpy_s(gsensor1_offset, sizeof(gsensor1_offset),
		user_info.nv_data, sizeof(gsensor1_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read acc1 offset %d %d %d\n",
		gsensor1_offset[0], gsensor1_offset[1], gsensor1_offset[2]);
#ifdef CONFIG_HUAWEI_DSM
	if (gsensor1_offset[0] == 0 && gsensor1_offset[1] == 0 && gsensor1_offset[2] == 0) {
		dsm_client_record(client, "dmd_case = %d", TAG_ACC1);
		dsm_client_notify(client, DSM_SHB_ERR_GSENSOR_DATA_ALL_ZERO);
		hwlog_info("set acc1 offset data all zero\n");
	}
#endif
	if (memcpy_s(gsensor1_calibrate_data, sizeof(gsensor1_calibrate_data),
		gsensor1_offset,
		(sizeof(gsensor1_calibrate_data) < ACC1_OFFSET_NV_SIZE) ?
		sizeof(gsensor1_calibrate_data) : ACC1_OFFSET_NV_SIZE) != EOK)
		return -1;

	if (send_calibrate_data_to_mcu(TAG_ACC1, SUB_CMD_SET_OFFSET_REQ,
		gsensor1_offset, ACC1_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}

/* read gsensor calibrate data from nv, and send to mcu */
int send_gsensor_calibrate_data_to_mcu(void)
{
	struct acc_device_info *dev_info = NULL;
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();
#endif

	dev_info = acc_get_device_info(TAG_ACCEL);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(ACC_OFFSET_NV_NUM, ACC_OFFSET_NV_SIZE,
		"gsensor"))
		return -1;

	dev_info->acc_first_start_flag = 1;
	/* copy to gsensor_offset by pass */
	if (memcpy_s(gsensor_offset, sizeof(gsensor_offset),
		user_info.nv_data, sizeof(gsensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gsensor_offset_sen %d %d %d %d %d %d\n",
		gsensor_offset[0], gsensor_offset[1], gsensor_offset[2],
		gsensor_offset[3], gsensor_offset[4], gsensor_offset[5]);
	hwlog_info("nve_direct_access read gsensor_xis_angle %d %d %d %d %d %d %d %d %d\n",
		gsensor_offset[6], gsensor_offset[7], gsensor_offset[8],
		gsensor_offset[9], gsensor_offset[10], gsensor_offset[11],
		gsensor_offset[12], gsensor_offset[13], gsensor_offset[14]);
#ifdef CONFIG_HUAWEI_DSM
	if (gsensor_offset[0] == 0 && gsensor_offset[1] == 0 && gsensor_offset[2] == 0) {
		dsm_client_record(client, "dmd_case = %d", TAG_ACCEL);
		dsm_client_notify(client, DSM_SHB_ERR_GSENSOR_DATA_ALL_ZERO);
		hwlog_info("set acc offset data all zero\n");
	}
#endif
	if (memcpy_s(gsensor_calibrate_data, sizeof(gsensor_calibrate_data),
		gsensor_offset,
		(sizeof(gsensor_calibrate_data) < ACC_OFFSET_NV_SIZE) ?
		sizeof(gsensor_calibrate_data) : ACC_OFFSET_NV_SIZE) != EOK)
		return -1;

	if (send_calibrate_data_to_mcu(TAG_ACCEL, SUB_CMD_SET_OFFSET_REQ,
		gsensor_offset, ACC_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}

void reset_acc_calibrate_data(void)
{
	send_calibrate_data_to_mcu(TAG_ACCEL, SUB_CMD_SET_OFFSET_REQ,
		gsensor_calibrate_data, ACC_OFFSET_NV_SIZE, true);
}

void reset_acc1_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(ACC1)))
		send_calibrate_data_to_mcu(TAG_ACC1, SUB_CMD_SET_OFFSET_REQ,
			gsensor1_calibrate_data, ACC1_OFFSET_NV_SIZE, true);
}

void reset_acc2_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(ACC2)))
		send_calibrate_data_to_mcu(TAG_ACC2, SUB_CMD_SET_OFFSET_REQ,
			gsensor2_calibrate_data, ACC2_OFFSET_NV_SIZE, true);
}

