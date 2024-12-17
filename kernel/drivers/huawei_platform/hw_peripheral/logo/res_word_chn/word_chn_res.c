/*
 * word_chn_res.c
 *
 * Manager of chinese word resoure.
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

#include "word_chn_res.h"
#include "securec.h"

typedef void (*word_data_func)(unsigned char *data_buf);

static void get_word_chn_00_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn00_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_01_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn01_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_02_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn02_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_03_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn03_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_04_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn04_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_05_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn05_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_06_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn06_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_07_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn07_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_08_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn08_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_09_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn09_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_10_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn10_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_11_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn11_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_12_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn12_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_13_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn13_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_14_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn14_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_15_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn15_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_16_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn16_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_17_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn17_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_18_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn18_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_19_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn19_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_20_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn20_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_21_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn21_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_22_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn22_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_23_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn23_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_24_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn24_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_25_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn25_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_26_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn26_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_27_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn27_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_28_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn28_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_29_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn29_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static void get_word_chn_30_data(unsigned char *data_buf)
{
	unsigned char word_chn_BGRA8888[] = {
		#include "word_chn30_261x1_BGRA8888.dat"
	};

	unsigned char *data = &word_chn_BGRA8888[0];
	(void)memcpy_s(data_buf, WORD_CHN_DATA_SIZE, data, WORD_CHN_DATA_SIZE);
}

static word_data_func g_word_data_func_list[] = {
	get_word_chn_00_data,
	get_word_chn_01_data,
	get_word_chn_02_data,
	get_word_chn_03_data,
	get_word_chn_04_data,
	get_word_chn_05_data,
	get_word_chn_06_data,
	get_word_chn_07_data,
	get_word_chn_08_data,
	get_word_chn_09_data,
	get_word_chn_10_data,
	get_word_chn_11_data,
	get_word_chn_12_data,
	get_word_chn_13_data,
	get_word_chn_14_data,
	get_word_chn_15_data,
	get_word_chn_16_data,
	get_word_chn_17_data,
	get_word_chn_18_data,
	get_word_chn_19_data,
	get_word_chn_20_data,
	get_word_chn_21_data,
	get_word_chn_22_data,
	get_word_chn_23_data,
	get_word_chn_24_data,
	get_word_chn_25_data,
	get_word_chn_26_data,
	get_word_chn_27_data,
	get_word_chn_28_data,
	get_word_chn_29_data,
	get_word_chn_30_data,
};

void get_word_chn_data(int line_num, unsigned char *data_buf)
{
	if (line_num >= WORD_CHN_LOGO_H) {
		return;
	}
	g_word_data_func_list[line_num](data_buf);
	return;
}
