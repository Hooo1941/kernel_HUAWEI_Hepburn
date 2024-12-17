/* SPDX-License-Identifier: GPL-2.0 */
/* cam_sensormotor_dmd_report.h
 *
 * dmd report
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

#ifndef _CAM_SENSORMOTOR_DMD_REPORT_H_
#define _CAM_SENSORMOTOR_DMD_REPORT_H_

#include <dsm/dsm_pub.h>

#define DSM_CAM_STAR_I2C_ERROR_NO              927006051
#define DSM_CAM_STAR_SET_BOOTLOADER_ERROR_NO   927006052
#define DSM_CAM_STAR_DOWNLOAD_FW_ERROR_NO      927006053
#define DSM_CAM_STAR_FLASH_CRC_CHECK_ERROR_NO  927006060

void cam_sensormotor_dsm_i2c_err(struct dsm_client *client);
void cam_sensormotor_dsm_set_bootloader_err(struct dsm_client *client);
void cam_sensormotor_dsm_download_fw_err(struct dsm_client *client);
void cam_sensormotor_dsm_flash_crc_check_err(struct dsm_client *client);

#endif /* _CAM_SENSORMOTOR_DMD_REPORT_H_ */
