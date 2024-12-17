// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * t91407_swi.c
 *
 * t91407_swi driver
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

#include "t91407_swi.h"
#include <securec.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <huawei_platform/log/hw_log.h>
#include "t91407_api.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG t91407_swi
HWLOG_REGIST();

#ifndef us2cycles
#define us2cycles(x)  (((((x) * 0x10c7UL) * loops_per_jiffy * HZ) + \
	(1UL << 31)) >> 32)
#endif

void t91407_udelay(uint32_t us_delay)
{
	cycles_t start = get_cycles();
	cycles_t cnt_gap = 0;
	cycles_t cnt_delay = us2cycles(us_delay);

	while (cnt_gap < cnt_delay)
		cnt_gap = (get_cycles() - start);
}

void t91407_gpio_dir_input(int onewire_gpio)
{
	gpio_direction_input(onewire_gpio);
}

void t91407_gpio_dir_output(int onewire_gpio, uint8_t value)
{
	gpio_direction_output(onewire_gpio, value);
}

void t91407_gpio_set_value(int onewire_gpio, uint8_t value)
{
	gpio_set_value(onewire_gpio, value);
}

int t91407_gpio_get_value(int onewire_gpio)
{
	return gpio_get_value(onewire_gpio);
}

void t91407_print_data(unsigned char *send, unsigned char send_length, char *write, unsigned char write_length)
{
	char num_str[T91407_DATA_STR_LEN] = { 0 };

	for (uint8_t m = 0; m < send_length; m++) {
		if (sprintf_s(num_str, T91407_DATA_STR_LEN, "%d ", send[m]) < 0)
			hwlog_err("[%s] sprintf_s error\n", __func__);
		if (strcat_s(write, write_length, num_str))
			hwlog_err("[%s] strcat_s fail\n", __func__);
	}
}

static void t91407_read_word_decode(struct t91407_dev *di, uint64_t *swi_time, uint8_t *buf)
{
	uint64_t threshold_time;
	uint8_t buf_word[T91407_SWI_LEN_OF_DATA_WORD] = { 0 };
	int i;

	threshold_time = us2cycles(di->t91407_swi.ow_tau_delay * T91407_SWI_CYCLES);
	for (i = T91407_CODE_BIT_BEGIN; i < (T91407_CODE_BIT_BEGIN + T91407_CODE_BIT_LEN); i++)
		buf_word[T91407_CDOE_OF_WORD] += ((swi_time[i] > threshold_time) ? 1u : 0u) <<
			((T91407_CODE_BIT_BEGIN + T91407_CODE_BIT_LEN) - i - 1);
	for (i = T91407_DATA_BIT_BEGIN; i < (T91407_DATA_BIT_BEGIN + T91407_DATA_BIT_LEN); i++)
		buf_word[T91407_DATA_OF_WORD] += ((swi_time[i] > threshold_time) ? 1u : 0u) <<
			((T91407_DATA_BIT_BEGIN + T91407_DATA_BIT_LEN) - i - 1);
	if (swi_time[T91407_INVERT_OF_BYT] > threshold_time)
		buf_word[T91407_INVERT_OF_WORD] = 1;

	for (uint32_t k = 0; k < T91407_SWI_LEN_OF_DATA_WORD; k++)
		buf[k] = buf_word[k];
}

static bool t91407_read_word_is_delay(struct t91407_dev *di, uint64_t before, uint64_t last, uint64_t delay_time)
{
	if ((last - before) > delay_time) {
		hwlog_err("[%s] ow_timeout_delay\n", __func__);
		t91407_gpio_dir_output(di->onewire_gpio, T91407_HIGH_VOLTAGE);
		return true;
	}
	return false;
}

static int t91407_read_word(struct t91407_dev *di, uint8_t *buf)
{
	volatile int previous_swi_state;
	volatile int current_swi_state;
	uint64_t swi_time[T91407_SWI_LEN_OF_EDGE_CAPTURE];
	uint64_t before, last;
	uint64_t delay_time;

	t91407_gpio_dir_input(di->onewire_gpio);

	before = get_cycles();
	delay_time = us2cycles(di->t91407_swi.ow_timeout_delay);
	while (t91407_gpio_get_value(di->onewire_gpio)) {
		last = get_cycles();
		if (t91407_read_word_is_delay(di, before, last, delay_time))
			return T91407_ERR_TIME_OUT;
	}
	current_swi_state = t91407_gpio_get_value(di->onewire_gpio);
	previous_swi_state = current_swi_state;

	before = get_cycles();
	delay_time = us2cycles(di->t91407_swi.ow_timeout_bit_delay);
	for (uint32_t i = 0; i < T91407_SWI_LEN_OF_EDGE_CAPTURE; i++) {
		while (current_swi_state == previous_swi_state) {
			current_swi_state = t91407_gpio_get_value(di->onewire_gpio);
			last = get_cycles();
			if (t91407_read_word_is_delay(di, before, last, delay_time))
				return T91407_ERR_READ_FAILED;
		}
		swi_time[i] = last - before;
		before = last;
		previous_swi_state = current_swi_state;
	}
	t91407_gpio_dir_output(di->onewire_gpio, T91407_HIGH_VOLTAGE);

	t91407_read_word_decode(di, swi_time, buf);

	return T91407_SUCCESS;
}

int t91407_read_data(struct t91407_dev *di, uint8_t *data_ret, uint8_t data_len)
{
	int ret;
	uint8_t word_contain[T91407_SWI_LEN_OF_DATA_WORD];
	uint8_t data;
	char read[T91407_DATA_READ_LEN] = { 0 };

	for (uint8_t i = 0 ; i < data_len; i++) {
		ret = t91407_read_word(di, word_contain);
		if (ret) {
			hwlog_err("[%s] t91407_read_word err : %d\n", __func__, ret);
			return ret;
		}
		t91407_udelay(T91407_SWI_RA_PUSH_PULL);

		data = word_contain[T91407_DATA_OF_WORD];
		if (word_contain[T91407_INVERT_OF_WORD])
			data = ~data;
		data_ret[i] = data;
	}
#ifdef ONEWIRE_STABILITY_DEBUG
	t91407_print_data(data_ret, data_len, read, T91407_DATA_READ_LEN);
	hwlog_info("[%s] %s\n", __func__, read);
#endif
	return T91407_SUCCESS;
}

void t91407_write_data(struct t91407_dev *di, uint32_t data)
{
	t91407_gpio_dir_output(di->onewire_gpio, T91407_HIGH_VOLTAGE);
	t91407_udelay(di->t91407_swi.ow_stop_delay);

	for (uint8_t i = T91407_LEN_OF_ONE_DATA; i > 0; i--) {
		if (i & 0x01)
			t91407_gpio_set_value(di->onewire_gpio, T91407_LOW_VOLTAGE);
		else
			t91407_gpio_set_value(di->onewire_gpio, T91407_HIGH_VOLTAGE);

		if ((data >> (i - 1)) & 0x01)
			t91407_udelay(di->t91407_swi.ow_bit1_delay);
		else
			t91407_udelay(di->t91407_swi.ow_bit0_delay);
	}

	t91407_gpio_set_value(di->onewire_gpio, T91407_HIGH_VOLTAGE);
}
