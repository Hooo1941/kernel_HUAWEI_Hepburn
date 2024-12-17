/*
 * word_eng_res.c
 *
 * Manager of english word resoure.
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

#include "word_eng_res.h"
#include "securec.h"

typedef void (*word_data_func)(unsigned char *data_buf);

static void get_word_eng_00_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng00_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_01_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng01_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_02_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng02_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_03_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng03_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_04_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng04_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_05_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng05_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_06_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng06_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_07_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng07_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_08_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng08_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_09_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng09_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_10_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng10_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_11_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng11_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_12_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng12_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_13_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng13_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_14_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng14_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_15_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng15_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_16_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng16_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_17_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng17_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_18_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng18_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_19_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng19_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_20_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng20_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_21_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng21_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_22_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng22_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_23_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng23_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_24_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng24_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_25_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng25_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_26_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng26_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_27_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng27_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_28_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng28_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_29_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng29_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_30_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng30_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_31_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng31_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static void get_word_eng_32_data(unsigned char *data_buf)
{
	unsigned char word_eng_BGRA8888[] = {
		#include "word_eng32_406x1_BGRA8888.dat"
	};

	unsigned char *data = &word_eng_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_ENG_DATA_SIZE, data, WORD_ENG_DATA_SIZE);
}

static word_data_func g_word_data_func_list[] = {
	get_word_eng_00_data,
	get_word_eng_01_data,
	get_word_eng_02_data,
	get_word_eng_03_data,
	get_word_eng_04_data,
	get_word_eng_05_data,
	get_word_eng_06_data,
	get_word_eng_07_data,
	get_word_eng_08_data,
	get_word_eng_09_data,
	get_word_eng_10_data,
	get_word_eng_11_data,
	get_word_eng_12_data,
	get_word_eng_13_data,
	get_word_eng_14_data,
	get_word_eng_15_data,
	get_word_eng_16_data,
	get_word_eng_17_data,
	get_word_eng_18_data,
	get_word_eng_19_data,
	get_word_eng_20_data,
	get_word_eng_21_data,
	get_word_eng_22_data,
	get_word_eng_23_data,
	get_word_eng_24_data,
	get_word_eng_25_data,
	get_word_eng_26_data,
	get_word_eng_27_data,
	get_word_eng_28_data,
	get_word_eng_29_data,
	get_word_eng_30_data,
	get_word_eng_31_data,
	get_word_eng_32_data,
};

void get_word_eng_data(int line_num, unsigned char *data_buf)
{
	if (line_num >= WORD_ENG_LOGO_H) {
		return;
	}
	g_word_data_func_list[line_num](data_buf);
	return;
}
