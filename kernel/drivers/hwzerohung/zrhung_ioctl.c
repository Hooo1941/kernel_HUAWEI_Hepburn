/*
 * zrhung_ioctl.c
 *
 * Frozen screen solution interface provided to the upper layer
 *
 * Copyright (c) 2017-2021 Huawei Technologies Co., Ltd.
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

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <linux/aio.h>
#include <uapi/linux/uio.h>
#include <asm/ioctls.h>
#include <chipset_common/hwlogger/hw_logger.h>
#include <platform/linux/zrhung.h>
#include "zrhung_config.h"
#include "zrhung_ioctl.h"

long zrhung_ioctl(struct file *file, unsigned int cmd, uintptr_t arg)
{
	long ret = ZRHUNG_CMD_INVALID;

	if ((cmd < LOGGER_WRITE_HEVENT) || (cmd >= LOGGER_CMD_MAX))
		return ret;

	switch (cmd) {
	case LOGGER_SET_HCFG:
		ret = hcfgk_set_cfg(file, (void *)arg);
		break;
	case LOGGER_GET_HCFG:
		ret = hcfgk_ioctl_get_cfg(file, (void *)arg);
		break;
	case LOGGER_SET_HCFG_FLAG:
		ret = hcfgk_set_cfg_flag(file, (void *)arg);
		break;
	case LOGGER_GET_HCFG_FLAG:
		ret = hcfgk_get_cfg_flag(file, (void *)arg);
		break;
	case LOGGER_SET_FEATURE:
		ret = hcfgk_set_feature(file, (void *)arg);
		break;
	case LOGGER_GET_STACK:
		ret = xcollie_get_stack(file, (void *)arg);
		break;
	default:
		break;
	}

	return ret;
}
EXPORT_SYMBOL(zrhung_ioctl);
