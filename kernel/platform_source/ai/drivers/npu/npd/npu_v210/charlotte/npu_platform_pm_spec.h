/*
 * npu_platform_pm.h
 *
 * about npu hwts plat
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
#ifndef __NPU_PLATFORM_PM_SPEC_H
#define __NPU_PLATFORM_PM_SPEC_H

enum npu_subip {
	NPU_TOP_COMMON              = 0, /* npu subsys, npu bus */
	NPU_TOP_SPECIFY             = 1, /* tcu */
	NPU_NON_TOP_COMMON          = 2, /* ts subsys, aicore subsys */
	NPU_NON_TOP_SPECIFY         = 3, /* hwts/aicore tbu init */
	NPU_NON_TOP_SPECIFY_NOSEC   = 4, /* tscpu startup, doorbell */
	NPU_SUBIP_MAX,
};

#endif
