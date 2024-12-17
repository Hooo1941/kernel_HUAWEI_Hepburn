/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hc32l110_fw.h
 *
 * hc32l110 firmware header file
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

#ifndef _MCU_HC32L110_FW_H_
#define _MCU_HC32L110_FW_H_

#define MCU_HC32L110_FW_DEV_ID_REG				0x00
#define MCU_HC32L110_FW_VER_ID_REG				0x01
#define MCU_HC32L110_FW_BOOT_VER_ID_REG			0x02

#define MCU_HC32L110_TIME_SEC					0x03
#define MCU_HC32L110_TIME_MIN					0x04
#define MCU_HC32L110_TIME_HOUR					0x05
#define MCU_HC32L110_TIME_WDAY					0x06
#define MCU_HC32L110_TIME_MDAY					0x07
#define MCU_HC32L110_TIME_MON					0x08
#define MCU_HC32L110_TIME_YEAR					0x09

#define MCU_HC32L110_FW_VER_UPDATE_REG			0x0E
#define MCU_HC32L110_FW_ENTER_BOOT_REG			0x0F

#define MCU_HC32L110_FW_PAGE_SIZE				128
#define MCU_HC32L110_FW_CMD_SIZE				2
#define MCU_HC32L110_FW_ERASE_SIZE				3
#define MCU_HC32L110_FW_ADDR_SIZE				5
#define MCU_HC32L110_FW_ACK_COUNT				10
#define MCU_HC32L110_FW_RETRY_COUNT				3
#define MCU_HC32L110_FW_ERASE_ACK_COUNT			5

/* cmd */
#define MCU_HC32L110_FW_MTP_ADDR				0x00002800
#define MCU_HC32L110_FW_GET_VER_CMD				0x01FE
#define MCU_HC32L110_FW_WRITE_CMD				0x32CD
#define MCU_HC32L110_FW_ERASE_CMD				0x45BA
#define MCU_HC32L110_FW_GO_CMD					0x21DE
#define MCU_HC32L110_FW_ACK_VAL					0x79

void mcu_hc32l110_fw_reset_mcu(struct mcu_hc32l110_device_info *di);
int mcu_hc32l110_fw_get_dev_id(struct mcu_hc32l110_device_info *di);
int mcu_hc32l110_fw_get_ver_id(struct mcu_hc32l110_device_info *di);
int mcu_hc32l110_fw_get_boot_ver_id(struct mcu_hc32l110_device_info *di);
int mcu_hc32l110_fw_update_empty_mtp(struct mcu_hc32l110_device_info *di);
int mcu_hc32l110_fw_update_latest_mtp(struct mcu_hc32l110_device_info *di);
int mcu_hc32l110_fw_update_online_mtp(struct mcu_hc32l110_device_info *di,
	u8 *mtp_data, int mtp_size, int ver_id);
int mcu_hc32l110_fw_check_bootloader_mode(struct mcu_hc32l110_device_info *di);
void mcu_hc32l110_update_check(struct mcu_hc32l110_device_info *di);
#endif /* _MCU_HC32L110_FW_H_ */
