/* SPDX-License-Identifier: GPL-2.0 */
/* hc32l110_fw.h
 *
 * hc32l110 firmware header file
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

#ifndef _HC32L110_FW_H_
#define _HC32L110_FW_H_

#define HC32L110_FW_PAGE_SIZE               128
#define HC32L110_FW_CMD_SIZE                2
#define HC32L110_FW_SPECIAL_ERASE_SIZE      3
#define HC32L110_FW_ERASE_CMD1_SIZE         3
#define HC32L110_FW_ERASE_CMD2_SIZE         63
#define HC32L110_FW_ADDR_SIZE               5
#define HC32L110_FW_READ_OPTOPN_SIZE        2
#define HC32L110_FW_ACK_COUNT               5
#define HC32L110_FW_ERASE_ACK_COUNT         8
#define HC32L110_FW_RETRY_COUNT             3
#define HC32L110_FW_GPIO_HIGH               1
#define HC32L110_FW_GPIO_LOW                0

/* cmd */
#define HC32L110_FW_BOOTN_ADDR              0x1FFF7803
#define HC32L110_FW_WRITE_ADDR              0x00001400
#define HC32L110_FW_VERSION_ID_ADDR         0x00007FF0
#define HC32L110_FW_GET_VER_CMD             0x01FE
#define HC32L110_FW_WRITE_CMD               0x32CD
#define HC32L110_FW_ERASE_CMD               0x45BA
#define HC32L110_FW_GO_CMD                  0x21DE
#define HC32L110_FW_READ_UNPROTECT_CMD      0x936C
#define HC32L110_FW_READ_CMD                0x11EE
#define HC32L110_FW_ACK_VAL                 0x79
#define HC32L110_FW_NBOOT_VAL               0xFE
#define HC32L110_FW_MATCHID_CMD             0x01FE
#define HC32L110_ERASE_PAGE_NUM             53
#define HC32L110_ERASE_PAGE_START           10

int hc32l110_fw_check_bootloader_mode(struct cam_star_module_device_info *di);
int hc32l110_fw_update_check(struct cam_star_module_device_info *di);
int hc32l110_fw_write_cmd(struct cam_star_module_device_info *di, u16 cmd);
int hc32l110_fw_write_addr(struct cam_star_module_device_info *di, u32 addr);
int hc32l110_fw_program_end(struct cam_star_module_device_info *di);

#endif /* _HC32L110_FW_H_ */
