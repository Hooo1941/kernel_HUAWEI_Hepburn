// SPDX-License-Identifier: GPL-2.0
/*
 * cam_sensormotor_dmd_report.c
 *
 * i2c interface for cam_sensormotor module
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include "cam_sensormotor_dmd_report.h"
#include "cam_star_module_adapter.h"
#include <linux/delay.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG cam_sensormotor_dmd_report
HWLOG_REGIST();


void cam_sensormotor_dsm_i2c_err(struct dsm_client *client)
{
	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client,
			"cam_star i2c transfer fail\n");
		dsm_client_notify(client, DSM_CAM_STAR_I2C_ERROR_NO);
		hwlog_err("[I/DSM] %s cam_star i2c fail", __func__);
	}
}

void cam_sensormotor_dsm_set_bootloader_err(struct dsm_client *client)
{
	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client,
			"cam_star set_bootloader fail\n");
		dsm_client_notify(client, DSM_CAM_STAR_SET_BOOTLOADER_ERROR_NO);
		hwlog_err("[I/DSM] %s cam_star set_bootloader fail", __func__);
	} else {
		hwlog_err("[E/DSM] %s", __func__);
	}
}

void cam_sensormotor_dsm_download_fw_err(struct dsm_client *client)
{
	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client,
			"cam_star download_fw fail\n");
		dsm_client_notify(client, DSM_CAM_STAR_DOWNLOAD_FW_ERROR_NO);
		hwlog_err("[I/DSM] %s cam_star download_fw fail", __func__);
	} else {
		hwlog_err("[E/DSM] %s", __func__);
	}
}

void cam_sensormotor_dsm_flash_crc_check_err(struct dsm_client *client)
{
	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client,
			"cam_star flash_crc_check fail\n");
		dsm_client_notify(client, DSM_CAM_STAR_FLASH_CRC_CHECK_ERROR_NO);
		hwlog_err("[I/DSM] %s cam_star flash_crc_check fail", __func__);
	} else {
		hwlog_err("[E/DSM] %s", __func__);
	}
}