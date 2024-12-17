/*
 * spi_commu.h
 *
 * head file for communication with external sensorhub
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef SPI_COMMU_H
#define SPI_COMMU_H
#include "ext_sensorhub_commu.h"

int ext_sensorhub_spi_direct_write(u8 *buf, u32 len, u32 tx_len, u8 *tx_buf);

/* mcu force upgrade */
int ext_sensorhub_set_force_upgrade_mode(int mode);

int get_spi_commu_mode(void);

int ext_sensorhub_mcu_boot(int value);

int ext_sensorhub_mcu_reset(int value);

int ext_sensorhub_reset(void);

void set_spi_read_len(u32 len);

void spi_suspend_clear_last_write(void);

#endif
