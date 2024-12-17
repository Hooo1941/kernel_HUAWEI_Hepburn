/* SPDX-License-Identifier: GPL-2.0 */
/*
 * stm32g031_fw.h
 *
 * stm32g031 firmware header file
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

#ifndef _STM32G031_FW_H_
#define _STM32G031_FW_H_

#define STM32G031_FW_PAGE_SIZE               128
#define STM32G031_FW_CMD_SIZE                2
#define STM32G031_FW_SPECIAL_ERASE_SIZE      3
#define STM32G031_FW_ERASE_CMD1_SIZE         3
#define STM32G031_FW_ERASE_CMD2_SIZE         63
#define STM32G031_FW_ADDR_SIZE               5
#define STM32G031_FW_READ_OPTOPN_SIZE        2
#define STM32G031_FW_ACK_COUNT               5
#define STM32G031_FW_ERASE_ACK_COUNT         8
#define STM32G031_FW_RETRY_COUNT             3
#define STM32G031_FW_GPIO_HIGH               1
#define STM32G031_FW_GPIO_LOW                0

/* cmd */
#define STM32G031_FW_OPTION_ADDR             0x1FFF7800
#define STM32G031_FW_BOOTN_ADDR              0x1FFF7803
#define STM32G031_FW_WRITE_ADDR              0x08000000
#define STM32G031_FW_VERSION_ID_ADDR         0x0800FFF0
#define STM32G031_FW_GET_VER_CMD             0x01FE
#define STM32G031_FW_WRITE_CMD               0x32CD
#define STM32G031_FW_ERASE_CMD               0x45BA
#define STM32G031_FW_GO_CMD                  0x21DE
#define STM32G031_FW_READ_UNPROTECT_CMD      0x936C
#define STM32G031_FW_READ_CMD                0x11EE
#define STM32G031_FW_ACK_VAL                 0x79
#define STM32G031_FW_NBOOT_VAL               0xFE
#define STM32G031_FW_MATCHID_CMD             0x02FD

static const u8 g_stm32g031_option_data[] = {
	0xAA, 0xFE, 0xFF, 0xFE,
};
#define STM32G031_OPTION_SIZE                ARRAY_SIZE(g_stm32g031_option_data)

int stm32g031_fw_check_bootloader_mode(struct cam_star_module_device_info *di);
int stm32g031_fw_update_check(struct cam_star_module_device_info *di);
int stm32g031_fw_write_cmd(struct cam_star_module_device_info *di, u16 cmd);

#endif /* _STM32G031_FW_H_ */
