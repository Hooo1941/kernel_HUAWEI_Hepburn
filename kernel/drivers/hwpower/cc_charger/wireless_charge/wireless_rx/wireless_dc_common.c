// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_dc_common.c
 *
 * direct charge common interface for wireless direct charge
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

#include <chipset_common/hwpower/wireless_charge/wireless_dc_common.h>
#include <linux/errno.h>
#include <linux/slab.h>

unsigned int wldc_get_mode_by_name(const char *mode_name)
{
	if (!mode_name)
		return WLDC_MODE_END;

	if (strstr(mode_name, WLRX_SCX_MODE_PREFIX))
		return WLDC_MODE_SCX;
	if (strstr(mode_name, WLRX_SC_MODE_PREFIX))
		return WLDC_MODE_SC;
	if (strstr(mode_name, WLRX_LVC_MODE_PREFIX))
		return WLDC_MODE_LVC;

	return WLDC_MODE_END;
}
