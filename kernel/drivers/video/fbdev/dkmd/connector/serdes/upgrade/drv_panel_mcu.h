/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
*/

#ifndef DRV_PANEL_MCU_H
#define DRV_PANEL_MCU_H
#include <linux/types.h>

#define RET_SUCCESS (0)
#define RET_FAIL (-1)
#define RET_WAIT (-2)

#define SHIFT_THIRTYTWO (32)
#define SHIFT_TWENTYFOUR (24)
#define SHIFT_SIXTEEN (16)
#define SHIFT_EIGHT (8)

#define RETURN_ERR_SID 0x7F
#define RETURN_WAIT_SID 0x78
#define RETURN_TIMEOUT_SID 0xFF

#define GENERAL_REGISTER 0x31
#define REG_CHECK_STATUS 0x07

#define SDID_GETVERSION 0x22
#define DID_BOOTVERSION 0xF180
#define DID_APPVERSION 0x0216

#define SDID_EXTENDED 0x10
#define DID_EXTENDED 0x03
#define DID_PROGESS 0x02

#define SDID_GETSLOT 0x22
#define DID_GETSLOT 0xF100

#define SDID_AUTHENTICATION 0x27
#define DID_AUTHN_SEED 0x09
#define DID_AUTHN_KEY 0x0A

#define SDID_REASE 0x31
#define DID_REASE 0x01FF0044

#define SDID_DOWNLOAD 0x34
#define DID_DOWNLOAD 0x0044

#define TRANSFER_TYPE 0x40
#define SDID_TRANSFER 0x36

#define SDID_EXIT 0x37

#define SDID_CRC 0x31
#define DID_CRC 0x010202

#define SDID_SETSLOT 0x31
#define DID_SETSLOT 0x010207

#define SDID_MCURESET 0x11
#define DID_MCURESET 0x61

#define MCU_GET_BOOTVERSION	((unsigned long)0x03 << 24 | (unsigned long)SDID_GETVERSION << 16 | \
	(unsigned long)DID_BOOTVERSION)
#define MCU_GET_APPVERSION ((unsigned long)0x03 << 24 | (unsigned long)SDID_GETVERSION << 16 | \
	(unsigned long)DID_APPVERSION)
#define MCU_EXTENDED_SESSION ((unsigned long)0x02 << 16 | (unsigned long)SDID_EXTENDED << 8 | \
	(unsigned long)DID_EXTENDED)
#define MCU_PROGRAM_SESSION ((unsigned long)0x02 << 16 | (unsigned long)SDID_EXTENDED << 8 | \
	(unsigned long)DID_PROGESS)
#define MCU_GET_SLOT ((unsigned long)0x03 << 24 | (unsigned long)SDID_GETSLOT << 16 | \
	(unsigned long)DID_GETSLOT)
#define MCU_GET_SEED ((unsigned long)0x02 << 16 | (unsigned long)SDID_AUTHENTICATION << 8 | \
	(unsigned long)DID_AUTHN_SEED)
#define MCU_SEND_KEY ((unsigned long)0x06 << 16 | (unsigned long)SDID_AUTHENTICATION << 8 | \
	(unsigned long)DID_AUTHN_KEY)
#define MCU_REASE ((unsigned long)0x0d << 40 | (unsigned long)SDID_REASE << 32 | DID_REASE)
#define MCU_DOWNLOAD ((unsigned long)0x0b << 24 | (unsigned long)SDID_DOWNLOAD << 16 | DID_DOWNLOAD)
#define MCU_EXIT_TRANSFER ((unsigned long)0x01 << 8 | (unsigned long)SDID_EXIT)
#define MCU_CRC ((unsigned long)0x08 << 32 | (unsigned long)SDID_CRC << 24 | (unsigned long)DID_CRC)
#define MCU_SET_SLOT ((unsigned long)0x05 << 32 | (unsigned long)SDID_SETSLOT << 24 | \
	(unsigned long)DID_SETSLOT)
#define MCU_RESET ((unsigned long)0x02 << 16 | (unsigned long)SDID_MCURESET << 8 | \
	(unsigned long)DID_MCURESET)


#define MCU_GET_VERSION_LENGTH 4
#define MCU_GET_BOOTVERSION_READ 24
#define MCU_GET_VERSION_READ 12
#define BOOTVERSION_LENGTH 20
#define APPVERSION_LENGTH 8

#define MCU_START_UPDATE_LENGTH 3
#define MCU_START_UPDATE_READ 7

#define MCU_GET_SLOT_LENGTH 4
#define MCU_GET_SLOT_READ 5

#define MCU_GET_SEED_LENGTH 3
#define MCU_GET_SEED_READ 7

#define MCU_SEND_KEY_LENGTH 7
#define MCU_SEND_KEY_READ 4

#define MCU_REASE_LENGTH 14
#define MCU_REASE_READ 6

#define MCU_DOWNLOAD_LENGTH 12
#define MCU_DOWNLOAD_READ 5

#define MCU_TRANSFER_LENGTH 4101
#define MCU_TRANSFER_READ 4

#define MCU_EXIT_LENGTH 2
#define MCU_EXIT_READ 4

#define MCU_CRC_LENGTH 9
#define MCU_CRC_READ 6

#define MCU_SET_SLOT_LENGTH 6
#define MCU_SET_SLOT_READ 6

#define MCU_RESET_LENGTH 3
#define MCU_RESET_READ 4

#define MCU_CHECK_STATUS_WRITE 1
#define MCU_CHECK_STATUS_READ 1

#define LENGTH_ERR_FRAME 3

typedef enum {
	I2C_CTR_BOOTVER = 1,
	I2C_CTR_APPVER,
	I2C_CTR_HARDWAREVER,
	I2C_CTR_EXTENDED,
	I2C_CTR_PROGRAM,
	I2C_CTR_GETSLOT,
	I2C_CTR_SEED,
	I2C_CTR_KEY,
	I2C_CTR_ADDLEN,
	I2C_CTR_DOWNLOAD,
	I2C_CTR_DATA,
	I2C_CTR_EXIT,
	I2C_CTR_CRC,
	I2C_CTR_SETSLOT,
	I2C_CTR_RESET,
}i2cbootwritedatatype;

int32_t drv_panel_mcu_get_ota_version(char *boot_ver, char *app_ver, uint32_t *status);
int32_t drv_panel_mcu_extended_session(uint32_t *status);
int32_t drv_panel_mcu_program_session(uint32_t *status);
int32_t drv_panel_mcu_get_slot(uint32_t *status);
int32_t drv_panel_mcu_get_seed(uint32_t *seed, uint32_t *status);
int32_t drv_panel_mcu_send_key(uint32_t key, uint32_t *status);
int32_t drv_panel_mcu_erase_flash(uint32_t address, uint32_t len, uint32_t *status);
int32_t drv_panel_mcu_request_download(uint32_t address, uint32_t len, uint32_t *status);
int32_t drv_panel_mcu_ota_update(const uint8_t *version_bin, uint32_t len, uint32_t *status);
int32_t drv_panel_mcu_exit_transfer(uint32_t *status);
int32_t drv_panel_mcu_crc_checksum(uint32_t crc_val, uint32_t *status);
int32_t drv_panel_mcu_set_slot(uint32_t partition, uint32_t *status);
int32_t drv_panel_mcu_get_process(void);
int32_t drv_panel_mcu_reset(uint32_t *status);
int32_t drv_panel_mcu_check_status(uint32_t *status);

#endif /* DRV_PANEL_MCU_H */
