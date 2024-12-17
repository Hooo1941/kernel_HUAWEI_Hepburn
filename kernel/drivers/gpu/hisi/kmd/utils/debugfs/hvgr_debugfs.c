/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#ifdef CONFIG_DFX_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include "hvgr_defs.h"

#ifdef CONFIG_DFX_DEBUG_FS
int hvgr_debugfs_init(struct hvgr_device * const gdev)
{
	if (gdev == NULL)
		return -ENOMEM;

	gdev->root_debugfs_dir = debugfs_create_dir(gdev->devname, NULL);
	if (IS_ERR_OR_NULL(gdev->root_debugfs_dir))
		return -ENOMEM;

	gdev->ctx_debugfs_dir = debugfs_create_dir("ctx", gdev->root_debugfs_dir);
	if (IS_ERR_OR_NULL(gdev->ctx_debugfs_dir)) {
		debugfs_remove_recursive(gdev->root_debugfs_dir);
		return -ENOMEM;
	}

	return 0;
}

void hvgr_debugfs_term(struct hvgr_device * const gdev)
{
	if (!IS_ERR_OR_NULL(gdev->ctx_debugfs_dir))
		debugfs_remove_recursive(gdev->ctx_debugfs_dir);

	if (!IS_ERR_OR_NULL(gdev->root_debugfs_dir))
		debugfs_remove_recursive(gdev->root_debugfs_dir);

	return;
}
#else
int hvgr_debugfs_init(struct hvgr_device * const gdev)
{
	return 0;
}

void hvgr_debugfs_term(struct hvgr_device * const gdev)
{
	return;
}
#endif
