/*
 * spi_slave_commu.h
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

#ifndef SPI_SLAVE_COMMU_H
#define SPI_SLAVE_COMMU_H
#include "ext_sensorhub_commu.h"

enum spi_slave_opt {
	SLAVE_WRITE,
	SLAVE_READ,
};

void commu_spi_slave_ready(enum spi_slave_opt opt);
int spi_slave_ext_sensorhub_reset(void);
#endif
