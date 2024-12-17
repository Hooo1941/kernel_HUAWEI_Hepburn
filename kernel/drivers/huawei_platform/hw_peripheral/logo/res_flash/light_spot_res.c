/*
 * light_spot_res.c
 *
 * Manager of light spot flash resource.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

#include "light_spot_res.h"
#include "securec.h"

typedef void (*flash_data_func)(unsigned char *data_buf);

static void get_flash_00_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_00_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_01_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_01_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_02_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_02_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_03_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_03_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_04_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_04_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_05_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_05_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_06_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_06_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_07_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_07_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_08_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_08_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_09_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_09_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_10_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_10_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_11_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_11_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_12_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_12_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_13_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_13_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_14_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_14_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_15_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_15_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_16_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_16_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_17_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_17_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_18_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_18_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_19_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_19_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_20_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_20_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static void get_flash_21_circle_data(unsigned char *data_buf)
{
	unsigned char flash_BGRA8888[] = {
		#include "flash_21_30x30_BGRA8888.dat"
	};

	unsigned char *data = &flash_BGRA8888[0];
	(void)memcpy_s(data_buf, FLASH_DATA_SIZE, data, FLASH_DATA_SIZE);
}

static flash_data_func g_flash_data_func_list[] = {
	get_flash_00_circle_data,
	get_flash_01_circle_data,
	get_flash_02_circle_data,
	get_flash_03_circle_data,
	get_flash_04_circle_data,
	get_flash_05_circle_data,
	get_flash_06_circle_data,
	get_flash_07_circle_data,
	get_flash_08_circle_data,
	get_flash_09_circle_data,
	get_flash_10_circle_data,
	get_flash_11_circle_data,
	get_flash_12_circle_data,
	get_flash_13_circle_data,
	get_flash_14_circle_data,
	get_flash_15_circle_data,
	get_flash_16_circle_data,
	get_flash_17_circle_data,
	get_flash_18_circle_data,
	get_flash_19_circle_data,	
	get_flash_20_circle_data,
	get_flash_21_circle_data,
};

void get_flash_circle_data(int index, unsigned char *data_buf)
{
	if (index >= FLASH_FRAME_NUM) {
		return;
	}
	g_flash_data_func_list[index](data_buf);
	return;
}
