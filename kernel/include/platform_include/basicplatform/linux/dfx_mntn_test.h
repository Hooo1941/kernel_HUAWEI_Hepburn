/*
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_MNTN_TEST_H__
#define __DFX_MNTN_TEST_H__

#ifdef CONFIG_DFX_BB_DEBUG
void mntn_test_startup_panic(void);
void mntn_test_shutdown_deadloop(void);
#else
static inline void mntn_test_startup_panic(void) {}
static inline void mntn_test_shutdown_deadloop(void) {}
#endif

#endif
