/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_test_wp.h
 *
 * common interface, variables, definition etc for wireless charge test
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

#ifndef _WIRELESS_TEST_WP_H_
#define _WIRELESS_TEST_WP_H_

#define WLRX_SAMPLE_WORK_IDLE       0
#define WLRX_SAMPLE_WORK_PENDING    1
#define WLRX_SAMPLE_WORK_RUNNING    2

int wp_get_sample_type(void);
void wp_start_sample_iout(void);

#endif /* _WIRELESS_TEST_WP_H_ */
